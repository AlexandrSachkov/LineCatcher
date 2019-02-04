#pragma once
#include "LineReader.h"
#include "PagedReader.h"

#include <unordered_map>

namespace PLP {
    class IndexedLineReader : public LineReader {
    public:
        IndexedLineReader();
        ~IndexedLineReader();

        bool initialize(PagedReader& pagedReader);
        bool getLine(unsigned long long lineNumber, char*& data, unsigned int& size);
    private:
        std::wstring getIndexFilePath(const std::wstring& dataFilePath);
        bool loadIndex(const std::wstring& indexPath);
        bool generateIndex(const std::wstring& dataFilePath, const std::wstring& indexPath);

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
        static const unsigned int LINE_INDEX_FREQUENCY = 2;//1000;

        std::unordered_map<unsigned long long, unsigned long long> _fileIndex;
        IndexHeader _indexHeader;
    };
}
