#pragma once

#include <opencv2/opencv.hpp>

class BufferedVideoCapture {
private:
    cv::VideoCapture& videoCapture;

    cv::Mat frame;
    double position;
    bool frameReadResult;

    volatile bool isFrameReady;

public:
    explicit BufferedVideoCapture(cv::VideoCapture& videoCapture);

    bool read(cv::Mat& outputFrame, double& outputPosition);
};
