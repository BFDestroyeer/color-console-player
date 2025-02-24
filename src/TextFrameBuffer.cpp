#include "TextFrameBuffer.hpp"

TextFrameBuffer::TextFrameBuffer(size_t bufferSize, size_t differentialBufferSize) {
    renderFrame.store(std::make_shared<TextFrame>(bufferSize, differentialBufferSize));
    readyFrame.store(std::make_shared<TextFrame>(bufferSize, differentialBufferSize));
    writeFrame.store(std::make_shared<TextFrame>(bufferSize, differentialBufferSize));
}

void TextFrameBuffer::resize(size_t bufferSize, size_t differentialBufferSize) {
    while (true) {
        std::shared_ptr<TextFrame> localReadyFrame = readyFrame.load();
        if (readyFrame.compare_exchange_strong(localReadyFrame, nullptr)) {
            break;
        }
    }

    while (true) {
        std::shared_ptr<TextFrame> localRenderFrame = renderFrame.load();
        if (renderFrame.compare_exchange_strong(localRenderFrame, nullptr)) {
            break;
        }
    }
    renderFrame.store(std::make_shared<TextFrame>(bufferSize, differentialBufferSize));

    while (true) {
        std::shared_ptr<TextFrame> localWriteFrame = writeFrame.load();
        if (writeFrame.compare_exchange_strong(localWriteFrame, nullptr)) {
            break;
        }
    }
    writeFrame.store(std::make_shared<TextFrame>(bufferSize, differentialBufferSize));

    readyFrame.store(std::make_shared<TextFrame>(bufferSize, differentialBufferSize));
}

std::shared_ptr<TextFrame> TextFrameBuffer::getRenderFrame() {
    std::shared_ptr<TextFrame> localReadyFrame;
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

std::shared_ptr<TextFrame> TextFrameBuffer::getWriteFrame() {
    std::shared_ptr<TextFrame> localReadyFrame;
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
