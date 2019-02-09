#pragma once
#include <string>

namespace PLP {
    struct ResultSetHeader {
        std::string dataFilePath;
        unsigned long long resultCount = 0;

        template<class Archive>
        void serialize(Archive& archive) {
            archive(dataFilePath, resultCount);
        }
    };

    struct Result {
        Result(unsigned long long lineNum, unsigned long long fileOffset) :
            lineNum(lineNum), fileOffset(fileOffset) {
        }

        unsigned long long lineNum = 0;
        unsigned long long fileOffset = 0;

        template<class Archive>
        void serialize(Archive& archive) {
            archive(lineNum, fileOffset);
        }
    };

    static const unsigned int RESULT_SET_VERSION = 1; // increment if format changes
}