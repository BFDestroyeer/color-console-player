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
#include "TextWriter.hpp"
#include "ThreadedVideoCapture.hpp"

#include <opencv2/opencv.hpp>

#include "color.hpp"

#pragma comment(lib, "winmm.lib")

constexpr auto FULL_SYMBOL_SIZE = 41;
constexpr auto DIFFERENTIAL_SYMBOL_SIZE = 10 + FULL_SYMBOL_SIZE;

uint16_t getColor(const cv::Vec3s& foreground, const cv::Vec3s& background, const cv::Vec3s& color) {
    if (cv::norm(foreground - color, cv::NORM_L2SQR) < cv::norm(background - color, cv::NORM_L2SQR)) {
        return 1;
    }
    return 0;
}

void imageToTextFull(const cv::Mat& image, const uint64_t horizontalOffset, uint8_t* buffer) {
#pragma omp parallel for num_threads(4)
    for (int32_t y = 0; y < image.rows; y += 4) {
        for (int32_t x = 0; x < image.cols; x += 4) {

            auto firstForeground = image.at<cv::Vec3b>(y, x);
            auto firstBackground = image.at<cv::Vec3b>(y + 3, x + 3);

            double maxNorm = DBL_MAX;
            double minNorm = DBL_MIN;
            for (int32_t localY = 0; localY < 4; localY++) {
                for (int32_t localX = 0; localX < 4; localX++) {
                    auto& color = image.at<cv::Vec3b>(y + localY, x + localX);
                    const auto colorNorm = cv::norm(color, cv::NORM_L2SQR);
                    if (colorNorm > maxNorm) {
                        maxNorm = colorNorm;
                        firstForeground = color;
                    }
                    if (colorNorm < minNorm) {
                        minNorm = colorNorm;
                        firstBackground = color;
                    }
                }
            }

            auto secondForeground = cv::Vec3s(0, 0, 0);
            auto secondBackground = cv::Vec3s(0, 0, 0);
            uint8_t foregroundClusterSize = 0;
            uint8_t backgroundClusterSize = 0;

            for (int32_t localY = 0; localY < 4; localY++) {
                for (int32_t localX = 0; localX < 4; localX++) {
                    auto& color = image.at<cv::Vec3b>(y + localY, x + localX);
                    if (getColor(firstForeground, firstBackground, color)) {
                        foregroundClusterSize++;
                        secondForeground += color;
                    } else {
                        backgroundClusterSize++;
                        secondBackground += color;
                    }
                }
            }
            secondForeground *= 1.0 / foregroundClusterSize;
            secondBackground *= 1.0 / backgroundClusterSize;

            uint16_t convolutionFull = 0x0;
            for (int32_t localY = 0; localY < 4; localY++) {
                for (int32_t localX = 0; localX < 4; localX++) {
                    const int32_t index = localY * 4 + localX;
                    convolutionFull += getColor(secondForeground, secondBackground, image.at<cv::Vec3b>(y + localY, x + localX)) << index;
                }
            }

            auto [symbol, needSwap] = symbolByConvolutionFull(convolutionFull, foregroundClusterSize, backgroundClusterSize);
            if (needSwap) {
                std::swap(secondForeground, secondBackground);
            }

            // \x1b[38;2;<R>;<G>;<B>m\x1b[48;2;<R>;<G>;<B>m<SYMBOL>
            std::memcpy(
                buffer + (y / 4) * (horizontalOffset + (image.cols / 4) * FULL_SYMBOL_SIZE + 7) + horizontalOffset +
                    (x / 4) * FULL_SYMBOL_SIZE,
                "\x1b[38;2;",
                7);
            std::memcpy(
                buffer + (y / 4) * (horizontalOffset + (image.cols / 4) * FULL_SYMBOL_SIZE + 7) + horizontalOffset +
                    (x / 4) * FULL_SYMBOL_SIZE + 7,
                colorToText(secondForeground[2]),
                3);
            std::memcpy(
                buffer + (y / 4) * (horizontalOffset + (image.cols / 4) * FULL_SYMBOL_SIZE + 7) + horizontalOffset +
                    (x / 4) * FULL_SYMBOL_SIZE + 10,
                ";",
                1);
            std::memcpy(
                buffer + (y / 4) * (horizontalOffset + (image.cols / 4) * FULL_SYMBOL_SIZE + 7) + horizontalOffset +
                    (x / 4) * FULL_SYMBOL_SIZE + 11,
                colorToText(secondForeground[1]),
                3);
            std::memcpy(
                buffer + (y / 4) * (horizontalOffset + (image.cols / 4) * FULL_SYMBOL_SIZE + 7) + horizontalOffset +
                    (x / 4) * FULL_SYMBOL_SIZE + 14,
                ";",
                1);
            std::memcpy(
                buffer + (y / 4) * (horizontalOffset + (image.cols / 4) * FULL_SYMBOL_SIZE + 7) + horizontalOffset +
                    (x / 4) * FULL_SYMBOL_SIZE + 15,
                colorToText(secondForeground[0]),
                3);
            std::memcpy(
                buffer + (y / 4) * (horizontalOffset + (image.cols / 4) * FULL_SYMBOL_SIZE + 7) + horizontalOffset +
                    (x / 4) * FULL_SYMBOL_SIZE + 18,
                "m\x1b[48;2;",
                8);
            std::memcpy(
                buffer + (y / 4) * (horizontalOffset + (image.cols / 4) * FULL_SYMBOL_SIZE + 7) + horizontalOffset +
                    (x / 4) * FULL_SYMBOL_SIZE + 26,
                colorToText(secondBackground[2]),
                3);
            std::memcpy(
                buffer + (y / 4) * (horizontalOffset + (image.cols / 4) * FULL_SYMBOL_SIZE + 7) + horizontalOffset +
                    (x / 4) * FULL_SYMBOL_SIZE + 29,
                ";",
                1);
            std::memcpy(
                buffer + (y / 4) * (horizontalOffset + (image.cols / 4) * FULL_SYMBOL_SIZE + 7) + horizontalOffset +
                    (x / 4) * FULL_SYMBOL_SIZE + 30,
                colorToText(secondBackground[1]),
                3);
            std::memcpy(
                buffer + (y / 4) * (horizontalOffset + (image.cols / 4) * FULL_SYMBOL_SIZE + 7) + horizontalOffset +
                    (x / 4) * FULL_SYMBOL_SIZE + 33,
                ";",
                1);
            std::memcpy(
                buffer + (y / 4) * (horizontalOffset + (image.cols / 4) * FULL_SYMBOL_SIZE + 7) + horizontalOffset +
                    (x / 4) * FULL_SYMBOL_SIZE + 34,
                colorToText(secondBackground[0]),
                3);
            std::memcpy(
                buffer + (y / 4) * (horizontalOffset + (image.cols / 4) * FULL_SYMBOL_SIZE + 7) + horizontalOffset +
                    (x / 4) * FULL_SYMBOL_SIZE + 37,
                "m",
                1);
            std::memcpy(
                buffer + (y / 4) * (horizontalOffset + (image.cols / 4) * FULL_SYMBOL_SIZE + 7) + horizontalOffset +
                    (x / 4) * FULL_SYMBOL_SIZE + 38,
                symbol,
                3);
        }

        // Reset color mode and end line
        std::memcpy(
            buffer + (y / 4) * (horizontalOffset + (image.cols / 4) * FULL_SYMBOL_SIZE + 7) + horizontalOffset +
                (image.cols / 4) * FULL_SYMBOL_SIZE,
            "\x1b[0;0m\n",
            7);
    }
}

