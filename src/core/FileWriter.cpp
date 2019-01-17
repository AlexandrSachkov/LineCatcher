#include "FileWriter.h"
#include "Utils.h"

namespace PLP {
    FileWriter::FileWriter() {}
    FileWriter::~FileWriter() {
        flush();
    }

    bool FileWriter::initialize(
        const std::wstring& path,
        unsigned long long preferredBuffSizeBytes,
        TaskRunner& asyncTaskRunner
    ) {
        try {
            unsigned long long optimalSize = preferredBuffSizeBytes / (OPTIMAL_BLOCK_SIZE_BYTES * 2) * (OPTIMAL_BLOCK_SIZE_BYTES * 2);
            if (optimalSize == 0) {
                return false;
            }
            _buff.resize(optimalSize, 0);
        } catch (std::bad_alloc&) {
            return false;
        }

        if (!_writer.initialize(path, _buff, asyncTaskRunner)) {
            return false;
        }

        return true;
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
        return append("\n", 1);
    }

    bool FileWriter::append(const char* data, unsigned long long size) {
        return _writer.write(data, size);
    }

    bool FileWriter::flush() {
        return _writer.flush();
    }
}