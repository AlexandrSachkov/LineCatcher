#pragma once

#include "LineBuffer.h"

namespace PLP {
    class CircularLineBuffer {
    public:
        CircularLineBuffer();
        bool initialize(unsigned int lineSize, unsigned int numLines);
        bool pushBack(const char* data, unsigned int size);
        bool get(unsigned int linePos, char*& data, unsigned int& size);
        unsigned int getNumLines();
    private:
        std::vector<LineBuffer> _lineBuffers;
        unsigned int _startPos = 0;
        unsigned int _currPos = 0;
    };
}