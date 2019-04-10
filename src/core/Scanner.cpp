#include "Scanner.h"

#include <cstdlib>

namespace PLP {
    LineScanner::LineScanner(FileReaderI* fileReader, ResultSetReaderI* indexReader, unsigned long long startLine, unsigned long long endLine)
        : _fileReader(fileReader), _indexReader(indexReader), _startLine(startLine), _endLine(endLine) {
    }

    bool LineScanner::initialize() {
        if (nullptr == _fileReader) {
            return false;
        }

        if (_endLine == 0) {
            _endLine = _fileReader->getNumberOfLines() - 1;
        } else if (_startLine > _endLine) {
            return false;
        }

        if (_startLine >= _fileReader->getNumberOfLines() || _endLine >= _fileReader->getNumberOfLines()) {
            return false;
        }

        if (_indexReader) {
            _nextLine = [&](unsigned long long& lineNum, unsigned long long& fileOffset, char*& data, unsigned int& size) {
                if (_firstLine) {
                    do {
                        if (!_indexReader->nextResult(lineNum)) {
                            return LineReaderResult::NOT_FOUND;
                        }
                    } while (_indexReader->getLineNumber() < _startLine);

                    if (_indexReader->getLineNumber() >= _endLine) {
                        return LineReaderResult::NOT_FOUND;
                    }

                    LineReaderResult res = _fileReader->getLineFromResult(_indexReader, data, size);
                    if (res != LineReaderResult::SUCCESS) {
                        return res;
                    }

                    fileOffset = _fileReader->getLineFileOffset();
                    _firstLine = false;
                } else {
                    if (!_indexReader->nextResult(lineNum)) {
                        return LineReaderResult::NOT_FOUND;
                    }

                    if (_indexReader->getLineNumber() >= _endLine) {
                        return LineReaderResult::NOT_FOUND;
                    }

                    LineReaderResult res = _fileReader->getLineFromResult(_indexReader, data, size);
                    if (res != LineReaderResult::SUCCESS) {
                        return res;
                    }

                    fileOffset = _fileReader->getLineFileOffset();
                }
                return LineReaderResult::SUCCESS;
            };
        } else {
            _nextLine = [&](unsigned long long& lineNum, unsigned long long& fileOffset, char*& data, unsigned int& size) {
                if (_firstLine && _startLine > 0) {
                    LineReaderResult res = _fileReader->getLine(_startLine, data, size);
                    if (res != LineReaderResult::SUCCESS) {
                        return res;
                    }

                    lineNum = _startLine;
                    fileOffset = _fileReader->getLineFileOffset();
                    _firstLine = false;
                } else {
                    if (_fileReader->getLineNumber() >= _endLine) {
                        return LineReaderResult::NOT_FOUND;
                    }

                    LineReaderResult res = _fileReader->nextLine(data, size);
                    if (res != LineReaderResult::SUCCESS) {
                        return res;
                    }
                    lineNum = _fileReader->getLineNumber();
                    fileOffset = _fileReader->getLineFileOffset();
                }
                return LineReaderResult::SUCCESS;
            };
        }

        return true;
    }

    LineReaderResult LineScanner::nextLine(unsigned long long& lineNum, unsigned long long& fileOffset, char*& data, unsigned int& size) {
        return _nextLine(lineNum, fileOffset, data, size);
    }

    std::tuple<int, unsigned long long, std::string> LineScanner::nextLine() {
        unsigned long long lineNum = 0;
        unsigned long long fileOffset = 0;
        char* lineStart = nullptr;
        unsigned int length = 0;

        LineReaderResult result = _nextLine(lineNum, fileOffset, lineStart, length);
        if (result != LineReaderResult::SUCCESS) {
            return { result, lineNum, std::string() };
        }
        return { result, lineNum, std::string(lineStart, length) };
    }

    MultilineScanner::MultilineScanner(
        FileReaderI* fileReader,
        ResultSetReaderI* indexReader,
        int startLineOffset,
        int endLineOffset,
        unsigned long long startLine,
        unsigned long long endLine
    ) : _fileReader(fileReader), _indexReader(indexReader), _startLineOffset(startLineOffset),
        _endLineOffset(endLineOffset), _startLine(startLine), _endLine(endLine) {
    }

