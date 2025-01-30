#include <csignal>
#include <filesystem>
#include <stdio.h>

#ifdef _WIN32
#include <Windows.h>
#endif _WIN32

#ifdef __unix__
#include <cstdio>
#include <sys/ioctl.h>
#include <unistd.h>
#endif __unix__

#include <opencv2/opencv.hpp>

#include "color.hpp"

#pragma comment(lib, "winmm.lib")

constexpr auto FULL_SYMBOL_SIZE = 41;
constexpr char TEMP_AUDIO_FILE_PATH[] = "~temp_audio.wav";

uint16_t getColor(const cv::Vec3s &foreground, const cv::Vec3s &background, const cv::Vec3s &color) {
    if (cv::norm(foreground - color) < cv::norm(background - color)) {
        return 1;
    }
    return 0;
}

void imageToTextFull(const cv::Mat &image, const uint64_t horizontalOffset, uint8_t *buffer) {
#pragma omp parallel for num_threads(4)
    for (int32_t y = 0; y < image.rows; y += 4) {
        for (int32_t x = 0; x < image.cols; x += 4) {

            auto firstForeground = image.at<cv::Vec3b>(y, x);
            auto firstBackground = image.at<cv::Vec3b>(y + 3, x + 3);

            double maxNorm = DBL_MAX;
            double minNorm = DBL_MIN;
            for (int32_t localY = 0; localY < 4; localY++) {
                for (int32_t localX = 0; localX < 4; localX++) {
                    auto &color = image.at<cv::Vec3b>(y + localY, x + localX);
                    const auto
                    colorNorm = cv::norm(color);
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
            std::memcpy(buffer + (y / 4) * (horizontalOffset + (image.cols / 4) * FULL_SYMBOL_SIZE + 7) + horizontalOffset +
                            (x / 4) * FULL_SYMBOL_SIZE,
                        "\x1b[38;2;", 7);
            std::memcpy(buffer + (y / 4) * (horizontalOffset + (image.cols / 4) * FULL_SYMBOL_SIZE + 7) + horizontalOffset +
                            (x / 4) * FULL_SYMBOL_SIZE + 7,
                        colorToText(secondForeground[2]), 3);
            std::memcpy(buffer + (y / 4) * (horizontalOffset + (image.cols / 4) * FULL_SYMBOL_SIZE + 7) + horizontalOffset +
                            (x / 4) * FULL_SYMBOL_SIZE + 10,
                        ";", 1);
            std::memcpy(buffer + (y / 4) * (horizontalOffset + (image.cols / 4) * FULL_SYMBOL_SIZE + 7) + horizontalOffset +
                            (x / 4) * FULL_SYMBOL_SIZE + 11,
                        colorToText(secondForeground[1]), 3);
            std::memcpy(buffer + (y / 4) * (horizontalOffset + (image.cols / 4) * FULL_SYMBOL_SIZE + 7) + horizontalOffset +
                            (x / 4) * FULL_SYMBOL_SIZE + 14,
                        ";", 1);
            std::memcpy(buffer + (y / 4) * (horizontalOffset + (image.cols / 4) * FULL_SYMBOL_SIZE + 7) + horizontalOffset +
                            (x / 4) * FULL_SYMBOL_SIZE + 15,
                        colorToText(secondForeground[0]), 3);
            std::memcpy(buffer + (y / 4) * (horizontalOffset + (image.cols / 4) * FULL_SYMBOL_SIZE + 7) + horizontalOffset +
                            (x / 4) * FULL_SYMBOL_SIZE + 18,
                        "m\x1b[48;2;", 8);
            std::memcpy(buffer + (y / 4) * (horizontalOffset + (image.cols / 4) * FULL_SYMBOL_SIZE + 7) + horizontalOffset +
                            (x / 4) * FULL_SYMBOL_SIZE + 26,
                        colorToText(secondBackground[2]), 3);
            std::memcpy(buffer + (y / 4) * (horizontalOffset + (image.cols / 4) * FULL_SYMBOL_SIZE + 7) + horizontalOffset +
                            (x / 4) * FULL_SYMBOL_SIZE + 29,
                        ";", 1);
            std::memcpy(buffer + (y / 4) * (horizontalOffset + (image.cols / 4) * FULL_SYMBOL_SIZE + 7) + horizontalOffset +
                            (x / 4) * FULL_SYMBOL_SIZE + 30,
                        colorToText(secondBackground[1]), 3);
            std::memcpy(buffer + (y / 4) * (horizontalOffset + (image.cols / 4) * FULL_SYMBOL_SIZE + 7) + horizontalOffset +
                            (x / 4) * FULL_SYMBOL_SIZE + 33,
                        ";", 1);
            std::memcpy(buffer + (y / 4) * (horizontalOffset + (image.cols / 4) * FULL_SYMBOL_SIZE + 7) + horizontalOffset +
                            (x / 4) * FULL_SYMBOL_SIZE + 34,
                        colorToText(secondBackground[0]), 3);
            std::memcpy(buffer + (y / 4) * (horizontalOffset + (image.cols / 4) * FULL_SYMBOL_SIZE + 7) + horizontalOffset +
                            (x / 4) * FULL_SYMBOL_SIZE + 37,
                        "m", 1);
            std::memcpy(buffer + (y / 4) * (horizontalOffset + (image.cols / 4) * FULL_SYMBOL_SIZE + 7) + horizontalOffset +
                            (x / 4) * FULL_SYMBOL_SIZE + 38,
                        symbol, 3);
        }

        // Reset color mode and end line
        std::memcpy(buffer + (y / 4) * (horizontalOffset + (image.cols / 4) * FULL_SYMBOL_SIZE + 7) + horizontalOffset +
                        (image.cols / 4) * FULL_SYMBOL_SIZE,
                    "\x1b[0;0m\n", 7);
    }
}

// 16 x 33 font size
// 236 x 63 symbol resolution
// 3776 x 2079 effective resolution
int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <path to file>" << std::endl;
        return EXIT_FAILURE;
    }
    if (!std::filesystem::exists(argv[1])) {
        std::cout << "File " << argv[1] << " does not exist" << std::endl;
        return EXIT_FAILURE;
    }

    auto capture = cv::VideoCapture(argv[1]);
    const auto frameRate = capture.get(cv::CAP_PROP_FPS);
    const auto frameDuration = std::chrono::nanoseconds(static_cast<int64_t>(1e9 / frameRate));

    auto frame = cv::Mat();

#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);

    const auto consoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD ret;
#endif _WIN32

    std::cout << "\x1b[?25l"; // Hide cursor

    int32_t previousColumns = -1;
    int32_t previousRows = -1;
    uint8_t *buffer = nullptr;

#ifdef _WIN32
    auto executableDir = std::filesystem::path(argv[0]).parent_path();
    _popen(std::format("{}\\ffplay.exe \"{}\" -nodisp -autoexit -loglevel quiet", executableDir.string(), argv[1]).c_str(), "r");
#endif _WIN32

    std::cout << "\x1b[2J";
    const auto beginPlayTime = std::chrono::high_resolution_clock::now();

    while (true) {
        auto beginFrameTime = std::chrono::high_resolution_clock::now();
#ifdef _WIN32
        CONSOLE_SCREEN_BUFFER_INFO consoleScreenBufferInfo;
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &consoleScreenBufferInfo);
        const int32_t columns = consoleScreenBufferInfo.srWindow.Right - consoleScreenBufferInfo.srWindow.Left + 1;
        const int32_t rows = consoleScreenBufferInfo.srWindow.Bottom - consoleScreenBufferInfo.srWindow.Top;
#endif _WIN32
#ifdef __unix__
        winsize windowSize{};
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &windowSize);
        const int32_t columns = windowSize.ws_col;
        const int32_t rows = windowSize.ws_row - 1;
