#include <opencv2/opencv.hpp>

#include <Windows.h>

#include "color.h"

#pragma comment(lib, "winmm.lib")

constexpr auto SYMBOL_SIZE = 24;
constexpr auto FULL_SYMBOL_SIZE = 41;

constexpr auto FRAME_DURATION = std::chrono::nanoseconds(static_cast<int64_t>(1e9 / 23.98));

constexpr char SPACE[] = {0x1, 0x1, 0x20, 0x0};

int getColor(
    const cv::Vec3s &foreground,
    const cv::Vec3s &background,
    const cv::Vec3s &color
) {
    if (cv::norm(foreground - color) < cv::norm(background - color)) {
        return 1;
    }
    return 0;
}

void imageToTextFull(
    const cv::Mat&image,
    const uint64_t horizontalOffset,
    char* buffer
) {
#pragma omp parallel for
    for (int32_t y = 0; y < image.rows; y += 2) {
        for (int32_t x = 0; x < image.cols; x += 2) {
            const auto ul = image.at<cv::Vec3b>(y, x);
            const auto ur = image.at<cv::Vec3b>(y, x + 1);
            const auto bl = image.at<cv::Vec3b>(y + 1, x);
            const auto br = image.at<cv::Vec3b>(y + 1, x + 1);

            auto colors = getColors(ul, ur, bl, br);

            const cv::Vec3b foreground = colors.first; // foreground color
            const cv::Vec3b background = colors.second; //background color

            const auto convolution = (getColor(foreground, background, br) << 3) + (getColor(foreground, background, ur) << 2) + (getColor(foreground, background,bl) << 1) + getColor(foreground, background, br);

            std::memcpy(
                buffer + (y / 2) * (horizontalOffset + (image.cols / 2) * FULL_SYMBOL_SIZE + 1) + horizontalOffset + (x / 2) * FULL_SYMBOL_SIZE,
                std::format("\x1b[38;2;{:03d};{:03d};{:03d}m\x1b[48;2;{:03d};{:03d};{:03d}m{}",
                            foreground[2], foreground[1], foreground[0],
                            background[2], background[1], background[0],
                            symbolByConvolution(convolution)
                ).c_str(),
                FULL_SYMBOL_SIZE
            );
        }
        buffer[(y / 2) * (horizontalOffset + (image.cols / 2) * FULL_SYMBOL_SIZE + 1) + horizontalOffset + (image.cols / 2) * FULL_SYMBOL_SIZE] = '\n';
    }
}


// 16 x 33
// 236 x 63
// 3776 x 2079
int main() {
    auto capture = cv::VideoCapture("E:/Source/color-console-player/video.mp4");
    auto frame = cv::Mat();

    SetConsoleOutputCP(CP_UTF8);

    std::cout << "\x1b[?25l";

    auto consoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD ret;

    PlaySound("E:/Source/color-console-player/audio.wav", NULL, SND_FILENAME | SND_ASYNC);

    while (true) {
        auto beginTime = std::chrono::high_resolution_clock::now();
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
        const auto columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        const auto rows = csbi.srWindow.Bottom - csbi.srWindow.Top;

        if (!capture.read(frame)) {
            std::cout << "Cant read frame" << std::endl;
            break;
        }

        const double screenHeight = rows * 33;
        const double screenWidth = columns * 16;
        const double frameAspectRatio = static_cast<double>(frame.rows) / static_cast<double>(frame.cols) ;
        uint64_t symbolHeight, symbolWidth;
        if (screenHeight / screenWidth > frameAspectRatio) {
            symbolHeight = static_cast<uint64_t>(columns * frameAspectRatio * (16.0 / 33.0));
            symbolWidth = columns;
        }
        else {
            symbolHeight = rows;
            symbolWidth = static_cast<uint64_t>(rows / frameAspectRatio * (33.0 / 16.0));
        }

        auto bufferSize = ((columns - symbolWidth) / 2 + FULL_SYMBOL_SIZE * symbolWidth + 1) * symbolHeight + 1;
        auto buffer = new char[bufferSize];
        std::memset(buffer, ' ', bufferSize - 1);
        buffer[bufferSize - 1] = 0x0;

        cv::resize(frame, frame, cv::Size(symbolWidth * 2, symbolHeight * 2));

        imageToTextFull(frame, (columns - symbolWidth) / 2, buffer);
        auto endRenderTime = std::chrono::high_resolution_clock::now();

        auto beginPrintingTime = std::chrono::high_resolution_clock::now();
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), {0, 0});
        WriteConsoleA(consoleOutput, buffer, bufferSize, &ret, nullptr);
        auto endPrintingTime = std::chrono::high_resolution_clock::now();

        delete[] buffer;
        while (std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - beginTime) < FRAME_DURATION) {}
        auto endFrameTime = std::chrono::steady_clock::now();

        std::cout << "\x1b[0;0m";
        std::cout << "Render time: " << std::format("{:10.3f}", std::chrono::duration_cast<std::chrono::nanoseconds>(endRenderTime - beginTime).count() / 1e6) << "ms "
        << " Printing time: " << std::format("{:10.3f}", std::chrono::duration_cast<std::chrono::nanoseconds>(endPrintingTime - beginPrintingTime).count() / 1e6) << "ms "
        << "Frame time: " << std::format("{:10.3f}", std::chrono::duration_cast<std::chrono::nanoseconds>(endFrameTime - beginTime).count() / 1e6) << "ms";
    }
}