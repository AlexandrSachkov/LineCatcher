#pragma once

#include <vector>
#include <string>

namespace LC {
    class GenFileTracker {
    public:
        static void addFile(const std::wstring& path);
        static std::vector<std::wstring> getFiles();
        static int size();
        static void clear();
    private:
        GenFileTracker() = default;
        GenFileTracker(const GenFileTracker&) = delete;
        GenFileTracker& operator=(const GenFileTracker&) = delete;

        static std::vector<std::wstring> _files;
    };
}