#pragma once

#include <thread>

#include <opencv2/opencv.hpp>

class ThreadedVideoCapture {
  private:
    cv::VideoCapture& videoCapture;

    cv::Mat frame;
    double position;
    bool frameReadResult;

    volatile bool isFrameReady;

  public:
    explicit ThreadedVideoCapture(cv::VideoCapture& videoCapture);

    bool read(cv::Mat& outputFrame, double& outputPosition);
};