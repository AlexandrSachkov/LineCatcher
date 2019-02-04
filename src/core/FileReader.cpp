#include "FileReader.h"
#include "MemMappedPagedReader.h"
#include "PagedReader.h"
#include "LineReader.h"
#include "IndexedLineReader.h"
#include "Utils.h"

namespace PLP {
    FileReader::FileReader() {}
    FileReader::~FileReader() {}

    bool FileReader::initialize(
        const std::wstring& path, 
        unsigned long long preferredBuffSizeBytes, 
        bool requireRandomAccess
    ) {
        _enableRandomAccess = requireRandomAccess;

        MemMappedPagedReader* pagedReader = new MemMappedPagedReader();
        _pager.reset(pagedReader);
        if (!pagedReader->initialize(path, preferredBuffSizeBytes)) {
            return false;
        }

        if (requireRandomAccess) {
            IndexedLineReader* idxLineReader = new IndexedLineReader();
            if (!idxLineReader->initialize(*_pager)) {
                return false;
            }
            _lineReader.reset(idxLineReader);
        } else {
            _lineReader.reset(new LineReader());
            if (!_lineReader->initialize(*_pager)) {
                return false;
            }
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

    bool FileReader::getLine(unsigned long long lineNumber, char*& data, unsigned int& size) {
        if (!_enableRandomAccess) {
            return false; //no random access enabled
        }
        return static_cast<IndexedLineReader*>(_lineReader.get())->getLine(lineNumber, data, size);
    }

    std::tuple<bool, std::string> FileReader::getLine(unsigned long long lineNumber) {
        char* lineStart = nullptr;
        unsigned int length = 0;
        if (!getLine(lineNumber, lineStart, length)) {
            return { false, std::string() };
        }

        return { true, std::string(lineStart, length) };
    }

    unsigned long long FileReader::getLineNumber() {
        return _lineReader->getLineNumber();
    }
}