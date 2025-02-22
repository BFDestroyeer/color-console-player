#pragma once

#include <chrono>

class TextFrame {
private:
    uint8_t* buffer;
    size_t bufferSize;

    uint64_t frameIndex;
    std::chrono::nanoseconds frameDuration;
    std::chrono::duration<int64_t, std::ratio<1, 1000000000>> framePosition;
    std::chrono::nanoseconds renderTime;

public:
    explicit TextFrame(size_t bufferSize);
    ~TextFrame();

    [[nodiscard]]
    uint8_t* getBuffer() const;

    [[nodiscard]]
    size_t getBufferSize() const;

    [[nodiscard]]
    uint64_t getFrameIndex() const;

    [[nodiscard]]
    std::chrono::nanoseconds getFrameDuration() const;

    [[nodiscard]]
    std::chrono::duration<int64_t, std::ratio<1, 1000000000>> getFramePosition() const;

    [[nodiscard]]
    std::chrono::nanoseconds getRenderTime() const;


    void updateFrame(uint64_t frameIndex, std::chrono::nanoseconds frameDuration, std::chrono::duration<int64_t, std::ratio<1, 1000000000>> framePosition, std::chrono::nanoseconds renderTime);
};