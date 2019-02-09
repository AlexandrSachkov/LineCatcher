#include "ResultSetReader.h"

namespace PLP {
    ResultSetReader::ResultSetReader() {}

    ResultSetReader::~ResultSetReader() {}

    bool ResultSetReader::initialize(
        std::wstring& path,
        unsigned long long preferredBufferSizeBytes,
        TaskRunner& asyncTaskRunner
    ) {
        return true;
    }
}