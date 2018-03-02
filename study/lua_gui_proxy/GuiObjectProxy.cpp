#include "stdafx.h"
#include "GuiObjectProxy.h"
#include "BaseDialog.h"
#include "lua_gui_proxy.h"
#include "lua_object.h"

namespace
{
    CBaseDialog *g_dialog = nullptr;
}

extern "C"
{
    //int CLuaProxy_NotMenberFn(lua_State *lua)
    //{
    //    printf("CLuaProxy_NotMenberFn \n");
    //    return 0;
    //}

    //int CLuaProxy_SayHello(lua_State *lua)
    //{
    //    //得到第一个传入的对象参数（在stack最底部）
    //    ProxyWrapper **wrapper = (ProxyWrapper**)luaL_checkudata(lua, 1, "CLuaProxy");
    //    luaL_argcheck(lua, wrapper != NULL, 1, "invalid data");

    //    (*wrapper)->get()->SayHello();

    //    //清空stack
    //    //lua_settop(lua, 0);

    //    //将数据放入stack中，供Lua使用
    //    //lua_pushstring(lua, "proxy say sth");

    //    return 0;
    //}

    //int CLuaProxy_gc(lua_State *lua)
    //{
    //    //得到第一个传入的对象参数（在stack最底部）
    //    ProxyWrapper **wrapper = (ProxyWrapper**)luaL_checkudata(lua, 1, "CLuaProxy");
    //    if (wrapper)
    //    {
    //        delete (*wrapper);
    //    }
    //    return 0;
    //}

    //int CLuaProxy_tostring(lua_State *lua)
    //{
    //    ProxyWrapper **wrapper = (ProxyWrapper**)luaL_checkudata(lua, 1, "CLuaProxy");
    //    luaL_argcheck(lua, wrapper != NULL, 1, "invalid data");

    //    lua_pushfstring(lua, "this is CLuaProxy info %d!", (*wrapper)->get()->ct());

    //    return 1;
    //}

    //luaL_Reg kProxyMemberFuncs[] =
    //{
    //    { "NotMenberFn", CLuaProxy_NotMenberFn },
    //    { "SayHello", CLuaProxy_SayHello },
    //    { "__gc", CLuaProxy_gc },
    //    { "__tostring", CLuaProxy_tostring },
    //    { nullptr, nullptr }
    //};

    int DoModal(lua_State *lua)
    {
        int ret = -1;
        if (lua_isuserdata(lua, -1))
        {
            RefLuaState *L = (RefLuaState*)lua_touserdata(lua, -1);
            // 要在其他模块调用另一dll中的mfc资源有两种方式，实质是要使能从对应的模块加载那个模块自己的资源而不是用主exe的资源
            // 一、进行模块状态切换
            AFX_MANAGE_STATE(AfxGetStaticModuleState());
            CBaseDialog dlg(L, AfxGetApp()->m_pMainWnd);
            ret = dlg.DoModal();

            // 方式二
            //HINSTANCE save_hInstance = AfxGetResourceHandle();
            //AfxSetResourceHandle(theApp.m_hInstance);
            //CBaseDialog dlg(AfxGetApp()->m_pMainWnd);
            //dlg.DoModal();
            ////方法2的状态还原
            //AfxSetResourceHandle(save_hInstance);
        }
        lua_pushnumber(lua, ret);
        return 1;
    }

    luaL_Reg cFuntions[] =
    {
        { "DoModal", DoModal },
        { nullptr, nullptr }
    };

    __declspec(dllexport) int luaopen_lua_gui_proxy(lua_State *lua)
    {
        ////创建全局元表（里面包含了对LUA_REGISTRYINDEX的操作），元表的位置为-1
        //luaL_newmetatable(lua, "CLuaProxy");
        ////将元表作为一个副本压栈到位置-1，原元表位置-2
        //lua_pushvalue(lua, -1);
        ////设置-2位置元表的__index索引的值为位置-1的元表，并弹出位置-1的元表，原元表的位置为-1
        //lua_setfield(lua, -2, "__index");
        ////将成员函数注册到元表中（到这里，全局元表的设置就全部完成了）
        //luaL_setfuncs(lua, kProxyMemberFuncs, 0);

        // 注册该库的函数
        luaL_newlib(lua, cFuntions);

        return 1;
    }
}
