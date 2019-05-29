#pragma once

#include <string>

namespace PLP {
    class IndexReaderI {
    public:
        virtual ~IndexReaderI() {}
        virtual unsigned long long getLineFileOffset() const = 0;
        virtual bool getResult(unsigned long long number, unsigned long long& lineNumber) = 0;
        virtual bool nextResult(unsigned long long& lineNumber) = 0;
        virtual unsigned long long getLineNumber() const = 0;
        virtual unsigned long long getNumResults() const = 0;
        virtual unsigned long long getResultNumber() const = 0;
        virtual const wchar_t* getFilePath() const = 0;
        virtual const char* getDataFilePath() const = 0;
        virtual void restart() = 0;
        virtual void release() = 0;
    };
}