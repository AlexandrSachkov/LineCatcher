#include "Thread.h"

#include <assert.h>

namespace PLP
{
	Thread::Thread(unsigned long long sleepPeriodNs) :
		_sleepPeriodNs(std::chrono::nanoseconds(sleepPeriodNs)) {

		_running.store(false);
		_stopped.store(true);
	}

	Thread::~Thread() {
		_running.store(false);			
        if (_thread.joinable()) {
            _thread.join();
        }
	}

    void Thread::runAsync(std::function<void()> task, TaskStatus& status) {
        status.setCompleted(false);
        _tasks.push(std::make_pair(task, &status));
    }

	bool Thread::start() {
		_running.store(true);
        _stopped.store(false);

        try {
            _thread = std::thread([&](
                std::chrono::duration<unsigned long long, std::nano>& sleepPeriod,
                tbb::concurrent_queue<std::pair<std::function<void()>, TaskStatus*>>* tasks,
                tbb::atomic<bool>& running,
                tbb::atomic<bool>& stopped
                ) {

                while (_running) {
                    std::this_thread::sleep_for(sleepPeriod);

                    std::pair<std::function<void()>, TaskStatus*> task;
                    while (_running && tasks->try_pop(task)) {
                        task.first();
                        task.second->setCompleted(true);
                    }
                }
                stopped.store(true);
            }, std::ref(_sleepPeriodNs), &_tasks, std::ref(_running), std::ref(_stopped));
        } catch (std::system_error&) {
            return false;
        }

        return true;
	}

	void Thread::stop() {
		_running.store(false);
	}

	void Thread::stopAndJoin() {
		_running.store(false);
		while (!_stopped) {}
	}

	bool Thread::isRunning() {
		return _running.load();
	}
}