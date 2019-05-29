#pragma once

namespace PLP {
    class FileReaderI;
    class IndexWriterI {
    public:
        virtual ~IndexWriterI() {}
        virtual bool appendCurrLine(const FileReaderI* fReader) = 0;
        virtual bool appendCurrLine(unsigned long long lineNumber, unsigned long long fileOffset) = 0;
        virtual unsigned long long getNumResults() const = 0;
        virtual void release() = 0;
    };
}