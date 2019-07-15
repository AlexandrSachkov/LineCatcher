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

#include "FileWriterI.h"
#include "FileLock.h"

#include <string>
#include <vector>
#include <memory>

namespace PLP {
    class PagedWriter;
    class TaskRunner;

    class FileWriter : public FileWriterI {
    public:
        FileWriter();
        ~FileWriter();

        bool initialize(
            const std::wstring& path,
            unsigned long long preferredBuffSizeBytes,
            bool overwriteIfExists,
            TaskRunner& asyncTaskRunner
        );
        
        //C++ interface
        bool append(const char* data, unsigned long long size) override;
        bool appendLine(const char* data, unsigned long long size) override;

        //Shared interface
        bool append(const std::string& data) override;
        bool appendLine(const std::string& line) override;
        void release() override;

    private:
        std::unique_ptr<PagedWriter> _writer;
        FileScopedLock _writingLock;
    };
}