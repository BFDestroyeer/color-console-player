#pragma once

#include <atomic>
#include <utility>

#ifdef _WIN32
#include <Windows.h>
#endif

class ConsoleSizeRecorder {
private:
#ifdef _WIN32
    std::atomic<std::pair<int16_t, int16_t>> consoleSize;
#endif

public:
    ConsoleSizeRecorder();

    [[nodiscard]]
    std::pair<int16_t, int16_t> getConsoleSize() const;

private:
#ifdef _WIN32
    static std::pair<int16_t, int16_t> extractConsoleSize(const CONSOLE_SCREEN_BUFFER_INFO& consoleScreenBufferInfo);
#endif
};
