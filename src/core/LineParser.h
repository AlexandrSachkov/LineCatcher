#pragma once
#include "FilePager.h"

namespace PLP {
    class LineParser {
    public:
        LineParser();
        ~LineParser();

        bool initialize(FilePager& pager);
        bool nextLine(char*& lineStart, unsigned int& length);
        unsigned long long getLineNumber();
    private:
        FilePager* _pager = nullptr;
        char* _pageData = nullptr;
        unsigned long long _pageSize = 0;
        unsigned long long _pageOffset = 0;

        unsigned long long _numPages = 0;
        unsigned long long _lineNum = 0;
    };
}