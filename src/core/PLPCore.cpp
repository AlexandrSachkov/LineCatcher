#include "PLPCore.h"

#include "Utils.h"
#include "Timer.h"
#include "FilePager.h"
#include "MemMappedFilePager.h"

#include "lua.hpp"
#include <fstream>
#include <cstring>

namespace PLP {
    PLPCore::PLPCore() {}
    PLPCore::~PLPCore() {
        _fileOpThread->stopAndJoin();
        if (_state) {
            lua_close(_state);
        }
    }

    bool PLPCore::initialize(
        unsigned long long searchBuffSizeBytes
    ) {
        _state = luaL_newstate();
        if (!_state) {
            return false;
        }

        luaL_openlibs(_state);

        _fileOpThread.reset(new Thread(1000000)); //1 ms
        TaskStatus status;
        _fileOpThread->runAsync([]() {
            printf("From task");
        }, status);
        if (!_fileOpThread->start()) {
            return false;
        }
        while (!status.isCompleted()) {

        }

        _buffSizeBytes = searchBuffSizeBytes;
        try {
            unsigned long long optimalSize = searchBuffSizeBytes / (FilePager::OPTIMAL_BLOCK_SIZE_BYTES * 2) * (FilePager::OPTIMAL_BLOCK_SIZE_BYTES * 2);
            _searchBuff.resize(optimalSize, 0);
        } catch (std::bad_alloc&) {
            return false;
        }

        return true;
    }

    bool PLPCore::runScript(const std::wstring& scriptLua, std::wstring& errMsg) {
        if (scriptLua.empty()) {
            return true;
        }

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
                printf("%i\n",lineNum);
            }
        }
        double numSeconds = timer.deltaT() / 1000000000;
        printf("Completed in: %f seconds\n", numSeconds);

        return true;
    }

    bool PLPCore::searchLineContainsWithPreload(const std::wstring path, const std::wstring& substr, unsigned int& numMatches) {
        std::string substring = wstring_to_string(substr);
        const char* cSubstr = substring.c_str();
        numMatches = 0;

        FilePager pager;
        if (!pager.initialize(path, _searchBuff, *_fileOpThread)) {
            return false;
        }

        Timer timer;

        unsigned long numPages = 0;
        unsigned long lineNum = 0;
        size_t pageSize;
        do {
            const char* data = pager.getNextPage(pageSize);
            numPages++;

            size_t pageOffset = 0;
            char* lineEnd = nullptr;
            while ((lineEnd = (char*)findNextLineEnding(data, pageSize, pageOffset)) != nullptr) {
                pageOffset = lineEnd - data + 1;
                lineNum++;
            }
            printf("Page loaded: %lu\n", numPages);
            printf("%lu\n", lineNum);
        } while (pageSize > 0);

        double numSeconds = timer.deltaT() / 1000000000;
        printf("Completed in: %f seconds\n", numSeconds);

        return true;
    }

    bool PLPCore::searchLineContainsWithPreloadMM(const std::wstring path, const std::wstring& substr, unsigned int& numMatches) {
        std::string substring = wstring_to_string(substr);
        const char* cSubstr = substring.c_str();
        numMatches = 0;

        for (int i = 0; i < 2; i++) {
            MemMappedFilePager pager;
            if (!pager.initialize(path, _buffSizeBytes, *_fileOpThread)) {
                return false;
            }

            Timer timer;

            unsigned long numPages = 0;
            unsigned long lineNum = 0;
            size_t pageSize;
            do {
                const char* data = pager.getNextPage(pageSize);
                numPages++;

                size_t pageOffset = 0;
                char* lineEnd = nullptr;
                while ((lineEnd = (char*)findNextLineEnding(data, pageSize, pageOffset)) != nullptr) {
                    pageOffset = lineEnd - data + 1;
                    lineNum++;
                    if (lineNum == 6384880) {
                        bool zzz = true;
                    }
                }
                printf("Page loaded: %lu\n", numPages);
                printf("%lu\n", lineNum);
            } while (pageSize > 0);

            double numSeconds = timer.deltaT() / 1000000000;
            printf("Completed in: %f seconds\n", numSeconds);
        }

        return true;
    }

    bool PLPCore::search(const std::wstring path, const std::wstring& frameFilterScriptLua, std::wstring& errMsg) {
        return true;
    }
}