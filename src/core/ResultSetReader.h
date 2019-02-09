#pragma once

#include "TaskRunner.h"
#include <string>

namespace PLP {
    class ResultSetReader {
    public:
        ResultSetReader();
        ~ResultSetReader();

        bool initialize(
            std::wstring& path, 
            unsigned long long preferredBufferSizeBytes,
            TaskRunner& asyncTaskRunner
        );
    private:
    };
}
