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

#include "FileReader.h"
#include "MemMappedPagedReader.h"
#include "PagedReader.h"
#include "LineReader.h"
#include "IndexedLineReader.h"
#include "Utils.h"
#include "IndexReader.h"
#include "FStreamPagedReader.h"
#include "Logger.h"

namespace PLP {
    FileReader::FileReader() {}
    FileReader::~FileReader() {
        Logger::send(INFO, "Releasing file reader");
        release();
    }

    bool FileReader::initialize(
        const std::wstring& path, 
        unsigned long long preferredBuffSizeBytes, 
        const std::atomic<bool>& cancelled,
        const std::function<void(int percent)>* progressUpdate
    ) {
        release();

        std::wstring unixPath = windowsToUnixPath(path);
        FileScopedLock readingLock = FileScopedLock::lockForReading(unixPath);
        if (!readingLock.isLocked()) {
            Logger::send(ERR, "Unable to acquare a read lock on file " + wstring_to_string(unixPath) + ". Release writers using the file");
            return false;
        }

        FStreamPagedReader* pagedReader = new FStreamPagedReader();
        _pager.reset(pagedReader);
        if (!pagedReader->initialize(unixPath, preferredBuffSizeBytes)) {
            return false;
        }

        IndexedLineReader* idxLineReader = new IndexedLineReader();
        _lineReader.reset(idxLineReader);
        if (!idxLineReader->initialize(*_pager, 100000, cancelled, progressUpdate)) {
            return false;
        }

        _readingLock = std::move(readingLock);
        return true;
    }

    void FileReader::release() {
        _lineReader = nullptr;
        _pager = nullptr;
    }

    LineReaderResult FileReader::nextLine(char*& lineStart, unsigned int& length) {
        return _lineReader->nextLine(lineStart, length);
    }

    std::tuple<int, std::string> FileReader::nextLine() {
        char* lineStart = nullptr;
        unsigned int length = 0;

        LineReaderResult result = _lineReader->nextLine(lineStart, length);
        if (result != LineReaderResult::SUCCESS) {
            return { result, std::string() };
        }
        return { result, std::string(lineStart, length) };
    }

    LineReaderResult FileReader::getLine(unsigned long long lineNumber, char*& data, unsigned int& size) {
        return _lineReader->getLine(lineNumber, data, size);
    }

    LineReaderResult FileReader::getLineFromResult(const IndexReaderI* rsReader, char*& data, unsigned int& size) {
        if (!rsReader) {
            return LineReaderResult::ERROR;
        }
        return _lineReader->getLineUnverified(rsReader->getLineNumber(), rsReader->getLineFileOffset(), data, size);
    }

    std::tuple<int, std::string> FileReader::getLineFromResult(const std::shared_ptr<IndexReader> rsReader) {
        char* lineStart = nullptr;
        unsigned int length = 0;

        LineReaderResult result = getLineFromResult(rsReader.get(), lineStart, length);
        if (result != LineReaderResult::SUCCESS) {
            return { result, std::string() };
        }
        return { result, std::string(lineStart, length) };
    }

    std::tuple<int, std::string> FileReader::getLine(unsigned long long lineNumber) {
        char* lineStart = nullptr;
        unsigned int length = 0;

        LineReaderResult result = getLine(lineNumber, lineStart, length);
        if (result != LineReaderResult::SUCCESS) {
            return { result, std::string() };
        }
        return { result, std::string(lineStart, length) };
    }

    unsigned long long FileReader::getLineNumber() const {
        return _lineReader->getLineNumber();
    }

    unsigned long long FileReader::getLineFileOffset() const {
        return _lineReader->getCurrentLineFileOffset();
    }

    const wchar_t* FileReader::getFilePath() const {
        return _pager->getFilePath().c_str();
    }

    void FileReader::restart() {
        _lineReader->restart();
    }

    unsigned long long FileReader::getNumberOfLines() const {
        return _lineReader->getNumberOfLines();
    }
}