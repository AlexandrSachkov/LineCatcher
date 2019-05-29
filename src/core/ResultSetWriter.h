#pragma once

#include "ResultSetWriterI.h"
#include "FileLock.h"

#include <string>
#include <memory>
#include <sstream>

namespace PLP {
    class PagedWriter;
    class TaskRunner;
    class FileReader;

    class ResultSetWriter : public ResultSetWriterI {
    public:
        ResultSetWriter();
        ~ResultSetWriter();

        bool initialize(
            const std::wstring& path,
            const std::wstring& dataFilePath,
            unsigned long long preferredBufferSizeBytes,
            bool overwriteIfExists,
            TaskRunner& asyncTaskRunner
        );

        //C++ interface
        bool appendCurrLine(const FileReaderI* fReader) override;
        bool appendCurrLine(unsigned long long lineNumber, unsigned long long fileOffset) override;

        //Lua interface
        bool appendCurrLine(const std::shared_ptr<FileReader> fReader);

        //Shared interface
        unsigned long long getNumResults() const override;
        void release() override;

    private:
        bool flush();
        bool updateResultCount();

        std::unique_ptr<PagedWriter> _writer = nullptr;
        std::string _dataFilePath;
        
        unsigned long long _prevLineNum = 0;
        unsigned long long _resultCount = 0;

        FileScopedLock _writingLock;
    };
}
