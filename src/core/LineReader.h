#pragma once
#include <vector>

namespace PLP {
    class PagedReader;
    class LineReader {
    public:
        virtual ~LineReader();

        bool initialize(PagedReader& pager);
        bool nextLine(char*& data, unsigned int& size);
        unsigned long long getLineNumber();
        unsigned long long getCurrentFileOffset();
        void resetToBeginning();
    private:
        bool appendPageBoundaryLineBuff(const char* data, unsigned int size);
        void resetPageBoundaryLineBuff();

        PagedReader* _pager = nullptr;
        char* _pageData = nullptr;
        unsigned long long _pageSize = 0;
        unsigned long long _fileOffset = 0;
        unsigned long long _pageOffset = 0;
        unsigned long long _lineNum = 0;

        std::vector<char> _pageBoundaryLineBuff;
    };
}