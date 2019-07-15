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
#include "LineReader.h"
#include "PagedReader.h"
#include "ReturnType.h"

#include <vector>
#include <atomic>
#include <functional>

namespace PLP {
    class IndexedLineReader : public LineReader {
    public:
        IndexedLineReader();
        ~IndexedLineReader();

        bool initialize(
            PagedReader& pagedReader, 
            unsigned int maxLineSize, 
            const std::atomic<bool>& cancelled,
            const std::function<void(int percent)>* progressUpdate
        );
        LineReaderResult getLine(unsigned long long lineNumber, char*& data, unsigned int& size);
        unsigned long long getNumberOfLines();
    private:
        std::wstring getIndexFilePath(const std::wstring& dataFilePath);
        bool loadIndex(const std::wstring& indexPath);
        bool generateIndex(
            const std::wstring& dataFilePath, 
            const std::wstring& indexPath,
            const std::atomic<bool>& cancelled,
            const unsigned long long fileSize,
            const std::function<void(int percent)>* progressUpdate
        );

        struct IndexHeader {
            std::string filePath;
            unsigned long long numLines = 0;
            unsigned int lineIndexFreq = 0;

            template<class Archive>
            void serialize(Archive& archive) {
                archive(filePath, numLines, lineIndexFreq);
            }
        };

        static const unsigned int INDEX_VERSION = 1; // increment if format changes
        static const unsigned int LINE_INDEX_FREQUENCY = 1000;
        static const unsigned long long ESTIMATED_NUM_CHARS_PER_LINE = 100;

        std::vector<unsigned long long> _fileIndex;
        IndexHeader _indexHeader;
    };
}