    bool MultilineScanner::initialize() {
        if (nullptr == _fileReader) {
            return false;
        }

        if (_endLine == 0) {
            _endLine = _fileReader->getNumberOfLines() - 1;
        } else if (_startLine > _endLine) {
            return false;
        }

        if (_startLineOffset > _endLineOffset) {
            return false;
        }

        if (_startLineOffset < 0 && _endLineOffset < 0) {
            _endLineOffset = 0;
        } else if (_startLineOffset > 0 && _endLineOffset > 0) {
            _startLineOffset = 0;
        } else if (_startLineOffset < 0 && _endLineOffset > 0) {
            if (abs(_startLineOffset) > _endLineOffset) {
                _endLineOffset = -_startLineOffset;
            } else if (abs(_startLineOffset) < _endLineOffset) {
                _startLineOffset = -_endLineOffset;
            }
        }

        if (_startLine >= _fileReader->getNumberOfLines() || _endLine >= _fileReader->getNumberOfLines()) {
            return false;
        }

        if (!_frameBuff.initialize(_startLineOffset, _endLineOffset, 100000)) {
            return false;
        }

        if (_indexReader) {
            _nextFrame = [&]() {
                if (_firstLine) {
                    unsigned long long refLineNum;
                    do {
                        if (!_indexReader->nextResult(refLineNum)) {
                            return LineReaderResult::NOT_FOUND;
                        }
                    } while (refLineNum < _startLine);

                    if (refLineNum > _endLine) {
                        return LineReaderResult::NOT_FOUND;
                    }

                    LineReaderResult res;
                    res = loadFrame(refLineNum);
                    if (res != LineReaderResult::SUCCESS) {
                        return res;
                    }
                } else {
                    unsigned long long refLineNum;
                    if (!_indexReader->nextResult(refLineNum)) {
                        return LineReaderResult::NOT_FOUND;
                    }

                    if (refLineNum > _endLine) {
                        return LineReaderResult::NOT_FOUND;
                    }

                    LineReaderResult res;
                    res = loadFrame(refLineNum);
                    if (res != LineReaderResult::SUCCESS) {
                        return res;
                    }
                }

                return LineReaderResult::SUCCESS;
            };
            return true;
        }

        _nextFrame = [&]() {
            if (_firstLine) {
                LineReaderResult res;
                res = loadFrame(_startLine);
                if (res != LineReaderResult::SUCCESS) {
                    return res;
                }

                _firstLine = false;

            } else {
                unsigned long long lineNum;
                unsigned long long fileOffset;
                char* data;
                unsigned int size;
                _frameBuff.get(0, lineNum, fileOffset, data, size);

                if (lineNum < _fileReader->getNumberOfLines() - 1) {
                    LineReaderResult res;
                    if ((res = _fileReader->nextLine(data, size)) != LineReaderResult::SUCCESS) {
                        return res;
                    }
                    _frameBuff.pushBack(_fileReader->getLineNumber(), _fileReader->getLineFileOffset(), data, size);
                } else {
                    return LineReaderResult::NOT_FOUND;
                }
            }
            return LineReaderResult::SUCCESS;
        };
        return true;
    }

    LineReaderResult MultilineScanner::loadFrame(unsigned long long refLine) {
        unsigned long long startLine;
        if (refLine >= _startLineOffset) {
            startLine = refLine - _startLineOffset;
        } else {
            startLine = 0;
        }

        unsigned long long endLine = refLine + _endLineOffset;
        unsigned int numLinesOverhang = 0;
        if (endLine > _fileReader->getNumberOfLines() - 1) {
            numLinesOverhang = (unsigned int)(endLine - _fileReader->getNumberOfLines() + 1);
        }
        unsigned int numLinesToRead = (unsigned int)(endLine - startLine + 1 - numLinesOverhang);

        char* data;
        unsigned int size;
        LineReaderResult res;
        if ((res = _fileReader->getLine(startLine, data, size)) != LineReaderResult::SUCCESS) {
            return res;
        }
        _frameBuff.pushBack(startLine, _fileReader->getLineFileOffset(), data, size);
        numLinesToRead--;

        while (numLinesToRead > 0) {
            if ((res = _fileReader->nextLine(data, size)) != LineReaderResult::SUCCESS) {
                return res;
            }
            _frameBuff.pushBack(_fileReader->getLineNumber(), _fileReader->getLineFileOffset(), data, size);
            numLinesToRead--;
        }

        while (numLinesOverhang > 0) {
            _frameBuff.pushBack(0, 0, "", 0);
            numLinesOverhang--;
        }

        return LineReaderResult::SUCCESS;
    }

    LineReaderResult MultilineScanner::nextFrame() {
        return _nextFrame();
    }

    bool MultilineScanner::getLine(int index, unsigned long long& lineNum, unsigned long long& fileOffset, char*& data, unsigned int& size) {
        return _frameBuff.get(index, lineNum, fileOffset, data, size);
    }
}