// BaseDialog.cpp : 实现文件
//

#include "stdafx.h"
#include "lua_gui_proxy.h"
#include "BaseDialog.h"
#include "afxdialogex.h"


// CBaseDialog 对话框

IMPLEMENT_DYNAMIC(CBaseDialog, CDialogEx)

CBaseDialog::CBaseDialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(CBaseDialog::IDD, pParent)
{

}

CBaseDialog::~CBaseDialog()
{
}

void CBaseDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CBaseDialog, CDialogEx)
END_MESSAGE_MAP()


// CBaseDialog 消息处理程序
