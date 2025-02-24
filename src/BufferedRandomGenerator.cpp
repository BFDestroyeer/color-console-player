#include "BufferedRandomGenerator.hpp"

void BufferedRandomGenerator::init() {
    std::random_device device;
    std::mt19937 randomGenerator(device());
    std::uniform_real_distribution<float> distribution(0.0, 100.0);

    for (float& i: buffer) {
        i = distribution(randomGenerator);
    }
    position = 0;
}

float BufferedRandomGenerator::getRandom() {
    position = (position + 1) % BUFFER_SIZE != 0 ? position + 1 : 0;
    return buffer[position];
}
