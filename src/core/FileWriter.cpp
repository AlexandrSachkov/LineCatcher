#include "FileWriter.h"
#include "FStreamPagedWriter.h"
#include "Utils.h"
#include "TaskRunner.h"
#include "PagedWriter.h"
#include "Logger.h"

namespace PLP {
    FileWriter::FileWriter() {}
    FileWriter::~FileWriter() {
        Logger::send(INFO, "Releasing file writer");
        release();
    }

    bool FileWriter::initialize(
        const std::wstring& path,
        unsigned long long preferredBuffSizeBytes,
        bool overwriteIfExists,
        TaskRunner& asyncTaskRunner
    ) {
        release();

        FileScopedLock writingLock = FileScopedLock::lockForWriting(path);
        if (!writingLock.isLocked()) {
            Logger::send(ERR, "Unable to acquare a write lock on file " + wstring_to_string(path) + ". File is in use");
            return false;
        }

        FStreamPagedWriter* writer = new FStreamPagedWriter();
        _writer.reset(writer);
        if (!writer->initialize(path, preferredBuffSizeBytes, overwriteIfExists, asyncTaskRunner)) {
            return false;
        }

        _writingLock = std::move(writingLock);
        return true;
    }

    void FileWriter::release() {
        if (_writer) {
            _writer->flush();
        }
        _writer = nullptr;
    }

    bool FileWriter::append(const std::string& data) {
        return append(data.c_str(), data.length());
    } 

    bool FileWriter::appendLine(const std::string& line) {
        return appendLine(line.c_str(), line.length());
    }

    bool FileWriter::appendLine(const char* data, unsigned long long size) {
        bool success = append(data, size);
        if (!success) {
            return false;
        }
        return append("\r\n", 2); //TODO change on linux
    }

    bool FileWriter::append(const char* data, unsigned long long size) {
        return _writer->write(data, size);
    }
}