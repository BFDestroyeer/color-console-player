#include "TextFrameBuffer.hpp"

TextFrameBuffer::TextFrameBuffer(size_t bufferSize) {
    renderFrame.store(new TextFrame(bufferSize));
    readyFrame.store(new TextFrame(bufferSize));
    writeFrame.store(new TextFrame(bufferSize));
}

void TextFrameBuffer::resize(size_t bufferSize) {
}

TextFrame* TextFrameBuffer::getRenderFrame() {
    TextFrame* localReadyFrame;
    while (true) {
        localReadyFrame = readyFrame.load();
        auto localRenderFrame = renderFrame.load();
        if (readyFrame.compare_exchange_strong(localReadyFrame, localRenderFrame)) {
            break;
        }
    }
    renderFrame.store(localReadyFrame);
    return localReadyFrame;
}

TextFrame* TextFrameBuffer::getWriteFrame() {
    TextFrame* localReadyFrame;
    while (true) {
        localReadyFrame = readyFrame.load();
        auto localWriteFrame = writeFrame.load();
        if (localWriteFrame->getFrameIndex() >= localReadyFrame->getFrameIndex()) {
            continue;
        }
        if (readyFrame.compare_exchange_strong(localReadyFrame, localWriteFrame)) {
            break;
        }
    }
    writeFrame.store(localReadyFrame);
    return localReadyFrame;
}
