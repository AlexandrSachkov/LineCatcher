#pragma once
#include "PagedReader.h"

namespace PLP {
    class LineReader {
    public:
        LineReader();
        virtual ~LineReader();

        bool initialize(PagedReader& pager);
        bool nextLine(char*& lineStart, unsigned int& length);
        unsigned long long getLineNumber();
        unsigned long long getCurrentFileOffset();
        void resetToBeginning();
    private:
        PagedReader* _pager = nullptr;
        char* _pageData = nullptr;
        unsigned long long _pageSize = 0;
        unsigned long long _pageOffset = 0;

        unsigned long long _numPages = 0;
        unsigned long long _lineNum = 0;
    };
}