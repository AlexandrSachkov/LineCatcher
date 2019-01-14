#include "FileReader.h"

namespace PLP {
    FileReader::FileReader() {}
    FileReader::~FileReader() {}

    bool FileReader::initialize(
        const std::wstring& path, 
        unsigned long long preferredBuffSizeBytes, 
        TaskRunner& asyncTaskRunner) {

        try {
            unsigned long long optimalSize = preferredBuffSizeBytes / (FilePager::OPTIMAL_BLOCK_SIZE_BYTES * 2) * (FilePager::OPTIMAL_BLOCK_SIZE_BYTES * 2);
            if (optimalSize == 0) {
                return false;
            }
            _buff.resize(optimalSize, 0);
        } catch (std::bad_alloc&) {
            return false;
        }

        if (!_pager.initialize(path, _buff, asyncTaskRunner)) {
            return false;
        }

        if (!_lineParser.initialize(_pager)) {
            return false;
        }

        return true;
    }

    bool FileReader::nextLine(char*& lineStart, unsigned int& length) {
        return _lineParser.nextLine(lineStart, length);
    }

    std::tuple<bool, std::string> FileReader::nextLine() {
        char* lineStart = nullptr;
        unsigned int length = 0;
        if (!_lineParser.nextLine(lineStart, length)) {
            return {false, std::string()};
        }

        return { true, std::string(lineStart, length) };
    }

    unsigned long long FileReader::getLineNumber() {
        return _lineParser.getLineNumber();
    }
}