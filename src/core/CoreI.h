#pragma once

#include <string>

namespace PLP {
    class FileReader;
    class FileWriter;
    class ResultSetReader;
    class ResultSetWriter;

    class CoreI {
    public:
        virtual ~CoreI() {}

        virtual bool initialize() = 0;
        virtual bool runScript(const std::wstring& scriptLua, std::wstring& errMsg) = 0;

        virtual FileReader* createFileReader(
            const std::string& path,
            unsigned long long preferredBuffSizeBytes,
            bool requireRandomAccess
        ) = 0;

        virtual FileWriter* createFileWriter(
            const std::string& path,
            unsigned long long preferredBuffSizeBytes,
            bool overwriteIfExists
        ) = 0;

        virtual ResultSetReader* createResultSetReader(
            const std::string& path,
            unsigned long long preferredBuffSizeBytes
        ) = 0;

        virtual ResultSetWriter* createResultSetWriter(
            const std::string& path,
            unsigned long long preferredBuffSizeBytes,
            const FileReader& fReader,
            bool overwriteIfExists
        ) = 0;
    };
}