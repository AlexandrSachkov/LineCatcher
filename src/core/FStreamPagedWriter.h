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