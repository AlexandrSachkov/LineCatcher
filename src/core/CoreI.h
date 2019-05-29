#pragma once

#include "TextComparator.h"

#include <string>
#include <functional>
#include <vector>

namespace PLP {
    class FileReaderI;
    class FileWriterI;
    class IndexReaderI;
    class ResultSetWriterI;

    class CoreI {
    public:
        virtual ~CoreI() {}

        virtual bool initialize() = 0;
        virtual bool runScript(const std::wstring* scriptLua) = 0;
        virtual void cancelOperation() = 0;
        virtual bool isCancelled() = 0;
        virtual bool attachLogOutput(const char* name, const std::function<void(int, const char*)>* func) = 0;
        virtual void detachLogOutput(const char* name) = 0;

        virtual FileReaderI* createFileReader(
            const std::string& path,
            unsigned long long preferredBuffSizeBytes,
            const std::function<void(int percent)>* progressUpdate
        ) = 0;

        virtual FileWriterI* createFileWriter(
            const std::string& path,
            unsigned long long preferredBuffSizeBytes,
            bool overwriteIfExists
        ) = 0;

        virtual IndexReaderI* createIndexReader(
            const std::string& path,
            unsigned long long preferredBuffSizeBytes
        ) = 0;

        virtual ResultSetWriterI* createResultSetWriter(
            const std::string& path,
            unsigned long long preferredBuffSizeBytes,
            const FileReaderI* fReader,
            bool overwriteIfExists
        ) = 0;

        virtual void release(FileReaderI*) = 0;
        virtual void release(FileWriterI*) = 0;
        virtual void release(IndexReaderI*) = 0;
        virtual void release(ResultSetWriterI*) = 0;

        virtual bool search(
            FileReaderI* fileReader,
            IndexReaderI* indexReader,
            ResultSetWriterI* indexWriter,
            unsigned long long start,
            unsigned long long end, //0 for end of file, inclusive
            unsigned long long maxNumResults,
            TextComparator* comparator,
            const std::function<void(int percent, unsigned long long numResults)>* progressUpdate
        ) = 0;

        virtual bool searchMultiline(
            FileReaderI* fileReader,
            IndexReaderI* indexReader,
            ResultSetWriterI* indexWriter,
            unsigned long long start,
            unsigned long long end, //0 for end of file, inclusive
            unsigned long long maxNumResults,
            const std::unordered_map<int, TextComparator*>& lineComparators,
            const std::function<void(int percent, unsigned long long numResults)>* progressUpdate
        ) = 0;
    };
}