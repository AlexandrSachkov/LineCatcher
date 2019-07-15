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