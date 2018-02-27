
// lua_gui_demoDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "lua_gui_demo.h"
#include "lua_gui_demoDlg.h"
#include "DlgProxy.h"
#include "afxdialogex.h"
#include <io.h>
#include <fcntl.h>
#include <string>
#include <map>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


namespace
{
    void InitConsoleWindow()
    {
        int nCrt = 0;
        FILE* fp;
        AllocConsole();
        nCrt = _open_osfhandle((long)GetStdHandle(STD_OUTPUT_HANDLE), _O_TEXT);
        fp = _fdopen(nCrt, "w"); //ON_COMMAND; WM_COMMAND
        *stdout = *fp;
        setvbuf(stdout, NULL, _IONBF, 0);
    }

    enum
    {
        DYNAMIC_CTRL_ID_BEGIN = 1000,

        MENU_ID_BEGIN,
        MENU_ID_END = MENU_ID_BEGIN + 100,

        BTN_ID_BEGIN,
        BTN_ID_END = BTN_ID_BEGIN + 100,

        DYNAMIC_CTRL_ID_END
    };

    int g_menu_id = MENU_ID_BEGIN;
    std::map<int, std::string> g_menu_map;

    int g_btn_id = BTN_ID_BEGIN;
    std::map<int, std::string> g_btn_map;

    const char* kGuiLua = "lua_script/gui.lua";
}

// Clua_gui_demoDlg �Ի���


IMPLEMENT_DYNAMIC(Clua_gui_demoDlg, CDialogEx);

Clua_gui_demoDlg::Clua_gui_demoDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(Clua_gui_demoDlg::IDD, pParent)
    , m_bTracking(FALSE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pAutoProxy = NULL;
}

Clua_gui_demoDlg::~Clua_gui_demoDlg()
{
	// ����öԻ������Զ���������
	//  ���˴���ָ��öԻ���ĺ���ָ������Ϊ NULL���Ա�
	//  �˴���֪���öԻ����ѱ�ɾ����
	if (m_pAutoProxy != NULL)
		m_pAutoProxy->m_pDialog = NULL;

    if (m_lua)
    {
        lua_close(m_lua);
        m_lua = nullptr;
    }
}

void Clua_gui_demoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(Clua_gui_demoDlg, CDialogEx)
    ON_WM_CLOSE()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_WM_MOUSEMOVE()
    ON_WM_MOUSELEAVE()
    ON_WM_LBUTTONDOWN()
    ON_COMMAND_RANGE(DYNAMIC_CTRL_ID_BEGIN, DYNAMIC_CTRL_ID_END, OnCommand)
END_MESSAGE_MAP()


// Clua_gui_demoDlg ��Ϣ�������

BOOL Clua_gui_demoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO:  �ڴ���Ӷ���ĳ�ʼ������

    //InitConsoleWindow();

    // ��һ�ַ�ʽ����UI�Ĳ������ú���Ϣ������luaȥ����
    // ����������Ĵ��������ƣ����ַ�ʽӦ��ֻ�����ڽ��߼��벼�ַ������
    // �������ڳ���������
    m_lua = luaL_newstate();
    if (m_lua)
    {
        // ����Lua������
        luaL_openlibs(m_lua);
        RestoreGuiLua();
    }
    //ListBox_AddItemData;

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void Clua_gui_demoDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR Clua_gui_demoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// ���û��ر� UI ʱ������������Ա���������ĳ��
//  �������Զ�����������Ӧ�˳���  ��Щ
//  ��Ϣ�������ȷ����������: �����������ʹ�ã�
//  ������ UI�������ڹرնԻ���ʱ��
//  �Ի�����Ȼ�ᱣ�������

void Clua_gui_demoDlg::OnClose()
{
	if (CanExit())
		CDialogEx::OnClose();
}

void Clua_gui_demoDlg::OnOK()
{
	/*if (CanExit())
		CDialogEx::OnOK();*/
    RestoreGuiLua();
}

void Clua_gui_demoDlg::OnCancel()
{
	if (CanExit())
		CDialogEx::OnCancel();
}

BOOL Clua_gui_demoDlg::CanExit()
{
	// �����������Ա�����������Զ���
	//  �������Իᱣ�ִ�Ӧ�ó���
	//  ʹ�Ի���������������� UI ����������
	if (m_pAutoProxy != NULL)
	{
		ShowWindow(SW_HIDE);
		return FALSE;
	}

	return TRUE;
}