#endif __unix__

        if (!capture.read(frame)) {
            break;
        }
        const auto framePosition =
            std::chrono::duration<int64_t, std::ratio<1, 1000000000>>(static_cast<int64_t>(capture.get(cv::CAP_PROP_POS_MSEC) * 1e6));
        if ((std::chrono::high_resolution_clock::now() - beginPlayTime) - framePosition > frameDuration) {
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
        if (previousColumns != columns || previousRows != rows) {
            delete[] buffer;
            buffer = new uint8_t[bufferSize];
            std::memset(buffer, ' ', bufferSize - 1);
            buffer[bufferSize - 1] = 0x0;
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

        imageToTextFull(frame, (columns - symbolWidth) / 2, buffer);
        auto endRenderTime = std::chrono::high_resolution_clock::now();

        auto beginPrintingTime = std::chrono::high_resolution_clock::now();
#ifdef _WIN32
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), {0, 0});
        WriteConsoleA(consoleOutput, buffer, bufferSize, &ret, nullptr);
#endif _WIN32
#ifdef __unix__
        std::cout << "\x1b[0;0H";
        std::fwrite(buffer, bufferSize, 1, stdout);
        std::fflush(stdout);
#endif __unix__
        auto endPrintingTime = std::chrono::high_resolution_clock::now();

        while (std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - beginPlayTime) -
                   framePosition <
               frameDuration) {
        }
        auto endFrameTime = std::chrono::high_resolution_clock::now();

        std::cout << "\x1b[0;0m";
        std::cout << "Render time: "
                  << std::format("{:10.3f}",
                                 std::chrono::duration_cast<std::chrono::nanoseconds>(endRenderTime - beginFrameTime).count() / 1e6)
                  << "ms "
                  << " Printing time: "
                  << std::format("{:10.3f}",
                                 std::chrono::duration_cast<std::chrono::nanoseconds>(endPrintingTime - beginPrintingTime).count() / 1e6)
                  << "ms "
                  << "Frame time: "
                  << std::format("{:10.3f}",
                                 std::chrono::duration_cast<std::chrono::nanoseconds>(endFrameTime - beginFrameTime).count() / 1e6)
                  << "ms "
                  << "Window size: "
                    << std::format("{:4d} {:4d}", symbolWidth, symbolHeight);
    }
    std::remove(TEMP_AUDIO_FILE_PATH);
    return EXIT_SUCCESS;
}
