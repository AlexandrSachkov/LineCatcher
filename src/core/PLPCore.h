#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include "Thread.h"
#include "FileReader.h"
#include "FileWriter.h"
#include "ResultSetReader.h"
#include "ResultSetWriter.h"

#include <string>
#include <memory>
#include <vector>

struct lua_State;

namespace PLP {
    class PLPCore {
    public:
        PLPCore();
        ~PLPCore();

        bool initialize();

        bool runScript(const std::wstring& scriptLua, std::wstring& errMsg);
        /*bool searchLineContains(const std::wstring path, const std::wstring& substr, unsigned int& numMatches);
        bool searchLineContainsMM(const std::wstring path, const std::wstring& substr, unsigned int& numMatches);
        bool searchLineContainsMMIndexed(const std::wstring path, const std::wstring& substr, unsigned int& numMatches);
        bool search(const std::wstring path, const std::wstring& frameFilterScriptLua, std::wstring& errMsg);*/

        std::shared_ptr<FileReader> createFileReader(
            const std::string& path,
            unsigned long long preferredBuffSizeBytes,
            bool requireRandomAccess
        );

        std::shared_ptr<FileWriter> createFileWriter(
            const std::string& path,
            unsigned long long preferredBuffSizeBytes
        );

        std::shared_ptr<ResultSetReader> createResultSetReader(
            const std::string& path,
            unsigned long long preferredBuffSizeBytes
        );

        std::shared_ptr<ResultSetWriter> createResultSetWriter(
            const std::string& path,
            unsigned long long preferredBuffSizeBytes,
            const FileReader& fReader
        );

    private:
        static void attachLuaBindings(lua_State* state);

        lua_State* _state;
        std::unique_ptr<Thread> _fileOpThread;
    };
}