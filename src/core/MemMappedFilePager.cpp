#include "MemMappedFilePager.h"

#include "Utils.h"

namespace PLP {
    MemMappedFilePager::MemMappedFilePager() {}
    MemMappedFilePager::~MemMappedFilePager() {
        if (_fileHandle) {
            CloseHandle(_fileHandle);
        }
    }

    bool MemMappedFilePager::initialize(const std::wstring& path, unsigned long long buffSize, TaskRunner& asyncTaskRunner) {
        _asyncTaskRunner = &asyncTaskRunner;
        _fileHandle = CreateFileW(
            path.c_str(), 
            GENERIC_READ, 
            0, 
            NULL, 
            OPEN_EXISTING, 
            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, //TODO try sequential access to see if it is faster
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
            PAGE_READONLY, //try SEC_LARGE_PAGES for performance
            0,
            0,
            NULL
        );

        if (_fileMappingHandle == NULL) {
            return false;
        }

        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        _allocGranularity = sysInfo.dwAllocationGranularity;
        _buffSize = (unsigned long long)buffSize / _allocGranularity * _allocGranularity;

        _fileOffset = 0;

        return true;
    }

    const char* MemMappedFilePager::getNextPage(size_t& size) {
        if (_data) {
            bool status = UnmapViewOfFile(_data);
            bool zzz = true;
        }

        unsigned long long bytesTillEnd = _fileSize - _fileOffset;
        if (0 == bytesTillEnd) {
            size = 0;
            return nullptr;
        }

        unsigned long long bytesToRead = bytesTillEnd > _buffSize ? _buffSize : bytesTillEnd;
        DWORD high = static_cast<DWORD>((_fileOffset >> 32) & 0xFFFFFFFFul);
        DWORD low = static_cast<DWORD>(_fileOffset & 0xFFFFFFFFul);
        _data = MapViewOfFile(
            _fileMappingHandle,
            FILE_MAP_READ,
            high,
            low,
            bytesToRead
        );
        DWORD err = GetLastError();

        if (_data == NULL) {
            size = 0;
            return nullptr;
        }

        _fileOffset += bytesToRead;
        
        size = bytesToRead;
        return (const char*)_data;
    }

    const char* MemMappedFilePager::getPreviousPage(size_t& size) {
        return nullptr;
    }

    void MemMappedFilePager::swapBuffers() {

    }

    void MemMappedFilePager::preloadNextPage() {

    }

    void MemMappedFilePager::preloadPreviousPage() {

    }
}