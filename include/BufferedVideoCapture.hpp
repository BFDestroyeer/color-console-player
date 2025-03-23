#pragma once

#include <opencv2/opencv.hpp>

class BufferedVideoCapture {
private:
    std::shared_ptr<cv::VideoCapture> videoCapture;

    cv::Mat frame;
    double position;
    bool frameReadResult;

    volatile bool isFrameReady;

public:
    explicit BufferedVideoCapture(const std::shared_ptr<cv::VideoCapture>& videoCapture);

    bool read(cv::Mat& outputFrame, double& outputPosition);
};
