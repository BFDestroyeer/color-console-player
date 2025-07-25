#pragma once

#include <atomic>

#include <opencv2/opencv.hpp>

/**
 * @brief OpenCV video capture with frame buffering
 */
class BufferedVideoCapture {
private:

    /**
     * @brief OpenCV video capture for buffering
     */
    std::shared_ptr<cv::VideoCapture> videoCapture;

    /**
     * @brief Next frame
     */
    cv::Mat frame;

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
    /**
     * @param videoCapture OpenCV video captrue for buffering
     */
    explicit BufferedVideoCapture(const std::shared_ptr<cv::VideoCapture>& videoCapture);

    /**
     * @brief Get next frame
     * @param [out] outputFrame next frame
     * @param [out] outputPosition next frame position in milliseconds
     * @return true if next frame is captured, false if not
     */
    bool read(cv::Mat& outputFrame, double& outputPosition);
};
