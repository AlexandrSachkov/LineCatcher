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
#include "FileLock.h"

#include <string>
#include <tuple>
#include <atomic>
#include <functional>

namespace PLP {
    class PagedReader;
    class IndexedLineReader;
    class IndexReader;

    class FileReader : public FileReaderI{
    public:
        FileReader();
        ~FileReader();

        bool initialize(
            const std::wstring& path, 
            unsigned long long preferredBuffSizeBytes,
            const std::atomic<bool>& cancelled,
            const std::function<void(int percent)>* progressUpdate
        );
        
        //C++ interface
        LineReaderResult nextLine(char*& lineStart, unsigned int& length) override;
        LineReaderResult getLine(unsigned long long lineNumber, char*& data, unsigned int& size) override;
        LineReaderResult getLineFromResult(const IndexReaderI* rsReader, char*& data, unsigned int& size) override;
        unsigned long long getLineFileOffset() const override;
        const wchar_t* getFilePath() const override;

        //Lua interface
        std::tuple<int, std::string> nextLine();
        std::tuple<int, std::string> getLine(unsigned long long lineNumber);
        std::tuple<int, std::string> getLineFromResult(const std::shared_ptr<IndexReader> rsReader);

        //Shared interface
        unsigned long long getLineNumber() const override;
        unsigned long long getNumberOfLines() const override;
        void restart() override;
        void release() override;

    private:
        std::unique_ptr<PagedReader> _pager = nullptr;
        std::unique_ptr<IndexedLineReader> _lineReader = nullptr;
        FileScopedLock _readingLock;
    };
}