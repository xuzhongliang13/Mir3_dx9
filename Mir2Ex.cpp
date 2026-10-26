#include "StdAfx.h"
#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")
#include "EffectLayer.h"

LONG				g_OriginWndProc;

CClientSocket		g_xClientSocket;
CWHDXGraphicWindow	g_xMainWnd;
SoundManager		g_SoundManager;
CSprite				g_xSpriteInfo;
CChatEdit			g_xChatEditBox;
StringSplitter		g_xStringSplitter;
CLoginProcess		g_xLoginProc;
CSelLoginProcess	g_xSelLoginProc;
CChrProcess			g_xChrSelProc;
CGameProcess		g_xGameProc;
CMsg				g_xMsg;

CMsg				g_xItemMsg;		// ������ ���� �߰�
/***** Exported global variable ********************************************/


INT					g_nCertifyCode;
char				g_szUserID[25];
char				g_szCharName[25];
char				g_szServerName[20];
BYTE				g_nUpgradeLevel;  //lee wun gyo
char				g_szServerIP[16];
INT					g_nServerPort;
INT					g_nVersion;
char				g_szPreServerIP[16];
INT					g_nPreServerPort;
DWORD				g_dwCurrTime;

BOOL				g_bTextrRistricted = FALSE;
BYTE				g_bProcState	   = _LOGIN_PROC;
BOOL				g_bAvi;

INT					g_nPaySystem = 0;

#define _WINDOW_NAME			"Legend Of Mir 3"


// [DX9][T48-2] 崩溃捕获：记录异常代�?精确地址/寄存�?�?
// [DX9][T60-CRASH] ����ջ����: ��λ������ʵ������(���ı��κ�ԭ���߼�)
static void Dx9DumpBacktrace(FILE* fp, struct _EXCEPTION_POINTERS* ep)
{
	HANDLE hProc   = GetCurrentProcess();
	HANDLE hThread = GetCurrentThread();

	if ( ep == NULL || ep->ContextRecord == NULL ) return;
	if ( IsBadReadPtr(ep->ContextRecord, sizeof(CONTEXT)) ) return;

	CONTEXT ctx;
	memcpy(&ctx, ep->ContextRecord, sizeof(CONTEXT));

	// ָ֡��/ջָ�����ɶ�, ���� StackWalk64 ���Լ��ȿ�
	if ( IsBadReadPtr((void*)ctx.Esp, 16) ) return;

	STACKFRAME64 sf;
	memset(&sf, 0, sizeof(sf));
	sf.AddrPC.Offset    = ctx.Eip; sf.AddrPC.Mode    = AddrModeFlat;
	sf.AddrFrame.Offset = ctx.Ebp; sf.AddrFrame.Mode = AddrModeFlat;
	sf.AddrStack.Offset = ctx.Esp; sf.AddrStack.Mode = AddrModeFlat;

	SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES);
	BOOL bSym = SymInitialize(hProc, NULL, TRUE);
	if ( !bSym )
	{
		SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);
		bSym = SymInitialize(hProc, NULL, FALSE);
	}

	BYTE abSym[sizeof(SYMBOL_INFO) + 512];
	SYMBOL_INFO* pSym = (SYMBOL_INFO*)abSym;
	pSym->SizeOfStruct = sizeof(SYMBOL_INFO);
	pSym->MaxNameLen   = 511;

	fprintf(fp, "BACKTRACE (%s):\r\n", bSym ? "symbols" : "no-symbols");
	for ( int i = 0; i < 32; i++ )
	{
		if ( !StackWalk64(IMAGE_FILE_MACHINE_I386, hProc, hThread, &sf, &ctx,
				NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL) )
			break;
		if ( sf.AddrPC.Offset == 0 )
			break;

		DWORD dwPC = (DWORD)sf.AddrPC.Offset;
		char  szModName[MAX_PATH] = "?";
		DWORD dwOff = 0;
		HMODULE hM = NULL;
		if ( GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
				(LPCSTR)dwPC, &hM) && hM )
		{
			szModName[MAX_PATH - 1] = '\0';
			GetModuleFileNameA(hM, szModName, MAX_PATH);
			szModName[MAX_PATH - 1] = '\0';
			char* pSlash = strrchr(szModName, '\\');
			if ( pSlash ) memmove(szModName, pSlash + 1, strlen(pSlash + 1) + 1);
			dwOff = dwPC - (DWORD)hM;
		}

		fprintf(fp, "  [%02d] %s+0x%05X  (pc=0x%08X)", i, szModName, dwOff, dwPC);

		if ( bSym )
		{
			DWORD64 dwDisp = 0;
			// �������ֻ���, ��֤��ʹ SymFromAddr ����ʧ��Ҳ�������������
			pSym->Name[0] = '\0';
			if ( SymFromAddr(hProc, (DWORD64)dwPC, &dwDisp, pSym) && pSym->Name[0] )
				fprintf(fp, "  %s+0x%X", pSym->Name, (unsigned)dwDisp);
			else
				fprintf(fp, "  (no symbol)");
		}
		fprintf(fp, "\r\n");
	}
	fprintf(fp, "END BACKTRACE\r\n");
}

