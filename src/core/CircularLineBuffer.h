#pragma once

#include "LineBuffer.h"

namespace PLP {
    class CircularLineBuffer {
    public:
        CircularLineBuffer();
        bool initialize(unsigned int lineSize, unsigned int numLines);
        bool pushBack(const char* data, unsigned int size);
        bool pushBack(unsigned long long lineNum, unsigned long long fileOffset, const char* data, unsigned int size);
        bool get(unsigned int linePos, char*& data, unsigned int& size);
        bool get(unsigned int linePos, unsigned long long& lineNum, unsigned long long& fileOffset, char*& data, unsigned int& size);
        unsigned int getSize();
    private:
        std::vector<LineBuffer> _lineBuffers;
        std::vector<std::pair<unsigned long long, unsigned long long>> _lineNumFileOffset;
        unsigned int _startPos = 0;
        unsigned int _currPos = 0;
    };
}