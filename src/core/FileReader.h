#pragma once
#include "TaskRunner.h"
#include "PagedReader.h"
#include "LineReader.h"

#include <string>
#include <vector>

namespace PLP {
    class FileReader {
    public:
        FileReader();
        ~FileReader();

        bool initialize(
            const std::wstring& path, 
            unsigned long long preferredBuffSizeBytes, 
            TaskRunner& asyncTaskRunner
        );

        bool nextLine(char*& lineStart, unsigned int& length);
        std::tuple<bool, std::string> nextLine();
        unsigned long long getLineNumber();
    private:
        PagedReader _pager;
        LineReader _lineParser;
        std::vector<char> _buff;
    };
}