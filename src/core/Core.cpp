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
#include "Logger.h"
#include "ReturnType.h"
#include "FrameBuffer.h"
#include "TextComparator.h"
#include "Scanner.h"

#include "lua.hpp"
#include "LuaIntf/LuaIntf.h"

#include <fstream>
#include <cstring>
#include <cmath>
#include <functional>
#include <regex>
#include <limits>
#include <algorithm>
#include <unordered_map>

namespace LuaIntf {
    LUA_USING_SHARED_PTR_TYPE(std::shared_ptr);
    LUA_USING_LIST_TYPE(std::vector);
    LUA_USING_MAP_TYPE(std::unordered_map);
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
            Logger::send(ERR, "Failed to start thread pool");
            return false;
        }

        luaL_openlibs(_state);
        attachLuaBindings(_state);

        _fileOpThread.reset(new Thread(1000000)); //1 ms sleep
        if (!_fileOpThread->start()) {
            Logger::send(ERR, "Failed to start thread pool");
            return false;
        }

        Logger::send(INFO, "Successfully initialized PLP core");
        return true;
    }

    bool Core::runScript(const std::wstring* scriptLua) {
        if (nullptr == scriptLua || scriptLua->empty()) {
            Logger::send(WARN, "No script provided");
            return true;
        }

        _cancelled = false;

        auto module = LuaIntf::LuaBinding(_state).beginModule("PLP");
        module.addFunction("core", [this] {
            return this;
        });
        module.endModule();

        lua_settop(_state, 0);
        if (luaL_loadstring(_state, wstring_to_string(*scriptLua).c_str())) {
            std::string err = lua_tostring(_state, -1);
            Logger::send(ERR, "Failed to compile lua script:\n" + err);
            return false;
        }
        if (lua_pcall(_state, 0, LUA_MULTRET, 0)) {
            std::string err = lua_tostring(_state, -1);
            Logger::send(ERR, "Failed to run lua script:\n" + err);
            lua_settop(_state, 0);
            lua_gc(_state, LUA_GCCOLLECT, 0);

            return false;
        }

        lua_settop(_state, 0);
        lua_gc(_state, LUA_GCCOLLECT, 0);

        return true;
    }

    void Core::cancelOperation() {
        _cancelled = true;
    }

    bool Core::attachLogOutput(const char* name, const std::function<void(int, const char*)>* func) {
        Logger::subscribe(name, *func);
        return true;
    }

    void Core::detachLogOutput(const char* name) {
        Logger::unsubscribe(name);
    }

    FileReaderI* Core::createFileReader(
        const std::string& path,
        unsigned long long preferredBuffSizeBytes,
        bool requireRandomAccess
    ) {
        _cancelled = false;

        std::unique_ptr<FileReader> fReader(new FileReader());
        if (!fReader->initialize(string_to_wstring(path), preferredBuffSizeBytes, requireRandomAccess, _cancelled)) {
            Logger::send(ERR, "Failed to create file reader");
            return nullptr;
        }
        Logger::send(INFO, "Successfully created file reader");
        return fReader.release();
    }

    FileWriterI* Core::createFileWriter(
        const std::string& path,
        unsigned long long preferredBuffSizeBytes,
        bool overwriteIfExists
    ) {
        std::unique_ptr<FileWriter> fileWriter(new FileWriter());
        if (!fileWriter->initialize(string_to_wstring(path), preferredBuffSizeBytes, overwriteIfExists, *_fileOpThread)) {
            Logger::send(ERR, "Failed to create file writer");
            return nullptr;
        }
        Logger::send(INFO, "Successfully created file writer");
        return fileWriter.release();
    }

    ResultSetReaderI* Core::createResultSetReader(
        const std::string& path,
        unsigned long long preferredBuffSizeBytes
    ) {
        std::unique_ptr<ResultSetReader> resSet(new ResultSetReader());
        if (!resSet->initialize(string_to_wstring(path), preferredBuffSizeBytes)) {
            Logger::send(ERR, "Failed to create index reader");
            return nullptr;
        }
        Logger::send(INFO, "Successfully created index reader");
        return resSet.release();
    }

    ResultSetWriterI* Core::createResultSetWriter(
        const std::string& path,
        unsigned long long preferredBuffSizeBytes,
        const FileReaderI* fReader,
        bool overwriteIfExists
    ) {
        if (!fReader) {
            Logger::send(ERR, "File reader is null");
            return false;
        }

        std::unique_ptr<ResultSetWriter> resSet(new ResultSetWriter());
        std::wstring dataPath;
        fReader->getFilePath(dataPath);

        if (!resSet->initialize(
            string_to_wstring(path), 
            dataPath,
            preferredBuffSizeBytes, overwriteIfExists, *_fileOpThread)) {
            Logger::send(ERR, "Failed to create index writer");
            return nullptr;
        }
        Logger::send(INFO, "Successfully created index writer");
        return resSet.release();
    }

    void Core::release(FileReaderI* obj) {
        if (obj) {
            delete obj;
        }
    }

    void Core::release(FileWriterI* obj) {
        if (obj) {
            delete obj;
        }
    }

    void Core::release(ResultSetReaderI* obj) {
        if (obj) {
            delete obj;
        }
    }

    void Core::release(ResultSetWriterI* obj) {
        if (obj) {
            delete obj;
        }
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
        const std::shared_ptr<FileReader> fReader,
        bool overwriteIfExists
    ) {
        return std::shared_ptr<ResultSetWriter>(
            static_cast<ResultSetWriter*>(createResultSetWriter(path, preferredBuffSizeBytes, fReader.get(), overwriteIfExists))
        );
    }

    bool Core::searchGeneral(
        FileReaderI* fileReader,
        ResultSetReaderI* indexReader,
        unsigned long long start,
        unsigned long long end,
        TextComparator* comparator,
        const std::function<bool(unsigned long long lineNum, unsigned long long fileOffset, const char* line, unsigned int length)> action,
        const std::function<void(int percent)>* progressUpdate
    ) {
        if (fileReader == nullptr) {
            Logger::send(ERR, "File reader and index writer cannot be null");
            return false;
        }
        if (comparator == nullptr) {
            Logger::send(ERR, "Comparator cannot be null");
            return false;
        }
        if (start > end) {
            Logger::send(ERR, "Start line must be smaller or equal to end line");
            return false;
        }
        end = end > 0 ? end : fileReader->getNumberOfLines() - 1;

        std::function<void(int)> defaultProgressUpdate = [](int) {};
        if (progressUpdate == nullptr) {
            progressUpdate = &defaultProgressUpdate;
        }

        const long double dLinesPerPercent = (end - start + 1) / 100.0;
        const unsigned long long numLinesTillProgressUpdate = dLinesPerPercent > 1.0 ? (unsigned long long)dLinesPerPercent : 1;
        const int percentPerProgressUpdate = dLinesPerPercent > 1.0 ? 1 : (int)(1.0 / dLinesPerPercent);

        LineScanner scanner(fileReader, indexReader, start, end);
        if (!scanner.initialize()) {
            Logger::send(ERR, "Failed to initialize line scanner");
            return false;
        }

        int progressPercent = 0;
        unsigned long long numProcessedLines = 0;

        unsigned long long lineNum;
        unsigned long long fileOffset;
        char* line;
        unsigned int lineSize;

        LineReaderResult result;
        while ((result = scanner.nextLine(lineNum, fileOffset, line, lineSize)) == LineReaderResult::SUCCESS) {
            if (comparator->match(line, lineSize)) {
                if (!action(lineNum, fileOffset, line, lineSize)) {
                    break;
                }
            }

            numProcessedLines++;
            if (numProcessedLines % numLinesTillProgressUpdate == 0) {
                progressPercent += percentPerProgressUpdate;
                (*progressUpdate)(progressPercent);
            }
        }

        if (result == LineReaderResult::ERROR) {
            Logger::send(ERR, "Failed to get line");
            return false;
        }

        if (progressPercent < 100) {
            (*progressUpdate)(100);
        }


        return true;
    }

    bool Core::searchMultilineGeneral(
        FileReaderI* fileReader,
        ResultSetReaderI* indexReader,
        unsigned long long start,
        unsigned long long end, //0 for end of file, inclusive
        const std::unordered_map<int, TextComparator*>& lineComparators,
        const std::function<bool(unsigned long long lineNum, unsigned long long fileOffset, const char* line, unsigned int length)> action,
        const std::function<void(int percent)>* progressUpdate
    ) {
        if (fileReader == nullptr) {
            Logger::send(ERR, "File reader and index writer cannot be null");
            return false;
        }
        if (start > end) {
            Logger::send(ERR, "Start line must be smaller or equal to end line");
            return false;
        }
        end = end > 0 ? end : fileReader->getNumberOfLines() - 1;

        std::vector<std::pair<int, TextComparator*>> vLineComparators;
        for (auto& pair : lineComparators) {
            vLineComparators.push_back(pair);
        }
        std::sort(vLineComparators.begin(), vLineComparators.end(),
            [](const std::pair<int, TextComparator*>& comp1, const std::pair<int, TextComparator*>& comp2) {
            return comp1.first < comp2.first;
        });

        if (vLineComparators.size() == 0) {
            Logger::send(ERR, "Must have at least one line comparator");
            return false;
        }

        MultilineScanner scanner(fileReader, indexReader, 
            vLineComparators[0].first, vLineComparators[vLineComparators.size() - 1].first, 
            start, end
        );
        if (!scanner.initialize()) {
            Logger::send(ERR, "Failed to initialize multi-line scanner");
            return false;
        }
        
        const long double dLinesPerPercent = (end - start + 1) / 100.0;
        const unsigned long long numLinesTillProgressUpdate = dLinesPerPercent > 1.0 ? (unsigned long long)dLinesPerPercent : 1;
        const int percentPerProgressUpdate = dLinesPerPercent > 1.0 ? 1 : (int)(1.0 / dLinesPerPercent);

        int progressPercent = 0;
        unsigned long long numProcessedLines = 0;

        unsigned long long lineNum;
        unsigned long long fileOffset;
        char* line;
        unsigned int lineSize;

        bool matched = true;
        LineReaderResult result;
        while ((result = scanner.nextFrame()) == LineReaderResult::SUCCESS) {
            for (auto& comparator : vLineComparators) {
                if (!scanner.getLine(comparator.first, lineNum, fileOffset, line, lineSize)) {
                    return false;
                }
                if (!comparator.second->match(line, lineSize)) {
                    matched = false;
                    break;
                }
                matched = true;
            }
            if (matched) {
                if (!scanner.getLine(0, lineNum, fileOffset, line, lineSize)) {
                    return false;
                }
                if (!action(lineNum, fileOffset, line, lineSize)) {
                    break;
                }
            }

            numProcessedLines++;
            if (numProcessedLines % numLinesTillProgressUpdate == 0) {
                progressPercent += percentPerProgressUpdate;
                (*progressUpdate)(progressPercent);
            }
        }

        if (result == LineReaderResult::ERROR) {
            Logger::send(ERR, "Failed to get line");
            return false;
        }

        if (progressPercent < 100) {
            (*progressUpdate)(100);
        }

        return true;
    }

    bool Core::search(
        FileReaderI* fileReader,
        ResultSetReaderI* indexReader,
        ResultSetWriterI* indexWriter,
        unsigned long long start,
        unsigned long long end, //0 for end of file, inclusive
        unsigned long long maxNumResults,
        TextComparator* comparator,
        const std::function<void(int percent, unsigned long long numResults)>* progressUpdate
    ) {
        maxNumResults = maxNumResults > 0 ? maxNumResults : ULLONG_MAX;
        auto action = [maxNumResults, indexWriter](unsigned long long lineNum, unsigned long long fileOffset, const char* line, unsigned int length) {
            if (!indexWriter->appendCurrLine(lineNum, fileOffset) || indexWriter->getNumResults() >= maxNumResults) {
                return false;
            }
            return true;
        };

        std::function<void(int)> progressUpdateInt = [&](int percent) {
            (*progressUpdate)(percent, indexWriter->getNumResults());
        };
        return searchGeneral(
            fileReader,
            indexReader,
            start,
            end,
            comparator,
            action,
            &progressUpdateInt
        );
    }

    bool Core::searchMultiline(
        FileReaderI* fileReader,
        ResultSetReaderI* indexReader,
        ResultSetWriterI* indexWriter,
        unsigned long long start,
        unsigned long long end, //0 for end of file, inclusive
        unsigned long long maxNumResults,
        const std::unordered_map<int, TextComparator*>& lineComparators,
        const std::function<void(int percent, unsigned long long numResults)>* progressUpdate
    ) {
        maxNumResults = maxNumResults > 0 ? maxNumResults : ULLONG_MAX;
        auto action = [maxNumResults, indexWriter](unsigned long long lineNum, unsigned long long fileOffset, const char* line, unsigned int length) {
            if (!indexWriter->appendCurrLine(lineNum, fileOffset) || indexWriter->getNumResults() >= maxNumResults) {
                return false;
            }
            return true;
        };

        std::function<void(int)> progressUpdateInt = [&](int percent) {
            (*progressUpdate)(percent, indexWriter->getNumResults());
        };
        return searchMultilineGeneral(
            fileReader,
            indexReader,
            start,
            end,
            lineComparators,
            action,
            &progressUpdateInt
        );
    }

    bool Core::searchL(
        std::shared_ptr<FileReader> fileReader,
        std::shared_ptr<ResultSetWriter> indexWriter,
        unsigned long long start,
        unsigned long long end, //0 for end of file, inclusive
        unsigned long long maxNumResults,
        std::shared_ptr<TextComparator> comparator
    ) {
        std::function<void(int, unsigned long long)> progressUpdate = [&](int percent, unsigned long long numResults) {
            printConsoleL(std::to_string(percent) + "%, Found results: " + std::to_string(numResults));
        };

        return search(
            fileReader.get(),
            nullptr,
            indexWriter.get(),
            start,
            end,
            maxNumResults,
            comparator.get(),
            &progressUpdate
        );
    }

    bool Core::searchIL(
        std::shared_ptr<FileReader> fileReader,
        std::shared_ptr<ResultSetReader> indexReader,
        std::shared_ptr<ResultSetWriter> indexWriter,
        unsigned long long start,
        unsigned long long end, //0 for end of file, inclusive
        unsigned long long maxNumResults,
        std::shared_ptr<TextComparator> comparator
    ) {
        std::function<void(int, unsigned long long)> progressUpdate = [&](int percent, unsigned long long numResults) {
            printConsoleL(std::to_string(percent) + "%, Found results: " + std::to_string(numResults));
        };

        return search(
            fileReader.get(),
            indexReader.get(),
            indexWriter.get(),
            start,
            end,
            maxNumResults,
            comparator.get(),
            &progressUpdate
        );
    }

    bool Core::searchMultilineL(
        std::shared_ptr<FileReader> fileReader,
        std::shared_ptr<ResultSetWriter> indexWriter,
        unsigned long long start,
        unsigned long long end, //0 for end of file, inclusive
        unsigned long long maxNumResults,
        const std::unordered_map<int, std::shared_ptr<TextComparator>>& lineComparators
    ) {
        std::function<void(int, unsigned long long)> progressUpdate = [&](int percent, unsigned long long numResults) {
            printConsoleL(std::to_string(percent) + "%, Found results: " + std::to_string(numResults));
        };

        std::unordered_map<int, TextComparator*> comparators;
        for (auto& pair : lineComparators) {
            comparators.emplace(pair.first, pair.second.get());
        }

        return searchMultiline(
            fileReader.get(),
            nullptr,
            indexWriter.get(),
            start,
            end,
            maxNumResults,
            comparators,
            &progressUpdate
        );
    }

    bool Core::searchMultilineIL(
        std::shared_ptr<FileReader> fileReader,
        std::shared_ptr<ResultSetReader> indexReader,
        std::shared_ptr<ResultSetWriter> indexWriter,
        unsigned long long start,
        unsigned long long end, //0 for end of file, inclusive
        unsigned long long maxNumResults,
        const std::unordered_map<int, std::shared_ptr<TextComparator>>& lineComparators
    ) {
        std::function<void(int, unsigned long long)> progressUpdate = [&](int percent, unsigned long long numResults) {
            printConsoleL(std::to_string(percent) + "%, Found results: " + std::to_string(numResults));
        };

        std::unordered_map<int, TextComparator*> comparators;
        for (auto& pair : lineComparators) {
            comparators.emplace(pair.first, pair.second.get());
        }

        return searchMultiline(
            fileReader.get(),
            indexReader.get(),
            indexWriter.get(),
            start,
            end,
            maxNumResults,
            comparators,
            &progressUpdate
        );
    }

    void Core::printConsoleL(const std::string& msg) {
        printConsoleExL(msg, 0);
    }

    void Core::printConsoleExL(const std::string& msg, int level) {
        Logger::send((LOG_LEVEL)level, msg);
    }

    bool Core::isCancelled() {
        return _cancelled;
    }

    void Core::attachLuaBindings(lua_State* state) {
        auto module = LuaIntf::LuaBinding(state).beginModule("PLP");
        module.addConstant("ERROR", LineReaderResult::ERROR);
        module.addConstant("NOT_FOUND", LineReaderResult::NOT_FOUND);
        module.addConstant("SUCCESS", LineReaderResult::SUCCESS);

        auto plpClass = module.beginClass<Core>("Core");
        plpClass.addFunction("createFileReader", &Core::createFileReaderL);
        plpClass.addFunction("createFileWriter", &Core::createFileWriterL);
        plpClass.addFunction("createResultSetReader", &Core::createResultSetReaderL);
        plpClass.addFunction("createResultSetWriter", &Core::createResultSetWriterL);
        plpClass.addFunction("search", &Core::searchL);
        plpClass.addFunction("searchI", &Core::searchIL);
        plpClass.addFunction("searchMultiline", &Core::searchMultilineL);
        plpClass.addFunction("searchMultilineI", &Core::searchMultilineIL);
        plpClass.addFunction("printConsole", &Core::printConsoleL);
        plpClass.addFunction("printConsoleEx", &Core::printConsoleExL);
        plpClass.addFunction("isCancelled", &Core::isCancelled);
        plpClass.endClass();

        {
            auto fileReaderClass = module.beginClass<FileReader>("FileReader");
            std::tuple<int, std::string>(FileReader::*nextLine)() = &FileReader::nextLine;
            std::tuple<int, std::string>(FileReader::*getLine)(unsigned long long) = &FileReader::getLine;
            std::tuple<int, std::string>(FileReader::*getLineFromResult)(const std::shared_ptr<ResultSetReader>)
                = &FileReader::getLineFromResult;
            fileReaderClass.addFunction("nextLine", nextLine);
            fileReaderClass.addFunction("getLine", getLine);
            fileReaderClass.addFunction("getLineFromResult", getLineFromResult);
            fileReaderClass.addFunction("getLineNumber", &FileReader::getLineNumber);
            fileReaderClass.addFunction("getNumberOfLines", &FileReader::getNumberOfLines);
            fileReaderClass.addFunction("restart", &FileReader::restart);
            fileReaderClass.addFunction("release", &FileReader::release);
            fileReaderClass.endClass();
        }

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
        resultReaderClass.addFunction("getFilePath", &ResultSetReader::getFilePath);
        resultReaderClass.addFunction("getDataFilePath", &ResultSetReader::getDataFilePath);
        resultReaderClass.addFunction("restart", &ResultSetReader::restart);
        resultReaderClass.addFunction("release", &ResultSetReader::release);
        resultReaderClass.endClass();

        auto resultWriterClass = module.beginClass<ResultSetWriter>("ResultSetWriter");
        bool(ResultSetWriter::*appendCurrLine)(std::shared_ptr<FileReader>) = &ResultSetWriter::appendCurrLine;
        resultWriterClass.addFunction("appendCurrLine", appendCurrLine);
        resultWriterClass.addFunction("getNumResults", &ResultSetWriter::getNumResults);
        resultWriterClass.addFunction("release", &ResultSetWriter::release);
        resultWriterClass.endClass();

        {
            auto lineScanner = module.beginClass<LineScanner>("LineScanner");
            lineScanner.addFactory([](std::shared_ptr<FileReader> fileReader, unsigned long long startLine, unsigned long long endLine) {
                auto lineScanner = std::shared_ptr<LineScanner>(new LineScanner(fileReader.get(), nullptr, startLine, endLine));
                if (lineScanner->initialize()) {
                    return lineScanner;
                }
                return std::shared_ptr<LineScanner>();
            });
            std::tuple<int, unsigned long long, std::string>(LineScanner::*nextLine)() = &LineScanner::nextLine;
            lineScanner.addFunction("nextLine", nextLine);
            lineScanner.endClass();
        }
        {
            auto lineScanner = module.beginClass<LineScanner>("LineScannerI");
            lineScanner.addFactory([](std::shared_ptr<FileReader> fileReader, std::shared_ptr<ResultSetReader> indexReader, 
                unsigned long long startLine, unsigned long long endLine
                ) {
                auto lineScanner = std::shared_ptr<LineScanner>(new LineScanner(fileReader.get(), indexReader.get(), startLine, endLine));
                if (lineScanner->initialize()) {
                    return lineScanner;
                }
                return std::shared_ptr<LineScanner>();
            });
            std::tuple<int, unsigned long long, std::string>(LineScanner::*nextLine)() = &LineScanner::nextLine;
            lineScanner.addFunction("nextLine", nextLine);
            lineScanner.endClass();
        }
        

        //Text compare
        auto tcTextComparator = module.beginClass<TextComparator>("TextComparator");
        bool(TextComparator::*match)(const std::string&) = &TextComparator::match;
        tcTextComparator.addFunction("initialize", &TextComparator::initialize);
        tcTextComparator.addFunction("match", match);
        tcTextComparator.endClass();

        auto tcMatchString = module.beginClass<MatchString>("MatchString");
        tcMatchString.addFactory([](const std::string& text, bool exact, bool ignoreCase) {
            return std::shared_ptr<TextComparator>(new MatchString(text, exact, ignoreCase));
        });
        tcMatchString.endClass();

        auto tcMatchMultiple = module.beginClass<MatchMultiple>("MatchMultiple");
        tcMatchMultiple.addFactory([](const std::vector<std::shared_ptr<TextComparator>>& comparators) {
            return std::shared_ptr<TextComparator>(new MatchMultiple(comparators));
        });
        tcMatchMultiple.endClass();

        auto tcMatchAny = module.beginClass<MatchAny>("MatchAny");
        tcMatchAny.addFactory([](const std::vector<std::shared_ptr<TextComparator>>& comparators) {
            return std::shared_ptr<TextComparator>(new MatchAny(comparators));
        });
        tcMatchAny.endClass();

        auto tcMatchRegex = module.beginClass<MatchRegex>("MatchRegex");
        tcMatchRegex.addFactory([](const std::string& regexPattern, bool ignoreCase) {
            return std::shared_ptr<TextComparator>(new MatchRegex(regexPattern, ignoreCase));
        });
        tcMatchRegex.endClass();

        auto tcMatchSubstrings = module.beginClass<MatchSubstrings>("MatchSubstrings");
        tcMatchSubstrings.addFactory([](const std::string& splitText, bool trimLine, const std::unordered_map<int, std::shared_ptr<TextComparator>>& sliceComparators) {
            return std::shared_ptr<TextComparator>(new MatchSubstrings(splitText, trimLine, sliceComparators));
        });
        tcMatchSubstrings.endClass();

        auto tcMatchWords = module.beginClass<MatchWords>("MatchWords");
        tcMatchWords.addFactory([](const std::unordered_map<int, std::shared_ptr<TextComparator>>& sliceComparators) {
            return std::shared_ptr<TextComparator>(new MatchWords(sliceComparators));
        });
        tcMatchWords.endClass();

        auto tcMatchCustom = module.beginClass<MatchCustom>("MatchCustom");
        tcMatchCustom.addFactory([](const std::function<bool(const std::string&)> func) {
            return std::shared_ptr<TextComparator>(new MatchCustom(func));
        });
        tcMatchCustom.endClass();

        module.endModule();
    }
}