LONG WINAPI Dx9CrashFilter(struct _EXCEPTION_POINTERS* ep)
{
#if MIR3_ENABLE_CRASHLOG
	// [DX9][LOGGATE] 崩溃捕获开关：定义 MIR3_ENABLE_CRASHLOG 才写 CrashLog.txt
	//   发布版默认关闭（不产生任何文件）；排障时�?-DMIR3_ENABLE_CRASHLOG 重新编译�?,
            
	FILE* fp = fopen("CrashLog.txt", "a");
	if ( fp ) {
		fprintf(fp, "=== CRASH %s ===\r\n", "start");
		fprintf(fp, "code=0x%08X addr=0x%08X\r\n",
			ep->ExceptionRecord->ExceptionCode,
			(DWORD)ep->ExceptionRecord->ExceptionAddress);
		if ( ep->ExceptionRecord->NumberParameters >= 1 )
			fprintf(fp, "faultAddr=0x%08X op=%s\r\n",
				(DWORD)ep->ExceptionRecord->ExceptionInformation[1],
				ep->ExceptionRecord->ExceptionInformation[0] ? "WRITE" : "READ");
		HMODULE hMod = NULL;
		GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
			(LPCSTR)ep->ExceptionRecord->ExceptionAddress, &hMod);
		if ( hMod )
		{
			char szMod[MAX_PATH] = "";
			GetModuleFileNameA(hMod, szMod, MAX_PATH);
			fprintf(fp, "module=0x%08X offset=0x%08X name=%s\r\n",
				(DWORD)hMod, (DWORD)ep->ExceptionRecord->ExceptionAddress - (DWORD)hMod, szMod);
		}
#ifdef _M_IX86
		fprintf(fp, "EAX=%08X EBX=%08X ECX=%08X EDX=%08X\r\n",
			ep->ContextRecord->Eax, ep->ContextRecord->Ebx,
			ep->ContextRecord->Ecx, ep->ContextRecord->Edx);
		fprintf(fp, "ESI=%08X EDI=%08X EBP=%08X ESP=%08X EIP=%08X\r\n",
			ep->ContextRecord->Esi, ep->ContextRecord->Edi,
			ep->ContextRecord->Ebp, ep->ContextRecord->Esp, ep->ContextRecord->Eip);
		fprintf(fp, "STACK:");
		DWORD* pStack = (DWORD*)ep->ContextRecord->Esp;
		for ( int i = 0; i < 16; i++ )
		{
			if ( IsBadReadPtr(pStack + i, 4) ) { fprintf(fp, " [??]"); break; }
			fprintf(fp, " %08X", pStack[i]);
		}
		fprintf(fp, "\r\n");
#endif

		// ע: �������� GCC ����, ��֧�� MSVC �� __try/__except;
		//     Dx9DumpBacktrace �ڲ��Ѷ�ÿһ������Ч������, ʧ��ʱ��ǰ���ء�
		Dx9DumpBacktrace(fp, ep);

		fclose(fp);
	}
	return EXCEPTION_EXECUTE_HANDLER;
#else
	// [FIX-R4] 关闭时不能落空: 原来的 return 在 #if 里面, 关掉后函数没有返回值,
	// 等于让未处理异常过滤器返回随机值。显式交回系统默认处理。
	(void)ep;
	return EXCEPTION_CONTINUE_SEARCH;
#endif // MIR3_ENABLE_CRASHLOG
}


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    MSG	msg;

	// [DX9][T48-2] 安装崩溃捕获
	SetUnhandledExceptionFilter(Dx9CrashFilter);

	// [2026-10-03 fix] 启动时把工作目录切换�?exe 所在目录，
	// 保证 CMList.Dat / Data / Map 等相对路径资源不受启动方�?
	// （资源管理器双击、快捷方式、第三方启动器等）工作目录差异的影响�?
	{
		char szExePath[MAX_PATH];
		if ( GetModuleFileName(NULL, szExePath, MAX_PATH) > 0 )
		{
			char *pSlash = strrchr(szExePath, '\\');
			if ( pSlash )
			{
				*pSlash = '\0';
				SetCurrentDirectory(szExePath);
			}
		}
	}

	g_xMsg.LoadMsg();
	g_xItemMsg.LoadMsg( "SALEITEMDETAIL.dat" );	// ������ ���� �߰�

	// Global Variables Init.
