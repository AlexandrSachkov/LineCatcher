#include "IndexedLineReader.h"
#include "Utils.h"
#include "Core.h"
#include "Logger.h"

#include "cereal/types/unordered_map.hpp"
#include "cereal/types/string.hpp"
#include "cereal/archives/binary.hpp"

#include <fstream>

namespace PLP {
    IndexedLineReader::IndexedLineReader() {}
    IndexedLineReader::~IndexedLineReader() {}

    bool IndexedLineReader::initialize(
        PagedReader& pagedReader, 
        unsigned int maxLineSize, 
        const std::atomic<bool>& cancelled, 
        const std::function<void(int percent)>* progressUpdate
    ) {
        if (!LineReader::initialize(pagedReader, maxLineSize)) {
            return false;
        }

        std::wstring indexPath = getIndexFilePath(pagedReader.getFilePath());
        (*progressUpdate)(0);
        if (!loadIndex(indexPath)) {
            if (!generateIndex(pagedReader.getFilePath(), indexPath, cancelled, pagedReader.getFileSize(), progressUpdate)) {
                return false;
            }
        } else {
            (*progressUpdate)(100);
        }

        return true;
    }

    std::wstring IndexedLineReader::getIndexFilePath(const std::wstring& dataFilePath) {
        std::wstring directory = getFileDirectory(dataFilePath);
        std::wstring fileNameNoExt = getFileNameNoExt(dataFilePath);
        return directory + fileNameNoExt + L".plpidx";
    }

    bool IndexedLineReader::loadIndex(const std::wstring& indexPath) {
        std::ifstream fs;
        fs.open(indexPath, std::fstream::in | std::fstream::binary);
        if (!fs.good()) {
            return false;
        }

        cereal::BinaryInputArchive iarchive(fs);

        unsigned int indexVersion = 0;
        iarchive(indexVersion);
        if (indexVersion != INDEX_VERSION) { // might have a different format. TODO: can we handle legacy formats?
            return false;
        }

        iarchive(_indexHeader, _fileIndex);
        return true;
    }

    bool IndexedLineReader::generateIndex(
        const std::wstring& dataFilePath, 
        const std::wstring& indexPath, 
        const std::atomic<bool>& cancelled,
        const unsigned long long fileSize,
        const std::function<void(int percent)>* progressUpdate
    ) {
        char* lineStart;
        unsigned int length;
        unsigned long long lineStartFileOffset = 0;
        unsigned long long numLines = 0;

        const long double dBytesPerPercent = (fileSize) / 100.0;
        const unsigned long long numBytesTillProgressUpdate = dBytesPerPercent > 1.0 ? (unsigned long long)dBytesPerPercent : 1;
        const int percentPerProgressUpdate = dBytesPerPercent > 1.0 ? 1 : (int)(1.0 / dBytesPerPercent);

        int progressPercent = 0;

        LineReaderResult result;
        while ((result = nextLine(lineStart, length)) == LineReaderResult::SUCCESS) {
            if (getLineNumber() % 10000000 == 0 && cancelled) {
                return false;
            }

            if (getLineNumber() > 0 && getLineNumber() % LINE_INDEX_FREQUENCY == 0) {
                _fileIndex.insert({ getLineNumber(), lineStartFileOffset });
            }

            lineStartFileOffset = getCurrentFileOffset();
            numLines++;

            if (lineStartFileOffset % numBytesTillProgressUpdate == 0) {
                progressPercent += percentPerProgressUpdate;
                (*progressUpdate)(progressPercent);
            }
        }
        if (result == LineReaderResult::ERROR) {
            return false;
        }

        restart();

        std::ofstream fs;
        fs.open(indexPath, std::fstream::out | std::fstream::binary);
        if (!fs.good()) {
            return false;
        }

        _indexHeader.filePath = wstring_to_string(dataFilePath);
        _indexHeader.lineIndexFreq = LINE_INDEX_FREQUENCY;
        _indexHeader.numLines = numLines;

        cereal::BinaryOutputArchive oarchive(fs);
        oarchive(INDEX_VERSION, _indexHeader, _fileIndex);
        fs.close();

        return true;
    }

    LineReaderResult IndexedLineReader::getLine(unsigned long long lineNumber, char*& data, unsigned int& size) {
        if (lineNumber < 0 || lineNumber > _indexHeader.numLines - 1) {
            return LineReaderResult::ERROR;
        }

        char* lineData = nullptr;
        unsigned int length = 0;

        unsigned long long prevIndexedLineNum = lineNumber / _indexHeader.lineIndexFreq * _indexHeader.lineIndexFreq;
        if (lineNumber <= getLineNumber() || prevIndexedLineNum > getLineNumber()) {
            unsigned long long prevIndexedLineFileOffset;
            if (prevIndexedLineNum == 0) {
                prevIndexedLineFileOffset = 0;
            } else {
                auto it = _fileIndex.find(prevIndexedLineNum);
                if (it == _fileIndex.end()) {
                    Logger::send(ERR, "Failed to find nearest known file location");
                    return LineReaderResult::ERROR;
                }
                prevIndexedLineFileOffset = it->second;
            }

            LineReaderResult result = getLineUnverified(prevIndexedLineNum, prevIndexedLineFileOffset, lineData, length);
            if (result != LineReaderResult::SUCCESS) {
                return result;
            }
        }

        while (getLineNumber() < lineNumber) {
            if (LineReaderResult::SUCCESS != nextLine(lineData, length)) {
                return LineReaderResult::ERROR;
            }
        }

        data = lineData;
        size = length;

        return LineReaderResult::SUCCESS;
    }

    unsigned long long IndexedLineReader::getNumberOfLines() {
        return _indexHeader.numLines;
    }
}