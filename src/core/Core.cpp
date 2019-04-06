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
#include "CircularLineBuffer.h"
#include "TextComparator.h"

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

    bool Core::search(
        FileReaderI* fileReader,
        ResultSetWriterI* indexWriter,
        unsigned long long startLine,
        unsigned long long endLine, //0 for end of file
        unsigned long long maxNumResults,
        const std::wstring& searchText,
        bool plainTextSearch, //false for regex
        bool ignoreCase,
        const std::function<void(int, unsigned long long)>* progressUpdate
    ) {
        if (fileReader == nullptr || indexWriter == nullptr) {
            Logger::send(ERR, "File reader and index writer cannot be null");
            return false;
        }
        if (startLine > endLine) {
            Logger::send(ERR, "Start line must be smaller or equal to end line");
            return false;
        }
        if (searchText.empty()) {
            Logger::send(ERR, "Search text cannot be empty");
            return false;
        }
        std::function<void(int, unsigned long long)> defaultProgressUpdate = [](int, unsigned long long) {};
        if (progressUpdate == nullptr) {
            progressUpdate = &defaultProgressUpdate;
        }
        if (maxNumResults == 0) {
            maxNumResults = ULLONG_MAX;
        }

        const unsigned long long start = startLine;
        const unsigned long long end = endLine > 0 ? endLine : fileReader->getNumberOfLines() - 1;
        const long double dLinesPerPercent = (end - start + 1) / 100.0;
        const unsigned long long numLinesTillProgressUpdate = dLinesPerPercent > 1.0 ? (unsigned long long)dLinesPerPercent : 1;
        const int percentPerProgressUpdate = dLinesPerPercent > 1.0 ? 1 : (int)(1.0 / dLinesPerPercent); 

        const std::string text = wstring_to_string(searchText);
        std::unique_ptr<TextComparator> comparator = nullptr;
        if (plainTextSearch) {
            comparator.reset(new MatchString(text, false));
        } else {
            comparator.reset(new MatchRegex(text, false));
        }
        if (!comparator->initialize()) {
            return false;
        }

        // find and process first line
        int progressPercent = 0;
        char* line;
        unsigned int lineSize;
        unsigned long long numProcessedLines = 0;
        if (fileReader->getLine(start, line, lineSize) == LineReaderResult::ERROR) {
            Logger::send(ERR, "Failed to get line");
            return false;
        }

        if (comparator->match(line, lineSize)) {
            if (!indexWriter->appendCurrLine(fileReader)) {
                Logger::send(ERR, "Failed to append search result");
                return false;
            }
            if (indexWriter->getNumResults() == maxNumResults) {
                if (progressPercent < 100) {
                    (*progressUpdate)(100, indexWriter->getNumResults());
                }
                return true;
            }
        }

        numProcessedLines++;
        if (numProcessedLines % numLinesTillProgressUpdate == 0) {
            progressPercent += percentPerProgressUpdate;
            (*progressUpdate)(progressPercent, indexWriter->getNumResults());
        }

        // find and process the rest 
        LineReaderResult result;
        while (fileReader->getLineNumber() <= end 
            && (result = fileReader->nextLine(line, lineSize)) == LineReaderResult::SUCCESS) {
            if (comparator->match(line, lineSize)) {
                if (!indexWriter->appendCurrLine(fileReader)) {
                    Logger::send(ERR, "Failed to append search result");
                    return false;
                }
                if (indexWriter->getNumResults() == maxNumResults) {
                    break;
                }
            }

            numProcessedLines++;
            if (numProcessedLines % numLinesTillProgressUpdate == 0) {
                progressPercent += percentPerProgressUpdate;
                (*progressUpdate)(progressPercent, indexWriter->getNumResults());
            }
        }
        if (result == LineReaderResult::ERROR) {
            Logger::send(ERR, "Failed to get line");
            return false;
        }

        if (progressPercent < 100) {
            (*progressUpdate)(100, indexWriter->getNumResults());
        }
        
        return true;
    }

    bool Core::searchI(
        FileReaderI* fileReader,
        ResultSetReaderI* indexReader,
        ResultSetWriterI* indexWriter,
        unsigned long long startIndex,
        unsigned long long endIndex,
        unsigned long long maxNumResults,
        const std::wstring& searchText,
        bool plainTextSearch,
        bool ignoreCase,
        const std::function<void(int percent, unsigned long long numResults)>* progressUpdate
    ) {
        if (fileReader == nullptr || indexReader == nullptr || indexWriter == nullptr) {
            Logger::send(ERR, "File reader, index reader and index writer cannot be null");
            return false;
        }
        if (startIndex > endIndex) {
            Logger::send(ERR, "Start index must be smaller or equal to end index");
            return false;
        }
        if (searchText.empty()) {
            Logger::send(ERR, "Search text cannot be empty");
            return false;
        }
        std::function<void(int, unsigned long long)> defaultProgressUpdate = [](int, unsigned long long) {};
        if (progressUpdate == nullptr) {
            progressUpdate = &defaultProgressUpdate;
        }
        if (maxNumResults == 0) {
            maxNumResults = ULLONG_MAX;
        }

        const unsigned long long start = startIndex;
        const unsigned long long end = endIndex > 0 ? endIndex : indexReader->getNumResults() - 1;
        const long double dLinesPerPercent = (end - start + 1) / 100.0;
        const unsigned long long numLinesTillProgressUpdate = dLinesPerPercent > 1.0 ? (unsigned long long)dLinesPerPercent : 1;
        const int percentPerProgressUpdate = dLinesPerPercent > 1.0 ? 1 : (int)(1.0 / dLinesPerPercent);
        const std::string text = wstring_to_string(searchText);

        char* line;
        unsigned int lineSize;

        std::function<bool()> match;
        if (plainTextSearch) {
            match = [&]() -> bool {
                return std::string::npos != std::string(line, lineSize).find(text);
            };
        } else {
            std::regex regex(text);
            match = [&, regex]() -> bool {
                return std::regex_match(std::string(line, lineSize), regex);
            };
        }

        int progressPercent = 0;
        unsigned long long numProcessedLines = 0;
        unsigned long long lineNumber = 0;
        if (!indexReader->getResult(start, lineNumber)) {
            Logger::send(ERR, "Failed to get index");
            return false;
        }
        if (fileReader->getLineFromResult(indexReader, line, lineSize) == LineReaderResult::ERROR) {
            Logger::send(ERR, "Failed to get line");
            return false;
        }
        if (match()) {
            if (!indexWriter->appendCurrLine(fileReader)) {
                Logger::send(ERR, "Failed to append search result");
                return false;
            }
            if (indexWriter->getNumResults() == maxNumResults) {
                if (progressPercent < 100) {
                    (*progressUpdate)(100, indexWriter->getNumResults());
                }
                return true;
            }
        }

        numProcessedLines++;
        if (numProcessedLines % numLinesTillProgressUpdate == 0) {
            progressPercent += percentPerProgressUpdate;
            (*progressUpdate)(progressPercent, indexWriter->getNumResults());
        }

        while (indexReader->getResultNumber() <= end && indexReader->nextResult(lineNumber)) {
            if (fileReader->getLineFromResult(indexReader, line, lineSize) == LineReaderResult::ERROR) {
                Logger::send(ERR, "Failed to get line");
                return false;
            }
            if (match()) {
                if (!indexWriter->appendCurrLine(fileReader)) {
                    Logger::send(ERR, "Failed to append search result");
                    return false;
                }
                if (indexWriter->getNumResults() == maxNumResults) {
                    break;
                }
            }

            numProcessedLines++;
            if (numProcessedLines % numLinesTillProgressUpdate == 0) {
                progressPercent += percentPerProgressUpdate;
                (*progressUpdate)(progressPercent, indexWriter->getNumResults());
            }
        }

        if (progressPercent < 100) {
            (*progressUpdate)(100, indexWriter->getNumResults());
        }
        return true;
    }

    bool Core::searchM(
        FileReaderI* fileReader,
        ResultSetWriterI* indexWriter,
        unsigned long long startLine,
        unsigned long long endLine, //0 for end of file, inclusive
        unsigned long long maxNumResults, //0 for all
        const std::vector<MultilineSearchParams>& searchParams,
        const std::function<void(int percent, unsigned long long numResults)>* progressUpdate
    ) {
        /*if (fileReader == nullptr || indexWriter == nullptr) {
            Logger::send(ERR, "File reader and index writer cannot be null");
            return false;
        }
        if (startLine > endLine) {
            Logger::send(ERR, "Start line must be smaller or equal to end line");
            return false;
        }
        if (searchParams.size() == 0) {
            Logger::send(ERR, "No search parameters provided");
            return false;
        }
        std::function<void(int, unsigned long long)> defaultProgressUpdate = [](int, unsigned long long) {};
        if (progressUpdate == nullptr) {
            progressUpdate = &defaultProgressUpdate;
        }
        if (maxNumResults == 0) {
            maxNumResults = ULLONG_MAX;
        }

        std::vector<MultilineSearchParams> params = searchParams;
        std::sort(params.begin(), params.end(), [](MultilineSearchParams& param1, MultilineSearchParams& param2) {
            return param1.getLineOffset() > param2.getLineOffset();
        });

        const int startLineOffset = params[0].getLineOffset();

        int numUniqueLines = 0;
        for(int i = 0; i < params.size(); i++){
            if (i == 0 || i != i - 1) {
                numUniqueLines++;
            }
        }

        CircularLineBuffer lineBuffer;
        if (!lineBuffer.initialize(100000, numUniqueLines)) {
            Logger::send(ERR, "Failed to initialize line buffer");
            return false;
        }*/
        

        return true;
    }

    bool Core::searchMI(
        FileReaderI* fileReader,
        ResultSetReaderI* indexReader,
        ResultSetWriterI* indexWriter,
        unsigned long long startLine,
        unsigned long long endLine, //0 for end of file, inclusive
        unsigned long long maxNumResults, //0 for all
        const std::vector<MultilineSearchParams>& searchParams,
        const std::function<void(int percent, unsigned long long numResults)>* progressUpdate
    ) {
        return true;
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

    bool Core::searchL(
        std::shared_ptr<FileReader> fileReader,
        std::shared_ptr<ResultSetWriter> indexWriter,
        unsigned long long startLine,
        unsigned long long endLine, //0 for end of file, inclusive
        unsigned long long maxNumResults,
        const std::string& searchText,
        bool plainTextSearch, //false for regex
        bool ignoreCase
    ) {
        std::function<void(int, unsigned long long)> progressUpdate = [&](int percent, unsigned long long numResults) {
            printConsoleL(std::to_string(percent) + "%, Found results: " + std::to_string(numResults));
        };
        return search(
            fileReader.get(),
            indexWriter.get(),
            startLine, endLine,
            maxNumResults,
            string_to_wstring(searchText),
            plainTextSearch,
            ignoreCase,
            &progressUpdate
        );
    }

    bool Core::searchIL(
        std::shared_ptr<FileReader> fileReader,
        std::shared_ptr<ResultSetReader> indexReader,
        std::shared_ptr<ResultSetWriter> indexWriter,
        unsigned long long startIndex,
        unsigned long long endIndex,
        unsigned long long maxNumResults,
        const std::string& searchText,
        bool plainTextSearch,
        bool ignoreCase
    ) {
        std::function<void(int, unsigned long long)> progressUpdate = [&](int percent, unsigned long long numResults) {
            printConsoleL(std::to_string(percent) + "%, Found results: " + std::to_string(numResults));
        };
        return searchI(
            fileReader.get(),
            indexReader.get(),
            indexWriter.get(),
            startIndex, endIndex,
            maxNumResults,
            string_to_wstring(searchText),
            plainTextSearch,
            ignoreCase,
            &progressUpdate
        );
    }

    bool Core::searchML(
        std::shared_ptr<FileReader> fileReader,
        std::shared_ptr<ResultSetWriter> indexWriter,
        unsigned long long startLine,
        unsigned long long endLine, //0 for end of file, inclusive
        unsigned long long maxNumResults, //0 for all
        const std::vector<MultilineSearchParams>& searchParams
    ) {
        std::function<void(int, unsigned long long)> progressUpdate = [&](int percent, unsigned long long numResults) {
            printConsoleL(std::to_string(percent) + "%, Found results: " + std::to_string(numResults));
        };
        return searchM(
            fileReader.get(),
            indexWriter.get(),
            startLine, endLine,
            maxNumResults,
            searchParams,
            &progressUpdate
        );
    }

    bool Core::searchMIL(
        std::shared_ptr<FileReader> fileReader,
        std::shared_ptr<ResultSetReader> indexReader,
        std::shared_ptr<ResultSetWriter> indexWriter,
        unsigned long long startIndex,
        unsigned long long endIndex, //0 for end of file, inclusive
        unsigned long long maxNumResults, //0 for all
        const std::vector<MultilineSearchParams>& searchParams
    ) {
        std::function<void(int, unsigned long long)> progressUpdate = [&](int percent, unsigned long long numResults) {
            printConsoleL(std::to_string(percent) + "%, Found results: " + std::to_string(numResults));
        };
        return searchMI(
            fileReader.get(),
            indexReader.get(),
            indexWriter.get(),
            startIndex, endIndex,
            maxNumResults,
            searchParams,
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

        auto mspClass = module.beginClass<MultilineSearchParams>("MSP");
        mspClass.addConstructor(LUA_ARGS(int, int, bool, const std::string&, bool, bool));
        mspClass.addFunction("getLineOffset", &MultilineSearchParams::getLineOffset);
        mspClass.addFunction("getWordIndex", &MultilineSearchParams::getWordIndex);
        mspClass.addFunction("getSearchText", &MultilineSearchParams::getSearchText);
        mspClass.addFunction("plainText", &MultilineSearchParams::plainText);
        mspClass.addFunction("ignoreCase", &MultilineSearchParams::ignoreCase);
        mspClass.endClass();

        auto plpClass = module.beginClass<Core>("Core");
        plpClass.addFunction("createFileReader", &Core::createFileReaderL);
        plpClass.addFunction("createFileWriter", &Core::createFileWriterL);
        plpClass.addFunction("createResultSetReader", &Core::createResultSetReaderL);
        plpClass.addFunction("createResultSetWriter", &Core::createResultSetWriterL);
        plpClass.addFunction("search", &Core::searchL);
        plpClass.addFunction("searchI", &Core::searchIL);
        plpClass.addFunction("searchM", &Core::searchML);
        plpClass.addFunction("searchMI", &Core::searchMIL);
        plpClass.addFunction("printConsole", &Core::printConsoleL);
        plpClass.addFunction("printConsoleEx", &Core::printConsoleExL);
        plpClass.addFunction("isCancelled", &Core::isCancelled);
        plpClass.endClass();

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

        //Text compare
        auto tcModule = module.beginModule("TC");

        auto tcTextComparator = tcModule.beginClass<TextComparator>("TextComparator");
        bool(TextComparator::*match)(const std::string&) = &TextComparator::match;
        tcTextComparator.addFunction("initialize", &TextComparator::initialize);
        tcTextComparator.addFunction("match", match);
        tcTextComparator.endClass();

        auto tcContains = tcModule.beginClass<MatchString>("MatchString");
        tcContains.addFactory([](const std::string& text, bool exact) {
            return std::shared_ptr<TextComparator>(new MatchString(text, exact));
        });
        tcContains.endClass();

        auto tcMatchMultiple = tcModule.beginClass<MatchMultiple>("MatchMultiple");
        tcMatchMultiple.addFactory([](const std::vector<std::shared_ptr<TextComparator>>& comparators) {
            return std::shared_ptr<TextComparator>(new MatchMultiple(comparators));
        });
        tcMatchMultiple.endClass();

        auto tcMatchAny = tcModule.beginClass<MatchAny>("MatchAny");
        tcMatchAny.addFactory([](const std::vector<std::shared_ptr<TextComparator>>& comparators) {
            return std::shared_ptr<TextComparator>(new MatchAny(comparators));
        });
        tcMatchAny.endClass();

        auto tcRegexMatch = tcModule.beginClass<MatchRegex>("MatchRegex");
        tcRegexMatch.addFactory([](const std::string& regexPattern, bool ignoreCase) {
            return std::shared_ptr<TextComparator>(new MatchRegex(regexPattern, ignoreCase));
        });
        tcRegexMatch.endClass();

        auto tcSplit = tcModule.beginClass<MatchSubstrings>("MatchSubstrings");
        tcSplit.addFactory([](const std::string& splitText, bool trimLine, const std::unordered_map<int, std::shared_ptr<TextComparator>>& sliceComparators) {
            return std::shared_ptr<TextComparator>(new MatchSubstrings(splitText, trimLine, sliceComparators));
        });
        tcSplit.endClass();

        tcModule.endModule();
        module.endModule();
    }
}