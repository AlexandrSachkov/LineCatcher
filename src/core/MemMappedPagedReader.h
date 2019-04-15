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