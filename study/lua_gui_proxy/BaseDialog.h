#pragma once
#include "Resource.h"


// CBaseDialog �Ի���

class CBaseDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CBaseDialog)

public:
	CBaseDialog(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CBaseDialog();

// �Ի�������
	enum { IDD = IDD_DIALOG1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
};
