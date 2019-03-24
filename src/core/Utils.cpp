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

    std::wstring getFileDirectory(const std::wstring& path) {
        size_t fileNamePos = path.find_last_of(L"/\\");
        if (fileNamePos == std::wstring::npos) { //relative path, same directory
            return L"";
        } else {
            return path.substr(0, fileNamePos + 1);
        }
    }

    std::wstring getFileName(const std::wstring& path) {
        size_t fileNamePos = path.find_last_of(L"/\\");
        if (fileNamePos == std::wstring::npos) { //relative path, same directory
            return path;
        } else {
            return path.substr(fileNamePos + 1);
        }
    }

    std::wstring getFileNameNoExt(const std::wstring& path) {
        std::wstring fileName = getFileName(path);
        size_t fileExtPos = fileName.find_last_of(L".");
        if (fileExtPos == std::wstring::npos) { //no file extension
            return fileName;
        } else {
            return fileName.substr(0, fileExtPos);
        }
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

    LineReaderResult findNextLineEnding(
        const char* buff, 
        unsigned long long buffSize, 
        unsigned long long startOffsetBytes, 
        unsigned int maxLineSize,
        char*& lineEnding
    ) {
        lineEnding = nullptr;
        if (startOffsetBytes >= buffSize) {
            return LineReaderResult::ERROR;
        }

        char* pos = const_cast<char*>(buff) + startOffsetBytes;
        while ((buff - pos  + 1) < maxLineSize 
            && pos < buff + buffSize 
            && *pos != '\n'
            ) {
            pos++;
        }

        if (buff - pos + 1 >= maxLineSize) {
            return LineReaderResult::ERROR;
        }

        if (pos == buff + buffSize) {
            return LineReaderResult::NOT_FOUND;
        }

        lineEnding = pos;
        return LineReaderResult::SUCCESS;
    }
}