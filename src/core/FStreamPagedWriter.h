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

#include "PagedWriter.h"
#include "TaskRunner.h"
#include <string>
#include <vector>
#include <fstream>
#include <functional>

namespace PLP {
    class FStreamPagedWriter : public PagedWriter {
    public:
        FStreamPagedWriter();
        ~FStreamPagedWriter();

        bool initialize(
            const std::wstring& path, 
            unsigned long long preferredBuffSize, 
            bool overwriteIfExists, 
            TaskRunner& asyncTaskRunner
        );
        bool write(const char* data, unsigned long long size);
        bool setPosition(unsigned long long fileOffset);
        bool setPositionEnd();
        bool flush();
    private:
        void swapBuffers();

        std::vector<char> _buffer;
        TaskRunner* _asyncTaskRunner = nullptr;
        std::fstream _ofs;

        unsigned long long _pageSizeBytes = 0;
        char* _frontBuff = nullptr;
        char* _backBuff = nullptr;
        unsigned long long _frontBuffContentSize = 0;

        TaskStatus _backBuffLoadStatus;
        std::function<void()> _writeBackBuff;
        bool _writeError = false;
    };
}