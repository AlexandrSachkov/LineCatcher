#pragma once

#include <string>

namespace PLP {
    std::wstring string_to_wstring(const std::string& str);
    std::string wstring_to_string(const std::wstring& wstr);

    const char* findLastLineEnding(const char* buff, unsigned long long buffSize, const char* currPos);
    const char* findNextLineEnding(const char* buff, unsigned long long buffSize, unsigned long long startOffsetBytes);
}