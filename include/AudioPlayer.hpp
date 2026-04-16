#pragma once

#include <string>

class AudioPlayer {
    public:

    explicit AudioPlayer(const std::string& mediaFilePath);

    void play();
};