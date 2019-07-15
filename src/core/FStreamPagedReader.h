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
#include "TaskRunner.h"
#include "Timer.h"

#include <string>
#include <vector>
#include <fstream>
#include <functional>

namespace PLP {
    class TaskRunner;

    class FStreamPagedReader : public PagedReader {
    public:
        FStreamPagedReader();
        ~FStreamPagedReader();

        bool initialize(const std::wstring& path, unsigned long long preferredBuffSize/*, TaskRunner& asyncTaskRunner*/);
        const char* read(unsigned long long fileOffset, unsigned long long& size) override;
        unsigned long long getFileSize() override;
        const std::wstring& getFilePath() override;
        
        /*const char* getNextPage(unsigned long long& size);
        void resetToBeginning();
        unsigned long long getCurrentPageFileOffset();*/
    private:
        
        std::wstring _filePath;
        std::vector<char> _buffer;
        TaskRunner* _asyncTaskRunner;
        
        std::ifstream _ifs;
        unsigned long long _fileSize;

        /*struct LoadingData {
            unsigned long long fileOffsetBytes = 0;
            unsigned long long buffStartOffsetBytes = 0;
            unsigned long long buffSizeBytes = 0;
            bool backtracked = false;
            bool endOfFile = false;
            bool readErrorOccurred = false;
        };

        Timer timer;
        
        
        
        

        unsigned long long _pageSizeBytes;
        char* _frontBuff;
        char* _backBuff;
        
        LoadingData _currentPageLD;
        LoadingData _lastPageLD;
        TaskStatus _currPageLoadStatus;

        std::function<void()> _preloadNextPage;
        std::function<void()> _preloadPreviousPage;*/
    };
}