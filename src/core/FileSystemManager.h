#pragma once

#include <string>

namespace LP {

    class FileSystemManager {
    public:
        FileSystemManager();
        ~FileSystemManager();

        bool initialize();
        bool openFile(const std::wstring& path, unsigned int frameSizeLines);
        bool readNextFrame();
        void closeFile();

    private:
    };
}