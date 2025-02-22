#pragma once

#include <chrono>

class TextFrame {
  private:
    uint8_t* buffer;
    size_t bufferSize;
    uint8_t* differentialBuffer;

    uint64_t frameIndex;
    bool isFullRedraw;
    std::chrono::nanoseconds frameDuration;
    std::chrono::duration<int64_t, std::ratio<1, 1000000000>> framePosition;
    std::chrono::nanoseconds renderTime;
    size_t differentialRealSize;
    int32_t symbolHeight;

  public:
    TextFrame(size_t bufferSize, size_t differentialBufferSize);

    ~TextFrame();

    [[nodiscard]]
    uint8_t* getBuffer() const;

    [[nodiscard]]
    size_t getBufferSize() const;

    [[nodiscard]]
    uint8_t* getDifferentialBuffer() const;

    [[nodiscard]]
    uint64_t getFrameIndex() const;

    [[nodiscard]]
    bool getIsFullRedraw() const;

    [[nodiscard]]
    std::chrono::nanoseconds getFrameDuration() const;

    [[nodiscard]]
    std::chrono::duration<int64_t, std::ratio<1, 1000000000>> getFramePosition() const;

    [[nodiscard]]
    std::chrono::nanoseconds getRenderTime() const;

    [[nodiscard]]
    size_t getDifferentialRealSize() const;

    [[nodiscard]]
    int32_t getSymbolHeight() const;

    void updateFrame(
        uint64_t frameIndex,
        bool isFullRedraw,
        std::chrono::nanoseconds frameDuration,
        std::chrono::duration<int64_t, std::ratio<1, 1000000000>> framePosition,
        std::chrono::nanoseconds renderTime,
        size_t differentialRealSize,
        int32_t symbolHeight);
};