/*
 * This file is part of the Line Catcher distribution (https://github.com/AlexandrSachkov/LineCatcher).
 * Copyright (c) 2019 Alexandr Sachkov.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "LineBuffer.h"

namespace PLP {
    LineBuffer::LineBuffer(unsigned int size) {
        _data.resize(size + 1);
        _data[0] = NULL;
        _size = 0;
    }

    bool LineBuffer::set(const char* data, unsigned int size) {
        if (size > _data.size() - 1) {
            return false;
        }

        memcpy(_data.data(), data, size);
        _data[size] = NULL;
        _size = size;
        return true;
    }

    bool LineBuffer::append(const char* data, unsigned int size) {
        if (_size + size > _data.size() - 1) {
            return false;
        }

        memcpy(_data.data() + _size, data, size);
        _size += size;
        _data[_size] = NULL;
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
        _data[0] = NULL;
        _size = 0;
    }
}