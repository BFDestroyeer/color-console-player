#pragma once
#include <string>

#include <opencv2/opencv.hpp>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

class VideoCapture {
private:
    uint8_t *buffer = nullptr;
    AVPacket *packet = nullptr;
    AVFrame *frame = nullptr;
    AVFrame *bgrFrame = nullptr;
    AVCodecContext *codecContext = nullptr;
    AVFormatContext *formatContext = nullptr;
    SwsContext *transcoderContext = nullptr;

    uint64_t videoStreamIndex;

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
    bool frameReadResult = true;

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
