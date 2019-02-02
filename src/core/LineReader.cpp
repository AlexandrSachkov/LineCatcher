#include "LineReader.h"
#include "Utils.h"

namespace PLP {
    LineReader::LineReader() {}
    LineReader::~LineReader() {}

    bool LineReader::initialize(PagedReader& pager) {
        _pager = &pager;
        return true;
    }

    bool LineReader::nextLine(char*& lineStart, unsigned int& length) {
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

    unsigned long long LineReader::getLineNumber() {
        return _lineNum;
    }

    unsigned long long LineReader::getCurrentFileOffset() {
        return _pager->getCurrentPageFileOffset() + _pageOffset;
    }

    void LineReader::resetToBeginning() {
        _pager->resetToBeginning();

        _pageData = nullptr;
        _pageSize = 0;
        _pageOffset = 0;

        _numPages = 0;
        _lineNum = 0;
    }
}