#pragma once
#include "LineReader.h"
#include "PagedReader.h"
#include "ReturnType.h"

#include <unordered_map>
#include <atomic>

namespace PLP {
    class IndexedLineReader : public LineReader {
    public:
        IndexedLineReader();
        ~IndexedLineReader();

        bool initialize(PagedReader& pagedReader, unsigned int maxLineSize, const std::atomic<bool>& cancelled);
        LineReaderResult getLine(unsigned long long lineNumber, char*& data, unsigned int& size);
        unsigned long long getNumberOfLines();
    private:
        std::wstring getIndexFilePath(const std::wstring& dataFilePath);
        bool loadIndex(const std::wstring& indexPath);
        bool generateIndex(
            const std::wstring& dataFilePath, 
            const std::wstring& indexPath,
            const std::atomic<bool>& cancelled
        );

        struct IndexHeader {
            std::string filePath;
            unsigned long long numLines = 0;
            unsigned int lineIndexFreq = 0;

            template<class Archive>
            void serialize(Archive& archive) {
                archive(filePath, numLines, lineIndexFreq);
            }
        };

        static const unsigned int INDEX_VERSION = 1; // increment if format changes
        static const unsigned int LINE_INDEX_FREQUENCY = 1000;

        std::unordered_map<unsigned long long, unsigned long long> _fileIndex;
        IndexHeader _indexHeader;
    };
}
