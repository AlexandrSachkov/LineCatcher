#pragma once

#include <string>

namespace PLP {
    class PagedReader {
    public:
        virtual ~PagedReader() {}
        virtual const char* read(unsigned long long fileOffset, unsigned long long& size) = 0;
        virtual unsigned long long getFileSize() = 0;
        virtual std::wstring getFilePath() = 0;
    };
}