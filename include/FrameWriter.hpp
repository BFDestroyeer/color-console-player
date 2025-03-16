#pragma once

#include <chrono>

#include "TextFrameBuffer.hpp"

class FrameWriter {
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> beginPlayTime;
    TextFrameBuffer* textFrameBuffer;

public:
    FrameWriter(std::chrono::time_point<std::chrono::high_resolution_clock> beginPlayTime, TextFrameBuffer* textFrameBuffer);
};
