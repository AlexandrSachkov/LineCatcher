#pragma once
#include "LineBuffer.h"
#include "ReturnType.h"

#include <vector>
#include <memory>

namespace PLP {
    class PagedReader;
    class LineReader {
    public:
        virtual ~LineReader();

        bool initialize(PagedReader& pager, unsigned int maxLineSize);
        LineReaderResult nextLine(char*& data, unsigned int& size);
        LineReaderResult getLineUnverified( //only use if you know what you are doing. Incorrect lineNum/fileOffset can cause undefined behavior
            unsigned long long lineNum, 
            unsigned long long fileOffset, 
            char*& data, 
            unsigned int& size
        );
        unsigned long long getLineNumber();
        unsigned long long getCurrentFileOffset();
        unsigned long long getCurrentLineFileOffset();
        void restart();

    protected:
        PagedReader* _pager = nullptr;
        char* _pageData = nullptr;
        unsigned long long _pageSize = 0;
        unsigned long long _fileOffset = 0;
        unsigned long long _pageOffset = 0;
        unsigned long long _lineCount = 0;
        unsigned int _currentLineLength = 0;

        std::unique_ptr<LineBuffer> _pageBoundaryLineBuff;
        unsigned int _maxLineSize = 0;
    };
}