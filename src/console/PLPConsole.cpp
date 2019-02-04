#include "PLPCore.h"
#include "Timer.h"

int main() {
    PLP::PLPCore core;
    if (!core.initialize()) {
        return 1;
    }

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
    const std::wstring smallFile = L"D:/Repositories/LogParser/resources/test.txt";
    const std::wstring largeFile = L"D:/Repositories/LogParser/resources/ShadowOfTombRaider_Vega_18.12.2_RS4_2_SOTTR_64_4b1d4982_1_11950.dump";

    unsigned int numMatches = 0;
    /*if (!core.searchLineContainsMM(largeFile, L"0x00000003dcf2a690", numMatches)) {
        return 1;
    }*/

    if (!core.searchLineContainsMMIndexed(smallFile, L"0x00000003dcf2a690", numMatches)) {
        return 1;
    }
    
    double numSeconds = timer.deltaT() / 1000000000;
    printf("Completed in: %f seconds\n", numSeconds);
    //printf("Num matches: %i\n", numMatches);
    return 0;
}