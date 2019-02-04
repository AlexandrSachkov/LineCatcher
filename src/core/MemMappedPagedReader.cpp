#include "MemMappedPagedReader.h"

#include <Windows.h>

namespace PLP {
    MemMappedPagedReader::MemMappedPagedReader() {}
    MemMappedPagedReader::~MemMappedPagedReader() {
        if (_fileHandle) {
            CloseHandle(_fileHandle);
        }
    }

    bool MemMappedPagedReader::initialize(const std::wstring& path, unsigned long long preferredBuffSize) {
        _filePath = path;

        _fileHandle = CreateFileW(
            path.c_str(), 
            GENERIC_READ, 
            0, 
            NULL, 
            OPEN_EXISTING, 
            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, //https://blogs.msdn.microsoft.com/oldnewthing/20120120-00/?p=8493
            NULL
        );

        if (_fileHandle == INVALID_HANDLE_VALUE) {
            return false;
        }

        LARGE_INTEGER fileSize;
        if (!GetFileSizeEx(_fileHandle, &fileSize)) {
            return false;
        }
        _fileSize = fileSize.QuadPart;

        if (_fileSize == INVALID_FILE_SIZE) {
            return false;
        }

        _fileMappingHandle = CreateFileMappingA(
            _fileHandle,
            NULL,
            PAGE_READONLY,
            0,
            0,
            NULL
        );

        if (_fileMappingHandle == NULL) {
            return false;
        }

        unsigned long long unadjustedBuffSize;
        if (preferredBuffSize > 0) {
            if (preferredBuffSize <= MAX_PAGE_SIZE_BYTES && _fileSize <= preferredBuffSize) {
                unadjustedBuffSize = _fileSize;
            } else if (preferredBuffSize <= MAX_PAGE_SIZE_BYTES && _fileSize > preferredBuffSize) {
                unadjustedBuffSize = preferredBuffSize;
            } else {
                unadjustedBuffSize = MAX_PAGE_SIZE_BYTES;
            }
        } else {
            if (_fileSize <= MAX_PAGE_SIZE_BYTES) {
                unadjustedBuffSize = _fileSize;
            } else {
                unadjustedBuffSize = MAX_PAGE_SIZE_BYTES;
            }
        }

        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        _allocGranularity = sysInfo.dwAllocationGranularity;

        if (unadjustedBuffSize <= _allocGranularity) {
            _buffSize = _allocGranularity;
        } else {
            _buffSize = unadjustedBuffSize / _allocGranularity * _allocGranularity;
        }

        return true;
    }

    const char* MemMappedPagedReader::read(unsigned long long fileOffset, unsigned long long& size) {
        size = 0;

        if (fileOffset >= _fileSize) {
            return nullptr;
        }

        if (_data) {
            bool status = UnmapViewOfFile(_data);
            DWORD err = GetLastError();
        }

        unsigned long long alignedOffset = fileOffset / _allocGranularity * _allocGranularity;
        unsigned long long bytesTillEnd = _fileSize - alignedOffset;
        unsigned long long bytesToRead = bytesTillEnd > _buffSize ? _buffSize : bytesTillEnd;
        DWORD high = static_cast<DWORD>((alignedOffset >> 32) & 0xFFFFFFFFul);
        DWORD low = static_cast<DWORD>(alignedOffset & 0xFFFFFFFFul);

        _data = MapViewOfFile(
            _fileMappingHandle,
            FILE_MAP_READ,
            high,
            low,
            bytesToRead
        );

        if (_data == nullptr) {
            DWORD err = GetLastError();
            printf("Failed during mapping: %i", err);
            return nullptr;
        }

        unsigned long long deltaOffset = fileOffset - alignedOffset;
        size = bytesToRead - deltaOffset;
        return (const char*)_data + deltaOffset;
    }

    unsigned long long MemMappedPagedReader::getFileSize() {
        return _fileSize;
    }

    std::wstring MemMappedPagedReader::getFilePath() {
        return _filePath;
    }
}