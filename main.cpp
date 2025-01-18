#include <csignal>
#include <filesystem>

#include <Windows.h>

#include <opencv2/opencv.hpp>

#include "color.hpp"

#pragma comment(lib, "winmm.lib")

constexpr auto FULL_SYMBOL_SIZE = 41;
constexpr char TEMP_AUDIO_FILE_PATH[] = "~temp_audio.wav";

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
    uint8_t* buffer
) {
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

            // \x1b[38;2;<R>;<G>;<B>m\x1b[48;2;<R>;<G>;<B>m<SYMBOL>
            std::memcpy(
                buffer + (y / 2) * (horizontalOffset + (image.cols / 2) * FULL_SYMBOL_SIZE + 7) + horizontalOffset + (x / 2) * FULL_SYMBOL_SIZE,
                "\x1b[38;2;",
                7
            );
            std::memcpy(
                buffer + (y / 2) * (horizontalOffset + (image.cols / 2) * FULL_SYMBOL_SIZE + 7) + horizontalOffset + (x / 2) * FULL_SYMBOL_SIZE + 7,
                colorToText(foreground[2]),
                3
            );
            std::memcpy(
                buffer + (y / 2) * (horizontalOffset + (image.cols / 2) * FULL_SYMBOL_SIZE + 7) + horizontalOffset + (x / 2) * FULL_SYMBOL_SIZE + 10,
                ";",
                1
            );
            std::memcpy(
                buffer + (y / 2) * (horizontalOffset + (image.cols / 2) * FULL_SYMBOL_SIZE + 7) + horizontalOffset + (x / 2) * FULL_SYMBOL_SIZE + 11,
                colorToText(foreground[1]),
                3
            );
            std::memcpy(
                buffer + (y / 2) * (horizontalOffset + (image.cols / 2) * FULL_SYMBOL_SIZE + 7) + horizontalOffset + (x / 2) * FULL_SYMBOL_SIZE + 14,
                ";",
                1
            );
            std::memcpy(
                buffer + (y / 2) * (horizontalOffset + (image.cols / 2) * FULL_SYMBOL_SIZE + 7) + horizontalOffset + (x / 2) * FULL_SYMBOL_SIZE + 15,
                colorToText(foreground[0]),
                3
            );
            std::memcpy(
                buffer + (y / 2) * (horizontalOffset + (image.cols / 2) * FULL_SYMBOL_SIZE + 7) + horizontalOffset + (x / 2) * FULL_SYMBOL_SIZE + 18,
                "m\x1b[48;2;",
                8
            );
            std::memcpy(
                buffer + (y / 2) * (horizontalOffset + (image.cols / 2) * FULL_SYMBOL_SIZE + 7) + horizontalOffset + (x / 2) * FULL_SYMBOL_SIZE + 26,
                colorToText(background[2]),
                3
            );
            std::memcpy(
                buffer + (y / 2) * (horizontalOffset + (image.cols / 2) * FULL_SYMBOL_SIZE + 7) + horizontalOffset + (x / 2) * FULL_SYMBOL_SIZE + 29,
                ";",
                1
            );
            std::memcpy(
                buffer + (y / 2) * (horizontalOffset + (image.cols / 2) * FULL_SYMBOL_SIZE + 7) + horizontalOffset + (x / 2) * FULL_SYMBOL_SIZE + 30,
                colorToText(background[1]),
                3
            );
            std::memcpy(
                buffer + (y / 2) * (horizontalOffset + (image.cols / 2) * FULL_SYMBOL_SIZE + 7) + horizontalOffset + (x / 2) * FULL_SYMBOL_SIZE + 33,
                ";",
                1
            );
            std::memcpy(
                buffer + (y / 2) * (horizontalOffset + (image.cols / 2) * FULL_SYMBOL_SIZE + 7) + horizontalOffset + (x / 2) * FULL_SYMBOL_SIZE + 34,
                colorToText(background[0]),
                3
            );
            std::memcpy(
                buffer + (y / 2) * (horizontalOffset + (image.cols / 2) * FULL_SYMBOL_SIZE + 7) + horizontalOffset + (x / 2) * FULL_SYMBOL_SIZE + 37,
                "m",
                1
            );
            std::memcpy(
                buffer + (y / 2) * (horizontalOffset + (image.cols / 2) * FULL_SYMBOL_SIZE + 7) + horizontalOffset + (x / 2) * FULL_SYMBOL_SIZE + 38,
                symbolByConvolution(convolution),
                3
            );
        }

        // Reset color mode and end line
        std::memcpy(
            buffer + (y / 2) * (horizontalOffset + (image.cols / 2) * FULL_SYMBOL_SIZE + 7) + horizontalOffset + (image.cols / 2) * FULL_SYMBOL_SIZE,
            "\x1b[0;0m\n",
            7
        );
    }
}

