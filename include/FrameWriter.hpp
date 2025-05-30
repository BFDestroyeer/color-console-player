#pragma once

#include <chrono>

#include "TextFrameBuffer.hpp"

class FrameWriter {
private:
    const std::chrono::time_point<std::chrono::high_resolution_clock> beginPlayTime;
    const std::shared_ptr<TextFrameBuffer> textFrameBuffer;

public:
    FrameWriter(
        const std::chrono::time_point<std::chrono::high_resolution_clock>& beginPlayTime,
        const std::shared_ptr<TextFrameBuffer>& textFrameBuffer
    );
};
