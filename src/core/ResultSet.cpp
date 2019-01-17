#include "ResultSet.h"

namespace PLP {
    ResultSet::ResultSet() {}

    ResultSet::~ResultSet() {}

    bool ResultSet::initialize(
        std::wstring& path,
        unsigned long long preferredBufferSizeBytes,
        TaskRunner& asyncTaskRunner
    ) {
        return true;
    }

    bool ResultSet::append(unsigned long long lineNumber) {
        return true;
    }
}