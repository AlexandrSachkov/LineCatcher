#include "Utils.h"

#include <locale>
#include <codecvt>

namespace PLP {
    std::wstring string_to_wstring(const std::string& str) {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        return converter.from_bytes(str);
    }

    std::string wstring_to_string(const std::wstring& wstr) {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        return converter.to_bytes(wstr);
    }

    const char* findLastLineEnding(const char* buff, unsigned long long buffSize, const char* currPos) {
        if (currPos >= buff + buffSize) {
            return nullptr;
        }

        char* pos = const_cast<char*>(currPos);
        while (pos > buff && *pos != '\n') {
            pos--;
        }

        if (*pos != '\n' && pos == buff) {
            return nullptr;
        }

        return pos;
    }

    const char* findNextLineEnding(const char* buff, unsigned long long buffSize, unsigned long long startOffsetBytes) {
        if (startOffsetBytes >= buffSize) {
            return nullptr;
        }

        char* pos = const_cast<char*>(buff) + startOffsetBytes;
        while (pos < buff + buffSize && *pos != '\n') {
            pos++;
        }

        if (pos == buff + buffSize) {
            return nullptr;
        }

        return pos;
    }
}