void Clua_gui_demoDlg::RestoreGuiLua()
{
    if (m_lua)
    {
        // ����Lua������
        luaL_openlibs(m_lua);

        if (LUA_OK == luaL_dofile(m_lua, kGuiLua))
        {
            RestoreGuiLuaMenu();
            RestoreGuiLuaButton();
        }
    }
}

void Clua_gui_demoDlg::RestoreGuiLuaMenu()
{
    g_menu_id = 1001;
    g_menu_map.clear();

    HMENU hMenu = CreateMenu();

    if (m_lua)
    {
        if (LUA_OK == luaL_dofile(m_lua, kGuiLua))
        {
            if (LUA_TTABLE == lua_getglobal(m_lua, "menu"))
            {
                if (lua_istable(m_lua, -1))
                {
                    // ������������
                    lua_Integer num = luaL_len(m_lua, -1);
                    for (lua_Integer i = 1; i <= num; ++i)
                    {
                        int type = lua_rawgeti(m_lua, -1, i);
                        if (type == LUA_TTABLE)
                        {
                            int ret = lua_getfield(m_lua, -1, "name");
                            std::string name = lua_tostring(m_lua, -1);
                            lua_pop(m_lua, 1);

                            ret = lua_getfield(m_lua, -1, "items");
                            if (lua_istable(m_lua, -1))
                            {
                                HMENU hSubMenu = CreateMenu();

                                lua_Integer items = luaL_len(m_lua, -1);
                                for (lua_Integer item = 1; item <= items; ++item)
                                {
                                    type = lua_rawgeti(m_lua, -1, item);
                                    if (type == LUA_TTABLE)
                                    {
                                        ret = lua_getfield(m_lua, -1, "id");
                                        if (ret == LUA_TSTRING)
                                        {
                                            std::string id = lua_tostring(m_lua, -1);
                                            lua_pop(m_lua, 1);

                                            ret = lua_getfield(m_lua, -1, "text");
                                            std::string text = lua_tostring(m_lua, -1);
                                            lua_pop(m_lua, 1);

                                            int mid = g_menu_id++;
                                            g_menu_map[mid] = id;

                                            ::AppendMenu(hSubMenu, MF_STRING, mid, CString(text.c_str()));
                                        }
                                        else if (ret == LUA_TNIL)
                                        {
                                            lua_pop(m_lua, 1);
                                            ::AppendMenu(hSubMenu, MF_SEPARATOR, 0, nullptr);
                                        }
                                    }
                                    lua_pop(m_lua, 1);
                                }

                                ::AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hSubMenu, CString(name.c_str()));
                            }
                            lua_pop(m_lua, 1);
                        }
                        lua_pop(m_lua, 1);
                    }
                }
            }
            lua_pop(m_lua, 1);
        }
    }

    if (hMenu)
    {
        ::SetMenu(m_hWnd, hMenu);
    }
}

void Clua_gui_demoDlg::RestoreGuiLuaButton()
{
    for (UINT i = 0; i < m_buttons.GetSize(); i++)
    {
        delete m_buttons[i];
    }
    m_buttons.RemoveAll();

    if (m_lua)
    {
        if (LUA_TTABLE == lua_getglobal(m_lua, "buttons"))
        {
            if (lua_istable(m_lua, -1))
            {
                // ������������
                lua_Integer num = luaL_len(m_lua, -1);
                for (lua_Integer i = 1; i <= num; ++i)
                {
                    int type = lua_rawgeti(m_lua, -1, i);
                    if (type == LUA_TTABLE)
                    {
                        type = lua_getfield(m_lua, -1, "id");
                        if (type == LUA_TSTRING)
                        {
                            std::string id = lua_tostring(m_lua, -1);
                            lua_pop(m_lua, 1);

                            type = lua_getfield(m_lua, -1, "text");
                            std::string text = lua_tostring(m_lua, -1);
                            lua_pop(m_lua, 1);

                            type = lua_getfield(m_lua, -1, "x");
                            int x = lua_tointeger(m_lua, -1);
                            lua_pop(m_lua, 1);

                            type = lua_getfield(m_lua, -1, "y");
                            int y = lua_tointeger(m_lua, -1);
                            lua_pop(m_lua, 1);

                            type = lua_getfield(m_lua, -1, "width");
                            int width = lua_tointeger(m_lua, -1);
                            lua_pop(m_lua, 1);

                            type = lua_getfield(m_lua, -1, "height");
                            int height = lua_tointeger(m_lua, -1);
                            lua_pop(m_lua, 1);

                            int mid = g_btn_id++;
                            g_btn_map[mid] = id;

                            CButton *button = new CButton();
                            button->Create(CString(text.c_str()), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                CRect(x, y, x + width, y + height), this, mid);
                            m_buttons.Add(button);
                        }
                        else if (type == LUA_TNIL)
                        {
                            lua_pop(m_lua, 1);
                        }
                    }
                    lua_pop(m_lua, 1);
                }
            }
        }
        lua_pop(m_lua, 1);
    }
}

