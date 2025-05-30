#pragma once

#include <opencv2/opencv.hpp>

#include "BufferedVideoCapture.hpp"
#include "ConsoleSizeRecorder.hpp"
#include "TextFrameBuffer.hpp"

class FrameRenderer {
private:
    static constexpr int32_t SYMBOL_SIZE = 41;

    const std::chrono::time_point<std::chrono::high_resolution_clock>& beginPlayTime;

    const std::shared_ptr<ConsoleSizeRecorder> consoleSizeRecorder;
    const std::shared_ptr<TextFrameBuffer> textFrameBuffer;
    const std::shared_ptr<cv::VideoCapture> videoCapture;

    BufferedVideoCapture bufferedVideoCapture;

public:
    FrameRenderer(
        const std::chrono::time_point<std::chrono::high_resolution_clock>& beginPlayTime,
        const std::shared_ptr<ConsoleSizeRecorder>& consoleSizeRecorder,
        const std::shared_ptr<TextFrameBuffer>& textFrameBuffer,
        const std::shared_ptr<cv::VideoCapture>& videoCapture
    );

    void start();

private:
    static void imageToText(
        const cv::Mat& image,
        uint64_t horizontalOffset,
        uint8_t* buffer
    );

    inline static uint16_t getColor(const cv::Vec3s& foreground, const cv::Vec3s& background, const cv::Vec3s& color);

    inline static const char* colorToText(uint8_t color);

    inline static std::pair<const char*, bool> symbolByConvolutionFull(
        uint16_t convolution,
        int32_t foregroundClusterSize,
        int32_t backgroundClusterSize
    );
};
