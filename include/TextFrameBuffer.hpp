#pragma once

#include <atomic>

#include <TextFrame.hpp>

class TextFrameBuffer {
private:
    std::atomic<TextFrame*> renderFrame;
    std::atomic<TextFrame*> readyFrame;
    std::atomic<TextFrame*> writeFrame;

public:
    explicit TextFrameBuffer(size_t bufferSize, size_t differentialBufferSize);

    void resize(size_t bufferSize, size_t differentialBufferSize);

    TextFrame* getRenderFrame();

    TextFrame* getWriteFrame();
};
