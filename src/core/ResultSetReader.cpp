#include "ResultSetReader.h"
#include "MemMappedPagedReader.h"
#include "Utils.h"
#include "Logger.h"

#include <fstream>

namespace PLP {
    ResultSetReader::ResultSetReader() {}

    ResultSetReader::~ResultSetReader() {
        Logger::send(INFO, "Releasing index reader");
        release();
    }

    bool ResultSetReader::initialize(
        std::wstring& path,
        unsigned long long preferredBufferSizeBytes
    ) {
        release();
        _path = path;

        MemMappedPagedReader* reader = new MemMappedPagedReader();
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
        return true;
    }

    void ResultSetReader::release() {
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

    bool ResultSetReader::getResult(unsigned long long number, unsigned long long& lineNumber) {
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

    std::tuple<bool, unsigned long long> ResultSetReader::getResult(unsigned long long number) {
        unsigned long long lineNum = 0;
        if (!getResult(number, lineNum)) {
            return { false, 0 };
        }
        return { true, lineNum };
    }

    bool ResultSetReader::nextResult(unsigned long long& lineNumber) {
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

    std::tuple<bool, unsigned long long> ResultSetReader::nextResult() {
        unsigned long long lineNum = 0;
        if (!nextResult(lineNum)) {
            return { false, 0 };
        }
        return { true, lineNum };
    }

    unsigned long long ResultSetReader::getNumResults() const {
        return _numResults;
    }

    void ResultSetReader::getFilePath(std::wstring& path) const {
        path = _path;
    }

    void ResultSetReader::getDataFilePath(std::wstring& path) const {
        path = string_to_wstring(_dataFilePath);
    }

    void ResultSetReader::restart() {
        _currPageOffset = 0;
        _resultCount = 0;
        _pageData = nullptr;
        _pageSize = 0;
        _currLineNum = 0;
        _currLineFileOffset = 0;
        _fileOffset = 0;
    }

    unsigned long long ResultSetReader::getLineNumber() const {
        return _currLineNum;
    }

    unsigned long long ResultSetReader::getLineFileOffset() const {
        return _currLineFileOffset;
    }
}