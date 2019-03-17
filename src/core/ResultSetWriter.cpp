#include "ResultSetWriter.h"
#include "FStreamPagedWriter.h"
#include "Utils.h"
#include "FileReader.h"
#include "TaskRunner.h"
#include "Logger.h"

namespace PLP {
    ResultSetWriter::ResultSetWriter() {}
    ResultSetWriter::~ResultSetWriter() {
        Logger::send(INFO, "Releasing index writer");
        release();
    }

    bool ResultSetWriter::initialize(
        std::wstring& path,
        std::wstring& dataFilePath,
        unsigned long long preferredBufferSizeBytes,
        bool overwriteIfExists,
        TaskRunner& asyncTaskRunner
    ) {
        release();
        _path = path;
        _dataFilePath = wstring_to_string(dataFilePath);

        FStreamPagedWriter* writer = new FStreamPagedWriter();
        _writer.reset(writer);
        if (!writer->initialize(path, preferredBufferSizeBytes, overwriteIfExists, asyncTaskRunner)) {
            return false;
        }

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

        return true;
    }

    void ResultSetWriter::release() {
        if (_writer) {
            flush();
        }
        _writer = nullptr;
        _path = L"";
        _dataFilePath = "";
        _prevLineNum = 0;
        _resultCount = 0;
    }

    bool ResultSetWriter::appendCurrLine(const FileReaderI* fReader) {
        if (!fReader) {
            return false;
        }

        unsigned long long lineNum = fReader->getLineNumber();
        if (!_writer->write(reinterpret_cast<const char*>(&lineNum), sizeof(unsigned long long))) {
            return false;
        }
        unsigned long long fileOffset = fReader->getLineFileOffset();
        if (!_writer->write(reinterpret_cast<const char*>(&fileOffset), sizeof(unsigned long long))) {
            return false;
        }

        _resultCount++;
        return true;
    }

    bool ResultSetWriter::appendCurrLine(std::shared_ptr<FileReaderI> fReader) {
        return appendCurrLine(fReader.get());
    }

    bool ResultSetWriter::flush() {
        if (!_writer->flush()) {
            return false;
        }
        return updateResultCount();
    }

    unsigned long long ResultSetWriter::getNumResults() const {
        return _resultCount;
    }

    bool ResultSetWriter::updateResultCount() {
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