#include "PLPCore.h"

int main() {
    PLP::PLPCore core;
    if (!core.initialize(4000000000)) {
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

    /*if (!core.runScript(script4, err)) {
        return 1;
    }*/

    unsigned int numMatches = 0;
    if (!core.searchLineContainsWithPreloadMM(L"D:/Repositories/LogParser/resources/ShadowOfTombRaider_Vega_18.12.2_RS4_2_SOTTR_64_4b1d4982_1_11950.dump", L"0x00000003dcf2a690", numMatches)) {
        return 1;
    }
    printf("Num matches: %i\n", numMatches);
    return 0;
}