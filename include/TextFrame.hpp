#pragma once

#include <chrono>

class TextFrame {
private:
    /**
     * @brief Frame buffer
     */
    uint8_t* buffer;

    /**
     * @brief Size of frame buffer in bytes
     */
    size_t bufferSize;

    /**
     * @brief Index of frame
     */
    uint64_t frameIndex;

    /**
     * @brief Frame position
     */
    std::chrono::duration<int64_t, std::ratio<1, 1000000000>> framePosition;

    /**
     * @brief Frame rendering time
     */
    std::chrono::nanoseconds renderTime;

    /**
     * @brief Height of console window in symbols while rendering
     */
    int32_t symbolHeight;

public:
    /**
     * @param bufferSize size of buffer
     */
    explicit TextFrame(size_t bufferSize);

    ~TextFrame();

    /**
     * @brief Get frame buffer
     * @return Frame buffer
     */
    [[nodiscard]]
    uint8_t* getBuffer() const;

    /**
     * @brief Get size of frame buffer in bytes
     * @return Size of frame buffer in bytes
     */
    [[nodiscard]]
    size_t getBufferSize() const;

    /**
     * @brief Get index of frame
     * @return Index of frame
     */
    [[nodiscard]]
    uint64_t getFrameIndex() const;

    /**
     * @brief Get frame position
     * @return Frame position
     */
    [[nodiscard]]
    std::chrono::duration<int64_t, std::ratio<1, 1000000000>> getFramePosition() const;

    /**
     * @brief Get frame rendering time
     * @return Frame rendering time
     */
    [[nodiscard]]
    std::chrono::nanoseconds getRenderTime() const;

    /**
     * @brief Get height of console window in symbols  while rendering
     * @return Height of console window in symbols while rendering
     */
    [[nodiscard]]
    int32_t getSymbolHeight() const;

    /**
     * @brief Update frame content
     * @param frameIndex Index of frame
     * @param framePosition Frame position
     * @param renderTime Frame rendering time
     * @param symbolHeight Height of console window in symbols while rendering
     */
    void updateFrame(
        uint64_t frameIndex,
        std::chrono::duration<int64_t, std::ratio<1, 1000000000>> framePosition,
        std::chrono::nanoseconds renderTime,
        int32_t symbolHeight
    );
};
