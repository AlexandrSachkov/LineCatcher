#include "IndexWriter.h"
#include "FStreamPagedWriter.h"
#include "Utils.h"
#include "FileReader.h"
#include "TaskRunner.h"
#include "Logger.h"
#include "GenFileTracker.h"

namespace PLP {
    IndexWriter::IndexWriter() {}
    IndexWriter::~IndexWriter() {
        Logger::send(INFO, "Releasing index writer");
        release();
    }

    bool IndexWriter::initialize(
        const std::wstring& path,
        const std::wstring& dataFilePath,
        unsigned long long preferredBufferSizeBytes,
        bool overwriteIfExists,
        TaskRunner& asyncTaskRunner
    ) {
        release();

        std::wstring unixPath = windowsToUnixPath(path);
        std::wstring directory = getFileDirectory(unixPath);
        std::wstring fileName = getFileName(unixPath);
        if (std::wstring::npos == fileName.find(string_to_wstring(FILE_INDEX_EXTENSION))) {
            fileName += string_to_wstring(FILE_INDEX_EXTENSION);
        }
        std::wstring indexPath = directory + fileName;

        FileScopedLock writingLock = FileScopedLock::lockForWriting(indexPath);
        if (!writingLock.isLocked()) {
            Logger::send(ERR, "Unable to acquare a write lock on file " + wstring_to_string(indexPath) + ". File is in use");
            return false;
        }

        _dataFilePath = wstring_to_string(dataFilePath);

        FStreamPagedWriter* writer = new FStreamPagedWriter();
        _writer.reset(writer);
        if (!writer->initialize(indexPath, preferredBufferSizeBytes, overwriteIfExists, asyncTaskRunner)) {
            return false;
        }

        LC::GenFileTracker::addFile(indexPath);

        if (!_writer->write(reinterpret_cast<const char*>(&RESULT_SET_VERSION), sizeof(RESULT_SET_VERSION))) {
            return false;
        }
        unsigned int dataFilePathLength = static_cast<unsigned int>(_dataFilePath.length());
        if (!_writer->write(reinterpret_cast<const char*>(&dataFilePathLength), sizeof(dataFilePathLength))) {
            return false;
        }
        if (!_writer->write(_dataFilePath.c_str(), dataFilePathLength)) {
            return false;
        }
        if (!_writer->write(reinterpret_cast<const char*>(&_resultCount), sizeof(_resultCount))) {
            return false;
        }
        if (!_writer->flush()) {
            return false;
        }

        _writingLock = std::move(writingLock);
        return true;
    }

    void IndexWriter::release() {
        if (_writer) {
            flush();
        }
        _writer = nullptr;
        _dataFilePath = "";
        _prevLineNum = 0;
        _resultCount = 0;
    }

    bool IndexWriter::appendCurrLine(const FileReaderI* fReader) {
        if (!fReader) {
            return false;
        }

        return appendCurrLine(fReader->getLineNumber(), fReader->getLineFileOffset());
    }

    bool IndexWriter::appendCurrLine(const std::shared_ptr<FileReader> fReader) {
        return appendCurrLine(fReader.get());
    }

    bool IndexWriter::appendCurrLine(unsigned long long lineNumber, unsigned long long fileOffset) {
        if (lineNumber == _prevLineNum && _resultCount > 0) {
            Logger::send(ERR, "Indexes must be consecutive");
            return false;
        }

        if (!_writer->write(reinterpret_cast<const char*>(&lineNumber), sizeof(unsigned long long))) {
            return false;
        }
        if (!_writer->write(reinterpret_cast<const char*>(&fileOffset), sizeof(unsigned long long))) {
            return false;
        }

        _resultCount++;
        _prevLineNum = lineNumber;
        return true;
    }

    bool IndexWriter::flush() {
        if (!_writer->flush()) {
            return false;
        }
        return updateResultCount();
    }

    unsigned long long IndexWriter::getNumResults() const {
        return _resultCount;
    }

    bool IndexWriter::updateResultCount() {
        unsigned long long resultCountFileOffset =
            sizeof(RESULT_SET_VERSION) +
            sizeof(unsigned int) + //length of data file path
            _dataFilePath.length();

        if (!_writer->setPosition(resultCountFileOffset)) {
            return false;
        }
        if (!_writer->write(reinterpret_cast<const char*>(&_resultCount), sizeof(_resultCount))) {
            return false;
        }
        if (!_writer->setPositionEnd()) {
            return false;
        }
        if (!_writer->flush()) {
            return false;
        }
        return true;
    }
}