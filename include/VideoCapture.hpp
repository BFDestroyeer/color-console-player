#pragma once
#include <string>

#include <opencv2/opencv.hpp>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

#include <atomic>

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

    int32_t currentWidth = 0;
    int32_t currentHeight = 0;


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

    bool read(cv::Mat& outputFrame, double& outputPosition, int32_t width, int32_t height);

    [[nodiscard]]
    double getFrameRate() const;

    int32_t getOriginalWidth() const;

    int32_t getOriginalHeight() const;
};
