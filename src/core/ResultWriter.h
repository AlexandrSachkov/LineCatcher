#pragma once

#include "TaskRunner.h"
#include <string>

namespace PLP {
    class ResultWriter {
    public:
        ResultWriter();
        ~ResultWriter();

        bool initialize(
            std::wstring& path,
            unsigned long long preferredBufferSizeBytes,
            TaskRunner& asyncTaskRunner
        );
    };
}
