#pragma once

#include "CircularLineBuffer.h"

namespace PLP {
    class FrameBuffer {
    public:
        bool initialize(int startIndex, int endIndex, unsigned int lineSize);
        bool pushBack(const char* data, unsigned int size);
        bool get(int index, char*& data, unsigned int& size);

    private:
        CircularLineBuffer _buffer;
        int startIndex = 0;
        int endIndex = 0;
    };
}