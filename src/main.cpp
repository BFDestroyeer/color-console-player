#include <filesystem>

#include "FrameWriter.hpp"
#include "TextFrameBuffer.hpp"

#include "AudioPlayer.hpp"
#include "ConsoleWindowSizeService.hpp"
#include "FrameRenderer.hpp"

#include <iostream>
#include <string>

#include "VideoCapture.hpp"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <path to file>" << std::endl;
        return EXIT_FAILURE;
    }
    if (!std::filesystem::exists(argv[1])) {
        std::cout << "File " << argv[1] << " does not exist" << std::endl;
        return EXIT_FAILURE;
    }

    const auto beginPlayTime = std::chrono::high_resolution_clock::now();

    const auto consoleWindowSizeService = std::make_shared<ConsoleWindowSizeService>();
    const auto textFrameBuffer = std::make_shared<TextFrameBuffer>(0);
    const auto frameWriter = std::make_shared<FrameWriter>(
        beginPlayTime,
        textFrameBuffer,
        consoleWindowSizeService
    );
    const auto frameRenderer = std::make_shared<FrameRenderer>(
        beginPlayTime,
        consoleWindowSizeService,
        textFrameBuffer,
        std::make_shared<VideoCapture>(argv[1])
    );
    const auto audioPlayer = std::make_shared<AudioPlayer>(argv[1]);

    audioPlayer->play();
    frameRenderer->start();

    return EXIT_SUCCESS;
}
