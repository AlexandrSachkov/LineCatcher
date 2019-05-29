#include "IndexReader.h"
#include "MemMappedPagedReader.h"
#include "FStreamPagedReader.h"
#include "Utils.h"
#include "Logger.h"

#include <fstream>

namespace PLP {
    IndexReader::IndexReader() {}

    IndexReader::~IndexReader() {
        Logger::send(INFO, "Releasing index reader");
        release();
    }

    bool IndexReader::initialize(
        const std::wstring& path,
        unsigned long long preferredBufferSizeBytes
    ) {
        release();
        _path = path;

        FileScopedLock readingLock = FileScopedLock::lockForReading(path);
        if (!readingLock.isLocked()) {
            Logger::send(ERR, "Unable to acquare a read lock on file " + wstring_to_string(path) + ". Release writers using the file");
            return false;
        }

        FStreamPagedReader* reader = new FStreamPagedReader();
        _reader.reset(reader);
        if (!reader->initialize(path, preferredBufferSizeBytes)) {
            return false;
        }

        _pageData = const_cast<char*>(_reader->read(0, _pageSize));
        if (!_pageData || _pageSize == 0) {
            return false;
        }
        _currPageOffset = 0;

        unsigned int version = 0;
        std::memcpy(&version, _pageData + _currPageOffset, sizeof(RESULT_SET_VERSION));
        if (version != RESULT_SET_VERSION) {
            return false; //TODO can we handle legacy versions
        }

        _currPageOffset += sizeof(RESULT_SET_VERSION);
        unsigned int dataFilePathLength = 0;
        std::memcpy(&dataFilePathLength, _pageData + _currPageOffset, sizeof(dataFilePathLength));

        _currPageOffset += sizeof(dataFilePathLength);
        _dataFilePath = std::string(_pageData + _currPageOffset, dataFilePathLength);

        _currPageOffset += dataFilePathLength;
        std::memcpy(&_numResults, _pageData + _currPageOffset, sizeof(unsigned long long));

        _currPageOffset += sizeof(unsigned long long);
        _fileOffset = _currPageOffset;

        _readingLock = std::move(readingLock);
        return true;
    }

    void IndexReader::release() {
        _reader = nullptr;
        _path;
        _dataFilePath;
        _numResults = 0;
        _resultCount = 0;
        _currPageOffset = 0;
        _currLineNum = 0;
        _currLineFileOffset = 0;
        _pageData = nullptr;
        _pageSize = 0;
        _fileOffset = 0;
    }

    bool IndexReader::getResult(unsigned long long number, unsigned long long& lineNumber) {
        if (number >= _numResults) {
            return false;
        }

        unsigned long long headerSize =
            sizeof(RESULT_SET_VERSION) +
            sizeof(unsigned int) + //length of data file path variable size
            _dataFilePath.length() +
            sizeof(_resultCount);

        _pageData = nullptr;
        _pageSize = 0;
        _resultCount = number;
        _fileOffset = headerSize + (sizeof(unsigned long long) * 2) * number;

        return nextResult(lineNumber);
    }

    std::tuple<bool, unsigned long long> IndexReader::getResult(unsigned long long number) {
        unsigned long long lineNum = 0;
        if (!getResult(number, lineNum)) {
            return { false, 0 };
        }
        return { true, lineNum };
    }

    bool IndexReader::nextResult(unsigned long long& lineNumber) {
        if (_resultCount >= _numResults) {
            return false;
        }

        if (_pageData == nullptr || _pageSize == 0 || _pageSize - _currPageOffset < sizeof(unsigned long long) * 2) {
            _pageData = const_cast<char*>(_reader->read(_fileOffset, _pageSize));
            if (!_pageData || _pageSize == 0) {
                return false;
            }
            _currPageOffset = 0;
        }
        char* data = _pageData + _currPageOffset;
        std::memcpy(&_currLineNum, _pageData + _currPageOffset, sizeof(_currLineNum));
        _currPageOffset += sizeof(_currLineNum);
        _fileOffset += sizeof(_currLineNum);

        std::memcpy(&_currLineFileOffset, _pageData + _currPageOffset, sizeof(_currLineFileOffset));
        _currPageOffset += sizeof(_currLineFileOffset);
        _fileOffset += sizeof(_currLineFileOffset);;

        _resultCount++;

        lineNumber = _currLineNum;
        return true;
    }

    std::tuple<bool, unsigned long long> IndexReader::nextResult() {
        unsigned long long lineNum = 0;
        if (!nextResult(lineNum)) {
            return { false, 0 };
        }
        return { true, lineNum };
    }

    unsigned long long IndexReader::getNumResults() const {
        return _numResults;
    }

    const wchar_t* IndexReader::getFilePath() const {
        return _path.c_str();
    }

    const char* IndexReader::getDataFilePath() const {
        return _dataFilePath.c_str();
    }

    void IndexReader::restart() {
        _currPageOffset = 0;
        _resultCount = 0;
        _pageData = nullptr;
        _pageSize = 0;
        _currLineNum = 0;
        _currLineFileOffset = 0;
        _fileOffset = 0;
    }

    unsigned long long IndexReader::getLineNumber() const {
        return _currLineNum;
    }

    unsigned long long IndexReader::getLineFileOffset() const {
        return _currLineFileOffset;
    }

    unsigned long long IndexReader::getResultNumber() const {
        if (_resultCount == 0) {
            return 0;
        }
        return _resultCount - 1;
    }
}