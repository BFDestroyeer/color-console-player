#pragma once
#include <string>

#include <opencv2/opencv.hpp>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

class VideoCapture {
private:
    uint8_t *buffer;
    AVPacket *packet;
    AVFrame *frame;
    AVFrame *frame_bgr;
    AVCodecContext *codec_ctx;
    AVFormatContext *format_ctx;
    SwsContext *sws_ctx;
    int video_stream_idx;

public:
    VideoCapture(const std::string& mediaFilePath);

    ~VideoCapture();

    bool read(cv::Mat& outputFrame, double& outputPosition);

    double getFrameRate() const;
};
