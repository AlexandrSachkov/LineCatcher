#include "FileReader.h"
#include "MemMappedPagedReader.h"
#include "PagedReader.h"
#include "LineReader.h"
#include "Utils.h"

namespace PLP {
    FileReader::FileReader() {}
    FileReader::~FileReader() {}

    bool FileReader::initialize(const std::wstring& path, unsigned long long preferredBuffSizeBytes) {
        MemMappedPagedReader* pagedReader = new MemMappedPagedReader();
        _pager.reset(pagedReader);
        if (!pagedReader->initialize(path, preferredBuffSizeBytes)) {
            return false;
        }

        _lineReader.reset(new LineReader());
        if (!_lineReader->initialize(*_pager)) {
            return false;
        }

        return true;
    }

    bool FileReader::nextLine(char*& lineStart, unsigned int& length) {
        return _lineReader->nextLine(lineStart, length);
    }

    std::tuple<bool, std::string> FileReader::nextLine() {
        char* lineStart = nullptr;
        unsigned int length = 0;
        if (!_lineReader->nextLine(lineStart, length)) {
            return { false, std::string() };
        }

        return { true, std::string(lineStart, length) };
    }

    unsigned long long FileReader::getLineNumber() {
        return _lineReader->getLineNumber();
    }
}