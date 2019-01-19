#include "ResultReader.h"

namespace PLP {
    ResultReader::ResultReader() {}

    ResultReader::~ResultReader() {}

    bool ResultReader::initialize(
        std::wstring& path,
        unsigned long long preferredBufferSizeBytes,
        TaskRunner& asyncTaskRunner
    ) {
        return true;
    }
}