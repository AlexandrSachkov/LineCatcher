#pragma once

#include "FileReaderI.h"

#include <string>
#include <tuple>

namespace PLP {
    class PagedReader;
    class LineReader;

    class FileReader : public FileReaderI{
    public:
        FileReader();
        ~FileReader();

        bool initialize(
            const std::wstring& path, 
            unsigned long long preferredBuffSizeBytes,
            bool requireRandomAccess
        );
        
        //C++ interface
        bool nextLine(char*& lineStart, unsigned int& length) override;
        bool getLine(unsigned long long lineNumber, char*& data, unsigned int& size) override;
        bool getLineFromResult(const ResultSetReaderI* rsReader, char*& data, unsigned int& size) override;
        unsigned long long getLineFileOffset() const override;
        void getFilePath(std::wstring& path) const override;

        //Lua interface
        std::tuple<bool, std::string> nextLine();
        std::tuple<bool, std::string> getLine(unsigned long long lineNumber);
        std::tuple<bool, std::string> getLineFromResult(std::shared_ptr<ResultSetReaderI> rsReader);

        //Shared interface
        unsigned long long getLineNumber() const override;
        unsigned long long getNumberOfLines() const override;
        void restart() override;
        void release() override;

    private:
        std::unique_ptr<PagedReader> _pager = nullptr;
        std::unique_ptr<LineReader> _lineReader = nullptr;
        bool _enableRandomAccess = false;
    };
}