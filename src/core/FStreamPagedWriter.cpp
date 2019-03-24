#include "FStreamPagedWriter.h"
#include "Utils.h"

namespace PLP {
    FStreamPagedWriter::FStreamPagedWriter() {}
    FStreamPagedWriter::~FStreamPagedWriter() {
        flush();
        _ofs.close();
    }

    bool FStreamPagedWriter::initialize(
        const std::wstring& path, 
        unsigned long long preferredBuffSize, 
        bool overwriteIfExists, 
        TaskRunner& asyncTaskRunner) {
        _asyncTaskRunner = &asyncTaskRunner;

        unsigned long long buffSize = preferredBuffSize / (OPTIMAL_BLOCK_SIZE_BYTES * 2) * (OPTIMAL_BLOCK_SIZE_BYTES * 2);
        if (buffSize == 0) {
            buffSize = OPTIMAL_BLOCK_SIZE_BYTES * 2;
        }

        try {
            _buffer.resize(buffSize, 0);
        } catch (std::bad_alloc&) {
            return false;
        }

        _pageSizeBytes = _buffer.size() / 2;
        _backBuff = &_buffer[0];
        _frontBuff = &_buffer[0] + _pageSizeBytes;
        _frontBuffContentSize = 0;

        _ofs.open(path, std::fstream::in | std::fstream::binary); //check file existence
        if (_ofs.good() && !overwriteIfExists) {
            _ofs.close();
            return false;
        }

        _ofs.close();
        _ofs.open(path, std::fstream::out | std::fstream::binary);
        if (!_ofs.good()) {
            return false;
        }
        _ofs.setf(std::ofstream::unitbuf);

        _writeBackBuff = [&]() {
            _ofs.write(_backBuff, _pageSizeBytes);
            if (!_ofs.good()) {
                _writeError = true;
                return;
            }
        };

        return true;
    }

    bool FStreamPagedWriter::write(const char* data, unsigned long long size) {
        if (!data || size == 0 || _writeError) {
            return false;
        }

        unsigned long long bytesLeftToWrite = size;
        while (bytesLeftToWrite > 0) {
            unsigned long long bytesFree = _pageSizeBytes - _frontBuffContentSize;
            unsigned long long bytesToWrite = bytesFree > bytesLeftToWrite ? bytesLeftToWrite : bytesFree;

            if (bytesToWrite > 0) {
                char* writeLocation = _frontBuff + _frontBuffContentSize;
                memcpy(writeLocation, data + (size - bytesLeftToWrite), bytesToWrite);
                _frontBuffContentSize += bytesToWrite;
            }

            if (_frontBuffContentSize == _pageSizeBytes) {
                while (!_backBuffLoadStatus.isCompleted()) {} // wait for the back buffer to save

                swapBuffers();

                _backBuffLoadStatus = TaskStatus();
                _frontBuffContentSize = 0;

                _asyncTaskRunner->runAsync(_writeBackBuff, _backBuffLoadStatus);
            }

            bytesLeftToWrite = bytesToWrite < bytesLeftToWrite ? bytesLeftToWrite - bytesToWrite : 0;
        }

        return true;
    }

    bool FStreamPagedWriter::flush() {
        if (_writeError) {
            return false;
        }

        if (_frontBuffContentSize > 0) {
            while (!_backBuffLoadStatus.isCompleted()) {} // wait for the back buffer to save
            _backBuffLoadStatus = TaskStatus();
            
            _ofs.write(_frontBuff, _frontBuffContentSize);
            if (!_ofs.good()) {
                _writeError = true;
                return false;
            }
            _frontBuffContentSize = 0;
        }
        return true;
    }

    void FStreamPagedWriter::swapBuffers() {
        char* temp = _frontBuff;
        _frontBuff = _backBuff;
        _backBuff = temp;
    }

    bool FStreamPagedWriter::setPosition(unsigned long long fileOffset) {
        if (!flush()) {
            return false;
        }

        unsigned long long currFileOffset = _ofs.tellg();
        _ofs.seekg(0, std::ios::end);
        unsigned long long fileSize = _ofs.tellg();

        if (fileOffset > fileSize) {
            _ofs.seekg(currFileOffset);
            return false;
        }
        _ofs.seekg(fileOffset);
        return true;
    }

    bool FStreamPagedWriter::setPositionEnd() {
        if (!flush()) {
            return false;
        }
        _ofs.seekg(0, std::ios::end);
        return true;
    }
}