#pragma once
#include "ReturnType.h"

#include <string>

namespace PLP {
    std::wstring string_to_wstring(const std::string& str);
    std::string wstring_to_string(const std::wstring& wstr);

    std::wstring getFileDirectory(const std::wstring& path);
    std::wstring getFileName(const std::wstring& path);
    std::wstring getFileNameNoExt(const std::wstring& path);

    const char* findLastLineEnding(const char* buff, unsigned long long buffSize, const char* currPos);
    LineReaderResult findNextLineEnding(
        const char* buff, 
        unsigned long long buffSize, 
        unsigned long long startOffsetBytes,
        unsigned int maxLineSize,
        char*& lineEnding
    );

    //https://docs.microsoft.com/en-us/previous-versions/windows/it-pro/windows-2000-server/cc938632(v=technet.10)
    const unsigned long long OPTIMAL_BLOCK_SIZE_BYTES = 64 * 1024; //64 KBytes 

    static const unsigned int RESULT_SET_VERSION = 1; // increment if format changes
}