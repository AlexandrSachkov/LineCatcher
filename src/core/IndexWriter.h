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

#include "IndexWriterI.h"
#include "FileLock.h"

#include <string>
#include <memory>
#include <sstream>

namespace PLP {
    class PagedWriter;
    class TaskRunner;
    class FileReader;

    class IndexWriter : public IndexWriterI {
    public:
        IndexWriter();
        ~IndexWriter();

        bool initialize(
            const std::wstring& path,
            const std::wstring& dataFilePath,
            unsigned long long preferredBufferSizeBytes,
            bool overwriteIfExists,
            TaskRunner& asyncTaskRunner
        );

        //C++ interface
        bool appendCurrLine(const FileReaderI* fReader) override;
        bool appendCurrLine(unsigned long long lineNumber, unsigned long long fileOffset) override;

        //Lua interface
        bool appendCurrLine(const std::shared_ptr<FileReader> fReader);

        //Shared interface
        unsigned long long getNumResults() const override;
        void release() override;

    private:
        bool flush();
        bool updateResultCount();

        std::unique_ptr<PagedWriter> _writer = nullptr;
        std::string _dataFilePath;
        
        unsigned long long _prevLineNum = 0;
        unsigned long long _resultCount = 0;

        FileScopedLock _writingLock;
    };
}
