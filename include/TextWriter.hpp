#pragma once

#include <chrono>

#ifdef _WIN32
#include <Windows.h>
#endif _WIN32

#include "TextFrameBuffer.hpp"

class TextWriter {
  private:
    std::chrono::time_point<std::chrono::high_resolution_clock> beginPlayTime;
    TextFrameBuffer* textFrameBuffer;
    CONSOLE_SCREEN_BUFFER_INFO* consoleScreenBufferInfo;


public:
    TextWriter(std::chrono::time_point<std::chrono::high_resolution_clock> beginPlayTime, TextFrameBuffer* textFrameBuffer, CONSOLE_SCREEN_BUFFER_INFO* consoleScreenBufferInfo);
};