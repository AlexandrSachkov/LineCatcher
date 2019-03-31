#pragma once

#include <string>
#include "Utils.h"

namespace PLP {
    class MultilineSearchParams {
    public:
        MultilineSearchParams(int lineOffset, int wordIndex, bool matchWordIndex, const std::string& searchText, bool plainText, bool ignoreCase)
        : _lineOffset(lineOffset), _wordIndex(wordIndex), _matchWordIndex(matchWordIndex), 
            _searchText(searchText), _plainText(plainText), _ignoreCase(ignoreCase) {}

        MultilineSearchParams(int lineOffset, int wordIndex, bool matchWordIndex, const std::wstring& searchText, bool plainText, bool ignoreCase)
            : _lineOffset(lineOffset), _wordIndex(wordIndex), _matchWordIndex(matchWordIndex), 
            _searchText(wstring_to_string(searchText)), _plainText(plainText), _ignoreCase(ignoreCase) {}

        int getLineOffset() {
            return _lineOffset;
        }

        int getWordIndex() {
            return _wordIndex;
        }

        bool matchWordIndex() {
            return _matchWordIndex;
        }

        const std::string& getSearchText() {
            return _searchText;
        }

        bool plainText() {
            return _plainText;
        }

        bool ignoreCase() {
            return _ignoreCase;
        }

    private:
        int _lineOffset;
        int _wordIndex;
        bool _matchWordIndex;
        std::string _searchText;
        bool _plainText;
        bool _ignoreCase;
    };
}