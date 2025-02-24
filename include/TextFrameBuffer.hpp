#pragma once

#include <atomic>

#include <TextFrame.hpp>

class TextFrameBuffer {
private:
    std::atomic<std::shared_ptr<TextFrame>> renderFrame;
    std::atomic<std::shared_ptr<TextFrame>> readyFrame;
    std::atomic<std::shared_ptr<TextFrame>> writeFrame;

public:
    explicit TextFrameBuffer(size_t bufferSize, size_t differentialBufferSize);

    void resize(size_t bufferSize, size_t differentialBufferSize);

    std::shared_ptr<TextFrame> getRenderFrame();

    std::shared_ptr<TextFrame> getWriteFrame();
};
