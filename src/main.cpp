#include <filesystem>

#include "FrameWriter.hpp"
#include "TextFrameBuffer.hpp"

#include <opencv2/opencv.hpp>

#include "ConsoleWindowSizeService.hpp"
#include "FrameRenderer.hpp"


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
    const auto executableDir = std::filesystem::path(argv[0]).parent_path();
    _popen(
        std::format("{}\\ffplay.exe \"{}\" -nodisp -autoexit -loglevel quiet", executableDir.string(), argv[1]).c_str(),
        "r"
    );
#endif
#if defined(__unix__) || defined(__APPLE__)
    popen(std::format("ffplay \"{}\" -nodisp -autoexit -loglevel quiet", argv[1]).c_str(), "r");
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
        std::make_shared<cv::VideoCapture>(argv[1])
    );

    frameRenderer->start();

    return EXIT_SUCCESS;
}
