#include "ResultWriter.h"

namespace PLP {
    ResultWriter::ResultWriter() {}
    ResultWriter::~ResultWriter() {}

    bool ResultWriter::initialize(
        std::wstring& path,
        unsigned long long preferredBufferSizeBytes,
        TaskRunner& asyncTaskRunner
    ) {
        return true;
    }
}