//	g_nVersion		 = 20050111;
//	g_nVersion		 = 20040922;	// [T88前测] 服务器正式模式nRecog=20040922, 但跳过校验后版本号需与正常客户端一�?
	g_nVersion		 = 20050527;	// [DX9][T88] 与正常客户端(Mir3_patched_v3_backup反汇编证实发20050527)一�? 版本校验已跳�?T88 skip-chk), 服务器正式模式可�?
	g_nCertifyCode	 = 0;
	g_nServerPort	 = 0;
	g_nPreServerPort = 0;
	strcpy(g_szUserID,	   "");
	strcpy(g_szCharName,   "");
	strcpy(g_szServerName, "");
	strcpy(g_szServerIP,   "");
	strcpy(g_szPreServerIP,"");
	g_execCmds.bNotPatchd = TRUE;	// Patch ON
	
	g_nUpgradeLevel = 0; //lee wun gyo

	// ORZ CMDLINE: ���� ����.
	if ( !ParseCmdLine() )
	{
		return -1;
	}

	g_bAvi = TRUE;

	g_xMainWnd.m_stDisplayInfo.nRefreshRate = g_execCmds.nRefreshRate;

	if ( FindWindow(NULL,_WINDOW_NAME) != NULL )
	{
		return -1;
	}

	HANDLE		hFile;
	INT			nPatchState;
	CHAR		szFullPath[MAX_PATH];
	CHAR		szFileName[MAX_PATH];

#ifdef _TEST_SERVER
#else
//	if ( g_execCmds.bNotPatchd == TRUE )
//	{
//		char szFileName[MAX_PATH];

//		GetCurrentDirectory(MAX_PATH, szFileName);

//		strcat(szFileName, ".\\Mir3FtpSelect.exe");
	
//		STARTUPINFO			si = {sizeof( STARTUPINFO ), 0,};
//		PROCESS_INFORMATION	pi = {0,};

//		if ( !CreateProcess( szFileName, lpCmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi ) )
//		{
//			MessageBox(NULL, "SelPatch Error", "Legend Of Mir 3", MB_OK|MB_ICONSTOP);
//		}

//		return -1;
//	}
#endif

	BOOL bCreate;
	BOOL bLarge;

	if ( g_execCmds.bNotSelPatchd == FALSE )
	{
		bLarge = TRUE;
	}
	else
	{
		bLarge = FALSE;
	}

#ifdef _DEBUG
	if ( g_execCmds.bFullScreen )
	{
		bCreate = g_xMainWnd.Create(hInstance, _WINDOW_NAME, NULL, MAKEINTRESOURCE(IDI_ICON), _DXG_SCREENMODE_FULLSCREEN, _DXG_DEVICEMODE_PRIMARY|_DXG_DEVICEMODE_D3D, bLarge);
	}
	else
	{
		bCreate = g_xMainWnd.Create(hInstance, _WINDOW_NAME, NULL, MAKEINTRESOURCE(IDI_ICON), _DXG_SCREENMODE_WINDOW, _DXG_DEVICEMODE_PRIMARY|_DXG_DEVICEMODE_D3D, bLarge);
	}
#else
	bCreate = g_xMainWnd.Create(hInstance, _WINDOW_NAME, NULL, MAKEINTRESOURCE(IDI_ICON), _DXG_SCREENMODE_WINDOW, _DXG_DEVICEMODE_PRIMARY|_DXG_DEVICEMODE_D3D, bLarge);
#endif

	if ( bCreate == FALSE )
	{
		return -1;
	}

