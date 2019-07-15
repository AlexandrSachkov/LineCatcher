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

#include "FileReaderI.h"
#include "IndexReaderI.h"
#include "FrameBuffer.h"

#include <functional>

namespace PLP {
    class LineScanner {
    public:
        LineScanner(FileReaderI* fileReader, IndexReaderI* indexReader, unsigned long long startLine, unsigned long long endLine);
        bool initialize();
        LineReaderResult nextLine(unsigned long long& lineNum, unsigned long long& fileOffset, char*& data, unsigned int& size);
        std::tuple<int, unsigned long long, std::string> nextLine();
    private:
        FileReaderI* _fileReader = nullptr;
        IndexReaderI* _indexReader = nullptr;
        unsigned long long _startLine = 0;
        unsigned long long _endLine = 0;

        bool _firstLine = true;
        std::function<LineReaderResult(unsigned long long& lineNum, unsigned long long& fileOffset, char*& data, unsigned int& size)> _nextLine;
    };

    class MultilineScanner {
    public:
        MultilineScanner(
            FileReaderI* fileReader, 
            IndexReaderI* indexReader, 
            int startLineOffset, 
            int endLineOffset,
            unsigned long long startLine, 
            unsigned long long endLine
        );

        bool initialize();
        LineReaderResult nextFrame();
        bool getLine(int index, unsigned long long& lineNum, unsigned long long& fileOffset, char*& data, unsigned int& size);
        std::tuple<bool, unsigned long long, std::string> getLine(int index);
    private:
        LineReaderResult loadFrame(unsigned long long refLine);
        FileReaderI* _fileReader = nullptr;
        IndexReaderI* _indexReader = nullptr;
        int _startLineOffset = 0;
        int _endLineOffset = 1;
        unsigned long long _startLine = 0;
        unsigned long long _endLine = 0;

        bool _firstLine = true;
        std::function<LineReaderResult()> _nextFrame;
        FrameBuffer _frameBuff;
    };
}