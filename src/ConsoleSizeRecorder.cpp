#include "ConsoleSizeRecorder.hpp"

#include <thread>

ConsoleSizeRecorder::ConsoleSizeRecorder() {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO initialConsoleScreenBufferInfo;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &initialConsoleScreenBufferInfo);
    consoleSize = extractConsoleSize(initialConsoleScreenBufferInfo);
    std::thread(
        [this] {
            CONSOLE_SCREEN_BUFFER_INFO consoleScreenBufferInfo;
            const auto consoleInput = GetStdHandle(STD_INPUT_HANDLE);
            const auto consoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
            INPUT_RECORD inputRecord;
            DWORD readCount;
            while (true) {
                ReadConsoleInput(consoleInput, &inputRecord, 1, &readCount);
                if (inputRecord.EventType == WINDOW_BUFFER_SIZE_EVENT) {
                    GetConsoleScreenBufferInfo(consoleOutput, &consoleScreenBufferInfo);
                    auto size = extractConsoleSize(consoleScreenBufferInfo);
                    consoleSize.store(size);
                }
            }
        }
    ).detach();
#endif _WIN32
}

std::pair<int16_t, int16_t> ConsoleSizeRecorder::getConsoleSize() const {
#ifdef _WIN32
    return consoleSize.load();
#endif _WIN32
#ifdef __unix__
    winsize windowSize{};
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &windowSize);
    return std::make_pair(windowSize.ws_col, windowSize.ws_row - 1)
#endif __unix__
}

std::pair<int16_t, int16_t> ConsoleSizeRecorder::extractConsoleSize(const CONSOLE_SCREEN_BUFFER_INFO& consoleScreenBufferInfo) {
    return std::make_pair(
        consoleScreenBufferInfo.srWindow.Right - consoleScreenBufferInfo.srWindow.Left + 1,
        consoleScreenBufferInfo.srWindow.Bottom - consoleScreenBufferInfo.srWindow.Top
    );
}
