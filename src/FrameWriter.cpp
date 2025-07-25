#include "FrameWriter.hpp"

#include <chrono>
#include <iostream>
#include <thread>

#ifdef _WIN32
#include <Windows.h>
#endif

FrameWriter::FrameWriter(
    const std::chrono::time_point<std::chrono::high_resolution_clock>& beginPlayTime,
    const std::shared_ptr<TextFrameBuffer>& textFrameBuffer,
    const std::shared_ptr<ConsoleWindowSizeService>& consoleWindowSizeService
)
    : beginPlayTime(beginPlayTime),
      textFrameBuffer(textFrameBuffer),
      consoleWindowSizeService(consoleWindowSizeService) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    std::cout << "\x1b[?25l"; // Hide cursor

    std::thread(
        [this] {
#ifdef _WIN32
            const auto consoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
            DWORD ret;
#endif
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
#endif
#if defined(__unix__) || defined(__APPLE__)
                std::cout << "\x1b[0;0H";
                std::fwrite(frame->getBuffer(), frame->getBufferSize(), 1, stdout);
                std::fflush(stdout);
#endif
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

                auto symbolCount = frame->getSymbolWidth() * frame->getSymbolHeight();
                auto renderTimeInMilliseconds = static_cast<double>(frame->getRenderTime().count()) / 1e6;
                auto writeTimeInMilliseconds = static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(
                    endWriteTime - beginWriteTime
                ).count()) / 1e6;

                std::stringstream statusBarStream;
                statusBarStream<< "Frame time: " << std::format(
                    "{:10.3f}",
                    std::chrono::duration_cast<std::chrono::nanoseconds>(endFrameTime - beginFrameTime).count() / 1e6
                );
                statusBarStream << " Render time: " << std::format("{:10.3f}", renderTimeInMilliseconds);
                statusBarStream << " Write time: " << std::format(
                    "{:10.3f}",
                    writeTimeInMilliseconds
                );
                statusBarStream << " Skipped frames: " << std::format("{:7d}", skippedFramesCount);
                statusBarStream << " Resolution: " << frame->getSymbolWidth() << "×" <<  frame->getSymbolHeight();
                statusBarStream << " Symbols: " << symbolCount;
                statusBarStream << " Render performance: " << std::format("{:10.3f}", static_cast<double>(symbolCount) / renderTimeInMilliseconds);
                statusBarStream << " Write performance: " << std::format("{:10.3f}", static_cast<double>(symbolCount) / writeTimeInMilliseconds);

                auto [consoleWidth, _] = this->consoleWindowSizeService->getConsoleSize();
                std::cout << statusBarStream.str().substr(0, consoleWidth);
            }
        }
    ).detach();
}
