#pragma once
#include "ReturnType.h"

#include <string>
#include <vector>
#include <locale>
#include <codecvt>
#include <cctype>

namespace PLP {
    static std::wstring string_to_wstring(const std::string& str) {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        return converter.from_bytes(str);
    }

    static std::string wstring_to_string(const std::wstring& wstr) {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        return converter.to_bytes(wstr);
    }

    static std::wstring getFileDirectory(const std::wstring& path) {
        size_t fileNamePos = path.find_last_of(L"/\\");
        if (fileNamePos == std::wstring::npos) { //relative path, same directory
            return L"";
        } else {
            return path.substr(0, fileNamePos + 1);
        }
    }

    static std::wstring getFileName(const std::wstring& path) {
        size_t fileNamePos = path.find_last_of(L"/\\");
        if (fileNamePos == std::wstring::npos) { //relative path, same directory
            return path;
        } else {
            return path.substr(fileNamePos + 1);
        }
    }

    static std::wstring getFileNameNoExt(const std::wstring& path) {
        std::wstring fileName = getFileName(path);
        size_t fileExtPos = fileName.find_last_of(L".");
        if (fileExtPos == std::wstring::npos) { //no file extension
            return fileName;
        } else {
            return fileName.substr(0, fileExtPos);
        }
    }

    const char* findLastLineEnding(const char* buff, unsigned long long buffSize, const char* currPos);
    LineReaderResult findNextLineEnding(
        const char* buff, 
        unsigned long long buffSize, 
        unsigned long long startOffsetBytes,
        unsigned int maxLineSize,
        char*& lineEnding
    );

    static void stringTrim(const char* str, unsigned int size, char*& strStartOut, unsigned int& sizeOut) {
        bool initialSeq = true;
        unsigned int numSpaces = 0;
        unsigned int startPos = 0;
        for (unsigned int i = 0; i < size; i++) {
            if (std::isspace(str[i])) {
                numSpaces++;
                continue;
            }

            if (initialSeq) {
                startPos = numSpaces;
            }

            numSpaces = 0;
            initialSeq = false;
        }

        if (numSpaces == size) {
            sizeOut = 0;
        } else {
            strStartOut = const_cast<char*>(str) + startPos;
            sizeOut = size - startPos - numSpaces;
        }
    }

    static std::string stringTrim(const std::string& str) {
        char* strStart = nullptr;
        unsigned int size = 0;
        stringTrim(str.data(), (unsigned int)str.length(), strStart, size);

        return std::string(strStart, size);
    }

    static void splitIntoWords(const char* str, unsigned int size, std::vector<std::pair<const char*, unsigned int>>& words) {
        words.clear();

        unsigned int numCharsInWord = 0;
        unsigned int wordStartPos = 0;
        for (unsigned int i = 0; i < size; i++) {
            if (!std::isspace(str[i])) {
                if (numCharsInWord == 0) {
                    wordStartPos = i;
                }
                
                numCharsInWord++;
            } else if (numCharsInWord > 0) {
                words.emplace_back(str + wordStartPos, numCharsInWord);
                numCharsInWord = 0;
            }
        }
    }

    static std::vector<std::string> splitIntoWords(const std::string& str) {
        std::vector<std::pair<const char*, unsigned int>> words;
        splitIntoWords(str.data(), (unsigned int)str.length(), words);

        std::vector<std::string> strWords;
        for (auto& pair : words) {
            strWords.emplace_back(pair.first, pair.second);
        }
        return strWords;
    }

    //https://docs.microsoft.com/en-us/previous-versions/windows/it-pro/windows-2000-server/cc938632(v=technet.10)
    const unsigned long long OPTIMAL_BLOCK_SIZE_BYTES = 64 * 1024; //64 KBytes 

    static const unsigned int RESULT_SET_VERSION = 1; // increment if format changes

    static const char* FILE_RANDOM_ACCESS_INDEX_EXTENSION = ".lcfraidx";
    static const char* FILE_INDEX_EXTENSION = ".lcidx";
}