#pragma once

#include "TaskRunner.h"
#include "Timer.h"

#include <string>
#include <vector>
#include <fstream>
#include <functional>

namespace PLP {
    class TaskRunner;

    class FilePager {
    public:
        FilePager();
        ~FilePager();

        bool initialize(const std::wstring& path, std::vector<char>& buffer, TaskRunner& asyncTaskRunner);
        const char* getNextPage(unsigned long long& size);
        const char* getPreviousPage(unsigned long long& size);

    private:
        struct LoadingData {
            unsigned long long fileOffsetBytes = 0;
            unsigned long long buffStartOffsetBytes = 0;
            unsigned long long buffSizeBytes = 0;
            bool backtracked = false;
            bool endOfFile = false;
            bool readErrorOccurred = false;
        };

        void swapBuffers();
        void preloadNextPage();
        void preloadPreviousPage();

        Timer timer;
        std::vector<char>* _buffer;
        TaskRunner* _asyncTaskRunner;
        std::ifstream _ifs;
        unsigned long long _fileSize;

        unsigned long long _pageSizeBytes;
        char* _frontBuff;
        char* _backBuff;
        
        LoadingData _currentPageLD;
        LoadingData _lastPageLD;
        TaskStatus _currPageLoadStatus;

        std::function<void()> _preloadNextPage;
        std::function<void()> _preloadPreviousPage;
    };
}