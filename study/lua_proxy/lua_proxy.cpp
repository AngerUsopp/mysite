// lua_proxy.cpp : ���� DLL Ӧ�ó���ĵ���������
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
        //�õ���һ������Ķ����������stack��ײ���
        CLuaProxy** s = (CLuaProxy**)luaL_checkudata(lua, 1, "CLuaProxy");
        luaL_argcheck(lua, s != NULL, 1, "invalid data");
        
        (*s)->SayHello();

        //���stack
        //lua_settop(lua, 0);

        //�����ݷ���stack�У���Luaʹ��
        lua_pushstring(lua, "proxy say sth");

        return 1;
    }

    int CLuaProxy_gc(lua_State *lua)
    {
        //�õ���һ������Ķ����������stack��ײ���
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
        //����һ������ָ��ŵ�stack����ظ�Lua��ʹ�ã�userdata��λ��-1
        CLuaProxy **proxy = (CLuaProxy**)lua_newuserdata(lua, sizeof(CLuaProxy*));
        *proxy = new CLuaProxy();

        //Lua->stack���õ�ȫ��Ԫ��λ��-1,userdata(proxy)λ��-2
        luaL_getmetatable(lua, "CLuaProxy");
        //��Ԫ��ֵ��λ��-2��userdata(proxy)��������-1��Ԫ��
        lua_setmetatable(lua, -2);

        return 1;
    }

    int average(lua_State *lua)
    {
        double sum = 0;
        int num = lua_gettop(lua);//��ȡ�����ĸ���
        int actual_num = 0;
        for (int i = 1; i <= num; i++)
        {
            if (lua_isnumber(lua, i))
            {
                actual_num++;
                sum += lua_tonumber(lua, i);
            }
        }
        //���λ�ȡ���в���ֵ�����
        lua_pushnumber(lua, sum / actual_num);//��ƽ����ѹ��ջ����lua��ȡ

        printf("c++ average funtion\n");

        return 1;//���ط���ֵ������֪ͨluaӦ����ջ��ȡ����ֵ��Ϊ���ؽ��
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
        //����ȫ��Ԫ����������˶�LUA_REGISTRYINDEX�Ĳ�������Ԫ���λ��Ϊ-1
        luaL_newmetatable(lua, "CLuaProxy");
        
        //��Ԫ����Ϊһ������ѹջ��λ��-1��ԭԪ��λ��-2
        lua_pushvalue(lua, -1);
        
        //����-2λ��Ԫ���__index������ֵΪλ��-1��Ԫ��������λ��-1��Ԫ��ԭԪ���λ��Ϊ-1
        lua_setfield(lua, -2, "__index");
        
        //����Ա����ע�ᵽԪ���У������ȫ��Ԫ������þ�ȫ������ˣ�
        luaL_setfuncs(lua, kProxyMemberFuncs, 0);

        luaL_newlib(lua, cFuntions);
        return 1;
    }
}
