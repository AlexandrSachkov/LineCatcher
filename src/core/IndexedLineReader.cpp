#include "IndexedLineReader.h"
#include "Utils.h"

#include "cereal/types/unordered_map.hpp"
#include "cereal/archives/binary.hpp"

#include <fstream>

namespace PLP {
    IndexedLineReader::IndexedLineReader() {}
    IndexedLineReader::~IndexedLineReader() {}

    bool IndexedLineReader::initialize(PagedReader& pagedReader, unsigned long long preferredIndexSize) {
        if (!LineReader::initialize(pagedReader)) {
            return false;
        }
        _pager = &pagedReader;

        std::wstring indexPath = getIndexFilePath(pagedReader.getFilePath());
        if (!loadIndex(indexPath)) {
            if (!generateIndex(pagedReader.getFilePath(), indexPath)) {
                return false;
            }
        }

        
        return true;
    }

    std::wstring IndexedLineReader::getIndexFilePath(const std::wstring& dataFilePath) {
        std::wstring directory = getFileDirectory(dataFilePath);
        std::wstring fileNameNoExt = getFileNameNoExt(dataFilePath);
        return directory + fileNameNoExt + L".plpidx";
    }

    bool IndexedLineReader::loadIndex(const std::wstring& indexPath) {
        std::ifstream fs;
        fs.open(indexPath, std::fstream::in | std::fstream::binary);
        if (!fs.good()) {
            return false;
        }

        cereal::BinaryInputArchive iarchive(fs);

        unsigned int indexVersion = 0;
        iarchive(indexVersion);
        if (indexVersion != INDEX_VERSION) { // might have a different format
            return false;
        }

        iarchive(_fileIndex);
        return true;
    }

    bool IndexedLineReader::generateIndex(const std::wstring& dataFilePath, const std::wstring& indexPath) {
        char* lineStart;
        unsigned int length;
        unsigned long long lineStartFileOffset = 0;
        while (nextLine(lineStart, length)) {
            if (getLineNumber() % LINE_INDEX_FREQUENCY == 0) {
                _fileIndex.insert({ getLineNumber(), lineStartFileOffset });
            }
            lineStartFileOffset = getCurrentFileOffset();
        }
        resetToBeginning();

        std::ofstream fs;
        fs.open(indexPath, std::fstream::out | std::fstream::binary);
        if (!fs.good()) {
            return false;
        }
        cereal::BinaryOutputArchive oarchive(fs);
        oarchive(INDEX_VERSION, _fileIndex);

        return true;
    }
}