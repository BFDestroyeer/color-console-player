#include "TextWriter.hpp"

#include <chrono>
#include <iostream>
#include <thread>
#ifdef _WIN32
#include <Windows.h>
#endif _WIN32

TextWriter::TextWriter(
    std::chrono::time_point<std::chrono::high_resolution_clock> beginPlayTime,
    TextFrameBuffer* textFrameBuffer
)
    : beginPlayTime(beginPlayTime), textFrameBuffer(textFrameBuffer) {
    std::thread(
        [this] {
            const auto consoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
            DWORD ret;
            uint64_t previousFrameIndex = 0;
            uint64_t skippedFramesCount = 0;
            while (true) {
                auto beginFrameTime = std::chrono::high_resolution_clock::now();
                const auto frame = this->textFrameBuffer->getWriteFrame();
#ifdef _WIN32
                SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), {0, 0});
                if (frame->getIsFullRedraw()) {
                    WriteConsoleA(consoleOutput, frame->getBuffer(), frame->getBufferSize(), &ret, nullptr);
                } else {
                    WriteConsoleA(consoleOutput, frame->getDifferentialBuffer(), frame->getDifferentialRealSize(), &ret, nullptr);
                }
                while (std::chrono::duration_cast<std::chrono::nanoseconds>(
                           std::chrono::high_resolution_clock::now() - this->beginPlayTime
                       ) -
                       frame->getFramePosition() <
                       frame->getFrameDuration()) {
                }
#endif _WIN32
#ifdef __unix__
            std::cout << "\x1b[0;0H";
            if (needFullRedraw) {
                std::fwrite(buffer, bufferSize, 1, stdout);
            } else {
                std::fwrite(differentialBuffer, differentialRealSize, 1, stdout);
            }
            std::fflush(stdout);
#endif __unix__
                if (previousFrameIndex != 0 && previousFrameIndex + 1 != frame->getFrameIndex()) {
                    skippedFramesCount++;
                }
                previousFrameIndex = frame->getFrameIndex();
                auto endFrameTime = std::chrono::high_resolution_clock::now();
                std::cout << "\x1b[0;0m";
                std::cout << std::format("\x1b[{};0H", frame->getSymbolHeight() + 1);
                std::cout << "Frame time: " << std::format(
                            "{:10.3f}",
                            std::chrono::duration_cast<std::chrono::nanoseconds>(endFrameTime - beginFrameTime).count() / 1e6
                        )
                        << " Render time: " << std::format("{:10.3f}", frame->getRenderTime().count() / 1e6)
                        << " Skipped frames: " << std::format("{:7d}", skippedFramesCount);
            }
        }
    ).detach();
}
