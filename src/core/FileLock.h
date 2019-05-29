#pragma once

#include <unordered_map>
#include <string>
#include <mutex>

namespace PLP {
    class FileScopedLock {
    public:
        FileScopedLock();
        ~FileScopedLock();
        FileScopedLock(FileScopedLock&& other);
        FileScopedLock& operator=(FileScopedLock&& other);

        bool isLocked();

        static FileScopedLock lockForReading(const std::wstring& path);
        static FileScopedLock lockForWriting(const std::wstring& path);
    private:
        FileScopedLock(bool locked, const std::wstring& path);

        FileScopedLock(const FileScopedLock&) = delete;
        FileScopedLock& operator=(const FileScopedLock&) = delete;

        std::wstring _path;
        bool _locked = false;
    };

    class GlobalFileLock {
        friend FileScopedLock;

    private:
        static bool lockForReading(const std::wstring& path);
        static bool lockForWriting(const std::wstring& path);
        static void release(const std::wstring& path);

        GlobalFileLock() = delete;
        GlobalFileLock(const GlobalFileLock&) = delete;
        GlobalFileLock(GlobalFileLock&&) = delete;
        GlobalFileLock& operator=(const GlobalFileLock&) = delete;
        GlobalFileLock& operator=(GlobalFileLock&&) = delete;

        static std::mutex _accessLock;
        static std::unordered_map<std::wstring, std::pair<bool, unsigned int>> _filesInUse; // write/# references
    };
}