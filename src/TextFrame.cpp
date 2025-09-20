#include "TextFrame.hpp"

#include <cstring>

TextFrame::TextFrame(const size_t bufferSize)
    : bufferSize(bufferSize),
      frameIndex(0) {
    if (bufferSize != 0) {
        buffer = new uint8_t[bufferSize];
        std::memset(buffer, ' ', bufferSize - 1);
        buffer[bufferSize - 1] = 0x0;
    } else {
        buffer = nullptr;
    }
}

TextFrame::~TextFrame() {
    delete[] this->buffer;
}

uint8_t* TextFrame::getBuffer() const {
    return this->buffer;
}

size_t TextFrame::getBufferSize() const {
    return this->bufferSize;
}

uint64_t TextFrame::getFrameIndex() const {
    return this->frameIndex;
}

std::chrono::duration<int64_t, std::ratio<1, 1000000000>> TextFrame::getFramePosition() const {
    return framePosition;
}

std::chrono::nanoseconds TextFrame::getRenderTime() const {
    return renderTime;
}

int32_t TextFrame::getSymbolHeight() const {
    return symbolHeight;
}

int32_t TextFrame::getSymbolWidth() const {
    return symbolWidth;
}


void TextFrame::updateFrame(
    const uint64_t frameIndex,
    std::chrono::duration<int64_t, std::ratio<1, 1000000000>> framePosition,
    std::chrono::nanoseconds renderTime,
    int32_t symbolHeight,
    int32_t symbolWidth
) {
    this->frameIndex = frameIndex;
    this->framePosition = framePosition;
    this->renderTime = renderTime;
    this->symbolHeight = symbolHeight;
    this->symbolWidth = symbolWidth;
}