void exitSignalHandler(int){
    std::remove(TEMP_AUDIO_FILE_PATH);
    std::exit(EXIT_SUCCESS);
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

    SetConsoleOutputCP(CP_UTF8);

    std::cout << "\x1b[?25l"; // Hide cursor

    const auto consoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD ret;

    int32_t previousColumns = -1;
    int32_t previousRows = -1;
    uint8_t* buffer = nullptr;

    signal(SIGINT, exitSignalHandler);
    signal(SIGTERM, exitSignalHandler);

    std::system(std::format("ffmpeg -i \"{}\" {}", argv[1], TEMP_AUDIO_FILE_PATH).c_str());
    std::cout << "\x1b[2J";
    PlaySound(TEMP_AUDIO_FILE_PATH, nullptr, SND_FILENAME | SND_ASYNC);
    const std::chrono::time_point<std::chrono::steady_clock> beginPlayTime = std::chrono::high_resolution_clock::now();

    while (true) {
        auto beginFrameTime = std::chrono::high_resolution_clock::now();
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
        const int32_t columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        const int32_t rows = csbi.srWindow.Bottom - csbi.srWindow.Top;

        if (!capture.read(frame)) {
            break;
        }
        const auto framePosition = std::chrono::duration<long long, std::ratio<1, 1000000000>>(static_cast<long long>(capture.get(cv::CAP_PROP_POS_MSEC) * 1e6));
        if ((std::chrono::high_resolution_clock::now() - beginPlayTime) - framePosition > frameDuration) {
            continue;
        }

        const double screenHeight = rows * 33;
        const double screenWidth = columns * 16;
        const double frameAspectRatio = static_cast<double>(frame.rows) / static_cast<double>(frame.cols) ;
        int symbolHeight, symbolWidth;
        if (screenHeight / screenWidth > frameAspectRatio) {
            symbolHeight = static_cast<int>(columns * frameAspectRatio * (16.0 / 33.0));
            symbolWidth = columns;
        }
        else {
            symbolHeight = rows;
            symbolWidth = static_cast<int>(rows / frameAspectRatio * (33.0 / 16.0));
        }

        const int32_t bufferSize = ((columns - symbolWidth) / 2 + FULL_SYMBOL_SIZE * symbolWidth + 7) * symbolHeight + 1;
        if (previousColumns != columns || previousRows != rows) {
            delete[] buffer;
            buffer = new uint8_t[bufferSize];
            std::memset(buffer, ' ', bufferSize - 1);
            buffer[bufferSize - 1] = 0x0;
        }
        previousColumns = columns;
        previousRows = rows;

        cv::resize(frame, frame, cv::Size(symbolWidth * 2, symbolHeight * 2));

        imageToTextFull(frame, (columns - symbolWidth) / 2, buffer);
        auto endRenderTime = std::chrono::high_resolution_clock::now();

        auto beginPrintingTime = std::chrono::high_resolution_clock::now();
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), {0, 0});
        WriteConsoleA(consoleOutput, buffer, bufferSize, &ret, nullptr);
        auto endPrintingTime = std::chrono::high_resolution_clock::now();

        while (std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - beginPlayTime) - framePosition < frameDuration) {}
        auto endFrameTime = std::chrono::steady_clock::now();

        std::cout << "\x1b[0;0m";
        std::cout << "Render time: " << std::format("{:10.3f}", std::chrono::duration_cast<std::chrono::nanoseconds>(endRenderTime - beginFrameTime).count() / 1e6) << "ms "
        << " Printing time: " << std::format("{:10.3f}", std::chrono::duration_cast<std::chrono::nanoseconds>(endPrintingTime - beginPrintingTime).count() / 1e6) << "ms "
        << "Frame time: " << std::format("{:10.3f}", std::chrono::duration_cast<std::chrono::nanoseconds>(endFrameTime - beginFrameTime).count() / 1e6) << "ms";
    }
    std::remove(TEMP_AUDIO_FILE_PATH);
    return EXIT_SUCCESS;
}