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

#include "PagedReader.h"

#include <string>

namespace PLP {
    class TaskRunner;

    class MemMappedPagedReader : public PagedReader {
    public:
        MemMappedPagedReader();
        ~MemMappedPagedReader();

        bool initialize(const std::wstring& path, unsigned long long preferredBuffSize = 0);
        const char* read(unsigned long long fileOffset, unsigned long long& size);
        unsigned long long getFileSize();
        const std::wstring& getFilePath();
    private:
        static const unsigned long long MAX_PAGE_SIZE_BYTES = 1073741824; //1 GB

        void* _fileHandle = nullptr;
        void* _fileMappingHandle = nullptr;
        void* _data = nullptr;

        std::wstring _filePath;
        unsigned long long _fileSize = 0;
        unsigned long long _allocGranularity = 0;
        unsigned long long _buffSize = 0;
    };
}