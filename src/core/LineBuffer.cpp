#include "LineBuffer.h"

namespace PLP {
    LineBuffer::LineBuffer(unsigned int size) {
        _data.resize(size);
        _size = 0;
    }

    bool LineBuffer::set(const char* data, unsigned int size) {
        if (size > _data.size()) {
            return false;
        }

        memcpy(_data.data(), data, size);
        _size = size;
        return true;
    }

    bool LineBuffer::append(const char* data, unsigned int size) {
        if (_size + size > _data.size()) {
            return false;
        }

        memcpy(_data.data() + _size, data, size);
        _size += size;
        return true;
    }

    void LineBuffer::get(char*& data, unsigned int& size) {
        data = _data.data();
        size = _size;
    }

    unsigned int LineBuffer::getSize() {
        return _size;
    }

    void LineBuffer::clear() {
        _size = 0;
    }
}