#include "PLPCore.h"

int main() {
    PLP::PLPCore core;
    if (!core.initialize(1)) {
        return 1;
    }

    std::wstring err;
    if (!core.runScript(L"file = io.open(\"test.txt\", \"a\"); file:write(\"hello\");", err)) {
        return 1;
    }
    return 0;
}