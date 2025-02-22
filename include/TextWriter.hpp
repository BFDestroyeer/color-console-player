#pragma once

#include <chrono>

#include "TextFrameBuffer.hpp"

class TextWriter {
  private:
    std::chrono::time_point<std::chrono::high_resolution_clock> beginPlayTime;
    TextFrameBuffer* textFrameBuffer;

public:
    TextWriter(std::chrono::time_point<std::chrono::high_resolution_clock> beginPlayTime, TextFrameBuffer* textFrameBuffer);
};