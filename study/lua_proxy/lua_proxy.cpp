// lua_proxy.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"

#include "lua.hpp"


extern "C"
{
    int average(lua_State *lua)
    {
        double sum = 0;
        int num = lua_gettop(lua);//获取参数的个数
        int actual_num = 0;
        for (int i = 1; i <= num; i++)
        {
            if (lua_isnumber(lua, i))
            {
                actual_num++;
                sum += lua_tonumber(lua, i);
            }
        }
        //依次获取所有参数值，相加
        lua_pushnumber(lua, sum / actual_num);//将平均数压如栈，供lua获取

        printf("c++ average funtion\n");

        return 1;//返回返回值个数，通知lua应该在栈里取几个值作为返回结果
    }

    int print_str(lua_State *lua)
    {
        const char *str = lua_tostring(lua, 1);
        printf("c++ print_str %s\n", str);

        lua_pushstring(lua, "lua pushstring");
        return 1;
    }

    luaL_Reg cFuntions[] = 
    {
        { "average", average },
        { "print_str", print_str },
        { nullptr, nullptr }
    };

    __declspec(dllexport) int luaopen_lua_proxy(lua_State *lua)
    {
        luaL_newlib(lua, cFuntions);
        return 1;
    }
}
