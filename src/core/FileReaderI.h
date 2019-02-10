#pragma once

#include <string>

namespace PLP {
    class ResultSetReaderI;
    class FileReaderI {
    public:
        virtual ~FileReaderI() {}
        virtual bool nextLine(char*& lineStart, unsigned int& length) = 0;
        virtual bool getLine(unsigned long long lineNumber, char*& data, unsigned int& size) = 0;
        virtual bool getLineFromResult(const ResultSetReaderI* rsReader, char*& data, unsigned int& size) = 0;
        virtual unsigned long long getLineFileOffset() const = 0;
        virtual void getFilePath(std::wstring& path) const = 0;
        virtual unsigned long long getLineNumber() const = 0;
        virtual void restart() = 0;
        virtual void release() = 0;
    };
}
