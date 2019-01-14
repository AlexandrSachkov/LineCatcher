#include "LineParser.h"
#include "Utils.h"

namespace PLP {
    LineParser::LineParser() {}
    LineParser::~LineParser() {}

    bool LineParser::initialize(FilePager& pager) {
        _pager = &pager;
        return true;
    }

    bool LineParser::nextLine(char*& lineStart, unsigned int& length) {
        if (!_pageData || _pageSize == 0 || _pageOffset == _pageSize) {
            _pageOffset = 0;
            unsigned long long size;
            char* data = const_cast<char*>(_pager->getNextPage(size));

            if (!data || size == 0) {
                return false;
            }
            _pageData = data;
            _pageSize = size;
            _numPages++;

            printf("Page loaded: %llu\n", _numPages);
            printf("%llu\n", _lineNum);
        }

        char* lineEnd = (char*)findNextLineEnding(_pageData, _pageSize, _pageOffset);
        if (lineEnd == nullptr) {
            return false;
        }

        lineStart = _pageData + _pageOffset;
        length = (unsigned int)(lineEnd - lineStart);

        _pageOffset = lineEnd - _pageData + 1;
        _lineNum++;

        return true;
    }

    unsigned long long LineParser::getLineNumber() {
        return _lineNum;
    }
}