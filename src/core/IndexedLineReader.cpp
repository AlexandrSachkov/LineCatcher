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

#include "IndexedLineReader.h"
#include "Utils.h"
#include "Core.h"
#include "Logger.h"
#include "GenFileTracker.h"

#include "cereal/types/vector.hpp"
#include "cereal/types/string.hpp"
#include "cereal/archives/binary.hpp"

#include <fstream>

namespace PLP {
    IndexedLineReader::IndexedLineReader() {}
    IndexedLineReader::~IndexedLineReader() {}

    bool IndexedLineReader::initialize(
        PagedReader& pagedReader, 
        unsigned int maxLineSize, 
        const std::atomic<bool>& cancelled, 
        const std::function<void(int percent)>* progressUpdate
    ) {
        if (!LineReader::initialize(pagedReader, maxLineSize)) {
            return false;
        }

        std::wstring indexPath = getIndexFilePath(pagedReader.getFilePath());
        (*progressUpdate)(0);
        if (!loadIndex(indexPath)) {
            if (!generateIndex(pagedReader.getFilePath(), indexPath, cancelled, pagedReader.getFileSize(), progressUpdate)) {
                return false;
            }
        } else {
            (*progressUpdate)(100);
        }

        return true;
    }

    std::wstring IndexedLineReader::getIndexFilePath(const std::wstring& dataFilePath) {
        std::wstring directory = getFileDirectory(dataFilePath);
        std::wstring fileNameNoExt = getFileNameNoExt(dataFilePath);
        return directory + fileNameNoExt + string_to_wstring(FILE_RANDOM_ACCESS_INDEX_EXTENSION);
    }

    bool IndexedLineReader::loadIndex(const std::wstring& indexPath) {
        std::ifstream fs;
        fs.open(indexPath, std::fstream::in | std::fstream::binary);
        if (!fs.good()) {
            return false;
        }

        try {
            cereal::BinaryInputArchive iarchive(fs);

            unsigned int indexVersion = 0;
            iarchive(indexVersion);
            if (indexVersion != INDEX_VERSION) { // might have a different format. TODO: can we handle legacy formats?
                return false;
            }

            iarchive(_indexHeader, _fileIndex);
        } catch (const cereal::Exception& e) {
            Logger::send(ERR, "Failed to load file random access index: " + std::string(e.what()));
            return false;
        }
        
        return true;
    }

    bool IndexedLineReader::generateIndex(
        const std::wstring& dataFilePath, 
        const std::wstring& indexPath, 
        const std::atomic<bool>& cancelled,
        const unsigned long long fileSize,
        const std::function<void(int percent)>* progressUpdate
    ) {
        char* lineStart;
        unsigned int length;
        unsigned long long lineStartFileOffset = 0;
        unsigned long long numLines = 0;

        const long double dBytesPerPercent = (fileSize) / 100.0;
        const unsigned long long numBytesPerProgressUpdate = dBytesPerPercent > 1.0 ? (unsigned long long)dBytesPerPercent : 1;
        const int percentPerProgressUpdate = dBytesPerPercent > 1.0 ? 1 : (int)(1.0 / dBytesPerPercent);

        unsigned long long numBytesTillProgressUpdate = numBytesPerProgressUpdate;
        int progressPercent = 0;

        LineReaderResult result;
        try {
            _fileIndex.reserve(fileSize / ESTIMATED_NUM_CHARS_PER_LINE);
            while ((result = nextLine(lineStart, length)) == LineReaderResult::SUCCESS) {
                if (getLineNumber() % 10000000 == 0 && cancelled) {
                    return false;
                }

                if (getLineNumber() % LINE_INDEX_FREQUENCY == 0) {
                    _fileIndex.push_back(lineStartFileOffset);
                }

                lineStartFileOffset = getCurrentFileOffset();
                numLines++;

                if (lineStartFileOffset > numBytesTillProgressUpdate) {
                    progressPercent += percentPerProgressUpdate;
                    numBytesTillProgressUpdate += numBytesPerProgressUpdate;
                    (*progressUpdate)(progressPercent);
                }
            }
        } catch (std::bad_alloc&) {
            Logger::send(ERR, "Failed to allocate enough space for index buffer");
            return false;
        }

        if (result == LineReaderResult::ERROR) {
            return false;
        }

        restart();

        std::ofstream fs;
        fs.open(indexPath, std::fstream::out | std::fstream::binary);
        if (!fs.good()) {
            Logger::send(ERR, "Failed to create index file: " + wstring_to_string(indexPath));
            return false;
        }

        LC::GenFileTracker::addFile(indexPath);

        _indexHeader.filePath = wstring_to_string(dataFilePath);
        _indexHeader.lineIndexFreq = LINE_INDEX_FREQUENCY;
        _indexHeader.numLines = numLines;

        try {
            cereal::BinaryOutputArchive oarchive(fs);
            oarchive(INDEX_VERSION, _indexHeader, _fileIndex);
        } catch (const cereal::Exception& e) {
            Logger::send(ERR, "Failed to write index file " + wstring_to_string(indexPath) + ": " + std::string(e.what()));
            return false;
        }

        return true;
    }

    LineReaderResult IndexedLineReader::getLine(unsigned long long lineNumber, char*& data, unsigned int& size) {
        if (lineNumber < 0 || lineNumber > _indexHeader.numLines - 1) {
            return LineReaderResult::NOT_FOUND;
        }

        char* lineData = nullptr;
        unsigned int length = 0;

        const unsigned long long prevIndexedLineNumIndex = lineNumber / _indexHeader.lineIndexFreq;
        const unsigned long long prevIndexedLineNum = prevIndexedLineNumIndex * _indexHeader.lineIndexFreq;

        if (lineNumber <= getLineNumber() // desired line is behind current
            || prevIndexedLineNum > getLineNumber() // random access will get us there faster than iterating
            ) {
            unsigned long long prevIndexedLineFileOffset;
            if (prevIndexedLineNum == 0) {
                prevIndexedLineFileOffset = 0;
            } else {
                if (prevIndexedLineNumIndex >= _fileIndex.size()) {
                    Logger::send(ERR, "Failed to find nearest known file location");
                    return LineReaderResult::ERROR;
                }
                prevIndexedLineFileOffset = _fileIndex[prevIndexedLineNumIndex];
            }

            LineReaderResult result = getLineUnverified(prevIndexedLineNum, prevIndexedLineFileOffset, lineData, length);
            if (result != LineReaderResult::SUCCESS) {
                return result;
            }
        }

        LineReaderResult result;
        while (getLineNumber() < lineNumber) {
            result = nextLine(lineData, length);
            if (result != LineReaderResult::SUCCESS) {
                return result;
            }
        }

        data = lineData;
        size = length;

        return LineReaderResult::SUCCESS;
    }

    unsigned long long IndexedLineReader::getNumberOfLines() {
        return _indexHeader.numLines;
    }
}