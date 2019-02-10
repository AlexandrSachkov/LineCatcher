#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <string>
#include <memory>
#include <vector>

#define PLP_LIB_EXPORT __declspec(dllexport)
#define PLP_LIB_IMPORT __declspec(dllimport)

#ifdef PLP_EXPORT
#define PLP_LIB_API PLP_LIB_EXPORT
#else
#define PLP_LIB_API PLP_LIB_IMPORT
#endif

struct lua_State;

namespace PLP {
    class FileReader;
    class FileWriter;
    class ResultSetReader;
    class ResultSetWriter;
    class Thread;

    class PLPCore {
    public:
        PLPCore();
        ~PLPCore();

        bool initialize();
        bool runScript(const std::wstring& scriptLua, std::wstring& errMsg);

        std::shared_ptr<FileReader> createFileReader(
            const std::string& path,
            unsigned long long preferredBuffSizeBytes,
            bool requireRandomAccess
        );

        std::shared_ptr<FileWriter> createFileWriter(
            const std::string& path,
            unsigned long long preferredBuffSizeBytes,
            bool overwriteIfExists
        );

        std::shared_ptr<ResultSetReader> createResultSetReader(
            const std::string& path,
            unsigned long long preferredBuffSizeBytes
        );

        std::shared_ptr<ResultSetWriter> createResultSetWriter(
            const std::string& path,
            unsigned long long preferredBuffSizeBytes,
            const FileReader& fReader,
            bool overwriteIfExists
        );

    private:
        static void attachLuaBindings(lua_State* state);

        lua_State* _state;
        std::unique_ptr<Thread> _fileOpThread;
    };

    extern PLP_LIB_API PLPCore* createCore();
}