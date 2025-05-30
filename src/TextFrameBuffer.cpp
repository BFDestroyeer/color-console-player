#include "TextFrameBuffer.hpp"

TextFrameBuffer::TextFrameBuffer(size_t bufferSize) {
    renderFrame = std::make_shared<TextFrame>(bufferSize);
    readyFrame = std::make_shared<TextFrame>(bufferSize);
    writeFrame = std::make_shared<TextFrame>(bufferSize);
}

void TextFrameBuffer::resize(size_t bufferSize) {
    while (true) {
        std::shared_ptr<TextFrame> localReadyFrame = std::atomic_load(&readyFrame);
        if (std::atomic_compare_exchange_strong(&readyFrame, &localReadyFrame, static_cast<std::shared_ptr<TextFrame>>(nullptr))) {
            break;
        }
    }

    while (true) {
        std::shared_ptr<TextFrame> localRenderFrame = std::atomic_load(&renderFrame);
        if (std::atomic_compare_exchange_strong(&renderFrame, &localRenderFrame, static_cast<std::shared_ptr<TextFrame>>(nullptr))) {
            break;
        }
    }
    std::atomic_store(&renderFrame, std::make_shared<TextFrame>(bufferSize));

    while (true) {
        std::shared_ptr<TextFrame> localWriteFrame = std::atomic_load(&writeFrame);
        if (std::atomic_compare_exchange_strong(&writeFrame, &localWriteFrame, static_cast<std::shared_ptr<TextFrame>>(nullptr))) {
            break;
        }
    }
    std::atomic_store(&writeFrame, std::make_shared<TextFrame>(bufferSize));

    std::atomic_store(&readyFrame, std::make_shared<TextFrame>(bufferSize));
}

void TextFrameBuffer::swapRenderAndReadyFrame() {
    std::shared_ptr<TextFrame> localReadyFrame;
    while (true) {
        localReadyFrame = std::atomic_load(&readyFrame);
        auto localRenderFrame = std::atomic_load(&renderFrame);
        if (localReadyFrame == nullptr || localRenderFrame == nullptr) {
            continue;
        }
        if (std::atomic_compare_exchange_strong(&readyFrame, &localReadyFrame, localRenderFrame)) {
            break;
        }
    }
    std::atomic_store(&renderFrame, localReadyFrame);
}

void TextFrameBuffer::swapWriteAndReadyFrame() {
    std::shared_ptr<TextFrame> localReadyFrame;
    while (true) {
        localReadyFrame = std::atomic_load(&readyFrame);
        auto localWriteFrame = std::atomic_load(&writeFrame);
        if (localReadyFrame == nullptr || localWriteFrame == nullptr) {
            continue;
        }
        if (localWriteFrame->getFrameIndex() >= localReadyFrame->getFrameIndex()) {
            continue;
        }
        if (std::atomic_compare_exchange_strong(&readyFrame, &localReadyFrame, localWriteFrame)) {
            break;
        }
    }
    std::atomic_store(&writeFrame, localReadyFrame);
}

std::shared_ptr<TextFrame> TextFrameBuffer::getRenderFrame() const {
    return std::atomic_load(&renderFrame);
}

std::shared_ptr<TextFrame> TextFrameBuffer::getWriteFrame() const {
    return std::atomic_load(&writeFrame);
}
