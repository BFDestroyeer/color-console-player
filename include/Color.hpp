#pragma once

template<typename T>
class Color {
private:
    T data[3]{};

public:
    Color() = default;

    explicit Color(T* data) {
        this->data[0] = data[0];
        this->data[1] = data[1];
        this->data[2] = data[2];
    }

    Color(T value1, T value2, T value3) {
        this->data[0] = value1;
        this->data[1] = value2;
        this->data[2] = value3;
    }

    Color& operator=(const Color<uint8_t>& color) {
        this->data[0] = color.data[0];
        this->data[1] = color.data[1];
        this->data[2] = color.data[2];
        return *this;
    }

    Color operator-(const Color<uint8_t>& color) const {
        return Color(this->data[0] - color.data[0], this->data[1] - color.data[1], this->data[2] - color.data[2]);
    }

    Color& operator+=(const Color<uint8_t>& color) {
        this->data[0] += color.data[0];
        this->data[1] += color.data[1];
        this->data[2] += color.data[2];
        return *this;
    }

    Color& operator*=(const double value) {
        data[0] *= value;
        data[1] *= value;
        data[2] *= value;
        return *this;
    }

    const T& operator[](int i) const {
        return data[i];
    }

    [[nodiscard]]
    double squareNorm() const {
        return static_cast<double>(data[0]) * static_cast<double>(data[0])
        + static_cast<double>(data[1]) * static_cast<double>(data[1])
        + static_cast<double>(data[2]) * static_cast<double>(data[2]);
    }

    friend class Color<uint8_t>;
    friend class Color<int16_t>;
};