int32_t imageToTextDifferential(
    const cv::Mat& image,
    const cv::Mat& previousImage,
    const uint64_t horizontalOffset,
    uint8_t* buffer,
    const float redrawOffset,
    const float minimalRedrawPercentage) {
    int32_t bufferPosition = 0;
    auto imageDifference = cv::Mat();
    cv::absdiff(image, previousImage, imageDifference);
    for (int32_t y = 0; y < image.rows; y += 4) {
        for (int32_t x = 0; x < image.cols; x += 4) {

            if (BufferedRandomGenerator::getRandom() > minimalRedrawPercentage) {
                auto localDifference = cv::Vec3s(0, 0, 0);
                for (int32_t localY = 0; localY < 4; localY++) {
                    for (int32_t localX = 0; localX < 4; localX++) {
                        localDifference += imageDifference.at<cv::Vec3b>(y + localY, x + localX);
                    }
                }
                if (cv::norm(localDifference) < redrawOffset) {
                    continue;
                }
            }

            auto firstForeground = image.at<cv::Vec3b>(y, x);
            auto firstBackground = image.at<cv::Vec3b>(y + 3, x + 3);

            double maxNorm = DBL_MAX;
            double minNorm = DBL_MIN;
            for (int32_t localY = 0; localY < 4; localY++) {
                for (int32_t localX = 0; localX < 4; localX++) {
                    auto& color = image.at<cv::Vec3b>(y + localY, x + localX);
                    const auto colorNorm = cv::norm(color, cv::NORM_L2SQR);
                    if (colorNorm > maxNorm) {
                        maxNorm = colorNorm;
                        firstForeground = color;
                    }
                    if (colorNorm < minNorm) {
                        minNorm = colorNorm;
                        firstBackground = color;
                    }
                }
            }

            auto secondForeground = cv::Vec3s(0, 0, 0);
            auto secondBackground = cv::Vec3s(0, 0, 0);
            uint8_t foregroundClusterSize = 0;
            uint8_t backgroundClusterSize = 0;

            for (int32_t localY = 0; localY < 4; localY++) {
                for (int32_t localX = 0; localX < 4; localX++) {
                    auto& color = image.at<cv::Vec3b>(y + localY, x + localX);
                    if (getColor(firstForeground, firstBackground, color)) {
                        foregroundClusterSize++;
                        secondForeground += color;
                    } else {
                        backgroundClusterSize++;
                        secondBackground += color;
                    }
                }
            }
            secondForeground *= 1.0 / foregroundClusterSize;
            secondBackground *= 1.0 / backgroundClusterSize;

            uint16_t convolutionFull = 0x0;
            for (int32_t localY = 0; localY < 4; localY++) {
                for (int32_t localX = 0; localX < 4; localX++) {
                    const int32_t index = localY * 4 + localX;
                    convolutionFull += getColor(secondForeground, secondBackground, image.at<cv::Vec3b>(y + localY, x + localX)) << index;
                }
            }

            auto [symbol, needSwap] = symbolByConvolutionFull(convolutionFull, foregroundClusterSize, backgroundClusterSize);
            if (needSwap) {
                std::swap(secondForeground, secondBackground);
            }

            // \x1b[0;0H\x1b[38;2;<R>;<G>;<B>m\x1b[48;2;<R>;<G>;<B>m<SYMBOL>
            std::memcpy(
                buffer + bufferPosition,
                "\x1b[",
                2
                );
            std::memcpy(
                buffer + bufferPosition + 2,
                std::format("{:03d}", y / 4 + 1).c_str(),
                3);
            std::memcpy(
                buffer + bufferPosition + 5,
                ";",
                3);
            std::memcpy(
                buffer + bufferPosition + 6,
                std::format("{:03d}", (x / 4 + horizontalOffset + 1)).c_str(),
                3);
            std::memcpy(
                buffer + bufferPosition + 9,
                "H",
                3);
            std::memcpy(
                buffer + bufferPosition + 10,
                "\x1b[38;2;",
                7);
            std::memcpy(
                buffer + bufferPosition + 17,
                colorToText(secondForeground[2]),
                3);
            std::memcpy(
                buffer + bufferPosition + 20,
                ";",
                1);
            std::memcpy(
                buffer + bufferPosition + 21,
                colorToText(secondForeground[1]),
                3);
            std::memcpy(
                buffer + bufferPosition + 24,
                ";",
                1);
            std::memcpy(
                buffer + bufferPosition + 25,
                colorToText(secondForeground[0]),
                3);
            std::memcpy(
                buffer + bufferPosition + 28,
                "m\x1b[48;2;",
                8);
            std::memcpy(
                buffer + bufferPosition + 36,
                colorToText(secondBackground[2]),
                3);
            std::memcpy(
                buffer + bufferPosition + 39,
                ";",
                1);
            std::memcpy(
                buffer + bufferPosition + 40,
                colorToText(secondBackground[1]),
                3);
            std::memcpy(
                buffer + bufferPosition + 43,
                ";",
                1);
            std::memcpy(
                buffer + bufferPosition + 44,
                colorToText(secondBackground[0]),
                3);
            std::memcpy(
                buffer + bufferPosition + 47,
                "m",
                1);
            std::memcpy(
                buffer + bufferPosition + 48,
                symbol,
                3);
            bufferPosition += DIFFERENTIAL_SYMBOL_SIZE;
        }
    }
    buffer[bufferPosition] = 0x0;
    return bufferPosition + 1;
}

