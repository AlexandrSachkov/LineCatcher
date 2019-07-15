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

#include "Utils.h"

namespace PLP {
    const char* findLastLineEnding(const char* buff, unsigned long long buffSize, const char* currPos) {
        if (currPos >= buff + buffSize) {
            return nullptr;
        }

        char* pos = const_cast<char*>(currPos);
        while (pos > buff && *pos != '\n') {
            pos--;
        }

        if (*pos != '\n' && pos == buff) {
            return nullptr;
        }

        return pos;
    }

    LineReaderResult findNextLineEnding(
        const char* buff, 
        unsigned long long buffSize, 
        unsigned long long startOffsetBytes, 
        unsigned int maxLineSize,
        char*& lineEnding
    ) {
        lineEnding = nullptr;
        if (startOffsetBytes >= buffSize) {
            return LineReaderResult::ERROR;
        }

        char* pos = const_cast<char*>(buff) + startOffsetBytes;
        while ((buff - pos  + 1) < maxLineSize 
            && pos < buff + buffSize 
            && *pos != '\n'
            ) {
            pos++;
        }

        if (buff - pos + 1 >= maxLineSize) {
            return LineReaderResult::ERROR;
        }

        if (pos == buff + buffSize) {
            return LineReaderResult::NOT_FOUND;
        }

        lineEnding = pos;
        return LineReaderResult::SUCCESS;
    }
}