//	SetFontList();	
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ����Ʈ�ڽ� ����.
	g_xChatEditBox.Create(g_xMainWnd.GetSafehInstance(), g_xMainWnd.GetSafehWnd(), 0, 0, 0, 0);
	ShowWindow(g_xChatEditBox.GetSafehWnd(), SW_HIDE);
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
    if ( FAILED(g_SoundManager.UsndInitDirectSound(g_xMainWnd.GetSafehWnd())) )
		g_SoundManager.mSndMngr_bInit = FALSE;
	else
	{
		g_SoundManager.mSndMngr_bInit = TRUE;

		g_SoundManager.ReadWaveFileList("SoundList.wwl");
		g_SoundManager.SndmngrReadBGMFileList("Bgmlist.wwl");
	}

	if ( !g_xSpriteInfo.SetInfo() )
	{
		return -1;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Random Seed�ʱ�ȭ.
	srand((unsigned)time(NULL));
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Windows Socket DLL�� �ʱ�ȭ�Ѵ�.
	WSAData wsd;
	if( WSAStartup( MAKEWORD(2, 2), &wsd ) != 0 )
	{
		return -1;
	}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  �����ϸ鼭 �α��� ���ν����� �����Ѵ�.
	if ( g_execCmds.bNotSelPatchd == TRUE )
	{
		g_xClientSocket.m_pxDefProc = g_xMainWnd.m_pxDefProcess = &g_xLoginProc;
		g_xLoginProc.Load();
		g_bProcState = _LOGIN_PROC;
#ifdef _DEBUG
		if ( g_execCmds.bFullScreen )
			g_xMainWnd.ResetDXG(640, 480, 16, _DXG_SCREENMODE_FULLSCREEN, _DXG_DEVICEMODE_PRIMARY|_DXG_DEVICEMODE_D3D);
		else
			g_xMainWnd.ResetDXG(640, 480, 16, _DXG_SCREENMODE_WINDOW, _DXG_DEVICEMODE_PRIMARY|_DXG_DEVICEMODE_D3D);
#else
		g_xMainWnd.ResetDXG(640, 480, 16, _DXG_SCREENMODE_WINDOW, _DXG_DEVICEMODE_PRIMARY|_DXG_DEVICEMODE_D3D);
#endif
	}
	else
	{
		g_xClientSocket.m_pxDefProc = g_xMainWnd.m_pxDefProcess = &g_xSelLoginProc;
		g_xSelLoginProc.Load();
		g_bProcState = _SELLOGIN_PROC;
	}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	DWORD dwLastTime, dwTime, dwDelay;
	dwLastTime = dwTime = dwDelay = 0;

//	WORD wClr = g_xMainWnd.ConvertColor24To16(RGB(255, 150, 150));
/*
	SYSTEMTIME	stTime1;
	GetLocalTime(&stTime1);

	CHAR pszTime1[100];
	sprintf(pszTime1, "[���� : %d�� %d�� %d�� %d�� %d�� %d��]", 
			stTime1.wYear, stTime1.wMonth, stTime1.wDay, stTime1.wHour, stTime1.wMinute, stTime1.wSecond);
*/

	g_bTextrRistricted = CehckTextrRistricted();

	while (TRUE)
    {
        if ( PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE) )
        {
            if ( 0 == GetMessage(&msg, NULL, 0, 0) )
                return (int) msg.wParam;

	        TranslateMessage(&msg);
            DispatchMessage(&msg);
		}
        else
        {
			dwTime = timeGetTime();
			g_dwCurrTime = dwTime;
			dwDelay = dwTime - dwLastTime;
			dwLastTime = dwTime;
//
//			Sleep(500);

			if ( dwDelay!=0 && g_xMainWnd.m_bIsWindowReady )
			{
				switch ( g_bProcState )
				{
				case _SELLOGIN_PROC:
					g_xSelLoginProc.RenderScene(dwDelay);
					break;
				case _LOGIN_PROC:
					g_xLoginProc.RenderScene(dwDelay);
					break;
				case _CHAR_SEL_PROC:
					g_xChrSelProc.RenderScene(dwDelay);
					break;
				case _GAME_PROC:
					{
						if ( g_OriginWndProc == GetWindowLong(g_xMainWnd.GetSafehWnd(), GWL_WNDPROC) )
						{
							if ( g_xMainWnd.m_bIsWindowActive )
							{
								g_xGameProc.RenderScene(dwDelay);
							}
							else
							{
								g_xGameProc.ProcessRecvPacket();
								g_xGameProc.ProcessDelQue();
							}
							break;
						}
					}
				}

				// ��ũ����.
				if ( GetAsyncKeyState( VK_SNAPSHOT ) )
				{
					SYSTEMTIME	stTime;
					GetLocalTime(&stTime);

					CHAR pszTime[100];
					sprintf(pszTime, "[%sServer %s] - %d/%d/%d %d:%d:%d", 
							g_szServerName, g_szCharName, stTime.wYear, stTime.wMonth, stTime.wDay, stTime.wHour, stTime.wMinute, stTime.wSecond);

					g_xMainWnd.PutsHan(NULL, 0, 0, RGB( 10,  10,  10), RGB(0, 0, 0), pszTime);
					g_xMainWnd.PutsHan(NULL, 1, 1, RGB(255, 200, 150), RGB(0, 0, 0), pszTime);

					scScreenCapture7( g_xMainWnd.GetBackBuffer() ); 
				}

				if ( FAILED(g_xMainWnd.Present()) )
				{
					g_xMainWnd.RestoreSurfaces();
				}

				// [SCHEME_B] 特效层：Present 之后�?D3D 特效，避免被软缓冲覆�?
			}
		}
	}

	return msg.wParam;
}
