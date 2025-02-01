#pragma once

#include <random>

class BufferedRandomGenerator {
  private:
    static constexpr size_t BUFFER_SIZE = 709;

    inline static size_t position = 0;
    inline static float buffer[BUFFER_SIZE];

    static BufferedRandomGenerator instance;

  public:
    BufferedRandomGenerator() = delete;

    static void init();

    static float getRandom();
};