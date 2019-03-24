#include "LineReader.h"
#include "PagedReader.h"
#include "Utils.h"
#include "Logger.h"

namespace PLP {
    LineReader::~LineReader() {
        _pageBoundaryLineBuff.reset();
    }

    bool LineReader::initialize(PagedReader& pager, unsigned int maxLineSize) {
        _pager = &pager;

        if (maxLineSize == 0) {
            Logger::send(ERR, "Maximum line size cannot be 0");
            return false;
        }
        _maxLineSize = maxLineSize;

        try {
            _pageBoundaryLineBuff.reset(new LineBuffer(maxLineSize));
        } catch (std::bad_alloc&) {
            Logger::send(ERR, "Page boundary line buffer failed to allocate");
            return false;
        }
        
        return true;
    }

    LineReaderResult LineReader::nextLine(char*& data, unsigned int& size) {
        data = nullptr;
        size = 0;

        bool loadNextPage = false;
        _pageBoundaryLineBuff->clear(); // prep for buffering next line

        char* lineStart = nullptr;
        char* lineEnd = nullptr;

        //__try {
            do {
                //load a new page if required
                if (!_pageData || _pageSize == 0 || _pageOffset >= _pageSize || loadNextPage) {
                    _pageOffset = 0;
                    _pageData = const_cast<char*>(_pager->read(_fileOffset, _pageSize));
                    if (!_pageData || _pageSize == 0) {
                        return LineReaderResult::NOT_FOUND;
                    }
                    loadNextPage = false;
                }

                lineStart = _pageData + _pageOffset;
                lineEnd = nullptr;

                // find line ending
                LineReaderResult result = findNextLineEnding(_pageData, _pageSize, _pageOffset, _maxLineSize, lineEnd);
                if (result == LineReaderResult::ERROR) {
                    return LineReaderResult::ERROR;
                }

                // if this line continues on the next page
                if (result == LineReaderResult::NOT_FOUND && (_pager->getFileSize() - _fileOffset) > (_pageSize - _pageOffset)) {
                    //can be converted to unsigned int because lineSegmentLength is smaller than _maxLineSize, and _maxLineSize is unsigned int
                    const unsigned int lineSegmentLength = (unsigned int)(_pageSize - _pageOffset);
                    if (!_pageBoundaryLineBuff->append(lineStart, lineSegmentLength)) {
                        Logger::send(ERR, "Line size exceeds maximum");
                        return LineReaderResult::ERROR;
                    }

                    _fileOffset += lineSegmentLength;
                    loadNextPage = true;
                }
            } while (loadNextPage);
        /*} __except (GetExceptionCode() == EXCEPTION_IN_PAGE_ERROR) {
            Logger::send(ERR, "Failed during line processing");
            return false;
        }*/


        // if this line ends on EOF
        if (lineEnd == nullptr && (_pager->getFileSize() - _fileOffset) == (_pageSize - _pageOffset)) {
            lineEnd = _pageData + _pageSize - 1;
        }

        //can be converted to unsigned int because lineSegmentLength is smaller than _maxLineSize, and _maxLineSize is unsigned int
        const unsigned int lineSegmentLength = (unsigned int)(lineEnd - lineStart + 1);
        if (_pageBoundaryLineBuff->getSize() > 0) { // line started on previous page
            if (!_pageBoundaryLineBuff->append(lineStart, lineSegmentLength)) {
                Logger::send(ERR, "Line size exceeds maximum");
                return LineReaderResult::ERROR;
            }

            _currentLineLength = _pageBoundaryLineBuff->getSize();
            _pageBoundaryLineBuff->get(data, size);
        } else {
            _currentLineLength = lineSegmentLength;
            data = lineStart;
            size = lineSegmentLength;
        }

        _pageOffset += lineSegmentLength;
        _fileOffset += lineSegmentLength;
        _lineCount++;

        return LineReaderResult::SUCCESS;
    }

    LineReaderResult LineReader::getLineUnverified(
        unsigned long long lineNum,
        unsigned long long fileOffset,
        char*& data,
        unsigned int& size
    ) {
        if (fileOffset >= _pager->getFileSize()) {
            return LineReaderResult::ERROR;
        }

        _pageData = nullptr;
        _pageSize = 0;

        _lineCount = lineNum;
        _fileOffset = fileOffset;

        return nextLine(data, size);
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