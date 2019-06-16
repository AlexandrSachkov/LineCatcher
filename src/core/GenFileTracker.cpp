#include "GenFileTracker.h"

namespace LC {
    std::vector<std::wstring> GenFileTracker::_files;

    void GenFileTracker::addFile(const std::wstring& path) {
        _files.push_back(path);
    }

    std::vector<std::wstring> GenFileTracker::getFiles() {
        return _files;
    }

    int GenFileTracker::size() {
        return (int)_files.size();
    }

    void GenFileTracker::clear() {
        _files.clear();
    }
}