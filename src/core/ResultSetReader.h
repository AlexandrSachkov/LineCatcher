#pragma once

#include <string>
#include <memory>

namespace PLP {
    class PagedReader;
    class ResultSetReader {
    public:
        ResultSetReader();
        ~ResultSetReader();

        bool initialize(
            std::wstring& path, 
            unsigned long long preferredBufferSizeBytes
        );
        void release();

        bool nextResult(unsigned long long& lineNumber, unsigned long long& fileOffset);
        unsigned long long getLineNumber() const;
        unsigned long long getLineFileOffset() const;
        unsigned long long getNumResults();
        std::string getDataFilePath();
        void resetToBeginning();

    private:
        std::unique_ptr<PagedReader> _reader;
        std::wstring _path;
        std::string _dataFilePath;
        unsigned long long _numResults = 0;
        unsigned long long _resultCount = 0;
        unsigned long long _currFileOffset = 0;
        unsigned long long _currLineNum = 0;
        unsigned long long _currLineFileOffset = 0;

        char* _pageData = nullptr;
        unsigned long long _pageSize = 0;
    };
}
