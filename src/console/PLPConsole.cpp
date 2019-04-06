#include "Core.h"
#include "Timer.h"
#include "FileReader.h"
#include "FileWriter.h"
#include "ResultSetReader.h"
#include "ResultSetWriter.h"
#include "TextComparator.h"

int main() {
    PLP::CoreI* core = PLP::createCore();
    //std::unique_ptr<PLP::Core> core = std::unique_ptr<PLP::Core>(createCore());
    if (!core->initialize()) {
        return 1;
    }

    std::function<void(int, const char*)> printConsole = [](int level, const char* msg) {
        printf("%s\n", msg);
    };
    core->attachLogOutput("console", &printConsole);

    std::wstring err;
    std::wstring script1 =
        L"file = io.open(\"test.txt\", \"a\");"
        "file:write(\"hello\");"
        ;

    std::wstring script2 =
        L"print(1234)";

    std::wstring script3 =
        L"local i = 0;"
        "file = io.open(\"D:/Repositories/LogParser/resources/ShadowOfTombRaider_Vega_18.12.2_RS4_2_SOTTR_64_4b1d4982_1_11950.dump\", \"r\");"
        ;

    std::wstring script4 =
        L"local i = 0;\n"
        "local lineNum = 1;\n"
        "file = io.open(\"D:/Repositories/LogParser/resources/ShadowOfTombRaider_Vega_18.12.2_RS4_2_SOTTR_64_4b1d4982_1_11950.dump\", \"rb\");\n"
        "local lineLast = \"\";\n"
        "local line = \"\";\n"
        "local index = {}"
        "while true do\n"
        "lineLast = line;\n"
        "line = file:read();\n"
        "if line == nil then break end\n"
        "    if string.find(line, \"ID3D12GraphicsCommandList[0x00000003dcf2a690]::DiscardResource\") then\n"
        "        table.insert(index, lineNum);\n"
        "    end\n"
        "    if (lineNum % 1000000 == 0) then\n"
        "        print(lineNum)\n"
        "    end\n"
        "    lineNum = lineNum + 1;\n"
        "end\n"
        "for k, v in pairs(index) do\n"
        "    print(v, \" \")\n"
        "end\n"
        ;

    std::wstring script5 =
        L"local fileReader = PLP:core():createFileReader(\"D:/Repositories/LogParser/resources/ShadowOfTombRaider_Vega_18.12.2_RS4_2_SOTTR_64_4b1d4982_1_11950.dump\", 4000000000);\n"
        "local fileWriter = PLP:core():createFileWriter(\"D:/Repositories/LogParser/resources/searchResults.txt\", 1000000);\n"
        "if (fileReader == nil) then\n"
        "   print(\"failed\")\n"
        "   return\n"
        "end\n"
        "local numResults = 0;\n"
        "local lineValid, line = fileReader:nextLine();\n"
        "while lineValid == true do\n"
        "   if (line:find(\"ID3D12GraphicsCommandList[0x00000003dcf2a690]::DiscardResource\", 1, true)) then\n"
        "       numResults = numResults + 1;\n"
        "       print(fileReader:lineNumber(), \" \",line)\n"
        "       fileWriter:appendLine(fileReader:lineNumber() .. \"  \" .. line)\n"
        "   end\n"
        "   lineValid, line = fileReader:nextLine();\n"
        "end\n"
        "print(\"NUM RESULTS: \",numResults)"
        ;

    std::wstring script6 =
        L"local fileReader = PLP:core():createFileReader(\"D:/Repositories/LogParser/resources/test.txt\", 1000000);\n"
        "local fileWriter = PLP:core():createFileWriter(\"D:/Repositories/LogParser/resources/searchResults.txt\", 1000000);\n"
        "if (fileReader == nil or fileWriter == nil) then\n"
        "   print(\"failed\")\n"
        "   return\n"
        "end\n"
        "local numResults = 0;\n"
        "local lineValid, line = fileReader:nextLine();\n"
        "while lineValid == true do\n"
        "   fileWriter:appendLine(fileReader:lineNumber() .. \"  \" .. line)\n"
        "   lineValid, line = fileReader:nextLine();\n"
        "end\n"
        "print(\"NUM RESULTS: \",numResults)"
        ;

    PLP::Timer timer;

    /*if (!core.runScript(script6, err)) {
        return 1;
    }*/
    const std::string smallFile = "D:/Repositories/LogParser/resources/test.txt";
    const std::string largeFile = "D:/Repositories/LogParser/resources/ShadowOfTombRaider_Vega_18.12.2_RS4_2_SOTTR_64_4b1d4982_1_11950.dump";

    unsigned int numMatches = 0;
    /*if (!core.searchLineContainsMM(largeFile, L"0x00000003dcf2a690", numMatches)) {
        return 1;
    }*/

    /*if (!core.searchLineContainsMMIndexed(smallFile, L"0x00000003dcf2a690", numMatches)) {
        return 1;
    }*/

    /*auto fReader = core->createFileReader(largeFile, 0, true);
    char* line;
    unsigned int lineSize;
    if (!fReader->getLine(100000000, line, lineSize)) {
        return 1;
    }
    if (!fReader->getLine(100, line, lineSize)) {
        return 1;
    }*/

    /*auto fReader = core->createFileReader(smallFile, 0, true);
    auto rWriter = core->createResultSetWriter("results.txt", 0, fReader, true);
    char* line;
    unsigned int lineSize;
    while (fReader->nextLine(line, lineSize)) {
        if (!rWriter->appendCurrLine(fReader)) {
            return 1;
        }
    }

    rWriter->release();

    auto rReader = core->createResultSetReader("results.txt", 0);
    std::wstring dataPath;
    rReader->getDataFilePath(dataPath);
    unsigned long long numResults = rReader->getNumResults();

    unsigned long long lineNum;
    if (!rReader->getResult(10, lineNum)) {
        return 1;
    }

    if (!rReader->getResult(5, lineNum)) {
        return 1;
    }*/

    /*auto rReader = core->createResultSetReader("results.txt", 0);
    std::wstring dataPath;
    rReader->getDataFilePath(dataPath);
    unsigned long long numResults = rReader->getNumResults();

    char* resLine;
    unsigned int resLineSize;
    unsigned long long lineNum;
    while (rReader->nextResult(lineNum)) {
        if (!fReader->getLineFromResult(rReader, resLine, resLineSize)) {
            return 1;
        }
    }*/
    

    /*auto fReader = core->createFileReader(largeFile, 0, true);
    auto rWriter = core->createResultSetWriter("largeFileResults2", 0, fReader, true);
    char* line;
    unsigned int lineSize;
    while (fReader->nextLine(line, lineSize)) {
        std::string lineData(line, lineSize);
        if (std::string::npos != lineData.find("ID3D12GraphicsCommandList[0x00000003dcf2a690]::DiscardResource")) {
            if (!rWriter->appendCurrLine(fReader)) {
                return 1;
            }
        }
        if (fReader->getLineNumber() % 10000000 == 0) {
            printf("%llu\n", fReader->getLineNumber());
            printf("Num results: %llu\n", rWriter->getNumResults());
        }
    }
    rWriter->release();
    fReader->release();*/

    std::string text = "   this   is a test ";
    std::unordered_map<int, std::shared_ptr<PLP::TextComparator>> map = {
        {0, std::shared_ptr<PLP::TextComparator>(new PLP::MatchString("this ", true))},
        {-1, std::shared_ptr<PLP::TextComparator>(new PLP::MatchString("test", true))}
    };
    std::shared_ptr<PLP::TextComparator> comp(new PLP::MatchSubstrings(" ", true, map));
    bool res = comp->initialize();
    res = comp->match(text);

    double numSeconds = timer.deltaT() / 1000000000;
    printf("Completed in: %f seconds\n", numSeconds);
    //printf("Num matches: %i\n", numMatches);
    return 0;
}