#include "FrameBuffer.h"

namespace PLP {
    bool FrameBuffer::initialize(int startIndex, int endIndex, unsigned int lineSize) {
        if (startIndex > endIndex || lineSize == 0) {
            return false;
        }

        this->startIndex = startIndex;
        this->endIndex = endIndex;

        const unsigned int numLines = endIndex - startIndex + 1;
        return _buffer.initialize(lineSize, numLines);
    }

    bool FrameBuffer::get(int index, char*& data, unsigned int& size) {
        if (index < startIndex || index > endIndex) {
            return false;
        }

        return _buffer.get(index - startIndex, data, size);
    }

    bool FrameBuffer::pushBack(const char* data, unsigned int size) {
        return _buffer.pushBack(data, size);
    }
}