#pragma once

#include "TextFrameBuffer.hpp"

class TextWriter {
private:
     TextFrameBuffer* textFrameBuffer;

public:
    TextWriter(TextFrameBuffer* textFrameBuffer);
};