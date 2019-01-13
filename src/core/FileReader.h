#pragma once

#include <string>

namespace PLP {
    class FileReader {
    public:
        FileReader();
        ~FileReader();

        bool initialize(const std::wstring& path, unsigned int preferredChunkSizeBytes);
        bool getNextChunk(char*& chunk, unsigned int& chunkSizeBytes);
    private:
    };
}