// lua_gui_proxy.cpp : ���� DLL �ĳ�ʼ�����̡�
//

#include "stdafx.h"
#include "lua_gui_proxy.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//
//TODO:  ����� DLL ����� MFC DLL �Ƕ�̬���ӵģ�
//		��Ӵ� DLL �������κε���
//		MFC �ĺ������뽫 AFX_MANAGE_STATE ����ӵ�
//		�ú�������ǰ�档
//
//		����: 
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// �˴�Ϊ��ͨ������
//		}
//
//		�˺������κ� MFC ����
//		������ÿ��������ʮ����Ҫ��  ����ζ��
//		��������Ϊ�����еĵ�һ�����
//		���֣������������ж������������
//		������Ϊ���ǵĹ��캯���������� MFC
//		DLL ���á�
//
//		�й�������ϸ��Ϣ��
//		����� MFC ����˵�� 33 �� 58��
//

// Clua_gui_proxyApp

BEGIN_MESSAGE_MAP(Clua_gui_proxyApp, CWinApp)
END_MESSAGE_MAP()


// Clua_gui_proxyApp ����

Clua_gui_proxyApp::Clua_gui_proxyApp()
{
	// TODO:  �ڴ˴���ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
}


// Ψһ��һ�� Clua_gui_proxyApp ����

Clua_gui_proxyApp theApp;


// Clua_gui_proxyApp ��ʼ��

BOOL Clua_gui_proxyApp::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
}
