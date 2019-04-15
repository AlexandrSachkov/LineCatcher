#include "FStreamPagedReader.h"

#include "Utils.h"

namespace PLP {
    FStreamPagedReader::FStreamPagedReader() {}
    FStreamPagedReader::~FStreamPagedReader() {
        _ifs.close();
    }

    bool FStreamPagedReader::initialize(const std::wstring& path, unsigned long long preferredBuffSize/*, TaskRunner& asyncTaskRunner*/) {
        _filePath = path;
        //_asyncTaskRunner = &asyncTaskRunner;

        unsigned long long buffSize;
        if (preferredBuffSize < OPTIMAL_BLOCK_SIZE_BYTES) {
            buffSize = OPTIMAL_BLOCK_SIZE_BYTES;
        } else {
            buffSize = preferredBuffSize / OPTIMAL_BLOCK_SIZE_BYTES * OPTIMAL_BLOCK_SIZE_BYTES;
        }

        try {
            _buffer.resize(buffSize);
        } catch (std::bad_alloc&) {
            return false;
        }

        _ifs.open(path, std::ifstream::in | std::ifstream::binary);
        if (!_ifs.good()) {
            return false;
        }

        _ifs.seekg(0, _ifs.end);
        _fileSize = _ifs.tellg();
        _ifs.seekg(0, _ifs.beg);

        //buffer size must be a multiple of (OPTIMAL_BLOCK_SIZE_BYTES * 2) for efficiency
        /*unsigned long long buffSize = buffer.size();
        if (buffSize == 0 || (buffSize > 0 && buffSize % (OPTIMAL_BLOCK_SIZE_BYTES * 2) != 0)) {
            return false;
        }

        _ifs.open(path, std::ifstream::in | std::ifstream::binary);
        if (!_ifs.good()) {
            return false;
        }

        _ifs.seekg(0, _ifs.end);
        _fileSize = _ifs.tellg();
        _ifs.seekg(0, _ifs.beg);

        _pageSizeBytes = buffer.size() / 2;
        _backBuff = &buffer[0];
        _frontBuff = &buffer[0] + _pageSizeBytes;

        _preloadNextPage = [&]() {
            _currentPageLD.backtracked = false;

            unsigned long long fileOffset = _lastPageLD.fileOffsetBytes + _lastPageLD.buffStartOffsetBytes + _lastPageLD.buffSizeBytes;
            unsigned long long bytesTillEnd = _fileSize - fileOffset;
            if (0 == bytesTillEnd) {
                _currentPageLD.endOfFile = true;
                return;
            }
            
            unsigned long long bytesToRead = bytesTillEnd > _pageSizeBytes ? _pageSizeBytes : bytesTillEnd;

            _ifs.seekg(fileOffset);
            _ifs.read(_backBuff, bytesToRead);
            if (!_ifs.good()) {
                _currentPageLD.readErrorOccurred = true;
                return;
            }

            const char* lastLineEnd = findLastLineEnding(_backBuff, bytesToRead, _backBuff + bytesToRead - 1);
            unsigned long long usefulBytesRead = 0;
            if (lastLineEnd) {
                usefulBytesRead = lastLineEnd - _backBuff + 1;
            }

            _currentPageLD.fileOffsetBytes = fileOffset;
            _currentPageLD.buffStartOffsetBytes = 0;
            _currentPageLD.buffSizeBytes = usefulBytesRead;
            //_currentPageLD.buffSizeBytes = bytesToRead;
            _currentPageLD.endOfFile = bytesTillEnd <= _pageSizeBytes ? true : false;
        };

        //start preloading next page
        _asyncTaskRunner->runAsync(_preloadNextPage, _currPageLoadStatus);*/
        return true;
    }

    const char* FStreamPagedReader::read(unsigned long long fileOffset, unsigned long long& size) {
        size = 0;
        if (fileOffset >= _fileSize) {
            return nullptr;
        }

        const unsigned long long bytesTillEnd = _fileSize - fileOffset;
        const unsigned long long bytesToRead = bytesTillEnd > _buffer.size() ? _buffer.size() : bytesTillEnd;
        if (bytesToRead == 0) {
            return nullptr;
        }

        _ifs.seekg(fileOffset);
        _ifs.read(_buffer.data(), bytesToRead);
        if (!_ifs.good()) {
            return nullptr;
        }

        size = bytesToRead;
        return _buffer.data();
    }

    /*const char* FStreamPagedReader::getNextPage(unsigned long long& size) {
        if (!_lastPageLD.backtracked && (_lastPageLD.endOfFile || _lastPageLD.readErrorOccurred)) {
            size = 0;
            return nullptr;
        }

        timer.deltaT();
        while (!_currPageLoadStatus.isCompleted()) {
            //printf("Waiting for next page\n"); //TODO: remove and add a timer to measure this window
        }
        double numSeconds = timer.deltaT() / 1000000000;
        printf("Waited for page %f seconds\n", numSeconds);

        //swap buffers
        char* temp = _frontBuff;
        _frontBuff = _backBuff;
        _backBuff = temp;

        //move current loading info to last, and reset current
        _lastPageLD = _currentPageLD;
        _currentPageLD = LoadingData();

        //reset loading status
        _currPageLoadStatus = TaskStatus();

        if (_lastPageLD.readErrorOccurred) {
            size = 0;
            return nullptr;
        }

        //start preloading next page
        _asyncTaskRunner->runAsync(_preloadNextPage, _currPageLoadStatus);

        size = _lastPageLD.buffSizeBytes;
        return _frontBuff + _lastPageLD.buffStartOffsetBytes;

        return nullptr;
    }
    
     unsigned long long FStreamPagedReader::getCurrentPageFileOffset() {
        return _currentPageLD.fileOffsetBytes + _currentPageLD.buffStartOffsetBytes;
    }

    void FStreamPagedReader::resetToBeginning() {
        while (!_currPageLoadStatus.isCompleted()) {
        }

        _currentPageLD = LoadingData();
        _lastPageLD = LoadingData();
        _currPageLoadStatus = TaskStatus();

        _asyncTaskRunner->runAsync(_preloadNextPage, _currPageLoadStatus);
    }
    */

    unsigned long long FStreamPagedReader::getFileSize() {
        return _fileSize;
    }

    const std::wstring& FStreamPagedReader::getFilePath() {
        return _filePath;
    }
}