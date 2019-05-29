#include "FileLock.h"

namespace PLP {
    std::mutex GlobalFileLock::_accessLock;
    std::unordered_map<std::wstring, std::pair<bool, unsigned int>> GlobalFileLock::_filesInUse;

    bool GlobalFileLock::lockForReading(const std::wstring& path) {
        std::lock_guard<std::mutex> lock(_accessLock);
        auto it = _filesInUse.find(path);
        if (it != _filesInUse.end()) {
            if (it->second.first) { //file is being written
                return false;
            }

            it->second.second++; //increment ref count
            return true;
        }

        _filesInUse.insert({ path, {false, 1} }); //insert a reference for reading
        return true;
    }

    bool GlobalFileLock::lockForWriting(const std::wstring& path) {
        std::lock_guard<std::mutex> lock(_accessLock);
        auto it = _filesInUse.find(path);
        if (it != _filesInUse.end()) {
            return false;
        }

        _filesInUse.insert({ path, {true, 1} }); //insert a reference for writing
        return true;
    }

    void GlobalFileLock::release(const std::wstring& path) {
        std::lock_guard<std::mutex> lock(_accessLock);
        auto it = _filesInUse.find(path);
        if (it != _filesInUse.end()) {
            it->second.second--; //decrement ref count
            if (0 == it->second.second) {
                _filesInUse.erase(it);
            }
        }
    }


    FileScopedLock::FileScopedLock() : _locked(false), _path(L"") {
    }

    FileScopedLock::FileScopedLock(bool locked, const std::wstring& path) : _locked(locked), _path(path) {
    }

    FileScopedLock::~FileScopedLock() {
        if (_locked) {
            GlobalFileLock::release(_path);
        }
    }

    FileScopedLock::FileScopedLock(FileScopedLock&& other) {
        _locked = other._locked;
        _path = other._path;

        other._locked = false;
        other._path = L"";
    }

    FileScopedLock& FileScopedLock::operator=(FileScopedLock&& other) {
        _locked = other._locked;
        _path = other._path;

        other._locked = false;
        other._path = L"";

        return *this;
    }

    bool FileScopedLock::isLocked() {
        return _locked;
    }

    FileScopedLock FileScopedLock::lockForReading(const std::wstring& path) {
        return FileScopedLock(GlobalFileLock::lockForReading(path), path);
    }

    FileScopedLock FileScopedLock::lockForWriting(const std::wstring& path) {
        return FileScopedLock(GlobalFileLock::lockForWriting(path), path);
    }
}