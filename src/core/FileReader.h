#pragma once

#include <string>
#include <tuple>

namespace PLP {
    class PagedReader;
    class LineReader;
    class FileReader {
    public:
        FileReader();
        ~FileReader();

        bool initialize(
            const std::wstring& path, 
            unsigned long long preferredBuffSizeBytes
        );

        bool nextLine(char*& lineStart, unsigned int& length);
        std::tuple<bool, std::string> nextLine();
        unsigned long long getLineNumber();
    private:
        std::unique_ptr<PagedReader> _pager = nullptr;
        std::unique_ptr<LineReader> _lineReader = nullptr;
    };
}