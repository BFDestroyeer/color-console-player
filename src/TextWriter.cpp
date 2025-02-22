#include "TextWriter.hpp"

#include <iostream>
#include <thread>
#ifdef _WIN32
#include <Windows.h>
#endif _WIN32


TextWriter::TextWriter(TextFrameBuffer* textFrameBuffer) : textFrameBuffer(textFrameBuffer) {
    std::thread([this] {
        const auto consoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD ret;
        while (true) {
            const auto frame = this->textFrameBuffer->getWriteFrame();
#ifdef _WIN32
            SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), {0, 0});
            if (true) {
                WriteConsoleA(consoleOutput, frame->getBuffer(), frame->getBufferSize(), &ret, nullptr);
            } else {
                //WriteConsoleA(consoleOutput, differentialBuffer, differentialRealSize, &ret, nullptr);
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
            std::cout << "\x1b[0;0m";
        }
    }).detach();
}


