#include "FileReader.h"
#include "MemMappedPagedReader.h"
#include "PagedReader.h"
#include "LineReader.h"
#include "IndexedLineReader.h"
#include "Utils.h"
#include "ResultSetReader.h"
#include "Logger.h"

namespace PLP {
    FileReader::FileReader() {}
    FileReader::~FileReader() {
        Logger::send(INFO, "Releasing file reader");
        release();
    }

    bool FileReader::initialize(
        const std::wstring& path, 
        unsigned long long preferredBuffSizeBytes, 
        bool requireRandomAccess
    ) {
        release();
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

    void FileReader::release() {
        _lineReader = nullptr;
        _pager = nullptr;
        _enableRandomAccess = false;
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
            return false;
        }
        return static_cast<IndexedLineReader*>(_lineReader.get())->getLine(lineNumber, data, size);
    }

    bool FileReader::getLineFromResult(const ResultSetReaderI* rsReader, char*& data, unsigned int& size) {
        if (!rsReader) {
            return false;
        }
        return _lineReader->getLineUnverified(rsReader->getLineNumber(), rsReader->getLineFileOffset(), data, size);
    }

    std::tuple<bool, std::string> FileReader::getLineFromResult(std::shared_ptr<ResultSetReaderI> rsReader) {
        char* lineStart = nullptr;
        unsigned int length = 0;
        if (!getLineFromResult(rsReader.get(), lineStart, length)) {
            return { false, std::string() };
        }
        return { true, std::string(lineStart, length) };
    }

    std::tuple<bool, std::string> FileReader::getLine(unsigned long long lineNumber) {
        char* lineStart = nullptr;
        unsigned int length = 0;
        if (!getLine(lineNumber, lineStart, length)) {
            return { false, std::string() };
        }

        return { true, std::string(lineStart, length) };
    }

    unsigned long long FileReader::getLineNumber() const {
        return _lineReader->getLineNumber();
    }

    unsigned long long FileReader::getLineFileOffset() const {
        return _lineReader->getCurrentLineFileOffset();
    }

    void FileReader::getFilePath(std::wstring& path) const {
        path = _pager->getFilePath();
    }

    void FileReader::restart() {
        _lineReader->restart();
    }

    unsigned long long FileReader::getNumberOfLines() const {
        if (!_enableRandomAccess) {
            return 0;
        }
        return static_cast<IndexedLineReader*>(_lineReader.get())->getNumberOfLines();
    }
}