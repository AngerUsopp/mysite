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

void stackDump(lua_State* L)
{
    std::cout << "begin dump lua stack" << std::endl;
    int i = 0;
    int top = lua_gettop(L);
    for (i = 1; i <= top; ++i)
    {
        int t = lua_type(L, i);
        switch (t)
        {
        case LUA_TSTRING: {
            printf("'%s' \n", lua_tostring(L, i));
        }
                          break;
        case LUA_TBOOLEAN: {
            printf(lua_toboolean(L, i) ? "true \n" : "false \n");
        }
                           break;
        case LUA_TNUMBER: {
            printf("%g \n", lua_tonumber(L, i));
        }
                          break;
        default: {
            printf("%s \n", lua_typename(L, t));
        }
                 break;
        }
    }
    std::cout << "end dump lua stack" << std::endl;
}

int lua_call_cpp_fn(lua_State *lua)
{
    printf("c++ lua_call_cpp_fn param order\n");
    stackDump(lua);

    lua_pushnumber(lua, 22);
    lua_pushnumber(lua, 33);
    return 2;
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

        // ���ز�ִ�нű�
        int code = luaL_dofile(lua, "lua_script/test.lua");
        if (LUA_OK == code)
        {
            std::cout << "--------------cpp begin----------------" << std::endl;

            // ��ȫ��Ԫ����һ
            //lua_getfield(lua, LUA_REGISTRYINDEX, "CLuaProxy");
            // ��ȫ��Ԫ������
            lua_pushstring(lua, "CLuaProxy");            
            lua_gettable(lua, LUA_REGISTRYINDEX);
            if (lua_istable(lua, -1))
            {
                lua_getfield(lua, -1, "NotMenberFn");
                if (lua_isfunction(lua, -1))
                {
                    lua_pcall(lua, 0, 0, 0); // ��Ч
                }
            }

            //��ȡ����-------------------------------------------------------------
            lua_getglobal(lua, "g_member");   //string to be indexed
            std::cout << "g_member = " << lua_tostring(lua, -1) << std::endl;

            //��ȡ��key-value---------------------------------------------------
            lua_getglobal(lua, "g_table");  //table to be indexed
            if (lua_istable(lua, -1))
            {
                //ȡ����Ԫ��
                lua_getfield(lua, -1, "name");
                std::cout << "table->name = " << lua_tostring(lua, -1) << std::endl;
                lua_pop(lua, 1);

                lua_getfield(lua, -1, "mail");
                std::cout << "table->mail = " << lua_tostring(lua, -1) << std::endl;
                lua_pop(lua, 1);
            }

            //�޸ı���Ԫ��
            lua_pushstring(lua, "bean");
            lua_setfield(lua, -2, "name");
            lua_getfield(lua, -1, "name");
            std::cout << "new table->name = " << lua_tostring(lua, -1) << std::endl;
            lua_pop(lua, 1);

            // ����key-value��
            lua_newtable(lua);
            lua_pushinteger(lua, 22);
            lua_setfield(lua, -2, "p1");

            lua_pushnumber(lua, 33.0f);
            lua_setfield(lua, -2, "p2");

            lua_pushstring(lua, "c++ string");
            lua_setfield(lua, -2, "p3");

            lua_setglobal(lua, "cpp_map");

            // ��ȡ����---------------------------------------------------------------
            lua_getglobal(lua, "g_vector");  //table to be indexed
            if (lua_istable(lua, -1))
            {
                lua_Integer num = luaL_len(lua, -1);
                for (lua_Integer i = 1; i <= num; ++i)
                {
                    int type = lua_rawgeti(lua, -1, i);
                    switch (type)
                    {
                    case LUA_TNUMBER:
                        if (lua_isinteger(lua, -1))
                        {
                            printf("vector[%lld] lua_isinteger %lld\n", i, lua_tointeger(lua, -1));
                        }
                        else if (lua_isnumber(lua, -1))
                        {
                            printf("vector[%lld] lua_isnumber %g\n", i, lua_tonumber(lua, -1));
                        }
                        break;
                    case LUA_TSTRING:
                        printf("vector[%lld] lua_isstring %s\n", i, lua_tostring(lua, -1));
                        break;
                    /*case LUA_TTABLE:
                        break;*/
                    default:
                        printf("vector[%lld] type is %s \n", i, lua_typename(lua, type));
                        break;
                    }
                    lua_pop(lua, 1);
                }
            }

            // ��������
            lua_newtable(lua);
            lua_pushinteger(lua, 22);
            lua_rawseti(lua, -2, 1);

            lua_pushnumber(lua, 33.0f);
            lua_rawseti(lua, -2, 2);

            lua_pushstring(lua, "c++ string");
            lua_rawseti(lua, -2, 3);

            lua_setglobal(lua, "cpp_vector");

            // ȡ����-----------------------------------------------------------
            std::cout << "c++ call lua check_cpp_global" << std::endl;
            lua_getglobal(lua, "check_cpp_global");
            lua_pcall(lua, 0, 0, 0);

            std::cout << "c++ call lua add" << std::endl;
            lua_getglobal(lua, "add");
            lua_pushnumber(lua, 22);
            lua_pushnumber(lua, 33);
            lua_pcall(lua, 2, 1, 0);//2-����������1-����ֵ���������ú���������ִ���꣬�Ὣ����ֵѹ��ջ��
            std::cout << "add fn result = " << lua_tonumber(lua, -1) << std::endl;

            //// ������ע��ĺ���ͨ��getglobal���ò�����
            //if(LUA_OK == lua_getglobal(lua, "lua_call_cpp_fn"))
            //{
            //    std::cout << "c++ call lua_call_cpp_fn" << std::endl;
            //    lua_pushnumber(lua, 11);
            //    lua_pushnumber(lua, 22);
            //    lua_pcall(lua, 0, 1, 0);//2-������ʽ��1-����ֵ���������ú���������ִ���꣬�Ὣ����ֵѹ��ջ��
            //}

            std::cout << "--------------cpp end----------------" << std::endl;

            ////�鿴ջ
            //stackDump(lua);

            ////�鿴ջ
            //stackDump(lua);

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

