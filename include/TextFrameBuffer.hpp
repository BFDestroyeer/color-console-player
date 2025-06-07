#pragma once

#include <memory>

#include <TextFrame.hpp>

/**
 * @brief Buffer of text frames
 */
class TextFrameBuffer {
private:
    /**
     * Frame for rendering
     */
    std::shared_ptr<TextFrame> renderFrame;

    /**
     * Frame ready for writing
     */
    std::shared_ptr<TextFrame> readyFrame;

    /**
     * Frame for writing
     */
    std::shared_ptr<TextFrame> writeFrame;

public:
    /**
     * @param bufferSize size of frame buffer in bytes
     */
    explicit TextFrameBuffer(size_t bufferSize);

    /**
     * @brief Resize frame buffers
     * @param bufferSize new size of frame buffer in bytes
     */
    void resize(size_t bufferSize);

    /**
     * @brief Swap frame for rendering with frame ready for writing
     */
    void swapRenderAndReadyFrame();

    /**
     * @brief Swap frame for writing with frame ready for writing
     */
    void swapWriteAndReadyFrame();

    /**
     * @brief Get frame for rendering
     * @return Frame for rendering
     */
    [[nodiscard]]
    std::shared_ptr<TextFrame> getRenderFrame() const;

    /**
     * @brief Get frame for writing
     * @return Frame for writing
     */
    [[nodiscard]]
    std::shared_ptr<TextFrame> getWriteFrame() const;
};