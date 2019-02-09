#include "ResultSetWriter.h"
#include "FStreamPagedWriter.h"
#include "Utils.h"
#include "FileReader.h"

#include "cereal/cereal.hpp"
#include "cereal/archives/binary.hpp"
#include "cereal/types/common.hpp"

namespace PLP {
    ResultSetWriter::ResultSetWriter() {
        //std::string buff;
        //buff.reserve(sizeof(std::pair<unsigned long long, unsigned long long>));
        //_serialBuff = std::ostringstream(buff);
    }
    ResultSetWriter::~ResultSetWriter() {}

    bool ResultSetWriter::initialize(
        std::wstring& path,
        std::wstring& dataFilePath,
        unsigned long long preferredBufferSizeBytes,
        TaskRunner& asyncTaskRunner
    ) {
        _path = path;
        _dataFilePath = dataFilePath;

        FStreamPagedWriter* writer = new FStreamPagedWriter();
        _writer.reset(writer);
        if (!writer->initialize(path, preferredBufferSizeBytes, asyncTaskRunner)) {
            return false;
        }
        return true;
    }

    bool ResultSetWriter::appendCurrentLine(const FileReader& fReader) {
        _serialBuff.clear();
        _serialBuff.seekp(0, std::ios::beg);
        {
            cereal::BinaryOutputArchive oarchive(_serialBuff);
            Result result(fReader.getLineNumber(), fReader.getLineFileOffset());
            oarchive(result);
        }
        _serialBuff.seekp(0, std::ios::end);
        return _writer->write(_serialBuff.str().c_str(), _serialBuff.tellp());
    }

    bool ResultSetWriter::flush() {
        return _writer->flush();
    }

    unsigned long long ResultSetWriter::getNumResults() {
        return _resultCount;
    }
}