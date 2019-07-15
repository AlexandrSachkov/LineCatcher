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