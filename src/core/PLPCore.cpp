#include "PLPCore.h"

#include "Utils.h"

#include "lua.hpp"

namespace PLP {
    PLPCore::PLPCore() {}
    PLPCore::~PLPCore() {}

    bool PLPCore::initialize(
        unsigned int readBuffSizeKBytes
    ) {
        _state = luaL_newstate();
        if (!_state) return false;

        luaL_openlibs(_state);
        return true;
    }

    bool PLPCore::runScript(const std::wstring& scriptLua, std::wstring& errMsg) {
        if (scriptLua.empty()) {
            return true;
        }

        lua_settop(_state, 0);
        if (luaL_loadstring(_state, wstring_to_string(scriptLua).c_str())) {
            errMsg = string_to_wstring(lua_tostring(_state, -1));
            return false;
        }
        if (lua_pcall(_state, 0, LUA_MULTRET, 0)) {
            errMsg = string_to_wstring(lua_tostring(_state, -1));
            return false;
        }
        return true;
    }
}