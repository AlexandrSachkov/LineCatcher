#pragma once

#include "TaskRunner.h"
#include <string>

namespace PLP {
    class ResultReader {
    public:
        ResultReader();
        ~ResultReader();

        bool initialize(
            std::wstring& path, 
            unsigned long long preferredBufferSizeBytes,
            TaskRunner& asyncTaskRunner
        );
    private:
    };
}
