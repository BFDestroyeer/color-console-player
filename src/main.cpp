#include <filesystem>

#include "FrameWriter.hpp"
#include "TextFrameBuffer.hpp"

#include <opencv2/opencv.hpp>

#include "ConsoleSizeRecorder.hpp"
#include "FrameRenderer.hpp"

// 16 x 33 font size
// 236 x 63 symbol resolution
// 3776 x 2079 effective resolution
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

    const auto consoleSizeRecorder = std::make_shared<ConsoleSizeRecorder>();
    const auto textFrameBuffer = std::make_shared<TextFrameBuffer>(0);
    const auto frameWriter = std::make_shared<FrameWriter>(
        beginPlayTime,
        textFrameBuffer
    );
    const auto frameRenderer = std::make_shared<FrameRenderer>(
        beginPlayTime,
        consoleSizeRecorder,
        textFrameBuffer,
        std::make_shared<cv::VideoCapture>(argv[1])
    );

    frameRenderer->start();

    return EXIT_SUCCESS;
}
