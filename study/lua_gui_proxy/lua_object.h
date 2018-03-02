#pragma once

#include "lua.hpp"
#include <memory>

namespace
{
    void lua_State_deleter(lua_State* lua);
}

typedef std::shared_ptr<lua_State> RefLuaState;

__declspec(dllexport) RefLuaState make_shared_lua_State();