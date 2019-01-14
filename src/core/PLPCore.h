#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include "Thread.h"
#include "FileReader.h"

#include <string>
#include <memory>
#include <vector>

struct lua_State;

namespace PLP {
    class PLPCore {
    public:
        PLPCore();
        ~PLPCore();

        bool initialize(
            unsigned long long searchBuffSizeBytes
        );

        bool runScript(const std::wstring& scriptLua, std::wstring& errMsg);
        bool searchLineContains(const std::wstring path, const std::wstring& substr, unsigned int& numMatches);
        bool searchLineContainsWithPreload(const std::wstring path, const std::wstring& substr, unsigned int& numMatches);
        bool searchLineContainsWithPreloadMM(const std::wstring path, const std::wstring& substr, unsigned int& numMatches);
        bool search(const std::wstring path, const std::wstring& frameFilterScriptLua, std::wstring& errMsg);

        std::shared_ptr<FileReader> createFileReader(
            const std::string& path,
            unsigned long long preferredBuffSizeBytes
        );

        FileReader* createFileReaderW(
            const std::string& path,
            unsigned long long preferredBuffSizeBytes
        );

        bool test(const std::string& val);

    private:
        static void attachLuaBindings(lua_State* state);

        lua_State* _state;
        std::unique_ptr<Thread> _fileOpThread;
        std::vector<char> _searchBuff;
        unsigned long long _buffSizeBytes;
    };
}