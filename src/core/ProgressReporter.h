#pragma once

#include <functional>

namespace LC {
    class ProgressReporter {
    public:
        ProgressReporter() = default;
        ~ProgressReporter();

        bool initialize(
            unsigned long long start,
            unsigned long long end,
            const std::function<void(int percent)>* progressUpdate
        );
        void update(unsigned long long current);
        int getCurrentPercent();
    private:
        std::function<void(int percent)>* _progressUpdate = nullptr;

        unsigned long long _start = 0;
        unsigned long long _numPerProgressUpdate = 0;
        int _percentPerProgressUpdate = 0;
        unsigned long long _numTillNextProgressUpdate = 0;
        int _currPercent = 0;
    };
}