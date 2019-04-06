#pragma once
#include "ReturnType.h"

#include <string>
#include <vector>

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

    static void stringTrim(const char* str, unsigned int size, char*& strStartOut, unsigned int& sizeOut) {
        bool initialSeq = true;
        unsigned int numSpaces = 0;
        unsigned int startPos = 0;
        for (unsigned int i = 0; i < size; i++) {
            if (str[i] == ' ') {
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

    static void splitIntoWords(const char* str, unsigned int size, std::vector<std::pair<const char*, unsigned int>>& words) {
        words.clear();

        unsigned int numCharsInWord = 0;
        unsigned int wordStartPos = 0;
        for (unsigned int i = 0; i < size; i++) {
            if (str[i] != ' ') {
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

    //https://docs.microsoft.com/en-us/previous-versions/windows/it-pro/windows-2000-server/cc938632(v=technet.10)
    const unsigned long long OPTIMAL_BLOCK_SIZE_BYTES = 64 * 1024; //64 KBytes 

    static const unsigned int RESULT_SET_VERSION = 1; // increment if format changes
}