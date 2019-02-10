#pragma once

#include "ResultSetReaderI.h"
#include <string>
#include <memory>
#include <tuple>

namespace PLP {
    class PagedReader;
    class ResultSetReader : public ResultSetReaderI {
    public:
        ResultSetReader();
        ~ResultSetReader();

        //C++ interface
        bool initialize(
            std::wstring& path, 
            unsigned long long preferredBufferSizeBytes
        );
        unsigned long long getLineFileOffset() const override;
        bool nextResult(unsigned long long& lineNumber) override;

        //Lua interface
        std::tuple<bool, unsigned long long> nextResult();

        //Shared interface
        unsigned long long getLineNumber() const override;
        unsigned long long getNumResults() const override;
        void getDataFilePath(std::string& path) const override;
        void restart() override;
        void release() override;

    private:
        std::unique_ptr<PagedReader> _reader = nullptr;
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
