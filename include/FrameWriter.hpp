#pragma once

#include <chrono>

#include "TextFrameBuffer.hpp"

class FrameWriter {
private:
    /**
     * @brief Playback start time
     */
    const std::chrono::time_point<std::chrono::high_resolution_clock> beginPlayTime;

    /**
     * @brief Buffer of text frames
     */
    const std::shared_ptr<TextFrameBuffer> textFrameBuffer;

public:

    /**
     * @param beginPlayTime Playback start time
     * @param textFrameBuffer Buffer of text frames
     */
    FrameWriter(
        const std::chrono::time_point<std::chrono::high_resolution_clock>& beginPlayTime,
        const std::shared_ptr<TextFrameBuffer>& textFrameBuffer
    );
};
