#include "AudioPlayer.hpp"

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}

#include <AL/al.h>
#include <AL/alut.h>

#include <iostream>
#include <vector>
#include <thread>

// Настройки буферизации
const int NUM_BUFFERS = 4;
const int BUFFER_SIZE = 40960; // ~0.25 сек при 44.1кГц

AudioPlayer::AudioPlayer(const std::string &mediaFilePath) {

}

void AudioPlayer::play() {
    std::thread(
        [this] {
            // 1. Инициализация FFmpeg
        AVFormatContext* formatCtx = avformat_alloc_context();
        if (avformat_open_input(&formatCtx, "/media/bfdestroyeer/Локальный диск/Media/TV Series/Elfen Lied (2004) {tmdb-42671} (S)/Season 01/Elfen Lied S01E01.mkv", nullptr, nullptr) != 0) return;
        if (avformat_find_stream_info(formatCtx, nullptr) < 0) return;

        int audioStreamIndex = -1;
        for (unsigned int i = 0; i < formatCtx->nb_streams; i++) {
            if (formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
                audioStreamIndex = i;
                break;
            }
        }
        if (audioStreamIndex == -1) return;

        AVCodecParameters* codecParams = formatCtx->streams[audioStreamIndex]->codecpar;
        const AVCodec* codec = avcodec_find_decoder(codecParams->codec_id);
        AVCodecContext* codecCtx = avcodec_alloc_context3(codec);
        avcodec_parameters_to_context(codecCtx, codecParams);
        if (avcodec_open2(codecCtx, codec, nullptr) < 0) return;

        // 2. Инициализация OpenAL
        ALCdevice* device = alcOpenDevice(nullptr);
        ALCcontext* alContext = alcCreateContext(device, nullptr);
        alcMakeContextCurrent(alContext);

        ALuint source;
        alGenSources(1, &source);
        ALuint buffers[NUM_BUFFERS];
        alGenBuffers(NUM_BUFFERS, buffers);

        // 3. Настройка Resampler (приводим любой звук к Stereo 16-bit PCM)
        SwrContext* swr = nullptr;

        // Определяем параметры выходного формата
        AVChannelLayout out_ch_layout;
        av_channel_layout_default(&out_ch_layout, 2); // Стерео
        AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16; // 16-бит PCM
        int out_sample_rate = 44100;

        // Инициализируем ресемплер с использованием новой функции
        swr_alloc_set_opts2(
            &swr,               // Указатель на контекст
            &out_ch_layout,         // Выходной макет каналов
            out_sample_fmt,         // Выходной формат сэмплов
            out_sample_rate,        // Выходная частота
            &codecCtx->ch_layout,  // Входной макет (из контекста декодера)
            codecCtx->sample_fmt,  // Входной формат
            codecCtx->sample_rate, // Входная частота
            0, nullptr              // Логирование
        );
        swr_init(swr);

        AVPacket* packet = av_packet_alloc();
        AVFrame* frame = av_frame_alloc();

        uint8_t* outputBuffer = (uint8_t*)av_malloc(BUFFER_SIZE);

        // Лямбда для заполнения буфера данными
        auto fill_buffer = [&](ALuint buffer_id) -> bool {
            std::vector<uint8_t> tempBuffer;
            tempBuffer.reserve(BUFFER_SIZE);

            while (tempBuffer.size() < BUFFER_SIZE) {
                if (av_read_frame(formatCtx, packet) < 0) break;

                if (packet->stream_index == audioStreamIndex) {
                    avcodec_send_packet(codecCtx, packet);
                    while (avcodec_receive_frame(codecCtx, frame) == 0) {
                        // Вычисляем, сколько сэмплов мы получим на выходе
                        int out_samples = av_rescale_rnd(swr_get_delay(swr, codecCtx->sample_rate) + frame->nb_samples,
                                                         out_sample_rate, codecCtx->sample_rate, AV_ROUND_UP);

                        uint8_t* out_data;
                        av_samples_alloc(&out_data, nullptr, 2, out_samples, out_sample_fmt, 0);

                        int converted = swr_convert(swr, &out_data, out_samples, (const uint8_t**)frame->data, frame->nb_samples);
                        int size_in_bytes = converted * 2 * 2; // Stereo S16

                        tempBuffer.insert(tempBuffer.end(), out_data, out_data + size_in_bytes);
                        av_freep(&out_data);
                    }
                }
                av_packet_unref(packet);
            }

            if (tempBuffer.empty()) return false;

            alBufferData(buffer_id, AL_FORMAT_STEREO16, tempBuffer.data(), tempBuffer.size(), out_sample_rate);
            return true;
        };

        // Предварительное заполнение буферов
        for (int i = 0; i < NUM_BUFFERS; i++) {
            fill_buffer(buffers[i]);
        }
        alSourceQueueBuffers(source, NUM_BUFFERS, buffers);
        alSourcePlay(source);

        // 4. Основной цикл воспроизведения
        ALint state;
        bool playing = true;
        while (playing) {
            alGetSourcei(source, AL_SOURCE_STATE, &state);
            ALint processed;
            alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);

            while (processed--) {
                ALuint buffer;
                alSourceUnqueueBuffers(source, 1, &buffer);
                if (fill_buffer(buffer)) {
                    alSourceQueueBuffers(source, 1, &buffer);
                } else {
                    playing = false;
                }
            }

            // Если вдруг источник остановился из-за опустошения очереди
            if (state != AL_PLAYING && playing) alSourcePlay(source);
        }

        // Очистка
        alSourceStop(source);
        alDeleteSources(1, &source);
        alDeleteBuffers(NUM_BUFFERS, buffers);
        alcMakeContextCurrent(nullptr);
        alcDestroyContext(alContext);
        alcCloseDevice(device);

        av_free(outputBuffer);
        av_frame_free(&frame);
        av_packet_free(&packet);
        avcodec_free_context(&codecCtx);
        avformat_close_input(&formatCtx);
        swr_free(&swr);
        }).detach();
}

