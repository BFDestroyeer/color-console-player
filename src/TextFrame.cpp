#include "TextFrame.hpp"

#include <cstring>

TextFrame::TextFrame(const size_t bufferSize): frameIndex(0), bufferSize(bufferSize) {
    this->buffer = new uint8_t[bufferSize];
    std::memset(buffer, ' ', bufferSize - 1);
    buffer[bufferSize - 1] = 0x0;
}

TextFrame::~TextFrame() {
    delete[] this->buffer;
}

uint64_t TextFrame::getFrameIndex() const {
    return this->frameIndex;
}


uint8_t* TextFrame::getBuffer() const {
    return this->buffer;
}

size_t TextFrame::getBufferSize() const {
    return this->bufferSize;
}

void TextFrame::updateFrame(const uint64_t frameIndex) {
    this->frameIndex = frameIndex;
}

