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