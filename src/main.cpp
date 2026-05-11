#include <filesystem>

#include "FrameWriter.hpp"
#include "TextFrameBuffer.hpp"

#include "AudioPlayer.hpp"
#include "ConsoleWindowSizeService.hpp"
#include "FrameRenderer.hpp"

#include <csignal>
#include <iostream>

#include "VideoCapture.hpp"

void handleSigint([[maybe_unused]] int signal){
    // Reset all styles
    std::cout << "\x1b[0m" << std::endl;
    exit(1);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <path to file>" << std::endl;
        return EXIT_FAILURE;
    }
    if (!std::filesystem::exists(argv[1])) {
        std::cout << "File " << argv[1] << " does not exist" << std::endl;
        return EXIT_FAILURE;
    }

#ifdef _WIN32
    std::signal(SIGINT, handleSigint);
#endif
#if defined(__unix__) || defined(__APPLE__)
    struct sigaction sigIntHandler{};
    sigIntHandler.sa_handler = handleSigint;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, nullptr);
#endif

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
