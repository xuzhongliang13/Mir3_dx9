/******************************************************************************************************************
                                                                                                                   
	����:																											
																													
	�ۼ���:																											
	�ۼ���:																											
																													
	[����][������] : ���� ����																						
                                                                                                                   
*******************************************************************************************************************/



#include "stdafx.h"
#include "WHEngine.h"

static CWHApp* g_pWHApp = NULL;

/******************************************************************************************************************

	CWHApp Functions.

*******************************************************************************************************************/

/******************************************************************************************************************

	�Լ��� : CWHApp::CWHApp()

	�ۼ��� : 
	�ۼ��� : 

	����   : 
	���   : 

	[����][������] : ��������

*******************************************************************************************************************/
CWHApp::CWHApp()
{
	g_pWHApp	= this;
	m_hInstance = NULL;
}

/******************************************************************************************************************

	�Լ��� : CWHApp::~CWHApp()

	�ۼ��� : 
	�ۼ��� : 

	����   : 
	���   : 

	[����][������] : ��������

*******************************************************************************************************************/
CWHApp::~CWHApp()
{
}

/******************************************************************************************************************

	�Լ��� : CWHApp::MainWndProc()

	�ۼ��� : 
	�ۼ��� : 

	����   : 
	�Է�   : HWND hWnd
	         UINT uMsg
	         WPARAM wParam
	         LPARAM lParam
	���   : LRESULT 

	[����][������] : ��������

*******************************************************************************************************************/
LRESULT CWHApp::MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

/******************************************************************************************************************

	�Լ��� : WndProc()

	�ۼ��� : 
	�ۼ��� : 

	����   : 
	�Է�   : HWND hWnd
	         UINT uMsg
	         WPARAM wParam
	         LPARAM lParam
	���   : LRESULT CALLBACK 

	[����][������] : ��������

*******************************************************************************************************************/
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return g_pWHApp->MainWndProc(hWnd, uMsg, wParam, lParam);
}










/******************************************************************************************************************

	CWHWindow Functions.

*******************************************************************************************************************/

/******************************************************************************************************************

	�Լ��� : CWHWindow::CWHWindow()

	�ۼ��� : 
	�ۼ��� : 

	����   : 
	���   : 

	[����][������] : ��������

*******************************************************************************************************************/
CWHWindow::CWHWindow()
{
	m_hWnd = NULL;
}

/******************************************************************************************************************

	�Լ��� : CWHWindow::~CWHWindow()

	�ۼ��� : 
	�ۼ��� : 

	����   : 
	���   : 

	[����][������] : ��������

*******************************************************************************************************************/
CWHWindow::~CWHWindow()
{
}

/******************************************************************************************************************

	�Լ��� : CWHWindow::RegisterClass()

	�ۼ��� : 
	�ۼ��� : 

	����   : 
	�Է�   : CHAR *pszMenuName
	���   : BOOL 

	[����][������] : ��������

*******************************************************************************************************************/
BOOL CWHWindow::RegisterClass(CHAR* pszMenuName, CHAR* pszIconName)
{
    WNDCLASSEX wc;

    wc.cbSize			= sizeof(wc);
    wc.lpszClassName	= _WINDHORN_CLASS;
    wc.lpfnWndProc		= WndProc;
	wc.style			= CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS;
    wc.hInstance		= m_hInstance;
    wc.hIcon			= LoadIcon(m_hInstance, pszIconName);
    wc.hIconSm			= LoadIcon(m_hInstance, pszIconName);
    wc.hCursor			= LoadCursor(m_hInstance, IDC_ARROW);
    wc.hbrBackground	= (HBRUSH)(GRAY_BRUSH);//(COLOR_WINDOW + 1);
    wc.lpszMenuName		= pszMenuName;
    wc.cbClsExtra		= 0;
    wc.cbWndExtra		= 0;
	
    if ( RegisterClassEx(&wc) == 0 )
	{
        return FALSE;
	}

	return TRUE;
}

/******************************************************************************************************************

	�Լ��� : CWHWindow::Create()

	�ۼ��� : 
	�ۼ��� : 

	����   : 
	�Է�   : LPTSTR lpCaption
	         CHAR *pszMenuName
	���   : BOOL 

	[����][������] : ��������

*******************************************************************************************************************/
BOOL CWHWindow::Create(HINSTANCE hInst, LPTSTR lpCaption, CHAR *pszMenuName, CHAR* pszIconName, BOOL bLarge)
{
	SethInstance(hInst);
	if ( !RegisterClass(pszMenuName, pszIconName) ) 
	{
		MessageBox(NULL, TEXT("[CWHWindow::Create]") TEXT("Register class failed."), "MirDXG", MB_ICONERROR | MB_OK);
		return FALSE;
	}

    DWORD	dwFrameWidth    = GetSystemMetrics(SM_CXSIZEFRAME);
    DWORD	dwFrameHeight   = GetSystemMetrics(SM_CYSIZEFRAME);
	DWORD	dwMenuHeight;
	
	if ( pszMenuName != NULL )		
		dwMenuHeight = GetSystemMetrics(SM_CYMENU);
	else
		dwMenuHeight = 0;

    DWORD dwCaptionHeight = GetSystemMetrics(SM_CYCAPTION);
	DWORD dwWindowWidth, dwWindowHeight;
	if ( bLarge == FALSE )
	{
		dwWindowWidth   = _SCREEN_WIDTH  + dwFrameWidth * 2;
		dwWindowHeight  = _SCREEN_HEIGHT + dwFrameHeight * 2 + dwMenuHeight + dwCaptionHeight;
	}
	else
	{
		dwWindowWidth   = _SCREEN_WIDTH_LARGE + dwFrameWidth * 2;
		dwWindowHeight  = _SCREEN_HEIGHT_LARGE + dwFrameHeight * 2 + dwMenuHeight + dwCaptionHeight;
	}

    DWORD dwStyle = 0;//WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_OVERLAPPED;

    m_hWnd = CreateWindowEx(NULL, _WINDHORN_CLASS, lpCaption,
						    dwStyle, CW_USEDEFAULT, CW_USEDEFAULT,
  						    dwWindowWidth, dwWindowHeight, NULL, NULL, m_hInstance, NULL);

	if ( m_hWnd == NULL )
    	return FALSE;

	ShowWindow(m_hWnd, SW_SHOW);
	UpdateWindow(m_hWnd);

	return TRUE;
}

/******************************************************************************************************************

	�Լ��� : CWHWindow::MainWndProc()

	�ۼ��� : 
	�ۼ��� : 

	����   : 
	�Է�   : HWND hWnd
	         UINT uMsg
	         WPARAM wParam
	         LPARAM lParam
	���   : LRESULT 

	[����][������] : ��������

*******************************************************************************************************************/
LRESULT CWHWindow::MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch (uMsg)
	{
		case WM_DESTROY:
			return OnDestroy();
	}

    return CWHApp::MainWndProc(hWnd, uMsg, wParam, lParam);
}

/******************************************************************************************************************

	�Լ��� : CWHWindow::OnDestroy()

	�ۼ��� : 
	�ۼ��� : 

	����   : 
	���   : BOOL 

	[����][������] : ��������

*******************************************************************************************************************/
LRESULT CWHWindow::OnDestroy()
{
	PostQuitMessage(NULL);
	return 0L;
}
