#include "TextFrameBuffer.hpp"

TextFrameBuffer::TextFrameBuffer(size_t bufferSize, size_t differentialBufferSize){
    renderFrame.store(new TextFrame(bufferSize, differentialBufferSize));
    readyFrame.store(new TextFrame(bufferSize, differentialBufferSize));
    writeFrame.store(new TextFrame(bufferSize, differentialBufferSize));
}

void TextFrameBuffer::resize(size_t bufferSize, size_t differentialBufferSize) {
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
