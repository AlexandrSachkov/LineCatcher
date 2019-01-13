#pragma once

#include "TaskRunner.h"
#include "Timer.h"

#include <string>
#include <vector>
#include <fstream>
#include <functional>

#include <Windows.h>

namespace PLP {
    class TaskRunner;

    class MemMappedFilePager {
    public:
        MemMappedFilePager();
        ~MemMappedFilePager();

        bool initialize(const std::wstring& path, unsigned long long buffSize, TaskRunner& asyncTaskRunner);
        const char* getNextPage(size_t& size);
        const char* getPreviousPage(size_t& size);

        static const size_t OPTIMAL_BLOCK_SIZE_BYTES = 64 * 1024; //64 KBytes 
        //https://docs.microsoft.com/en-us/previous-versions/windows/it-pro/windows-2000-server/cc938632(v=technet.10)

    private:
        struct LoadingData {
            size_t fileOffsetBytes = 0;
            size_t buffStartOffsetBytes = 0;
            size_t buffSizeBytes = 0;
            bool backtracked = false;
            bool endOfFile = false;
            bool readErrorOccurred = false;
        };

        void swapBuffers();
        void preloadNextPage();
        void preloadPreviousPage();

        void* _fileHandle;
        void* _fileMappingHandle;
        unsigned long long _fileSize;
        unsigned long long _allocGranularity;

        unsigned long long _buffSize;
        unsigned long long _fileOffset;
        void* _data;

        Timer timer;
        std::vector<char>* _buffer;
        TaskRunner* _asyncTaskRunner;
        

        size_t _pageSizeBytes;
        char* _frontBuff;
        char* _backBuff;

        LoadingData _currentPageLD;
        LoadingData _lastPageLD;
        TaskStatus _currPageLoadStatus;

        std::function<void()> _preloadNextPage;
        std::function<void()> _preloadPreviousPage;
    };
}