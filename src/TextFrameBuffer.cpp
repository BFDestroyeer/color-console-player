#include "TextFrameBuffer.hpp"

TextFrameBuffer::TextFrameBuffer(size_t bufferSize, size_t differentialBufferSize){
    renderFrame.store(new TextFrame(bufferSize, differentialBufferSize));
    readyFrame.store(new TextFrame(bufferSize, differentialBufferSize));
    writeFrame.store(new TextFrame(bufferSize, differentialBufferSize));
}

void TextFrameBuffer::resize(size_t bufferSize, size_t differentialBufferSize) {
    TextFrame* localReadyFrame;
    while (true) {
        localReadyFrame = readyFrame.load();
        if (readyFrame.compare_exchange_strong(localReadyFrame, nullptr)) {
            break;
        }
    }

    TextFrame* localRenderFrame;
    while (true) {
        localRenderFrame = renderFrame.load();
        if (renderFrame.compare_exchange_strong(localRenderFrame, nullptr)) {
            break;
        }
    }
    delete localRenderFrame;
    renderFrame.store(new TextFrame(bufferSize, differentialBufferSize));

    TextFrame* localWriteFrame;
    while (true) {
        localWriteFrame = writeFrame.load();
        if (writeFrame.compare_exchange_strong(localRenderFrame, nullptr)) {
            break;
        }
    }
    delete localWriteFrame;
    writeFrame.store(new TextFrame(bufferSize, differentialBufferSize));

    delete localReadyFrame;
    readyFrame.store(new TextFrame(bufferSize, differentialBufferSize));
}

TextFrame* TextFrameBuffer::getRenderFrame() {
    TextFrame* localReadyFrame;
    while (true) {
        localReadyFrame = readyFrame.load();
        auto localRenderFrame = renderFrame.load();
        if (localReadyFrame == nullptr || localRenderFrame == nullptr) {
            continue;
        }
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
        if (localReadyFrame == nullptr || localWriteFrame == nullptr) {
            continue;
        }
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
