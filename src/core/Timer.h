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

#include <chrono>
#include <assert.h>

namespace PLP {
    class Timer {
    public:
        Timer() {
            _previousTime = std::chrono::high_resolution_clock::now();
        }

        ~Timer() {
        }

        double deltaT() {
            auto time = std::chrono::high_resolution_clock::now();
            double deltaT = std::chrono::duration_cast<std::chrono::duration<double>>(time - _previousTime).count() * 1000000000;
            _previousTime = time;
            return deltaT;
        }

    private:
        std::chrono::time_point<std::chrono::high_resolution_clock, std::chrono::nanoseconds> _previousTime;
    };
}
