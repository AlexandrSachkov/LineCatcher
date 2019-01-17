#pragma once

#include "TaskRunner.h"
#include <string>

namespace PLP {
    class ResultSet {
    public:
        ResultSet();
        ~ResultSet();

        bool initialize(
            std::wstring& path, 
            unsigned long long preferredBufferSizeBytes,
            TaskRunner& asyncTaskRunner
        );

        bool append(unsigned long long lineNumber);
    private:
    };
}
