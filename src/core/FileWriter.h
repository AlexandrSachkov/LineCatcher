#pragma once

#include "TaskRunner.h"
#include "PagedWriter.h"

#include <string>
#include <vector>

namespace PLP {
    class FileWriter {
    public:
        FileWriter();
        ~FileWriter();

        bool initialize(
            const std::wstring& path,
            unsigned long long preferredBuffSizeBytes,
            TaskRunner& asyncTaskRunner
        );

        bool append(const std::string& data);
        bool append(const char* data, unsigned long long size);
        bool appendLine(const std::string& line);
        bool appendLine(const char* data, unsigned long long size);
        bool flush();
    private:
        PagedWriter _writer;
        std::vector<char> _buff;
    };
}