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