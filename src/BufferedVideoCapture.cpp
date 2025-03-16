#include "BufferedVideoCapture.hpp"

BufferedVideoCapture::BufferedVideoCapture(cv::VideoCapture& videoCapture) : videoCapture(videoCapture) {
    isFrameReady = false;
    std::thread(
        [&] {
            while (true) {
                while (isFrameReady) {
                }
                frameReadResult = videoCapture.read(frame);
                position = videoCapture.get(cv::CAP_PROP_POS_MSEC);
                isFrameReady = true;
                if (!frameReadResult) {
                    break;
                }
            }
        }
    ).detach();
}

bool BufferedVideoCapture::read(cv::Mat& outputFrame, double& outputPosition) {
    while (!isFrameReady) {
    }
    outputFrame = std::move(frame);
    outputPosition = position;
    isFrameReady = false;
    return frameReadResult;
}
