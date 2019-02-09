#pragma once

#include "TaskRunner.h"
#include <string>
#include <memory>
#include <sstream>

namespace PLP {
    class PagedWriter;
    class FileReader;
    class ResultSetWriter {
    public:
        ResultSetWriter();
        ~ResultSetWriter();

        bool initialize(
            std::wstring& path,
            std::wstring& dataFilePath,
            unsigned long long preferredBufferSizeBytes,
            TaskRunner& asyncTaskRunner
        );

        bool appendCurrentLine(const FileReader& fReader);
        unsigned long long getNumResults();
        bool flush();

    private:
        struct Result {
            Result(unsigned long long lineNum, unsigned long long fileOffset) : 
                lineNum(lineNum), fileOffset(fileOffset) {}

            unsigned long long lineNum = 0;
            unsigned long long fileOffset = 0;

            template<class Archive>
            void serialize(Archive& archive) {
                archive(lineNum, fileOffset);
            }
        };

        static const unsigned int RESULT_SET_VERSION = 1; // increment if format changes

        std::unique_ptr<PagedWriter> _writer = nullptr;
        std::ostringstream _serialBuff;
        std::wstring _path;
        std::wstring _dataFilePath;

        unsigned long long _resultCount = 0;
        unsigned long long _prevLineNum = 0;
    };
}
