#pragma once
#include <cstdint>

class TextFrame {
private:
    uint64_t frameIndex;
    uint8_t* buffer;
    size_t bufferSize;

public:
    explicit TextFrame(size_t bufferSize);
    ~TextFrame();

    [[nodiscard]]
    uint64_t getFrameIndex() const;

    [[nodiscard]]
    uint8_t* getBuffer() const;

    [[nodiscard]]
    size_t getBufferSize() const;

    void updateFrame(uint64_t frameIndex);
};