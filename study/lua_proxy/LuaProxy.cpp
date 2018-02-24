#include "stdafx.h"
#include "LuaProxy.h"
#include <stdio.h>


int CLuaProxy::g_inc = 0;

CLuaProxy::CLuaProxy() 
    : m_id(g_inc++)
    , m_nRef(1)
{
    printf("CLuaProxy construct %d \n", m_id);
}

CLuaProxy::~CLuaProxy()
{
    printf("CLuaProxy destruct %d \n", m_id);
}

void CLuaProxy::SayHello()
{
    printf("CLuaProxy SayHello %d \n", m_id);
}

void CLuaProxy::AddRef()
{
    ++m_nRef;
}

void CLuaProxy::Release()
{
    --m_nRef;

    if (m_nRef <= 0)
    {
        delete this;
    }
}
