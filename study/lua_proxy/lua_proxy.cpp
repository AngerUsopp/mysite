// lua_proxy.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"

#include "lua.hpp"


class CLuaProxy
{
public:
    CLuaProxy()
        : m_count(100)
    {
        printf("CLuaProxy construct \n");
    }

    ~CLuaProxy()
    {
        printf("CLuaProxy distruct \n");
    }

    void SayHello()
    {
        printf("CLuaProxy SayHello \n");
    }

private:
    int m_count;
};


void stackDump(lua_State* L)
{
    printf("\nbegin dump lua stack \n");
    int i = 0;
    int top = lua_gettop(L);
    printf("total elem %d \n", top);
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
            printf(lua_toboolean(L, i) ? "true " : "false ");
        }
                           break;
        case LUA_TNUMBER: {
            printf("%g \n", lua_tonumber(L, i));
        }
                          break;
        /*case LUA_TTABLE:
        {
            while (lua_next(L, i) != 0)
            {

            }
        }
            break;*/
        default: {
            printf("%s \n", lua_typename(L, t));
        }
                 break;
        }
    }
    printf("end dump lua stack \n");
}

extern "C"
{
    int CLuaProxy_SayHello(lua_State *lua)
    {
        //得到第一个传入的对象参数（在stack最底部）
        CLuaProxy** s = (CLuaProxy**)luaL_checkudata(lua, 1, "CLuaProxy");
        luaL_argcheck(lua, s != NULL, 1, "invalid data");
        
        (*s)->SayHello();

        //清空stack
        //lua_settop(lua, 0);

        //将数据放入stack中，供Lua使用
        lua_pushstring(lua, "proxy say sth");

        return 1;
    }

    int CLuaProxy_gc(lua_State *lua)
    {
        //得到第一个传入的对象参数（在stack最底部）
        CLuaProxy** s = (CLuaProxy**)luaL_checkudata(lua, 1, "CLuaProxy");
        if (s)
        {
            delete *s;
        }
        return 0;
    }

    luaL_Reg kProxyMemberFuncs[] =
    {
        { "SayHello", CLuaProxy_SayHello },
        { "__gc", CLuaProxy_gc },
        { nullptr, nullptr }
    };

    int CreateCLuaProxy(lua_State *lua)
    {
        //创建一个对象指针放到stack里，返回给Lua中使用，userdata的位置-1
        CLuaProxy **proxy = (CLuaProxy**)lua_newuserdata(lua, sizeof(CLuaProxy*));
        *proxy = new CLuaProxy();

        //Lua->stack，得到全局元表位置-1,userdata(proxy)位置-2
        luaL_getmetatable(lua, "CLuaProxy");
        //将元表赋值给位置-2的userdata(proxy)，并弹出-1的元表
        lua_setmetatable(lua, -2);

        return 1;
    }

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
        const char *str = lua_tostring(lua, -1);
        printf("c++ print_str %s\n", str);

        lua_pushstring(lua, "c++ pushstring");
        return 1;
    }

    luaL_Reg cFuntions[] =
    {
        { "CreateCLuaProxy", CreateCLuaProxy },
        { "average", average },
        { "print_str", print_str },
        { nullptr, nullptr }
    };

    __declspec(dllexport) int luaopen_lua_proxy(lua_State *lua)
    {
        //创建全局元表（里面包含了对LUA_REGISTRYINDEX的操作），元表的位置为-1
        luaL_newmetatable(lua, "CLuaProxy");
        
        //将元表作为一个副本压栈到位置-1，原元表位置-2
        lua_pushvalue(lua, -1);
        
        //设置-2位置元表的__index索引的值为位置-1的元表，并弹出位置-1的元表，原元表的位置为-1
        lua_setfield(lua, -2, "__index");
        
        //将成员函数注册到元表中（到这里，全局元表的设置就全部完成了）
        luaL_setfuncs(lua, kProxyMemberFuncs, 0);

        luaL_newlib(lua, cFuntions);
        return 1;
    }
}
