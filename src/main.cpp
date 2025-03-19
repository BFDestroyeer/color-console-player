#include <csignal>
#include <cstdio>
#include <filesystem>

#ifdef _WIN32
#include <Windows.h>
#endif _WIN32

#ifdef __unix__
#include <cstdio>
#include <sys/ioctl.h>
#include <unistd.h>
#endif __unix__

#include "BufferedRandomGenerator.hpp"
#include "TextFrameBuffer.hpp"
#include "FrameWriter.hpp"
#include "BufferedVideoCapture.hpp"

#include <opencv2/opencv.hpp>

#include "FrameRenderer.hpp"

// 16 x 33 font size
// 236 x 63 symbol resolution
// 3776 x 2079 effective resolution
int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] <<
                " <path to file>" <<
                std::endl;
        return EXIT_FAILURE;
    }
    if (!std::filesystem::exists(argv[1])) {
        std::cout << "File " << argv[1] << " does not exist" << std::endl;
        return EXIT_FAILURE;
    }

    BufferedRandomGenerator::init();

    auto capture = cv::VideoCapture(argv[1]);
    auto bufferedVideoCapture = BufferedVideoCapture(capture);
    const auto frameRate = capture.get(cv::CAP_PROP_FPS);
    const auto frameDuration = std::chrono::nanoseconds(static_cast<int64_t>(1e9 / frameRate));

    auto frame = cv::Mat();
    auto previousFrame = cv::Mat();

#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);

#endif _WIN32

    std::cout << "\x1b[?25l"; // Hide cursor

    int32_t previousColumns = -1;
    int32_t previousRows = -1;
    TextFrameBuffer* textFrameBuffer = nullptr;
    FrameWriter* textWriter = nullptr;

#ifdef _WIN32
    auto executableDir = std::filesystem::path(argv[0]).parent_path();
    _popen(
        std::format("{}\\ffplay.exe \"{}\" -nodisp -autoexit -loglevel quiet", executableDir.string(), argv[1]).c_str(),
        "r"
    );
#endif _WIN32
#ifdef __unix__
    popen(std::format("ffplay \"{}\" -nodisp -autoexit -loglevel quiet", argv[1]).c_str(), "r");
#endif

    std::cout << "\x1b[2J";
    const auto beginPlayTime = std::chrono::high_resolution_clock::now();

#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO consoleScreenBufferInfo;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &consoleScreenBufferInfo);
    std::thread(
        [&consoleScreenBufferInfo] {
            const auto consoleInput = GetStdHandle(STD_INPUT_HANDLE);
            INPUT_RECORD inputRecord;
            DWORD readCount;
            while (true) {
                ReadConsoleInput(consoleInput, &inputRecord, 1, &readCount);
                if (inputRecord.EventType == WINDOW_BUFFER_SIZE_EVENT) {
                    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &consoleScreenBufferInfo);
                }
            }
        }
    ).detach();
#endif _WIN32

    uint64_t frameIndex = 0;
    while (true) {
        auto beginRenderTime = std::chrono::high_resolution_clock::now();
#ifdef _WIN32
        const int32_t columns = consoleScreenBufferInfo.srWindow.Right - consoleScreenBufferInfo.srWindow.Left + 1;
        const int32_t rows = consoleScreenBufferInfo.srWindow.Bottom - consoleScreenBufferInfo.srWindow.Top;
#endif _WIN32
#ifdef __unix__
        winsize windowSize{};
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &windowSize);
        const int32_t columns = windowSize.ws_col;
        const int32_t rows = windowSize.ws_row - 1;
#endif __unix__

        double capturePosition;
        if (!bufferedVideoCapture.read(frame, capturePosition)) {
            break;
        }
        const auto framePosition =
                std::chrono::duration<int64_t, std::ratio<1, 1000000000> >(static_cast<int64_t>(capturePosition * 1e6));
        if ((std::chrono::high_resolution_clock::now() - beginPlayTime) - framePosition > frameDuration / 3) {
            continue;
        }

        const double screenHeight = rows * 33;
        const double screenWidth = columns * 16;
        const double frameAspectRatio = static_cast<double>(frame.rows) / static_cast<double>(frame.cols);
        int32_t symbolHeight, symbolWidth;
        if (screenHeight / screenWidth > frameAspectRatio) {
            symbolHeight = static_cast<int32_t>(columns * frameAspectRatio * (16.0 / 33.0));
            symbolWidth = columns;
        } else {
            symbolHeight = rows;
            symbolWidth = static_cast<int32_t>(rows / frameAspectRatio * (33.0 / 16.0));
        }

        const int32_t bufferSize = ((columns - symbolWidth) / 2 + FrameRenderer::getSymbolSize() * symbolWidth + 7) * symbolHeight + 1;

        if (textFrameBuffer == nullptr) {
            textFrameBuffer = new TextFrameBuffer(bufferSize);
            textWriter = new FrameWriter(beginPlayTime, textFrameBuffer);
        }
        if (previousColumns != columns || previousRows != rows) {
            textFrameBuffer->resize(bufferSize);
            previousFrame = cv::Mat();
#ifdef _WIN32
            std::system("cls");
#endif _WIN32
#ifdef __unix__
            std::system("clear");
#endif __unix__
        }
        previousColumns = columns;
        previousRows = rows;

        cv::resize(frame, frame, cv::Size(symbolWidth * 4, symbolHeight * 4));

        auto renderFrame = textFrameBuffer->getRenderFrame();
        FrameRenderer::redner(frame, (columns - symbolWidth) / 2, renderFrame->getBuffer());
        auto endRenderTime = std::chrono::high_resolution_clock::now();
        renderFrame->updateFrame(
            frameIndex++,
            frameDuration,
            framePosition,
            std::chrono::duration_cast<std::chrono::nanoseconds>(endRenderTime - beginRenderTime),
            symbolHeight
        );
        textFrameBuffer->swapRenderAndReadyFrame();
        std::swap(frame, previousFrame);

        while (std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - beginPlayTime) - framePosition < std::chrono::nanoseconds::zero()) {
        }
    }
    return EXIT_SUCCESS;
}
