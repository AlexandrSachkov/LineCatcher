#include "FileReader.h"
#include "MemMappedPagedReader.h"
#include "PagedReader.h"
#include "LineReader.h"
#include "IndexedLineReader.h"
#include "Utils.h"
#include "ResultSetReader.h"
#include "FStreamPagedReader.h"
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
        const std::atomic<bool>& cancelled,
        const std::function<void(int percent)>* progressUpdate
    ) {
        release();

        FileScopedLock readingLock = FileScopedLock::lockForReading(path);
        if (!readingLock.isLocked()) {
            Logger::send(ERR, "Unable to acquare a read lock on file " + wstring_to_string(path) + ". Release writers using the file");
            return false;
        }

        FStreamPagedReader* pagedReader = new FStreamPagedReader();
        _pager.reset(pagedReader);
        if (!pagedReader->initialize(path, preferredBuffSizeBytes)) {
            return false;
        }

        IndexedLineReader* idxLineReader = new IndexedLineReader();
        _lineReader.reset(idxLineReader);
        if (!idxLineReader->initialize(*_pager, 100000, cancelled, progressUpdate)) {
            return false;
        }

        _readingLock = std::move(readingLock);
        return true;
    }

    void FileReader::release() {
        _lineReader = nullptr;
        _pager = nullptr;
    }

    LineReaderResult FileReader::nextLine(char*& lineStart, unsigned int& length) {
        return _lineReader->nextLine(lineStart, length);
    }

    std::tuple<int, std::string> FileReader::nextLine() {
        char* lineStart = nullptr;
        unsigned int length = 0;

        LineReaderResult result = _lineReader->nextLine(lineStart, length);
        if (result != LineReaderResult::SUCCESS) {
            return { result, std::string() };
        }
        return { result, std::string(lineStart, length) };
    }

    LineReaderResult FileReader::getLine(unsigned long long lineNumber, char*& data, unsigned int& size) {
        return _lineReader->getLine(lineNumber, data, size);
    }

    LineReaderResult FileReader::getLineFromResult(const ResultSetReaderI* rsReader, char*& data, unsigned int& size) {
        if (!rsReader) {
            return LineReaderResult::ERROR;
        }
        return _lineReader->getLineUnverified(rsReader->getLineNumber(), rsReader->getLineFileOffset(), data, size);
    }

    std::tuple<int, std::string> FileReader::getLineFromResult(const std::shared_ptr<ResultSetReader> rsReader) {
        char* lineStart = nullptr;
        unsigned int length = 0;

        LineReaderResult result = getLineFromResult(rsReader.get(), lineStart, length);
        if (result != LineReaderResult::SUCCESS) {
            return { result, std::string() };
        }
        return { result, std::string(lineStart, length) };
    }

    std::tuple<int, std::string> FileReader::getLine(unsigned long long lineNumber) {
        char* lineStart = nullptr;
        unsigned int length = 0;

        LineReaderResult result = getLine(lineNumber, lineStart, length);
        if (result != LineReaderResult::SUCCESS) {
            return { result, std::string() };
        }
        return { result, std::string(lineStart, length) };
    }

    unsigned long long FileReader::getLineNumber() const {
        return _lineReader->getLineNumber();
    }

    unsigned long long FileReader::getLineFileOffset() const {
        return _lineReader->getCurrentLineFileOffset();
    }

    const wchar_t* FileReader::getFilePath() const {
        return _pager->getFilePath().c_str();
    }

    void FileReader::restart() {
        _lineReader->restart();
    }

    unsigned long long FileReader::getNumberOfLines() const {
        return _lineReader->getNumberOfLines();
    }
}