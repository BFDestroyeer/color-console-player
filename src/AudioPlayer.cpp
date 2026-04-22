#include "AudioPlayer.hpp"

#include <vector>
#include <thread>

AudioPlayer::AudioPlayer(const std::string& mediaFilePath) {
    formatContext = avformat_alloc_context();
    if (avformat_open_input(&formatContext, mediaFilePath.c_str(), nullptr, nullptr) != 0) {
        return;
    }
    if (avformat_find_stream_info(formatContext, nullptr) < 0) {
        return;
    }

    audioStreamIndex = -1;
    for (unsigned int i = 0; i < formatContext->nb_streams; i++) {
        if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioStreamIndex = i;
            break;
        }
    }
    if (audioStreamIndex == -1) {
        return;
    }

    const auto codecParameters = formatContext->streams[audioStreamIndex]->codecpar;
    const auto codec = avcodec_find_decoder(codecParameters->codec_id);
    codecContext = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(codecContext, codecParameters);
    if (avcodec_open2(codecContext, codec, nullptr) < 0) {
        return;
    }

    device = alcOpenDevice(nullptr);
    alContext = alcCreateContext(device, nullptr);
    alcMakeContextCurrent(alContext);

    alGenSources(1, &source);
    alGenBuffers(BUFFERS_COUNT, buffers);

    resamplerContext = nullptr;

    AVChannelLayout out_ch_layout;
    av_channel_layout_default(&out_ch_layout, OUTPUT_CHANNELS);

    swr_alloc_set_opts2(
        &resamplerContext,
        &out_ch_layout,
        OUTPUT_SAMPLE_FORMAT,
        OUTPUT_SAMPLE_RATE,
        &codecContext->ch_layout,
        codecContext->sample_fmt,
        codecContext->sample_rate,
        0,
        nullptr
    );
    swr_init(resamplerContext);

    packet = av_packet_alloc();
    frame = av_frame_alloc();

    outputBuffer = new uint8_t[BUFFER_SIZE];
    fillerTemporaryBuffer.reserve(BUFFER_SIZE);
}

AudioPlayer::~AudioPlayer() {
    alSourceStop(source);
    alDeleteSources(1, &source);
    alDeleteBuffers(BUFFERS_COUNT, buffers);
    alcMakeContextCurrent(nullptr);
    alcDestroyContext(alContext);
    alcCloseDevice(device);

    av_free(outputBuffer);
    av_frame_free(&frame);
    av_packet_free(&packet);
    avcodec_free_context(&codecContext);
    avformat_close_input(&formatContext);
    swr_free(&resamplerContext);
}

void AudioPlayer::play() {
    std::thread(
        [this] {
            for (unsigned int buffer : buffers) {
                fillBuffer(buffer);
            }
            alSourceQueueBuffers(source, BUFFERS_COUNT, buffers);
            alSourcePlay(source);

            ALint state;
            bool playing = true;
            while (playing) {
                alGetSourcei(source, AL_SOURCE_STATE, &state);
                ALint processed;
                alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);

                while (processed--) {
                    ALuint buffer;
                    alSourceUnqueueBuffers(source, 1, &buffer);
                    if (fillBuffer(buffer)) {
                        alSourceQueueBuffers(source, 1, &buffer);
                    } else {
                        playing = false;
                    }
                }

                if (state != AL_PLAYING && playing) alSourcePlay(source);
            }
        }).detach();
}

bool AudioPlayer::fillBuffer(const ALuint bufferId) {
    while (fillerTemporaryBuffer.size() < BUFFER_SIZE) {
        if (av_read_frame(formatContext, packet) < 0) break;

        if (packet->stream_index == audioStreamIndex) {
            avcodec_send_packet(codecContext, packet);
            while (avcodec_receive_frame(codecContext, frame) == 0) {
                int out_samples = av_rescale_rnd(
                    swr_get_delay(resamplerContext, codecContext->sample_rate) + frame->nb_samples,
                    OUTPUT_SAMPLE_RATE,
                    codecContext->sample_rate,
                    AV_ROUND_UP
                );

                uint8_t* out_data;
                av_samples_alloc(&out_data, nullptr, 2, out_samples, OUTPUT_SAMPLE_FORMAT, 0);

                int converted = swr_convert(resamplerContext, &out_data, out_samples, frame->data, frame->nb_samples);
                int size_in_bytes = converted * 2 * 2;

                fillerTemporaryBuffer.insert(fillerTemporaryBuffer.end(), out_data, out_data + size_in_bytes);
                av_freep(&out_data);
            }
        }
        av_packet_unref(packet);
    }

    if (fillerTemporaryBuffer.empty()) {
        return false;
    }

    alBufferData(
        bufferId,
        AL_FORMAT_STEREO16,
        fillerTemporaryBuffer.data(),
        fillerTemporaryBuffer.size(),
        OUTPUT_SAMPLE_RATE
    );
    fillerTemporaryBuffer.clear();
    return true;
}
