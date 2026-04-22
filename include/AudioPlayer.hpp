#pragma once

#include <string>
#include <vector>

#include <AL/al.h>
#include <AL/alut.h>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libswresample/swresample.h>
}

class AudioPlayer {
private:
    static constexpr int64_t BUFFERS_COUNT = 4;
    static constexpr size_t BUFFER_SIZE = 40960;
    static constexpr AVSampleFormat OUTPUT_SAMPLE_FORMAT = AV_SAMPLE_FMT_S16; // 16 bit PCM audio
    static constexpr int OUTPUT_SAMPLE_RATE = 44100; // 44.1 KHz
    static constexpr int OUTPUT_CHANNELS = 2; // Stereo sound

    AVFormatContext* formatContext;
    AVPacket* packet;
    int audioStreamIndex;
    AVCodecContext* codecContext;
    AVFrame* frame;
    SwrContext* resamplerContext;
    ALuint source;
    ALCdevice* device;
    ALCcontext* alContext;
    uint8_t* outputBuffer;
    ALuint buffers[BUFFERS_COUNT];
    std::vector<uint8_t> fillerTemporaryBuffer;

public:
    explicit AudioPlayer(const std::string& mediaFilePath);

    ~AudioPlayer();

    void play();

private:
    bool fillBuffer(ALuint bufferId);
};
