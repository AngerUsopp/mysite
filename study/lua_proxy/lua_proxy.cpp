// lua_proxy.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"

#include "lua.hpp"


extern "C"
{
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
