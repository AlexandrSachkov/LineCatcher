#include "FileSystemManager.h"

#include <fstream>

namespace LP {
    FileSystemManager::FileSystemManager() {}

    FileSystemManager::~FileSystemManager() {}

    bool FileSystemManager::initialize() {
        return true;
    }

    bool FileSystemManager::openFile(const std::wstring& path, unsigned int frameSizeLines) {
        std::ifstream infile("thefile.txt");
        return true;
    }

    bool FileSystemManager::readNextFrame() {
        return true;
    }

    void FileSystemManager::closeFile() {

    }
}