#pragma once

#include "TaskRunner.h"

#include "tbb\atomic.h"
#include "tbb\concurrent_queue.h"
#include <thread>
#include <functional>
#include <chrono>

namespace PLP
{
	class Thread : public TaskRunner
	{
	public:
		Thread(unsigned long long sleepPeriodNs);
		~Thread();

		void runAsync(std::function<void()> task, TaskStatus& status);
		bool start();
		void stop();
		void stopAndJoin();
		bool isRunning();

	private:
        std::thread _thread;
        tbb::concurrent_queue<std::pair<std::function<void()>, TaskStatus*>> _tasks;
		std::chrono::duration<unsigned long long, std::nano> _sleepPeriodNs;
			
		tbb::atomic<bool> _running;
		tbb::atomic<bool> _stopped;
	};
}
