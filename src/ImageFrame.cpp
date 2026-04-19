#include "ImageFrame.hpp"

ImageFrame::ImageFrame(const uint64_t width, const uint64_t height) {
    buffer = new uint8_t[width * height * 3];
    position = 0;
}

ImageFrame::~ImageFrame() {
    delete[] buffer;
}

void ImageFrame::resize(const uint64_t width, const uint64_t height) {
    if (this->width == width && this->height == height) {
        return;
    }
    delete[] buffer;
    buffer = new uint8_t[width * height * 3];
    this->width = width;
    this->height = height;
}

uint8_t* ImageFrame::getBuffer() const {
    return buffer;
}

uint64_t ImageFrame::getWidth() const {
    return width;
}

uint64_t ImageFrame::getHeight() const {
    return height;
}

void ImageFrame::setPosition(const double position) {
    this->position = position;
}
