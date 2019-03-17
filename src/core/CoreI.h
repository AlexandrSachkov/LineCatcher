#pragma once

#include <string>
#include <functional>

namespace PLP {
    class FileReaderI;
    class FileWriterI;
    class ResultSetReaderI;
    class ResultSetWriterI;

    class CoreI {
    public:
        virtual ~CoreI() {}

        virtual bool initialize() = 0;
        virtual bool runScript(const std::wstring* scriptLua) = 0;
        virtual bool attachLogOutput(const char* name, const std::function<void(int, const char*)>* func) = 0;
        virtual void detachLogOutput(const char* name) = 0;

        virtual FileReaderI* createFileReader(
            const std::string& path,
            unsigned long long preferredBuffSizeBytes,
            bool requireRandomAccess
        ) = 0;

        virtual FileWriterI* createFileWriter(
            const std::string& path,
            unsigned long long preferredBuffSizeBytes,
            bool overwriteIfExists
        ) = 0;

        virtual ResultSetReaderI* createResultSetReader(
            const std::string& path,
            unsigned long long preferredBuffSizeBytes
        ) = 0;

        virtual ResultSetWriterI* createResultSetWriter(
            const std::string& path,
            unsigned long long preferredBuffSizeBytes,
            const FileReaderI* fReader,
            bool overwriteIfExists
        ) = 0;
    };
}