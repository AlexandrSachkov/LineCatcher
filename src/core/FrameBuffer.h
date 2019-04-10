#pragma once

#include "CircularLineBuffer.h"

namespace PLP {
    class FrameBuffer {
    public:
        bool initialize(int startIndex, int endIndex, unsigned int lineSize);
        bool pushBack(unsigned long long lineNum, unsigned long long fileOffset, const char* data, unsigned int size);
        bool get(int index, unsigned long long& lineNum, unsigned long long& fileOffset, char*& data, unsigned int& size);
        unsigned int getSize();
    private:
        CircularLineBuffer _buffer;
        int startIndex = 0;
        int endIndex = 0;
    };
}