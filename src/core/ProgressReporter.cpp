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

#include "ProgressReporter.h"

#include <cmath>

namespace LC {
    ProgressReporter::~ProgressReporter() {
        if (_progressUpdate && _currPercent < 100) {
            (*_progressUpdate)(100);
        }
    }

    bool ProgressReporter::initialize(
        unsigned long long start,
        unsigned long long end,
        const std::function<void(int percent)>* progressUpdate
    ) {
        if (start > end) {
            return false;
        }

        _start = start;
        _progressUpdate = const_cast<std::function<void(int percent)>*>(progressUpdate);

        const long double dNumPerPercent = (end - start + 1) / 100.0;
        _numPerProgressUpdate = dNumPerPercent > 1.0 ? (unsigned long long)nearbyintl(dNumPerPercent) : 1;
        _percentPerProgressUpdate = dNumPerPercent > 1.0 ? 1 : (int)(1.0 / dNumPerPercent);
        _numTillNextProgressUpdate = start + _numPerProgressUpdate;

        return true;
    }

    void ProgressReporter::update(unsigned long long current) {
        if (!_progressUpdate) {
            return;
        }

        if (current >= _numTillNextProgressUpdate) {
            int numUpdatesToDo = (int)((current - _start) / _numPerProgressUpdate);
            _numTillNextProgressUpdate = (numUpdatesToDo + 1) * _numPerProgressUpdate + _start;
            _currPercent = numUpdatesToDo * _percentPerProgressUpdate;

            (*_progressUpdate)(_currPercent);
        }
    }

    int ProgressReporter::getCurrentPercent() {
        return _currPercent;
    }
}