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

    bool FrameBuffer::get(int index, unsigned long long& lineNum, unsigned long long& fileOffset, char*& data, unsigned int& size) {
        if (index < startIndex || index > endIndex) {
            return false;
        }

        return _buffer.get(index - startIndex, lineNum, fileOffset, data, size);
    }

    bool FrameBuffer::pushBack(unsigned long long lineNum, unsigned long long fileOffset, const char* data, unsigned int size) {
        return _buffer.pushBack(lineNum, fileOffset, data, size);
    }

    unsigned int FrameBuffer::getSize() {
        return _buffer.getSize();
    }
}