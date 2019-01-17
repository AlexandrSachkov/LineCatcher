#include "PagedWriter.h"
#include "Utils.h"

namespace PLP {
    PagedWriter::PagedWriter() {}
    PagedWriter::~PagedWriter() {
        _ofs.close();
    }

    bool PagedWriter::initialize(const std::wstring& path, std::vector<char>& buffer, TaskRunner& asyncTaskRunner) {
        _buffer = &buffer;
        _asyncTaskRunner = &asyncTaskRunner;

        //buffer size must be a multiple of (OPTIMAL_BLOCK_SIZE_BYTES * 2) for efficiency
        unsigned long long buffSize = buffer.size();
        if (buffSize == 0 || (buffSize > 0 && buffSize % (OPTIMAL_BLOCK_SIZE_BYTES * 2) != 0)) {
            return false;
        }

        _pageSizeBytes = buffer.size() / 2;
        _backBuff = &buffer[0];
        _frontBuff = &buffer[0] + _pageSizeBytes;
        _frontBuffContentSize = 0;

        _ofs.open(path, std::ofstream::app | std::ofstream::binary);
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

    bool PagedWriter::write(const char* data, unsigned long long size) {
        if (!data || size == 0 || _writeError) {
            return false;
        }

        unsigned long long bytesLeftToWrite = size;
        while (bytesLeftToWrite > 0) {
            unsigned long long bytesFree = _pageSizeBytes - _frontBuffContentSize;
            unsigned long long bytesToWrite = bytesFree > bytesLeftToWrite ? bytesLeftToWrite : bytesFree;

            if (bytesToWrite > 0) {
                char* writeLocation = _frontBuff + _frontBuffContentSize;
                memcpy(writeLocation, data, bytesToWrite);
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

    bool PagedWriter::flush() {
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

    void PagedWriter::swapBuffers() {
        char* temp = _frontBuff;
        _frontBuff = _backBuff;
        _backBuff = temp;
    }
}