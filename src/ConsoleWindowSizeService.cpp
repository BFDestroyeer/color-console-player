#include "ConsoleWindowSizeService.hpp"

#if defined(__unix__) || defined(__APPLE__)
#include <sys/ioctl.h>
#include <unistd.h>
#endif

ConsoleWindowSizeService::ConsoleWindowSizeService() {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO initialConsoleScreenBufferInfo;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &initialConsoleScreenBufferInfo);
    consoleSize = extractConsoleWindowSize(initialConsoleScreenBufferInfo);
    thread = std::jthread(
        [this] (const std::stop_token& stopToken) {
            CONSOLE_SCREEN_BUFFER_INFO consoleScreenBufferInfo;
            const auto consoleInput = GetStdHandle(STD_INPUT_HANDLE);
            const auto consoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
            INPUT_RECORD inputRecord;
            DWORD readCount;
            while (true) {
                if (stopToken.stop_requested()) {
                    return;
                }
                ReadConsoleInput(consoleInput, &inputRecord, 1, &readCount);
                if (inputRecord.EventType == WINDOW_BUFFER_SIZE_EVENT) {
                    GetConsoleScreenBufferInfo(consoleOutput, &consoleScreenBufferInfo);
                    auto size = extractConsoleWindowSize(consoleScreenBufferInfo);
                    consoleSize.store(size);
                }
            }
        }
    );
#endif
}

ConsoleWindowSizeService::~ConsoleWindowSizeService() {
#ifdef _WIN32
    thread.request_stop();
    if (thread.joinable()) {
        thread.join();
    }
#endif
}

std::pair<int16_t, int16_t> ConsoleWindowSizeService::getConsoleSize() const {
#ifdef _WIN32
    return consoleSize.load();
#endif
#if defined(__unix__) || defined(__APPLE__)
    winsize windowSize{};
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &windowSize);
    return std::make_pair(windowSize.ws_col, windowSize.ws_row - 1);
#endif
}

#ifdef _WIN32
std::pair<int16_t, int16_t> ConsoleWindowSizeService::extractConsoleWindowSize(const CONSOLE_SCREEN_BUFFER_INFO& consoleScreenBufferInfo) {
    return std::make_pair(
        consoleScreenBufferInfo.srWindow.Right - consoleScreenBufferInfo.srWindow.Left + 1,
        consoleScreenBufferInfo.srWindow.Bottom - consoleScreenBufferInfo.srWindow.Top
    );
}
#endif
