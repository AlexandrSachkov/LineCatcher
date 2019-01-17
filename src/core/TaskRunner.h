#pragma once

#include <functional>
#include "tbb/atomic.h"

namespace PLP {
    class TaskStatus {
    public:
        TaskStatus() {
            setCompleted(true);
        }
        ~TaskStatus() {}

        void setCompleted(bool completed) {
            _completed.store(completed);
        }

        bool isCompleted() {
            return _completed.load();
        }

    private:
        tbb::atomic<bool> _completed;
    };

    class TaskRunner {
    public:
        virtual void runAsync(std::function<void()> task, TaskStatus& status) = 0;
        virtual ~TaskRunner() {}
    };
}