void Clua_gui_demoDlg::OnMouseMove(UINT nFlags, CPoint point)
{
    // TODO:  �ڴ������Ϣ�����������/�����Ĭ��ֵ
    if (!m_bTracking)
    {
        //   ������봰ʱ������WM_MOUSEHOVER��WM_MOUSELEAVE ��Ϣ  
        TRACKMOUSEEVENT tme;
        tme.cbSize = sizeof(tme);
        tme.hwndTrack = m_hWnd;
        tme.dwFlags = TME_LEAVE | TME_HOVER;
        tme.dwHoverTime = 1;
        m_bTracking = _TrackMouseEvent(&tme);
    }

    if (m_lua)
    {
        if (LUA_TFUNCTION == lua_getglobal(m_lua, "OnMouseMove"))
        {
            lua_pushnumber(m_lua, point.x);
            lua_pushnumber(m_lua, point.y);
            lua_pcall(m_lua, 2, 0, 0);

            /*std::string ret = lua_tostring(m_lua, -1);
            TRACE((ret + "\r\n").c_str());*/
        }
    }

    CDialogEx::OnMouseMove(nFlags, point);
}


void Clua_gui_demoDlg::OnMouseLeave()
{
    // TODO:  �ڴ������Ϣ�����������/�����Ĭ��ֵ
    m_bTracking = FALSE;

    if (m_lua)
    {
        if (LUA_TFUNCTION == lua_getglobal(m_lua, "OnMouseExit"))
        {
            lua_pcall(m_lua, 0, 1, 0);

            std::string ret = lua_tostring(m_lua, -1);
            TRACE((ret + "\r\n").c_str());
        }
    }
    CDialogEx::OnMouseLeave();
}


void Clua_gui_demoDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
    // TODO:  �ڴ������Ϣ�����������/�����Ĭ��ֵ
    if (m_lua)
    {
        if (LUA_TFUNCTION == lua_getglobal(m_lua, "OnMousePressed"))
        {
            lua_pushnumber(m_lua, point.x);
            lua_pushnumber(m_lua, point.y);
            lua_pcall(m_lua, 2, 1, 0);

            std::string ret = lua_tostring(m_lua, -1);
            TRACE((ret + "\r\n").c_str());
        }
    }
    CDialogEx::OnLButtonDown(nFlags, point);
}

void Clua_gui_demoDlg::OnCommand(UINT uId)
{
    if (MENU_ID_BEGIN <= uId && MENU_ID_END >= uId)
    {
        if (g_menu_map.find(uId) != g_menu_map.end())
        {
            if (m_lua)
            {
                if (LUA_TFUNCTION == lua_getglobal(m_lua, "OnMenuSelected"))
                {
                    lua_pushstring(m_lua, g_menu_map[uId].c_str());
                    lua_pcall(m_lua, 1, 1, 0);

                    std::string ret = lua_tostring(m_lua, -1);
                    TRACE((ret + "\r\n").c_str());
                }
            }
        }
    }

    if (BTN_ID_BEGIN <= uId && BTN_ID_END >= uId)
    {
        if (g_btn_map.find(uId) != g_btn_map.end())
        {
            if (m_lua)
            {
                if (LUA_TFUNCTION == lua_getglobal(m_lua, "OnButtonClick"))
                {
                    lua_pushstring(m_lua, g_btn_map[uId].c_str());
                    lua_pcall(m_lua, 1, 1, 0);

                    std::string ret = lua_tostring(m_lua, -1);
                    TRACE((ret + "\r\n").c_str());
                }
            }
        }
    }
}
