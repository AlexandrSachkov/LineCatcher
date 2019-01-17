#pragma once

#include "TaskRunner.h"
#include <string>
#include <vector>
#include <fstream>
#include <functional>

namespace PLP {
    class PagedWriter {
    public:
        PagedWriter();
        ~PagedWriter();

        bool initialize(const std::wstring& path, std::vector<char>& buffer, TaskRunner& asyncTaskRunner);
        bool write(const char* data, unsigned long long size);
        bool flush();
    private:
        void swapBuffers();

        std::vector<char>* _buffer;
        TaskRunner* _asyncTaskRunner;
        std::ofstream _ofs;

        unsigned long long _pageSizeBytes;
        char* _frontBuff;
        char* _backBuff;
        unsigned long long _frontBuffContentSize;

        TaskStatus _backBuffLoadStatus;
        std::function<void()> _writeBackBuff;
        bool _writeError = false;
    };
}