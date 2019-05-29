#pragma once

#include "FileReaderI.h"
#include "FileLock.h"

#include <string>
#include <tuple>
#include <atomic>
#include <functional>

namespace PLP {
    class PagedReader;
    class IndexedLineReader;
    class ResultSetReader;

    class FileReader : public FileReaderI{
    public:
        FileReader();
        ~FileReader();

        bool initialize(
            const std::wstring& path, 
            unsigned long long preferredBuffSizeBytes,
            const std::atomic<bool>& cancelled,
            const std::function<void(int percent)>* progressUpdate
        );
        
        //C++ interface
        LineReaderResult nextLine(char*& lineStart, unsigned int& length) override;
        LineReaderResult getLine(unsigned long long lineNumber, char*& data, unsigned int& size) override;
        LineReaderResult getLineFromResult(const ResultSetReaderI* rsReader, char*& data, unsigned int& size) override;
        unsigned long long getLineFileOffset() const override;
        const wchar_t* getFilePath() const override;

        //Lua interface
        std::tuple<int, std::string> nextLine();
        std::tuple<int, std::string> getLine(unsigned long long lineNumber);
        std::tuple<int, std::string> getLineFromResult(const std::shared_ptr<ResultSetReader> rsReader);

        //Shared interface
        unsigned long long getLineNumber() const override;
        unsigned long long getNumberOfLines() const override;
        void restart() override;
        void release() override;

    private:
        std::unique_ptr<PagedReader> _pager = nullptr;
        std::unique_ptr<IndexedLineReader> _lineReader = nullptr;
        FileScopedLock _readingLock;
    };
}