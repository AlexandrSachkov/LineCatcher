#include "PLPCore.h"

#include "Utils.h"
#include "Timer.h"
#include "PagedReader.h"
#include "MemMappedPagedReader.h"
#include "LineReader.h"
#include "FileWriter.h"

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
    PLPCore::PLPCore() {}
    PLPCore::~PLPCore() {
        _fileOpThread->stopAndJoin();
        if (_state) {
            lua_close(_state);
        }
    }

    bool PLPCore::initialize() {
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

    bool PLPCore::runScript(const std::wstring& scriptLua, std::wstring& errMsg) {
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

    bool PLPCore::searchLineContains(const std::wstring path, const std::wstring& substr, unsigned int& numMatches) {
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

    bool PLPCore::search(const std::wstring path, const std::wstring& frameFilterScriptLua, std::wstring& errMsg) {
        auto module = LuaIntf::LuaBinding(_state).beginModule("PLP");
        module.addConstant("Core", this);
        module.endModule();

        return true;
    }

    std::shared_ptr<FileReader> PLPCore::createFileReader(
        const std::string& path,
        unsigned long long preferredBuffSizeBytes
    ) {
        std::shared_ptr<FileReader> fReader(new FileReader());
        if (!fReader->initialize(string_to_wstring(path), preferredBuffSizeBytes)) {
            return nullptr;
        }
        return fReader;
    }

    std::shared_ptr<FileWriter> PLPCore::createFileWriter(
        const std::string& path,
        unsigned long long preferredBuffSizeBytes
    ) {
        std::shared_ptr<FileWriter> fileWriter(new FileWriter());
        if (!fileWriter->initialize(string_to_wstring(path), preferredBuffSizeBytes, *_fileOpThread)) {
            return nullptr;
        }
        return fileWriter;
    }

    std::shared_ptr<ResultReader> PLPCore::createResultReader(
        const std::string& path,
        unsigned long long preferredBuffSizeBytes
    ) {
        std::shared_ptr<ResultReader> resSet(new ResultReader());
        if (!resSet->initialize(string_to_wstring(path), preferredBuffSizeBytes, *_fileOpThread)) {
            return nullptr;
        }
        return resSet;
    }

    std::shared_ptr<ResultWriter> PLPCore::createResultWriter(
        const std::string& path,
        unsigned long long preferredBuffSizeBytes
    ) {
        std::shared_ptr<ResultWriter> resSet(new ResultWriter());
        if (!resSet->initialize(string_to_wstring(path), preferredBuffSizeBytes, *_fileOpThread)) {
            return nullptr;
        }
        return resSet;
    }

    void PLPCore::attachLuaBindings(lua_State* state) {
        auto module = LuaIntf::LuaBinding(state).beginModule("PLP");

        auto plpClass = module.beginClass<PLPCore>("Core");
        plpClass.addFunction("createFileReader", &PLPCore::createFileReader);
        plpClass.addFunction("createFileWriter", &PLPCore::createFileWriter);
        plpClass.addFunction("createResultReader", &PLPCore::createResultReader);
        plpClass.addFunction("createResultWriter", &PLPCore::createResultWriter);
        plpClass.endClass();

        auto fileReaderClass = module.beginClass<FileReader>("FileReader");
        std::tuple<bool, std::string>(FileReader::*nextLine)() = &FileReader::nextLine;
        fileReaderClass.addFunction("nextLine", nextLine);
        fileReaderClass.addFunction("lineNumber", &FileReader::getLineNumber);
        fileReaderClass.endClass();

        auto resultReaderClass = module.beginClass<ResultReader>("ResultReader");
        resultReaderClass.endClass();

        auto resultWriterClass = module.beginClass<ResultWriter>("ResultWriter");
        resultWriterClass.endClass();

        auto fileWriterClass = module.beginClass<FileWriter>("FileWriter");
        bool(FileWriter::*append)(const std::string&) = &FileWriter::append;
        bool(FileWriter::*appendLine)(const std::string&) = &FileWriter::appendLine;
        fileWriterClass.addFunction("append", append);
        fileWriterClass.addFunction("appendLine", appendLine);
        fileWriterClass.addFunction("flush", &FileWriter::flush);
        fileWriterClass.endClass();

        module.endModule();
    }
}