#pragma once
#include "Resource.h"
#include <string>
#include "lua_object.h"


// CBaseDialog 对话框

class CBaseDialog : public CDialogEx
{
    struct ctrl_info
    {
        ctrl_info()
        {
            id = -1;
            x = y = width = height = 0;
        }

        std::string type;
        UINT id;
        std::string text;
        int x, y, width, height;
    };

	DECLARE_DYNAMIC(CBaseDialog)

public:
    CBaseDialog(RefLuaState *lua, CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CBaseDialog();

// 对话框数据
	enum { IDD = IDD_DIALOG1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

private:
    void Layout();
    void RemoveAllChild();
    CWnd* CreateCrtl(const ctrl_info &info);

private:
    RefLuaState m_lua;
    CArray<CWnd*> m_ctrls;

public:
    virtual BOOL DestroyWindow();
    virtual BOOL OnInitDialog();
};