// 16 x 33 font size
// 236 x 63 symbol resolution
// 3776 x 2079 effective resolution
int main(int argc, char* argv[]) {
    if (argc != 5) {
        std::cout << "Usage: " << argv[0] << " <path to file> <enable differential render> <symbol redraw offset> <minimal redraw percentage>" << std::endl;
        return EXIT_FAILURE;
    }
    if (!std::filesystem::exists(argv[1])) {
        std::cout << "File " << argv[1] << " does not exist" << std::endl;
        return EXIT_FAILURE;
    }
    bool enableDifferentialRender = std::stoi(argv[2]) == 1;
    auto redrawOffset = std::stof(argv[3]);
    auto minimalRedrawPercentage = std::stof(argv[4]);

    BufferedRandomGenerator::init();

    auto capture = cv::VideoCapture(argv[1]);
    auto bufferedVideoCapture = ThreadedVideoCapture(capture);
    const auto frameRate = capture.get(cv::CAP_PROP_FPS);
    const auto frameDuration = std::chrono::nanoseconds(static_cast<int64_t>(1e9 / frameRate));

    auto frame = cv::Mat();
    auto previousFrame = cv::Mat();

#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);

    const auto consoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD ret;
#endif _WIN32

    std::cout << "\x1b[?25l"; // Hide cursor

    int32_t previousColumns = -1;
    int32_t previousRows = -1;
    TextFrameBuffer* textFrameBuffer = nullptr;
    TextWriter* textWriter = nullptr;

#ifdef _WIN32
    auto executableDir = std::filesystem::path(argv[0]).parent_path();
    _popen(std::format("{}\\ffplay.exe \"{}\" -nodisp -autoexit -loglevel quiet", executableDir.string(), argv[1]).c_str(), "r");
#endif _WIN32
#ifdef __unix__
    popen(std::format("ffplay \"{}\" -nodisp -autoexit -loglevel quiet", argv[1]).c_str(), "r");
#endif

    std::cout << "\x1b[2J";
    const auto beginPlayTime = std::chrono::high_resolution_clock::now();

    CONSOLE_SCREEN_BUFFER_INFO consoleScreenBufferInfo;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &consoleScreenBufferInfo);
    uint64_t frameIndex = 0;
    while (true) {
#ifdef _WIN32
        auto beginRenderTime = std::chrono::high_resolution_clock::now();
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
            std::chrono::duration<int64_t, std::ratio<1, 1000000000>>(static_cast<int64_t>(capturePosition * 1e6));
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

        const int32_t bufferSize = ((columns - symbolWidth) / 2 + FULL_SYMBOL_SIZE * symbolWidth + 7) * symbolHeight + 1;
        const int32_t differentialBufferSize = symbolHeight * symbolWidth * DIFFERENTIAL_SYMBOL_SIZE + 1;

        if (textFrameBuffer == nullptr) {
            textFrameBuffer = new TextFrameBuffer(bufferSize, differentialBufferSize);
            textWriter = new TextWriter(beginPlayTime, textFrameBuffer);
        }
        if (previousColumns != columns || previousRows != rows) {
            previousFrame = cv::Mat();
#ifdef _WIN32
            std::system("cls");
#endif _WIN32
#ifdef __unix__
            std::cout << "x1b[2J";
#endif __unix__
        }
        previousColumns = columns;
        previousRows = rows;

        cv::resize(frame, frame, cv::Size(symbolWidth * 4, symbolHeight * 4));

        int32_t differentialRealSize = differentialBufferSize;
        bool needFullRedraw = previousFrame.empty() || !enableDifferentialRender;
        auto renderFrame = textFrameBuffer->getRenderFrame();
        if (needFullRedraw) {
            imageToTextFull(frame, (columns - symbolWidth) / 2, renderFrame->getBuffer());
        } else {
            differentialRealSize = imageToTextDifferential(frame, previousFrame, (columns - symbolWidth) / 2, renderFrame->getDifferentialBuffer(), redrawOffset, minimalRedrawPercentage);
        }
        auto endRenderTime = std::chrono::high_resolution_clock::now();
        renderFrame->updateFrame(
            frameIndex++,
            needFullRedraw,
            frameDuration,
            framePosition,
            std::chrono::duration_cast<std::chrono::nanoseconds>(endRenderTime - beginRenderTime),
            differentialRealSize,
            symbolHeight);

//         auto beginPrintingTime = std::chrono::high_resolution_clock::now();
// #ifdef _WIN32
//         SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), {0, 0});
//         if (needFullRedraw) {
//             WriteConsoleA(consoleOutput, buffer, bufferSize, &ret, nullptr);
//         } else {
//             WriteConsoleA(consoleOutput, differentialBuffer, differentialRealSize, &ret, nullptr);
//         }
// #endif _WIN32
// #ifdef __unix__
//         std::cout << "\x1b[0;0H";
//         if (needFullRedraw) {
//             std::fwrite(buffer, bufferSize, 1, stdout);
//         } else {
//             std::fwrite(differentialBuffer, differentialRealSize, 1, stdout);
//         }
//
//         std::fflush(stdout);
// #endif __unix__
//         auto endPrintingTime = std::chrono::high_resolution_clock::now();
        std::swap(frame, previousFrame);

        while (std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - beginPlayTime) -
                   framePosition <
               (frameDuration - frameDuration)) {
        }
        // auto endFrameTime = std::chrono::high_resolution_clock::now();
        //
        // std::cout << "\x1b[0;0m";
        // std::cout << std::format("\x1b[{};0H", symbolHeight + 1);
        // std::cout
        //     << "Render time: "
        //     << std::format("{:10.3f}", std::chrono::duration_cast<std::chrono::nanoseconds>(endRenderTime - beginFrameTime).count() / 1e6)
        //     << "ms "
        //     << " Printing time: "
        //     << std::format(
        //            "{:10.3f}", std::chrono::duration_cast<std::chrono::nanoseconds>(endPrintingTime - beginPrintingTime).count() / 1e6)
        //     << "ms "
        //     << "Frame time: "
        //     << std::format("{:10.3f}", std::chrono::duration_cast<std::chrono::nanoseconds>(endFrameTime - beginFrameTime).count() / 1e6)
        //     << "ms "
        //     << "Redraw: " << std::format("{:10.3f}%", static_cast<double>(differentialRealSize) / static_cast<double>(differentialBufferSize) * 100);
    }
    return EXIT_SUCCESS;
}
