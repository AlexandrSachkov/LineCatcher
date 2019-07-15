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

#include "IndexReaderI.h"
#include "FileLock.h"

#include <string>
#include <memory>
#include <tuple>

namespace PLP {
    class PagedReader;
    class IndexReader : public IndexReaderI {
    public:
        IndexReader();
        ~IndexReader();

        //C++ interface
        bool initialize(
            const std::wstring& path, 
            unsigned long long preferredBufferSizeBytes
        );
        unsigned long long getLineFileOffset() const override;
        bool getResult(unsigned long long number, unsigned long long& lineNumber) override;
        bool nextResult(unsigned long long& lineNumber) override;

        //Lua interface
        std::tuple<bool, unsigned long long> getResult(unsigned long long number);
        std::tuple<bool, unsigned long long> nextResult();

        //Shared interface
        unsigned long long getLineNumber() const override;
        unsigned long long getNumResults() const override;
        unsigned long long getResultNumber() const override;
        const wchar_t* getFilePath() const override;
        const char* getDataFilePath() const override;
        void restart() override;
        void release() override;

    private:
        std::unique_ptr<PagedReader> _reader = nullptr;
        std::wstring _path;
        std::string _dataFilePath;
        unsigned long long _numResults = 0;
        unsigned long long _resultCount = 0;
        unsigned long long _currPageOffset = 0;
        unsigned long long _currLineNum = 0;
        unsigned long long _currLineFileOffset = 0;
        unsigned long long _fileOffset = 0;

        char* _pageData = nullptr;
        unsigned long long _pageSize = 0;
        FileScopedLock _readingLock;
    };
}
