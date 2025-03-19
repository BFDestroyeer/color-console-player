#pragma once

#include <memory>

#include <TextFrame.hpp>

class TextFrameBuffer {
private:
    std::atomic<std::shared_ptr<TextFrame> > renderFrame;
    std::atomic<std::shared_ptr<TextFrame> > readyFrame;
    std::atomic<std::shared_ptr<TextFrame> > writeFrame;

public:
    explicit TextFrameBuffer(size_t bufferSize);

    void resize(size_t bufferSize);

    void swapRenderAndReadyFrame();

    void swapWriteAndReadyFrame();

    std::shared_ptr<TextFrame> getRenderFrame() const;

    std::shared_ptr<TextFrame> getWriteFrame() const;
};
