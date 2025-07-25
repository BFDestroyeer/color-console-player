#pragma once

#include <opencv2/opencv.hpp>

#include "BufferedVideoCapture.hpp"
#include "ConsoleWindowSizeService.hpp"
#include "TextFrameBuffer.hpp"

class FrameRenderer {
private:
    /**
     * @brief Size in bytes of drawable symbols
     */
    static constexpr int32_t SYMBOL_SIZE = 41;

    /**
     * @brief Playback start time
     */
    const std::chrono::time_point<std::chrono::high_resolution_clock>& beginPlayTime;

    /**
     * @brief Console window size service
     */
    const std::shared_ptr<ConsoleWindowSizeService> consoleWindowSizeService;

    /**
     * @brief Buffer of text frames
     */
    const std::shared_ptr<TextFrameBuffer> textFrameBuffer;

    /**
     * @brief OpenCV video capture
     */
    const std::shared_ptr<cv::VideoCapture> videoCapture;

    /**
     * @brief Buffered video capture
     */
    BufferedVideoCapture bufferedVideoCapture;

public:

    /**
     * @param beginPlayTime Playback start time
     * @param consoleWindowSizeService Console window size service
     * @param textFrameBuffer Buffer of text frames
     * @param videoCapture OpenCV video capture
     */
    FrameRenderer(
        const std::chrono::time_point<std::chrono::high_resolution_clock>& beginPlayTime,
        const std::shared_ptr<ConsoleWindowSizeService>& consoleWindowSizeService,
        const std::shared_ptr<TextFrameBuffer>& textFrameBuffer,
        const std::shared_ptr<cv::VideoCapture>& videoCapture
    );

    /**
     * @brief Start render frames
     */
    void start();

private:
    /**
     * @brief Convert OpenCV image to text
     * @param image OpenCV image
     * @param horizontalOffset Offset from left screen border
     * @param [out] buffer Output text buffer
     */
    static void imageToText(
        const cv::Mat& image,
        uint64_t horizontalOffset,
        uint8_t* buffer
    );

    /**
     * @brief Get nearest from foreground and background colors
     * @param foreground Foreground color
     * @param background Background color
     * @param color Color to test
     * @return 1 if foreground color is nearest, 0 if background
     */
    inline static uint16_t getColor(const cv::Vec3s& foreground, const cv::Vec3s& background, const cv::Vec3s& color);

    /**
     * @brief Convert unsigned integer value to text literal
     * @param value Unsigned integer value
     * @return String representation of unsigned integer value
     */
    inline static const char* unsignedToText(uint8_t value);

    /**
     * @brief Get symbol that best matches the convolution
     *
     * @param convolution Color convolution
     * @return Text literal and flag that indicates that you should swap foreground and background colors
     */
    inline static std::pair<const char*, bool> symbolByConvolutionFull(uint16_t convolution);
};
