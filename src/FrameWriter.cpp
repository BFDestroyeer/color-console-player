#include "FrameWriter.hpp"

#include <chrono>
#include <iostream>
#include <thread>

#ifdef _WIN32
#include <Windows.h>
#endif _WIN32

FrameWriter::FrameWriter(
    const std::chrono::time_point<std::chrono::high_resolution_clock>& beginPlayTime,
    const std::shared_ptr<TextFrameBuffer>& textFrameBuffer
)
    : beginPlayTime(beginPlayTime),
      textFrameBuffer(textFrameBuffer) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif _WIN32
    std::cout << "\x1b[?25l"; // Hide cursor

    std::thread(
        [this] {
#ifdef _WIN32
            const auto consoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
            DWORD ret;
#endif _WIN32
            uint64_t previousFrameIndex = 0;
            uint64_t skippedFramesCount = 0;
            while (true) {
                auto beginFrameTime = std::chrono::high_resolution_clock::now();
                this->textFrameBuffer->swapWriteAndReadyFrame();
                const auto frame = this->textFrameBuffer->getWriteFrame();
                auto beginWriteTime = std::chrono::high_resolution_clock::now();
#ifdef _WIN32
                SetConsoleCursorPosition(consoleOutput, {0, 0});
                WriteConsoleA(consoleOutput, frame->getBuffer(), frame->getBufferSize(), &ret, nullptr);
#endif _WIN32
#ifdef __unix__
                std::cout << "\x1b[0;0H";
                std::fwrite(frame->getBuffer(), frame->getBufferSize(), 1, stdout);
                std::fflush(stdout);
#endif __unix__
                std::cout << "\x1b[0;0m";
                auto endWriteTime = std::chrono::high_resolution_clock::now();
                if (previousFrameIndex != 0) {
                    skippedFramesCount += frame->getFrameIndex() - previousFrameIndex - 1;
                }
                previousFrameIndex = frame->getFrameIndex();
                while (std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - this->beginPlayTime)
                       - frame->getFramePosition() <
                       std::chrono::nanoseconds::zero()) {
                }
                auto endFrameTime = std::chrono::high_resolution_clock::now();
                std::cout << std::format("\x1b[{};0H", frame->getSymbolHeight() + 1);
                std::cout << "Frame time: " << std::format(
                    "{:10.3f}",
                    std::chrono::duration_cast<std::chrono::nanoseconds>(endFrameTime - beginFrameTime).count() / 1e6
                );
                std::cout << " Render time: " << std::format("{:10.3f}", frame->getRenderTime().count() / 1e6);
                std::cout << " Write time: " << std::format(
                    "{:10.3f}",
                    std::chrono::duration_cast<std::chrono::nanoseconds>(endWriteTime - beginWriteTime).count() / 1e6
                );
                std::cout << " Skipped frames: " << std::format("{:7d}", skippedFramesCount);
            }
        }
    ).detach();
}
