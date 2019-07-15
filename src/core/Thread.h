/*
 * This file is part of the Line Catcher distribution (https://github.com/AlexandrSachkov/LineCatcher).
 * Copyright (c) 2019 Alexandr Sachkov.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

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
