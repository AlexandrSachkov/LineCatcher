#include "Core.h"

#include "Thread.h"
#include "FileReader.h"
#include "FileWriter.h"
#include "ResultSetReader.h"
#include "ResultSetWriter.h"
#include "Utils.h"
#include "Timer.h"
#include "PagedReader.h"
#include "MemMappedPagedReader.h"
#include "LineReader.h"

#include "lua.hpp"
#include "LuaIntf/LuaCompat.h"
#include "LuaIntf/LuaIntf.h"
#include "LuaIntf/LuaRef.h"

#include "LuaIntf/LuaContext.h"
#include "LuaIntf/LuaState.h"
#include <fstream>
#include <cstring>

namespace LuaIntf {
    LUA_USING_SHARED_PTR_TYPE(std::shared_ptr)
}

namespace PLP {
    PLP::CoreI* createCore() {
        return new PLP::Core();
    }

    Core::Core() {}
    Core::~Core() {
        _fileOpThread->stopAndJoin();
        if (_state) {
            lua_close(_state);
        }
    }

    bool Core::initialize() {
        _state = luaL_newstate();
        if (!_state) {
            return false;
        }

        luaL_openlibs(_state);
        attachLuaBindings(_state);

        _fileOpThread.reset(new Thread(1000000)); //1 ms sleep
        if (!_fileOpThread->start()) {
            return false;
        }

        return true;
    }

    bool Core::runScript(const std::wstring& scriptLua, std::wstring& errMsg) {
        if (scriptLua.empty()) {
            return true;
        }

        auto module = LuaIntf::LuaBinding(_state).beginModule("PLP");
        module.addFunction("core", [this] {
            return this;
        });
        module.endModule();

        lua_settop(_state, 0);
        if (luaL_loadstring(_state, wstring_to_string(scriptLua).c_str())) {
            errMsg = string_to_wstring(lua_tostring(_state, -1));
            return false;
        }
        if (lua_pcall(_state, 0, LUA_MULTRET, 0)) {
            errMsg = string_to_wstring(lua_tostring(_state, -1));
            return false;
        }
        return true;
    }

    /*bool PLPCore::searchLineContains(const std::wstring path, const std::wstring& substr, unsigned int& numMatches) {
        std::ifstream fs(wstring_to_string(path), std::ifstream::in | std::ifstream::binary);
        if (!fs.good()) {
            return false;
        }

        std::string substring = wstring_to_string(substr);
        numMatches = 0;

        Timer timer;

        int lineNum = 1;
        std::string line;
        while (std::getline(fs, line, '\n')) {
            if (line.find(substring) != std::string::npos) {
                numMatches++;
            }
            lineNum++;
            if (lineNum % 1000000 == 0) {
                printf("%i\n", lineNum);
            }
        }
        double numSeconds = timer.deltaT() / 1000000000;
        printf("Completed in: %f seconds\n", numSeconds);

        return true;
    }

    bool PLPCore::searchLineContainsMM(const std::wstring path, const std::wstring& substr, unsigned int& numMatches) {
        std::string substring = wstring_to_string(substr);
        const char* cSubstr = substring.c_str();
        numMatches = 0;

        MemMappedPagedReader pager;
        if (!pager.initialize(path)) {
            return false;
        }

        LineReader lineReader;
        if (!lineReader.initialize(pager)) {
            return false;
        }

        unsigned long long lineNum = 0;

        char* lineStart;
        unsigned int lineSize;
        while (lineReader.nextLine(lineStart, lineSize)) {
            lineNum++;
            if (lineNum % 10000000 == 0) {
                printf("%llu\n", lineNum);
            }
        }
        printf("TOTAL LINES: %llu\n", lineNum);

        return true;
    }

    bool PLPCore::searchLineContainsMMIndexed(const std::wstring path, const std::wstring& substr, unsigned int& numMatches) {
        std::string substring = wstring_to_string(substr);
        const char* cSubstr = substring.c_str();
        numMatches = 0;

        std::shared_ptr<FileReader> fileReader = createFileReader(wstring_to_string(path), 0, true);

        char* lineStart;
        unsigned int lineSize;
        while (fileReader->nextLine(lineStart, lineSize)) {
            if (fileReader->getLineNumber() % 10000000 == 0) {
                printf("%llu\n", fileReader->getLineNumber());
            }
        }
        printf("TOTAL LINES: %llu\n", fileReader->getLineNumber() + 1);

        return true;
    }

    bool PLPCore::search(const std::wstring path, const std::wstring& frameFilterScriptLua, std::wstring& errMsg) {
        auto module = LuaIntf::LuaBinding(_state).beginModule("PLP");
        module.addConstant("Core", this);
        module.endModule();

        return true;
    }*/

    FileReaderI* Core::createFileReader(
        const std::string& path,
        unsigned long long preferredBuffSizeBytes,
        bool requireRandomAccess
    ) {
        FileReader* fReader = new FileReader();
        if (!fReader->initialize(string_to_wstring(path), preferredBuffSizeBytes, requireRandomAccess)) {
            return nullptr;
        }
        return fReader;
    }

    FileWriterI* Core::createFileWriter(
        const std::string& path,
        unsigned long long preferredBuffSizeBytes,
        bool overwriteIfExists
    ) {
        FileWriter* fileWriter = new FileWriter();
        if (!fileWriter->initialize(string_to_wstring(path), preferredBuffSizeBytes, overwriteIfExists, *_fileOpThread)) {
            return nullptr;
        }
        return fileWriter;
    }

