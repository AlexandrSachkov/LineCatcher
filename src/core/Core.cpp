#include "Core.h"

#include "Thread.h"
#include "FileReader.h"
#include "FileWriter.h"
#include "IndexReader.h"
#include "IndexWriter.h"
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

    void release(PLP::CoreI* obj) {
        if (obj) {
            delete obj;
        }
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

        auto module = LuaIntf::LuaBinding(_state).beginModule("LC");
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
        const std::function<void(int percent)>* progressUpdate
    ) {
        _cancelled = false;

        std::function<void(int percent)>* progressUpdateInt = const_cast<std::function<void(int percent)>*>(progressUpdate);
        std::function<void(int percent)> defaultProgressUpdate = [](int percent) {};
        if (!progressUpdate) {
            progressUpdateInt = &defaultProgressUpdate;
        }

        std::unique_ptr<FileReader> fReader(new FileReader());
        if (!fReader->initialize(string_to_wstring(path), preferredBuffSizeBytes, _cancelled, progressUpdateInt)) {
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

    IndexReaderI* Core::createIndexReader(
        const std::string& path,
        unsigned long long preferredBuffSizeBytes
    ) {
        std::unique_ptr<IndexReader> resSet(new IndexReader());
        if (!resSet->initialize(string_to_wstring(path), preferredBuffSizeBytes)) {
            Logger::send(ERR, "Failed to create index reader");
            return nullptr;
        }
        Logger::send(INFO, "Successfully created index reader");
        return resSet.release();
    }

    IndexWriterI* Core::createIndexWriter(
        const std::string& path,
        unsigned long long preferredBuffSizeBytes,
        const FileReaderI* fReader,
        bool overwriteIfExists
    ) {
        if (!fReader) {
            Logger::send(ERR, "File reader is null");
            return false;
        }

        std::unique_ptr<IndexWriter> resSet(new IndexWriter());
        std::wstring dataPath(fReader->getFilePath());

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

    void Core::release(IndexReaderI* obj) {
        if (obj) {
            delete obj;
        }
    }

    void Core::release(IndexWriterI* obj) {
        if (obj) {
            delete obj;
        }
    }

    void Core::releaseFileReaderL(std::shared_ptr<FileReader>& p) {
        p.reset();
    }

    void Core::releaseFileWriterL(std::shared_ptr<FileWriter>& p) {
        p.reset();
    }

    void Core::releaseIndexReaderL(std::shared_ptr<IndexReader>& p) {
        p.reset();
    }

    void Core::releaseIndexWriterL(std::shared_ptr<IndexWriter>& p) {
        p.reset();
    }

    std::shared_ptr<FileReader> Core::createFileReaderL(
        const std::string& path,
        unsigned long long preferredBuffSizeBytes
    ) {
        std::function<void(int)> progressUpdate = [&](int percent) {
            printConsoleL("Opening file: " + std::to_string(percent) + "%");
        };

        return std::shared_ptr<FileReader>(
            static_cast<FileReader*>(createFileReader(path, preferredBuffSizeBytes, &progressUpdate))
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

    std::shared_ptr<IndexReader> Core::createIndexReaderL(
        const std::string& path,
        unsigned long long preferredBuffSizeBytes
    ) {
        return std::shared_ptr<IndexReader>(
            static_cast<IndexReader*>(createIndexReader(path, preferredBuffSizeBytes))
        );
    }

    std::shared_ptr<IndexWriter> Core::createIndexWriterL(
        const std::string& path,
        unsigned long long preferredBuffSizeBytes,
        const std::shared_ptr<FileReader> fReader,
        bool overwriteIfExists
    ) {
        return std::shared_ptr<IndexWriter>(
            static_cast<IndexWriter*>(createIndexWriter(path, preferredBuffSizeBytes, fReader.get(), overwriteIfExists))
        );
    }

    bool Core::searchGeneral(
        FileReaderI* fileReader,
        IndexReaderI* indexReader,
        unsigned long long start,
        unsigned long long end,
        TextComparator* comparator,
        const std::function<bool(unsigned long long lineNum, unsigned long long fileOffset, const char* line, unsigned int length)> action,
        const std::function<void(int percent)>* progressUpdate
    ) {
        _cancelled = false;

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

            if (numProcessedLines % 10000000 == 0 && _cancelled) {
                return false;
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
        IndexReaderI* indexReader,
        unsigned long long start,
        unsigned long long end, //0 for end of file, inclusive
        const std::unordered_map<int, TextComparator*>& lineComparators,
        const std::function<bool(unsigned long long lineNum, unsigned long long fileOffset, const char* line, unsigned int length)> action,
        const std::function<void(int percent)>* progressUpdate
    ) {
        _cancelled = false;

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

            if (numProcessedLines % 10000000 == 0 && _cancelled) {
                return false;
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
        IndexReaderI* indexReader,
        IndexWriterI* indexWriter,
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
        IndexReaderI* indexReader,
        IndexWriterI* indexWriter,
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
        std::shared_ptr<IndexWriter> indexWriter,
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
        std::shared_ptr<IndexReader> indexReader,
        std::shared_ptr<IndexWriter> indexWriter,
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
        std::shared_ptr<IndexWriter> indexWriter,
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
        std::shared_ptr<IndexReader> indexReader,
        std::shared_ptr<IndexWriter> indexWriter,
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
        auto module = LuaIntf::LuaBinding(state).beginModule("LC");
        module.addConstant("ERROR", LineReaderResult::ERROR);
        module.addConstant("NOT_FOUND", LineReaderResult::NOT_FOUND);
        module.addConstant("SUCCESS", LineReaderResult::SUCCESS);

        auto plpClass = module.beginClass<Core>("Core");
        plpClass.addFunction("createFileReader", &Core::createFileReaderL);
        plpClass.addFunction("createFileWriter", &Core::createFileWriterL);
        plpClass.addFunction("createIndexReader", &Core::createIndexReaderL);
        plpClass.addFunction("createIndexWriter", &Core::createIndexWriterL);
        plpClass.addFunction("releaseFileReader", &Core::releaseFileReaderL);
        plpClass.addFunction("releaseFileWriter", &Core::releaseFileWriterL);
        plpClass.addFunction("releaseIndexReader", &Core::releaseIndexReaderL);
        plpClass.addFunction("releaseIndexWriter", &Core::releaseIndexWriterL);
        plpClass.addFunction("search", &Core::searchL);
        plpClass.addFunction("searchI", &Core::searchIL);
        plpClass.addFunction("searchMultiline", &Core::searchMultilineL);
        plpClass.addFunction("searchMultilineI", &Core::searchMultilineIL);
        plpClass.addFunction("printConsole", &Core::printConsoleL);
        plpClass.addFunction("printConsoleEx", &Core::printConsoleExL);
        plpClass.addFunction("isCanceled", &Core::isCancelled);
        plpClass.endClass();

        {
            auto fileReaderClass = module.beginClass<FileReader>("FileReader");
            std::tuple<int, std::string>(FileReader::*nextLine)() = &FileReader::nextLine;
            std::tuple<int, std::string>(FileReader::*getLine)(unsigned long long) = &FileReader::getLine;
            std::tuple<int, std::string>(FileReader::*getLineFromResult)(const std::shared_ptr<IndexReader>)
                = &FileReader::getLineFromResult;
            fileReaderClass.addFunction("nextLine", nextLine);
            fileReaderClass.addFunction("getLine", getLine);
            fileReaderClass.addFunction("getLineFromResult", getLineFromResult);
            fileReaderClass.addFunction("getLineNumber", &FileReader::getLineNumber);
            fileReaderClass.addFunction("getNumberOfLines", &FileReader::getNumberOfLines);
            fileReaderClass.addFunction("restart", &FileReader::restart);
            fileReaderClass.endClass();
        }

        auto fileWriterClass = module.beginClass<FileWriter>("FileWriter");
        bool(FileWriter::*append)(const std::string&) = &FileWriter::append;
        bool(FileWriter::*appendLine)(const std::string&) = &FileWriter::appendLine;
        fileWriterClass.addFunction("append", append);
        fileWriterClass.addFunction("appendLine", appendLine);
        fileWriterClass.endClass();

        auto resultReaderClass = module.beginClass<IndexReader>("IndexReader");
        std::tuple<bool, unsigned long long>(IndexReader::*nextResult)() = &IndexReader::nextResult;
        resultReaderClass.addFunction("nextIndex", nextResult);
        resultReaderClass.addFunction("getLineNumber", &IndexReader::getLineNumber);
        resultReaderClass.addFunction("getNumIndexes", &IndexReader::getNumResults);
        resultReaderClass.addFunction("getFilePath", &IndexReader::getFilePath);
        resultReaderClass.addFunction("getDataFilePath", &IndexReader::getDataFilePath);
        resultReaderClass.addFunction("restart", &IndexReader::restart);
        resultReaderClass.endClass();

        auto resultWriterClass = module.beginClass<IndexWriter>("IndexWriter");
        bool(IndexWriter::*appendCurrLine)(std::shared_ptr<FileReader>) = &IndexWriter::appendCurrLine;
        resultWriterClass.addFunction("appendCurrentLine", appendCurrLine);
        resultWriterClass.addFunction("getNumIndexes", &IndexWriter::getNumResults);
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
            lineScanner.addFactory([](std::shared_ptr<FileReader> fileReader, std::shared_ptr<IndexReader> indexReader, 
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
        tcTextComparator.addFunction("match", match);
        tcTextComparator.endClass();

        auto tcMatchString = module.beginClass<MatchString>("MatchString");
        tcMatchString.addFactory([](const std::string& text, bool exact, bool ignoreCase) -> std::shared_ptr<TextComparator> {
            std::shared_ptr<TextComparator> comparator(new MatchString(text, exact, ignoreCase));
            if (!comparator->initialize()) {
                return nullptr;
            }
            return comparator;
        });
        tcMatchString.endClass();

        auto tcMatchMultiple = module.beginClass<MatchAll>("MatchAll");
        tcMatchMultiple.addFactory([](const std::vector<std::shared_ptr<TextComparator>>& comparators) -> std::shared_ptr<TextComparator> {
            std::shared_ptr<TextComparator> comparator(new MatchAll(comparators));
            if (!comparator->initialize()) {
                return nullptr;
            }
            return comparator;
        });
        tcMatchMultiple.endClass();

        auto tcMatchAny = module.beginClass<MatchAny>("MatchAny");
        tcMatchAny.addFactory([](const std::vector<std::shared_ptr<TextComparator>>& comparators) -> std::shared_ptr<TextComparator> {
            std::shared_ptr<TextComparator> comparator(new MatchAny(comparators));
            if (!comparator->initialize()) {
                return nullptr;
            }
            return comparator;
        });
        tcMatchAny.endClass();

        auto tcMatchRegex = module.beginClass<MatchRegex>("MatchRegex");
        tcMatchRegex.addFactory([](const std::string& regexPattern, bool ignoreCase)  -> std::shared_ptr<TextComparator> {
            std::shared_ptr<TextComparator> comparator(new MatchRegex(regexPattern, ignoreCase));
            if (!comparator->initialize()) {
                return nullptr;
            }
            return comparator;
        });
        tcMatchRegex.endClass();

        auto tcMatchSubstrings = module.beginClass<MatchSubstrings>("MatchSubstrings");
        tcMatchSubstrings.addFactory([](const std::string& splitText, bool trimLine, const std::unordered_map<int, std::shared_ptr<TextComparator>>& sliceComparators)  -> std::shared_ptr<TextComparator> {
            std::shared_ptr<TextComparator> comparator(new MatchSubstrings(splitText, trimLine, sliceComparators));
            if (!comparator->initialize()) {
                return nullptr;
            }
            return comparator;
        });
        tcMatchSubstrings.endClass();

        auto tcMatchWords = module.beginClass<MatchWords>("MatchWords");
        tcMatchWords.addFactory([](const std::unordered_map<int, std::shared_ptr<TextComparator>>& sliceComparators)  -> std::shared_ptr<TextComparator> {
            std::shared_ptr<TextComparator> comparator(new MatchWords(sliceComparators));
            if (!comparator->initialize()) {
                return nullptr;
            }
            return comparator;
        });
        tcMatchWords.endClass();

        auto tcMatchCustom = module.beginClass<MatchCustom>("MatchCustom");
        tcMatchCustom.addFactory([](const std::function<bool(const std::string&)> func)  -> std::shared_ptr<TextComparator> {
            std::shared_ptr<TextComparator> comparator(new MatchCustom(func));
            if (!comparator->initialize()) {
                return nullptr;
            }
            return comparator;
        });
        tcMatchCustom.endClass();

        module.endModule();
    }
}
