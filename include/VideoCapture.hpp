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
    AVFrame *bgrFrame;
    AVCodecContext *codecContext;
    AVFormatContext *formatContext;
    SwsContext *transcoderContext;
    int videoStreamIndex;

    /**
    * @brief Next frame
    */
    cv::Mat opencvFrame;

    /**
     * @brief Frame position in milliseconds
     */
    double position;

    /**
     * @brief Return value of videoCapture->read()
     */
    bool frameReadResult;

    /**
     * @brief true if next frame is ready, false if not
     */
    std::atomic<bool> isFrameReady;

public:
    explicit VideoCapture(const std::string& mediaFilePath);

    ~VideoCapture();

    bool read(cv::Mat& outputFrame, double& outputPosition);

    [[nodiscard]]
    double getFrameRate() const;
};
