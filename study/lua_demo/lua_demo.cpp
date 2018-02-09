// lua_demo.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <process.h>

#include "lua.hpp"

#include <io.h>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <conio.h>
#include <winuser.rh>

int lua_call_cpp_fn(lua_State *lua)
{
    lua_pushnumber(lua, 2233);
    return 1;
}

void stackDump(lua_State* L)
{
    std::cout << "\nbegin dump lua stack" << std::endl;
    int i = 0;
    int top = lua_gettop(L);
    for (i = 1; i <= top; ++i)
    {
        int t = lua_type(L, i);
        switch (t)
        {
        case LUA_TSTRING: {
            printf("'%s' ", lua_tostring(L, i));
        }
                          break;
        case LUA_TBOOLEAN: {
            printf(lua_toboolean(L, i) ? "true " : "false ");
        }
                           break;
        case LUA_TNUMBER: {
            printf("%g ", lua_tonumber(L, i));
        }
                          break;
        default: {
            printf("%s ", lua_typename(L, t));
        }
                 break;
        }
    }
    std::cout << "\nend dump lua stack" << std::endl;
}

bool test_lua()
{
    bool ret = false;
    lua_State *lua = luaL_newstate();
    if (lua)
    {
        // 载入Lua基本库
        luaL_openlibs(lua);

        // lua中有调用到宿主函数的要先注册函数
        lua_register(lua, "lua_call_cpp_fn", lua_call_cpp_fn);

        luaL_newlib; luaL_requiref; luaL_newmetatable; luaL_setfuncs;

        // 加载脚本
        //if (LUA_OK == luaL_loadfile(lua, "lua_script/test.lua"))
        // 执行脚本
        //lua_pcall(lua, 0, 0, 0);

        int code = luaL_dofile(lua, "lua_script/test.lua"); 
        if (LUA_OK == code)// 加载并执行脚本
        {
            std::cout << "--------------cpp begin----------------" << std::endl;

            //读取变量
            lua_getglobal(lua, "name");   //string to be indexed
            std::cout << "name = " << lua_tostring(lua, -1) << std::endl;

            //读取数字
            lua_getglobal(lua, "date"); //number to be indexed
            std::cout << "date = " << lua_tonumber(lua, -1) << std::endl;

            //读取表
            lua_getglobal(lua, "me");  //table to be indexed
            if (!lua_istable(lua, -1))
            {
                std::cout << "error:it is not a table" << std::endl;
            }
            //取表中元素
            lua_getfield(lua, -1, "name");
            std::cout << "user name = " << lua_tostring(lua, -1) << std::endl;
            lua_getfield(lua, -2, "mail");
            std::cout << "user mail = " << lua_tostring(lua, -1) << std::endl;

            //查看栈
            stackDump(lua);

            //修改表中元素
            lua_pushstring(lua, "wh");
            lua_setfield(lua, -4, "name");
            lua_getfield(lua, -3, "name");
            std::cout << "user new name = " << lua_tostring(lua, -1) << std::endl;

            //查看栈
            stackDump(lua);

            //取函数
            lua_getglobal(lua, "add");
            lua_pushnumber(lua, 15);
            lua_pushnumber(lua, 5);
            lua_pcall(lua, 2, 1, 0);//2-参数格式，1-返回值个数，调用函数，函数执行完，会将返回值压入栈中
            std::cout << "add fn result = " << lua_tonumber(lua, -1) << std::endl;

            //查看栈
            stackDump(lua);

            std::cout << "--------------cpp end----------------" << std::endl;

            ret = true;
        }
        else
        {
            std::cout << lua_tostring(lua, -1) << std::endl;
        }

        //关闭state
        lua_close(lua);
    }
    return ret;
}

int _tmain(int argc, _TCHAR* argv[])
{
    do 
    {
        test_lua();
    } while (/*getchar()*//*getc(stdin)*/_getche() != VK_ESCAPE);
	return 0;
}

