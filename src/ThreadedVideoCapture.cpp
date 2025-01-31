#include "ThreadedVideoCapture.hpp"

ThreadedVideoCapture::ThreadedVideoCapture(cv::VideoCapture& videoCapture) : videoCapture(videoCapture) {
    isFrameReady = false;
    thread = std::thread([&] {
        while (true) {
            while (isFrameReady) {}
            frameReadResult = videoCapture.read(frame);
            position = videoCapture.get(cv::CAP_PROP_POS_MSEC);
            if (!frameReadResult) {
                break;
            }
            isFrameReady = true;
        }
    });
}

bool ThreadedVideoCapture::read(cv::Mat& outputFrame, double& outputPosition) {
    while (!isFrameReady) {}
    outputFrame = std::move(frame);
    outputPosition = position;
    isFrameReady = false;
    return frameReadResult;
}