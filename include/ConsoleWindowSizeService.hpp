#pragma once

#include <atomic>
#include <utility>

#ifdef _WIN32
#include <Windows.h>
#endif

/**
 * @brief Service that provides access to current console window size
 * @note For Windows create separate thread to handle console window resize events
 */
class ConsoleWindowSizeService {
private:
#ifdef _WIN32
    /**
     * @brief Current console window size (width, height)
     */
    std::atomic<std::pair<int16_t, int16_t>> consoleSize;
#endif

public:
    ConsoleWindowSizeService();

    /**
     * @brief Get current console window size (width, height)
     * @return Current console window size (width, height)
     */
    [[nodiscard]]
    std::pair<int16_t, int16_t> getConsoleSize() const;

private:
#ifdef _WIN32
    /**
     * Extract console window size from Windows CONSOLE_SCREEN_BUFFER_INFO
     * @param consoleScreenBufferInfo output value of GetConsoleScreenBufferInfo(...)
     * @return Console window size
     */
    static std::pair<int16_t, int16_t> extractConsoleWindowSize(const CONSOLE_SCREEN_BUFFER_INFO& consoleScreenBufferInfo);
#endif
};
