#pragma once

#include "MultilineSearchParams.h"
#include "TextComparator.h"

#include <string>
#include <functional>
#include <vector>

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
        virtual void cancelOperation() = 0;
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

        virtual bool search(
            FileReaderI* fileReader,
            ResultSetReaderI* indexReader,
            ResultSetWriterI* indexWriter,
            unsigned long long start,
            unsigned long long end, //0 for end of file, inclusive
            unsigned long long maxNumResults,
            std::shared_ptr<TextComparator> comparator,
            const std::function<void(int percent, unsigned long long numResults)>* progressUpdate
        ) = 0;

        virtual bool parse(
            FileReaderI* fileReader,
            ResultSetReaderI* indexReader,
            unsigned long long start,
            unsigned long long end, //0 for end of file/index, inclusive
            std::shared_ptr<TextComparator> comparator,
            const std::function<bool(unsigned long long lineNum, unsigned long long fileOffset, const char* line, unsigned int length)> action,
            const std::function<void(int percent)>* progressUpdate
        ) = 0;
    };
}