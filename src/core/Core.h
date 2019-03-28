#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include "CoreI.h"

#include <string>
#include <memory>
#include <vector>
#include <atomic>

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
        void cancelOperation() override;
        bool isCancelled();
        
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

        bool search(
            FileReaderI* fileReader,
            ResultSetWriterI* indexWriter,
            unsigned long long startLine,
            unsigned long long endLine, //0 for end of file, inclusive
            const std::wstring& searchText,
            bool plainTextSearch, //false for regex
            const std::function<void(int percent, unsigned long long numResults)>* progressUpdate
        ) override;

        bool searchI(
            FileReaderI* fileReader,
            ResultSetReaderI* indexReader,
            ResultSetWriterI* indexWriter,
            unsigned long long startIndex,
            unsigned long long endIndex, //0 for end of file, inclusive
            const std::wstring& searchText,
            bool plainTextSearch, //false for regex
            const std::function<void(int percent, unsigned long long numResults)>* progressUpdate
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

        bool searchL(
            std::shared_ptr<FileReader> fileReader,
            std::shared_ptr<ResultSetWriter> indexWriter,
            unsigned long long startLine,
            unsigned long long endLine, //0 for end of file, inclusive
            const std::string& searchText,
            bool plainTextSearch //false for regex
        );

        bool searchIL(
            std::shared_ptr<FileReader> fileReader,
            std::shared_ptr<ResultSetReader> indexReader,
            std::shared_ptr<ResultSetWriter> indexWriter,
            unsigned long long startIndex,
            unsigned long long endIndex, //0 for end of file, inclusive
            const std::string& searchText,
            bool plainTextSearch //false for regex
        );

        void printConsoleL(const std::string& msg);
        void printConsoleExL(const std::string& msg, int level);
    private:
        static void attachLuaBindings(lua_State* state);

        lua_State* _state;
        std::unique_ptr<Thread> _fileOpThread;
        std::atomic<bool> _cancelled = false;
    };

    PLP_LIB_API PLP::CoreI* createCore();
}