#pragma once
#include "LineReader.h"
#include "PagedReader.h"

#include <unordered_map>

namespace PLP {
    class IndexedLineReader : public LineReader {
    public:
        IndexedLineReader();
        ~IndexedLineReader();

        bool initialize(PagedReader& pagedReader, unsigned long long preferredIndexSize);
    private:
        std::wstring getIndexFilePath(const std::wstring& dataFilePath);
        bool loadIndex(const std::wstring& indexPath);
        bool generateIndex(const std::wstring& dataFilePath, const std::wstring& indexPath);

        static const unsigned int INDEX_VERSION = 1; // increment if format changes
        static const unsigned int LINE_INDEX_FREQUENCY = 1000;

        std::unordered_map<unsigned long long, unsigned long long> _fileIndex;

        PagedReader* _pager = nullptr;
    };
}
