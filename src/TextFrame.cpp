#include "TextFrame.hpp"

#include <cstring>

TextFrame::TextFrame(const size_t bufferSize, const size_t differentialBufferSize): bufferSize(bufferSize), frameIndex(0) {
    buffer = new uint8_t[bufferSize];
    std::memset(buffer, ' ', bufferSize - 1);
    buffer[bufferSize - 1] = 0x0;
    differentialBuffer = new uint8_t[differentialBufferSize];
    std::memset(differentialBuffer, ' ', differentialBufferSize - 1);
    differentialBuffer[differentialBufferSize - 1] = 0x0;
}

TextFrame::~TextFrame() {
    delete[] this->buffer;
    delete[] this->differentialBuffer;
}

uint8_t* TextFrame::getBuffer() const {
    return this->buffer;
}

size_t TextFrame::getBufferSize() const {
    return this->bufferSize;
}

uint8_t* TextFrame::getDifferentialBuffer() const {
    return this->differentialBuffer;
}

uint64_t TextFrame::getFrameIndex() const {
    return this->frameIndex;
}

bool TextFrame::getIsFullRedraw() const {
    return this->isFullRedraw;
}

std::chrono::nanoseconds TextFrame::getFrameDuration() const {
    return frameDuration;
}

std::chrono::duration<int64_t, std::ratio<1, 1000000000>> TextFrame::getFramePosition() const {
    return framePosition;
}

std::chrono::nanoseconds TextFrame::getRenderTime() const {
    return renderTime;
}

size_t TextFrame::getDifferentialRealSize() const {
    return differentialRealSize;
}

int32_t TextFrame::getSymbolHeight() const {
    return symbolHeight;
}


void TextFrame::updateFrame(
    const uint64_t frameIndex,
    bool isFullRedraw,
    std::chrono::nanoseconds frameDuration,
    std::chrono::duration<int64_t, std::ratio<1, 1000000000>> framePosition,
    std::chrono::nanoseconds renderTime,
    size_t differentialRealSize,
    int32_t symbolHeight) {
    this->frameIndex = frameIndex;
    this->isFullRedraw = isFullRedraw;
    this->frameDuration = frameDuration;
    this->framePosition = framePosition;
    this->renderTime = renderTime;
    this->differentialRealSize = differentialRealSize;
    this->symbolHeight = symbolHeight;
}

