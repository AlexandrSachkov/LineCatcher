#pragma once
#include <string>

struct lua_State;

namespace PLP {
    class PLPCore {
    public:
        PLPCore();
        ~PLPCore();

        bool initialize(
            unsigned int readBuffSizeKBytes
        );

        bool runScript(const std::wstring& scriptLua, std::wstring& errMsg);

    private:
        lua_State* _state;
    };
}