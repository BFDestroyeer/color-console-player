# pragma once
#include <cstdint>

class ImageFrame {
    uint8_t* buffer;

    uint64_t width;

    uint64_t height;

    double position;

public:
    explicit ImageFrame(uint64_t width, uint64_t height);

    ~ImageFrame();

    void resize(uint64_t width, uint64_t height);

    inline uint8_t* getBuffer() const;

    inline uint64_t getWidth() const;

    inline uint64_t getHeight() const;

    inline void setPosition(double position);
};
