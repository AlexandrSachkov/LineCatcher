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

#include <string>
#include "ReturnType.h"

namespace PLP {
    class IndexReaderI;
    class FileReaderI {
    public:
        virtual ~FileReaderI() {}
        virtual LineReaderResult nextLine(char*& lineStart, unsigned int& length) = 0;
        virtual LineReaderResult getLine(unsigned long long lineNumber, char*& data, unsigned int& size) = 0;
        virtual LineReaderResult getLineFromResult(const IndexReaderI* rsReader, char*& data, unsigned int& size) = 0;
        virtual unsigned long long getLineFileOffset() const = 0;
        virtual const wchar_t* getFilePath() const = 0;
        virtual unsigned long long getLineNumber() const = 0;
        virtual unsigned long long getNumberOfLines() const = 0;
        virtual void restart() = 0;
        virtual void release() = 0;
    };
}
