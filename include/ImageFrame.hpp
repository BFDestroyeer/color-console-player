# pragma once
#include <cstdint>

#include "Color.hpp"

class ImageFrame {
    uint8_t* buffer;

    uint64_t width;

    uint64_t height;

    double position;

public:
    explicit ImageFrame(uint64_t width, uint64_t height);

    ~ImageFrame();

    Color<uint8_t> getColorAt(int y, int x) const;

    void resize(uint64_t width, uint64_t height);

     uint8_t* getBuffer() const;

     uint64_t getWidth() const;

     uint64_t getHeight() const;

    double getPosition() const;

     void setPosition(double position);
};
