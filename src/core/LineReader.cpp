#include "LineReader.h"
#include "PagedReader.h"
#include "Utils.h"

#include <Windows.h>

namespace PLP {
    LineReader::~LineReader() {}

    bool LineReader::initialize(PagedReader& pager) {
        _pager = &pager;
        try {
            _pageBoundaryLineBuff.reserve(200);
        } catch (std::bad_alloc&) {
            return false;
        }
        
        return true;
    }

    bool LineReader::nextLine(char*& data, unsigned int& size) {
        bool loadNextPage = false;
        do {
            //load a new page if required
            if (!_pageData || _pageSize == 0 || _pageOffset >= _pageSize || loadNextPage) {
                _pageOffset = 0;
                _pageData = const_cast<char*>(_pager->read(_fileOffset, _pageSize));
                if (!_pageData || _pageSize == 0) {
                    return false;
                }
                loadNextPage = false;
            }

            char* lineStart = _pageData + _pageOffset;
            char* lineEnd = nullptr;

            // find line ending. If does not exist, the end is on the next page
            __try {
                lineEnd = (char*)findNextLineEnding(_pageData, _pageSize, _pageOffset);
            }
            __except (GetExceptionCode() == EXCEPTION_IN_PAGE_ERROR) {
                printf("Failed during line processing");
                return false;
            }

            // if this line does not end on EOF
            if (lineEnd == nullptr && (_pager->getFileSize() - _fileOffset) > (_pageSize - _pageOffset)) {
                loadNextPage = true;
                continue;
            } else if (lineEnd == nullptr && (_pager->getFileSize() - _fileOffset) == (_pageSize - _pageOffset)){ //process last line as usual
                lineEnd = _pageData + _pageSize - 1;
            }
                
            unsigned int length = (unsigned int)(lineEnd - lineStart + 1);
            _currentLineLength = length;
            _pageOffset += length;
            _fileOffset += length;
            _lineCount++;

            data = lineStart;
            size = length;

        } while (loadNextPage);

        return true;
    }

    bool LineReader::getLineUnverified(
        unsigned long long lineNum,
        unsigned long long fileOffset,
        char*& data,
        unsigned int& size
    ) {
        if (fileOffset >= _pager->getFileSize()) {
            return false;
        }

        _pageData = nullptr;
        _pageSize = 0;

        _lineCount = lineNum;
        _fileOffset = fileOffset;

        if (!nextLine(data, size)) {
            return false;
        }
        return true;
    }

    bool LineReader::appendPageBoundaryLineBuff(const char* data, unsigned int size) {
        unsigned int requiredSize = (unsigned int)_pageBoundaryLineBuff.size() + size;
        if (requiredSize > _pageBoundaryLineBuff.capacity()) {
            try {
                _pageBoundaryLineBuff.reserve(requiredSize * 2);
            } catch (std::bad_alloc&) {
                return false;
            }
        }

        _pageBoundaryLineBuff.insert(_pageBoundaryLineBuff.end(), data, data + size);
        return true;
    }

    void LineReader::resetPageBoundaryLineBuff() {
        _pageBoundaryLineBuff.clear();
    }

    unsigned long long LineReader::getLineNumber() {
        if (_lineCount == 0) {
            return 0;
        }
        return _lineCount - 1;
    }

    unsigned long long LineReader::getCurrentFileOffset() {
        return _fileOffset;
    }

    unsigned long long LineReader::getCurrentLineFileOffset() {
        return _fileOffset - _currentLineLength;
    }

    void LineReader::restart() {
        _pageData = nullptr;
        _pageSize = 0;
        _pageOffset = 0;
        _fileOffset = 0;
        _lineCount = 0;
        _currentLineLength = 0;
    }
}