    ResultSetReaderI* Core::createResultSetReader(
        const std::string& path,
        unsigned long long preferredBuffSizeBytes
    ) {
        ResultSetReader* resSet = new ResultSetReader();
        if (!resSet->initialize(string_to_wstring(path), preferredBuffSizeBytes)) {
            return nullptr;
        }
        return resSet;
    }

    ResultSetWriterI* Core::createResultSetWriter(
        const std::string& path,
        unsigned long long preferredBuffSizeBytes,
        const FileReaderI* fReader,
        bool overwriteIfExists
    ) {
        if (!fReader) {
            return false;
        }

        ResultSetWriter* resSet = new ResultSetWriter();
        std::wstring dataPath;
        fReader->getFilePath(dataPath);

        if (!resSet->initialize(
            string_to_wstring(path), 
            dataPath,
            preferredBuffSizeBytes, overwriteIfExists, *_fileOpThread)) {
            return nullptr;
        }
        return resSet;
    }

    std::shared_ptr<FileReader> Core::createFileReaderL(
        const std::string& path,
        unsigned long long preferredBuffSizeBytes,
        bool requireRandomAccess
    ) {
        return std::shared_ptr<FileReader>(
            static_cast<FileReader*>(createFileReader(path, preferredBuffSizeBytes, requireRandomAccess))
        );
    }

    std::shared_ptr<FileWriter> Core::createFileWriterL(
        const std::string& path,
        unsigned long long preferredBuffSizeBytes,
        bool overwriteIfExists
    ) {
        return std::shared_ptr<FileWriter>(
            static_cast<FileWriter*>(createFileWriter(path, preferredBuffSizeBytes, overwriteIfExists))
        );
    }

    std::shared_ptr<ResultSetReader> Core::createResultSetReaderL(
        const std::string& path,
        unsigned long long preferredBuffSizeBytes
    ) {
        return std::shared_ptr<ResultSetReader>(
            static_cast<ResultSetReader*>(createResultSetReader(path, preferredBuffSizeBytes))
        );
    }

    std::shared_ptr<ResultSetWriter> Core::createResultSetWriterL(
        const std::string& path,
        unsigned long long preferredBuffSizeBytes,
        const FileReaderI* fReader,
        bool overwriteIfExists
    ) {
        return std::shared_ptr<ResultSetWriter>(
            static_cast<ResultSetWriter*>(createResultSetWriter(path, preferredBuffSizeBytes, fReader, overwriteIfExists))
        );
    }

    void Core::attachLuaBindings(lua_State* state) {
        auto module = LuaIntf::LuaBinding(state).beginModule("PLP");

        auto plpClass = module.beginClass<Core>("Core");
        plpClass.addFunction("createFileReader", &Core::createFileReaderL);
        plpClass.addFunction("createFileWriter", &Core::createFileWriterL);
        plpClass.addFunction("createResultSetReader", &Core::createResultSetReaderL);
        plpClass.addFunction("createResultSetWriter", &Core::createResultSetWriterL);
        plpClass.endClass();

        auto fileReaderClass = module.beginClass<FileReader>("FileReader");
        std::tuple<bool, std::string>(FileReader::*nextLine)() = &FileReader::nextLine;
        std::tuple<bool, std::string>(FileReader::*getLine)(unsigned long long) = &FileReader::getLine;
        std::tuple<bool, std::string>(FileReader::*getLineFromResult)(std::shared_ptr<ResultSetReaderI>) 
            = &FileReader::getLineFromResult;
        fileReaderClass.addFunction("nextLine", nextLine);
        fileReaderClass.addFunction("getLine", getLine);
        fileReaderClass.addFunction("getLineFromResult", getLineFromResult);
        fileReaderClass.addFunction("getLineNumber", &FileReader::getLineNumber);
        fileReaderClass.addFunction("restart", &FileReader::restart);
        fileReaderClass.addFunction("release", &FileReader::release);
        fileReaderClass.endClass();

        auto fileWriterClass = module.beginClass<FileWriter>("FileWriter");
        bool(FileWriter::*append)(const std::string&) = &FileWriter::append;
        bool(FileWriter::*appendLine)(const std::string&) = &FileWriter::appendLine;
        fileWriterClass.addFunction("append", append);
        fileWriterClass.addFunction("appendLine", appendLine);
        fileWriterClass.addFunction("release", &FileWriter::release);
        fileWriterClass.endClass();

        auto resultReaderClass = module.beginClass<ResultSetReader>("ResultSetReader");
        std::tuple<bool, unsigned long long>(ResultSetReader::*nextResult)() = &ResultSetReader::nextResult;
        resultReaderClass.addFunction("nextResult", nextResult);
        resultReaderClass.addFunction("getLineNumber", &ResultSetReader::getLineNumber);
        resultReaderClass.addFunction("getNumResults", &ResultSetReader::getNumResults);
        resultReaderClass.addFunction("getDataFilePath", &ResultSetReader::getDataFilePath);
        resultReaderClass.addFunction("restart", &ResultSetReader::restart);
        resultReaderClass.addFunction("release", &ResultSetReader::release);
        resultReaderClass.endClass();

        auto resultWriterClass = module.beginClass<ResultSetWriter>("ResultSetWriter");
        bool(ResultSetWriter::*appendCurrLine)(std::shared_ptr<FileReaderI>) = &ResultSetWriter::appendCurrLine;
        resultWriterClass.addFunction("appendCurrLine", appendCurrLine);
        resultWriterClass.addFunction("getNumResults", &ResultSetWriter::getNumResults);
        resultWriterClass.addFunction("release", &ResultSetWriter::release);
        resultWriterClass.endClass();

        module.endModule();
    }
}