// lua_demo.cpp : �������̨Ӧ�ó������ڵ㡣
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
        // ����Lua������
        luaL_openlibs(lua);

        // lua���е��õ�����������Ҫ��ע�ắ��
        lua_register(lua, "lua_call_cpp_fn", lua_call_cpp_fn);

        luaL_newlib; luaL_requiref; luaL_newmetatable; luaL_setfuncs;

        // ���ؽű�
        //if (LUA_OK == luaL_loadfile(lua, "lua_script/test.lua"))
        // ִ�нű�
        //lua_pcall(lua, 0, 0, 0);

        int code = luaL_dofile(lua, "lua_script/test.lua"); 
        if (LUA_OK == code)// ���ز�ִ�нű�
        {
            std::cout << "--------------cpp begin----------------" << std::endl;

            //��ȡ����
            lua_getglobal(lua, "name");   //string to be indexed
            std::cout << "name = " << lua_tostring(lua, -1) << std::endl;

            //��ȡ����
            lua_getglobal(lua, "date"); //number to be indexed
            std::cout << "date = " << lua_tonumber(lua, -1) << std::endl;

            //��ȡ��
            lua_getglobal(lua, "me");  //table to be indexed
            if (!lua_istable(lua, -1))
            {
                std::cout << "error:it is not a table" << std::endl;
            }
            //ȡ����Ԫ��
            lua_getfield(lua, -1, "name");
            std::cout << "user name = " << lua_tostring(lua, -1) << std::endl;
            lua_getfield(lua, -2, "mail");
            std::cout << "user mail = " << lua_tostring(lua, -1) << std::endl;

            //�鿴ջ
            stackDump(lua);

            //�޸ı���Ԫ��
            lua_pushstring(lua, "wh");
            lua_setfield(lua, -4, "name");
            lua_getfield(lua, -3, "name");
            std::cout << "user new name = " << lua_tostring(lua, -1) << std::endl;

            //�鿴ջ
            stackDump(lua);

            //ȡ����
            lua_getglobal(lua, "add");
            lua_pushnumber(lua, 15);
            lua_pushnumber(lua, 5);
            lua_pcall(lua, 2, 1, 0);//2-������ʽ��1-����ֵ���������ú���������ִ���꣬�Ὣ����ֵѹ��ջ��
            std::cout << "add fn result = " << lua_tonumber(lua, -1) << std::endl;

            //�鿴ջ
            stackDump(lua);

            std::cout << "--------------cpp end----------------" << std::endl;

            ret = true;
        }
        else
        {
            std::cout << lua_tostring(lua, -1) << std::endl;
        }

        //�ر�state
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

