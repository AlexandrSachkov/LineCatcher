#pragma once

#include <string>
#include <tuple>

namespace PLP {
    class PagedReader;
    class LineReader;
    class ResultSetReader;

    class FileReader {
    public:
        FileReader();
        ~FileReader();

        bool initialize(
            const std::wstring& path, 
            unsigned long long preferredBuffSizeBytes,
            bool requireRandomAccess
        );

        bool nextLine(char*& lineStart, unsigned int& length);
        bool getLine(unsigned long long lineNumber, char*& data, unsigned int& size);
        bool getLineFromResult(const ResultSetReader& rsReader, char*& data, unsigned int& size);
        std::tuple<bool, std::string> nextLine();
        std::tuple<bool, std::string> getLine(unsigned long long lineNumber);
        unsigned long long getLineNumber() const;
        unsigned long long getLineFileOffset() const;
        std::wstring getFilePath() const;
        void resetToBeginning();
    private:
        std::unique_ptr<PagedReader> _pager = nullptr;
        std::unique_ptr<LineReader> _lineReader = nullptr;
        bool _enableRandomAccess = false;
    };
}