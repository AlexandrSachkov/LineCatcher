#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include "CoreI.h"

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
    class Thread;
    class FileReader;
    class FileWriter;
    class ResultSetReader;
    class ResultSetWriter;

    class Core : public CoreI {
    public:
        Core();
        ~Core();

        bool initialize();
        bool runScript(const std::wstring* scriptLua) override;
        bool attachLogOutput(const char* name, const std::function<void(int, const char*)>* func);
        void detachLogOutput(const char* name);

        //C++ interface
        FileReaderI* createFileReader(
            const std::string& path,
            unsigned long long preferredBuffSizeBytes,
            bool requireRandomAccess
        ) override;

        FileWriterI* createFileWriter(
            const std::string& path,
            unsigned long long preferredBuffSizeBytes,
            bool overwriteIfExists
        ) override;

        ResultSetReaderI* createResultSetReader(
            const std::string& path,
            unsigned long long preferredBuffSizeBytes
        ) override;

        ResultSetWriterI* createResultSetWriter(
            const std::string& path,
            unsigned long long preferredBuffSizeBytes,
            const FileReaderI* fReader,
            bool overwriteIfExists
        ) override;


        //Lua interface
        std::shared_ptr<FileReader> createFileReaderL(
            const std::string& path,
            unsigned long long preferredBuffSizeBytes,
            bool requireRandomAccess
        );

        std::shared_ptr<FileWriter> createFileWriterL(
            const std::string& path,
            unsigned long long preferredBuffSizeBytes,
            bool overwriteIfExists
        );

        std::shared_ptr<ResultSetReader> createResultSetReaderL(
            const std::string& path,
            unsigned long long preferredBuffSizeBytes
        );

        std::shared_ptr<ResultSetWriter> createResultSetWriterL(
            const std::string& path,
            unsigned long long preferredBuffSizeBytes,
            const std::shared_ptr<FileReader> fReader,
            bool overwriteIfExists
        );

        void printConsoleL(const std::string& msg);
        void printConsoleExL(const std::string& msg, int level);

    private:
        static void attachLuaBindings(lua_State* state);

        lua_State* _state;
        std::unique_ptr<Thread> _fileOpThread;
    };

    PLP_LIB_API PLP::CoreI* createCore();
}