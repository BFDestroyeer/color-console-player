#pragma once

#include <opencv2/opencv.hpp>

class FrameRenderer {
private:
    static constexpr int64_t SYMBOL_SIZE = 41;

public:
    static int64_t getSymbolSize();

    static void redner(
        const cv::Mat& image,
        uint64_t horizontalOffset,
        uint8_t* buffer
    );

private:
    inline static uint16_t getColor(const cv::Vec3s& foreground, const cv::Vec3s& background, const cv::Vec3s& color);

    inline static const char* colorToText(uint8_t color);

    inline static std::pair<const char*, bool> symbolByConvolutionFull(
        uint16_t convolution,
        int32_t foregroundClusterSize,
        int32_t backgroundClusterSize
    );
};
