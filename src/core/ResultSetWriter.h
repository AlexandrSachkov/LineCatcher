#pragma once

#include <string>
#include <memory>
#include <sstream>

namespace PLP {
    class PagedWriter;
    class FileReader;
    class TaskRunner;

    class ResultSetWriter {
    public:
        ResultSetWriter();
        ~ResultSetWriter();

        bool initialize(
            std::wstring& path,
            std::wstring& dataFilePath,
            unsigned long long preferredBufferSizeBytes,
            bool overwriteIfExists,
            TaskRunner& asyncTaskRunner
        );

        void release();

        bool appendCurrentLine(const FileReader& fReader);
        unsigned long long getNumResults();
        bool flush();

    private:
        bool updateResultCount();

        std::unique_ptr<PagedWriter> _writer = nullptr;
        std::wstring _path;
        std::string _dataFilePath;
        
        unsigned long long _prevLineNum = 0;
        unsigned long long _resultCount = 0;
    };
}
