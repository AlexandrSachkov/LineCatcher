#include "ResultSetReader.h"
#include "MemMappedPagedReader.h"
#include "Utils.h"

#include <fstream>

namespace PLP {
    ResultSetReader::ResultSetReader() {}

    ResultSetReader::~ResultSetReader() {
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
        
        unsigned long long headerSize =
            sizeof(RESULT_SET_VERSION) +
            sizeof(unsigned int) + //length of data file path
            _dataFilePath.length() +
            sizeof(_resultCount);

        if (_pageSize < headerSize) {
            return false;
        }

        _currFileOffset = 0;

        unsigned int version = 0;
        std::memcpy(&version, _pageData + _currFileOffset, sizeof(RESULT_SET_VERSION));
        if (version != RESULT_SET_VERSION) {
            return false; //TODO can we handle legacy versions
        }

        _currFileOffset += sizeof(RESULT_SET_VERSION);
        unsigned int dataFilePathLength = 0;
        std::memcpy(&dataFilePathLength, _pageData + _currFileOffset, sizeof(dataFilePathLength));

        _currFileOffset += sizeof(dataFilePathLength);
        _dataFilePath = std::string(_pageData + sizeof(RESULT_SET_VERSION) + sizeof(dataFilePathLength), dataFilePathLength);

        _currFileOffset += dataFilePathLength;
        std::memcpy(&_numResults, _pageData + _currFileOffset, sizeof(unsigned long long));

        _currFileOffset += sizeof(unsigned long long);
        return true;
    }

    void ResultSetReader::release() {
        _reader = nullptr;
        _path;
        _dataFilePath;
        _numResults = 0;
        _resultCount = 0;
        _currFileOffset = 0;
        _currLineNum = 0;
        _currLineFileOffset = 0;
        _pageData = nullptr;
        _pageSize = 0;
    }

    bool ResultSetReader::nextResult(unsigned long long& lineNumber) {
        if (_resultCount >= _numResults) {
            return false;
        }

        if (_pageSize - _currFileOffset < sizeof(unsigned long long) * 2) {
            _pageData = const_cast<char*>(_reader->read(0, _pageSize));
            if (!_pageData || _pageSize == 0) {
                return false;
            }
        }
        std::memcpy(&_currLineNum, _pageData + _currFileOffset, sizeof(_currLineNum));
        _currFileOffset += sizeof(_currLineNum);

        std::memcpy(&_currLineFileOffset, _pageData + _currFileOffset, sizeof(_currLineFileOffset));
        _currFileOffset += sizeof(_currLineFileOffset);

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

    void ResultSetReader::getDataFilePath(std::string& path) const {
        path = _dataFilePath;
    }

    void ResultSetReader::restart() {
        _currFileOffset = 0;
        _resultCount = 0;
        _pageData = nullptr;
        _pageSize = 0;
        _currLineNum = 0;
        _currLineFileOffset = 0;
    }

    unsigned long long ResultSetReader::getLineNumber() const {
        return _currLineNum;
    }

    unsigned long long ResultSetReader::getLineFileOffset() const {
        return _currLineFileOffset;
    }
}