#include "CircularLineBuffer.h"

namespace PLP {
    CircularLineBuffer::CircularLineBuffer() {}

    bool CircularLineBuffer::initialize(unsigned int lineSize, unsigned int numLines) {
        try {
            for (unsigned int i = 0; i < numLines; i++) {
                _lineBuffers.emplace_back(lineSize);
            }
        } catch (std::bad_alloc&) {
            return false;
        }

        return true;
    }

    bool CircularLineBuffer::pushBack(const char* data, unsigned int size) {
        if (!_lineBuffers[_currPos].set(data, size)) {
            return false;
        }

        if (_currPos == _startPos) {
            _startPos = (_startPos + 1) % _lineBuffers.size();
        }

        _currPos = (_currPos + 1) % _lineBuffers.size();
        return true;
    }

    bool CircularLineBuffer::get(unsigned int linePos, char*& data, unsigned int& size) {
        if (linePos >= _lineBuffers.size()) {
            return false;
        }

        _lineBuffers[(_startPos + linePos) % _lineBuffers.size()].get(data, size);
        return true;
    }
}