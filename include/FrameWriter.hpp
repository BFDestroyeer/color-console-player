#pragma once

#include <chrono>

#include "ConsoleWindowSizeService.hpp"
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

    /**
     * @brief Console window size service
     */
    const std::shared_ptr<ConsoleWindowSizeService> consoleWindowSizeService;

public:

    /**
     * @param beginPlayTime Playback start time
     * @param textFrameBuffer Buffer of text frames
     * @param consoleWindowSizeService Console window size service
     */
    FrameWriter(
        const std::chrono::time_point<std::chrono::high_resolution_clock>& beginPlayTime,
        const std::shared_ptr<TextFrameBuffer>& textFrameBuffer,
        const std::shared_ptr<ConsoleWindowSizeService>& consoleWindowSizeService
    );
};
