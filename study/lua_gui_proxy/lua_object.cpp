#include "stdafx.h"
#include "lua_object.h"

namespace
{
    void lua_State_deleter(lua_State* lua)
    {
        lua_close(lua);
        lua = nullptr;
    }
}

__declspec(dllexport) RefLuaState make_shared_lua_State()
{
    RefLuaState lua(luaL_newstate(), lua_State_deleter);
    return lua;
}
