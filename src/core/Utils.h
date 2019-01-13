#pragma once

#include <string>

namespace PLP {
    std::wstring string_to_wstring(const std::string& str);
    std::string wstring_to_string(const std::wstring& wstr);

    const char* findLastLineEnding(const char* buff, size_t buffSize, const char* currPos);
    const char* findNextLineEnding(const char* buff, size_t buffSize, size_t startOffsetBytes);
}