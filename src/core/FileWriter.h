#pragma once

#include "FileWriterI.h"

#include <string>
#include <vector>
#include <memory>

namespace PLP {
    class PagedWriter;
    class TaskRunner;

    class FileWriter : public FileWriterI {
    public:
        FileWriter();
        ~FileWriter();

        bool initialize(
            const std::wstring& path,
            unsigned long long preferredBuffSizeBytes,
            bool overwriteIfExists,
            TaskRunner& asyncTaskRunner
        );

        //Shared interface
        bool append(const std::string& data) override;
        bool append(const char* data, unsigned long long size) override;
        bool appendLine(const std::string& line) override;
        bool appendLine(const char* data, unsigned long long size) override;
        void release() override;

    private:
        std::unique_ptr<PagedWriter> _writer;
    };
}