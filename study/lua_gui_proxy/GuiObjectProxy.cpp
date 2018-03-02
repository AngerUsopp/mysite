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
    //    //�õ���һ������Ķ����������stack��ײ���
    //    ProxyWrapper **wrapper = (ProxyWrapper**)luaL_checkudata(lua, 1, "CLuaProxy");
    //    luaL_argcheck(lua, wrapper != NULL, 1, "invalid data");

    //    (*wrapper)->get()->SayHello();

    //    //���stack
    //    //lua_settop(lua, 0);

    //    //�����ݷ���stack�У���Luaʹ��
    //    //lua_pushstring(lua, "proxy say sth");

    //    return 0;
    //}

    //int CLuaProxy_gc(lua_State *lua)
    //{
    //    //�õ���һ������Ķ����������stack��ײ���
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
            // Ҫ������ģ�������һdll�е�mfc��Դ�����ַ�ʽ��ʵ����Ҫʹ�ܴӶ�Ӧ��ģ������Ǹ�ģ���Լ�����Դ����������exe����Դ
            // һ������ģ��״̬�л�
            AFX_MANAGE_STATE(AfxGetStaticModuleState());
            CBaseDialog dlg(L, AfxGetApp()->m_pMainWnd);
            ret = dlg.DoModal();

            // ��ʽ��
            //HINSTANCE save_hInstance = AfxGetResourceHandle();
            //AfxSetResourceHandle(theApp.m_hInstance);
            //CBaseDialog dlg(AfxGetApp()->m_pMainWnd);
            //dlg.DoModal();
            ////����2��״̬��ԭ
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
        ////����ȫ��Ԫ����������˶�LUA_REGISTRYINDEX�Ĳ�������Ԫ���λ��Ϊ-1
        //luaL_newmetatable(lua, "CLuaProxy");
        ////��Ԫ����Ϊһ������ѹջ��λ��-1��ԭԪ��λ��-2
        //lua_pushvalue(lua, -1);
        ////����-2λ��Ԫ���__index������ֵΪλ��-1��Ԫ��������λ��-1��Ԫ��ԭԪ���λ��Ϊ-1
        //lua_setfield(lua, -2, "__index");
        ////����Ա����ע�ᵽԪ���У������ȫ��Ԫ������þ�ȫ������ˣ�
        //luaL_setfuncs(lua, kProxyMemberFuncs, 0);

        // ע��ÿ�ĺ���
        luaL_newlib(lua, cFuntions);

        return 1;
    }
}
