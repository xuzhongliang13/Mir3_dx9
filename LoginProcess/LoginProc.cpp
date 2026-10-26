#include "StdAfx.h"

extern LONG	g_OriginWndProc;
//-----------------------------------------------------------------------------------------------------------------
// Procedure �����Լ�.
//-----------------------------------------------------------------------------------------------------------------
/******************************************************************************************************************
[������]
*******************************************************************************************************************/
CLoginProcess::CLoginProcess()
{
	Init();
}


/******************************************************************************************************************
[�Ҹ���]
*******************************************************************************************************************/
CLoginProcess::~CLoginProcess()
{
	DeleteProc();
}


/******************************************************************************************************************
������ ��簴ü�� �ʱ�ȭ.
*******************************************************************************************************************/
VOID CLoginProcess::Init()
{
	m_xMsgBox.InitMsgBox();
	m_xImageInter.Init();
	m_xImageLogin.Init();
	m_xAvi.Init();

	m_bProcState = _LOGINPROC_LOGO;
	m_bBackState = _LOGINPROC_LOGO;
	m_nSvrPort   = _LOGIN_SERVER_PORT_DATA;

	m_bLoginInputState = _LOGININPUT_ID;
	SetRect(&m_rcEditID, 0, 0, 0, 0);
	SetRect(&m_rcEditPW, 0, 0, 0, 0);

	ZeroMemory(&m_ptMousePos, sizeof(POINT));

	ZeroMemory(m_pszID, MAX_PATH);
	ZeroMemory(m_pszPW, MAX_PATH);

	ZeroMemory(&m_pszSvrIP, MAX_PATH);

	m_stSvrList.ClearAll();
	SetRect(&m_rcSvrListRgn, 0, 0, 0, 0);
	m_nClickedSvr = -1;

	m_nFadeProcTime = 0;

	m_hBrush = NULL;

	for ( INT nCnt = 0; nCnt < _MAX_LOGIN_BTN; nCnt++ )
	{
		m_xLoginBtn[nCnt].Init();
	}

 	D3DVECTOR vNorm(0, 0, -1);
	m_avBillBoard[0]  = D3DVERTEX(D3DVECTOR(-0.5f, 0.5f, 0), vNorm, 0, 0);
	m_avBillBoard[1]  = D3DVERTEX(D3DVECTOR(-0.5f,-0.5f, 0), vNorm, 0, 1);
	m_avBillBoard[2]  = D3DVERTEX(D3DVECTOR( 0.5f, 0.5f, 0), vNorm, 1, 0);
	m_avBillBoard[3]  = D3DVERTEX(D3DVECTOR( 0.5f,-0.5f, 0), vNorm, 1, 1);

	m_dwBGMPlayTime = 0;
	m_bBGMPlay		= TRUE;
	m_nBackIdx	= 0;
//	m_xClientSysMsg.Init();
}




BOOL CLoginProcess::LoadNoticeText()
{
	// ���帮��Ʈ �ʱ�ȭ.
	m_xNoticeList.ClearAll();

	// �������� ����.
	SetRect(&m_rcNotice, 23, 120, 247, 353);

	INT nW = m_rcNotice.right - m_rcNotice.left;

	INT nCnt;
	INT nLineCnt = 0;
	CHAR pszDivied[_MAX_STRING_LEN*2];
	CHAR pszArg[10][MAX_PATH];
	ZeroMemory(pszDivied, _MAX_STRING_LEN*2);
	ZeroMemory(pszArg,	  MAX_PATH*10);

	// �������� �ε�.

	// ��ȣȭ ��ƾ����.
	char* pszBuf = NULL;

	crypto_seed s = {(char) 0xF0, (char) 0x39, (char) 0xAB, (char) 0x8E, 0x9FDE1A93};
	crypto::setSeed( &s );

#ifdef _TEST_SERVER
	FILE *fp = fopen("NoticeTest.ntc", "rb");
#else
	FILE *fp = fopen("Notice.ntc", "rb");
#endif

	if ( !fp )
	{
		return FALSE;
	}

	int len = _filelength( _fileno( fp ) );
	pszBuf = new char[ len ];

	if ( !pszBuf )
	{
		fclose( fp );
		delete [] pszBuf;
		return FALSE;
	}

	fread( pszBuf, 1, len, fp );
	fclose( fp );

	if ( !crypto::decrypt( (unsigned char *)pszBuf, len ) )
	{
		delete[] pszBuf;
		return FALSE;
	}

	CHAR* pszTemp = pszBuf+8;

	INT nLen = strlen(pszTemp);

#ifdef _TEST_SERVER
	fp = fopen( "NoticeTest.txt", "wb" );
#else
	fp = fopen( "Notice.txt", "wb" );
#endif

	fwrite( pszTemp, sizeof( char ), nLen, fp );
	fclose( fp );

#ifdef _TEST_SERVER
	fp = fopen("NoticeTest.txt", "rb");
#else
	fp = fopen("Notice.txt", "rb");
#endif
	if ( !fp )
	{
		return FALSE;
	}

	CHAR	pszLine[MAX_PATH];
	CHAR	pszGet[MAX_PATH];
	CHAR*	pszNext = NULL;
	CHAR*	pszMark = NULL;
	CHAR*	pszStr  = NULL;

	while ( fgets(pszLine, MAX_PATH, fp) )
	{
		if ( pszLine[0] != ';' )
		{
			if ( pszNext = (CHAR*)strchr(pszLine, '\r') )
			{
				*pszNext = '\0';
			}

			pszStr = pszLine;

			INT	nSize   = 9;
			INT nBC     = 0;
			INT nFC     = 2;
			INT nWeight = FW_NORMAL;

			// ��Ʈ������.
			if ( pszMark = (CHAR*)strchr(pszLine, '!') )
			{
				ZeroMemory(pszGet, MAX_PATH);
				sscanf(++pszMark, "%s %*[^\0]", pszGet);
				nSize = atoi(pszGet);
			}

			// ��ƮBackColor.
			if ( pszMark = (CHAR*)strchr(pszLine, '@') )
			{
				ZeroMemory(pszGet, MAX_PATH);
				sscanf(++pszMark, "%s %*[^\0]", pszGet);
				nBC = atoi(pszGet);
			}

			// ��ƮColor.
			if ( pszMark = (CHAR*)strchr(pszLine, '#') )
			{
				ZeroMemory(pszGet, MAX_PATH);
				sscanf(++pszMark, "%s %*[^\0]", pszGet);
				nFC = atoi(pszGet);
			}

			// ��ƮBold.(�β���)
			if ( (CHAR*)strchr(pszLine, '$') )
			{
				nWeight = FW_BOLD;
			}

			// ����.
			if ( pszMark = (CHAR*)strchr(pszLine, '*') )
			{
				pszStr = ++pszMark;
			}

			ZeroMemory(pszDivied, _MAX_STRING_LEN*2);
			ZeroMemory(pszArg,	  MAX_PATH*10);
			g_xMainWnd.StringDivide(nW, nLineCnt, pszStr, pszDivied, g_xMainWnd.CreateGameFont(NULL, nSize, 0, nWeight));
			sscanf(pszDivied, "%[^`]%*c %[^`]%*c %[^`]%*c %[^`]%*c %[^`]%*c %[^`]%*c %[^`]%*c %[^`]%*c %[^`]%*c %[^`]%*c",
				   pszArg[0], pszArg[1], pszArg[2], pszArg[3], pszArg[4], 
				   pszArg[5], pszArg[6], pszArg[7], pszArg[8], pszArg[9]);

			if ( nLineCnt > 10 )
			{
				nLineCnt = 10;
			}

			for ( INT nCnt = 0; nCnt < nLineCnt; nCnt++ )
			{
				LPNOTICE pstNotice = new NOTICE;
				pstNotice->nBC  = nBC;
				pstNotice->nFC  = nFC;
				pstNotice->nSize = nSize;
				pstNotice->nWeight = nWeight;
				strcpy(pstNotice->pszMsg, pszArg[nCnt]);
				m_xNoticeList.Insert(pstNotice);
			}
		}
	}

	fclose( fp );
#ifdef _TEST_SERVER
	DeleteFile("NoticeTest.txt");
#else
	DeleteFile("Notice.txt");
#endif
	return TRUE;
}


VOID CLoginProcess::ShowNotice(INT nLoopTime)
{
	if ( m_xNoticeList.GetCount() > 0 && m_bProcState == _LOGINPROC_LOGIN )
	{
		// ȭ�����.
		LPNOTICE pstNotice   = NULL;
		CListNode < NOTICE > *pNode;

		INT nCnt;

		RECT rc = {m_rcNotice.left-5, m_rcNotice.top-5, m_rcNotice.right+5, m_rcNotice.bottom};
		INT nW = rc.right - rc.left;
		INT nH = rc.bottom - rc.top;

		D3DVECTOR	 vecTrans(rc.left, rc.top, 0);
		D3DVECTOR	 vecScale(nW, nH, 1);
		D3DMATERIAL9 mtrl;
/// ���� ��ġ
		INT nRed, nGreen, nBlue;
		INT nRedLine, nGreenLine, nBlueLine;

		switch ( m_nBackIdx )
		{
		case 20:
		case 27:
			nRed = 57;
			nGreen = 77;
			nBlue = 51;
			nRedLine = 43;
			nGreenLine = 55;
			nBlueLine = 38;
			break;
		case 21:
		case 22:
		case 23:
		case 30:
			nRed = 75;
			nGreen = 50;
			nBlue = 0;
			nRedLine = 100;
			nGreenLine = 75;
			nBlueLine = 50;
			break;
		case 24:
		case 25:
		case 26:
			nRed = 25;
			nGreen = 78;
			nBlue = 104;
			nRedLine = 40;
			nGreenLine = 95;
			nBlueLine = 120;
			break;
		case 28:
			nRed = 40;
			nGreen = 30;
			nBlue = 98;
			nRedLine = 27;
			nGreenLine = 20;
			nBlueLine = 65;
			break;
		case 29:
			nRed = 54;
			nGreen = 90;
			nBlue = 143;
			nRedLine = 37;
			nGreenLine = 62;
			nBlueLine = 99;
			break;
		case 31:
		case 32:
		case 33:
			nRed = 10;
			nGreen = 10;
			nBlue = 10;
			nRedLine = 5;
			nGreenLine = 5;
			nBlueLine = 5;
			break;
		}

		D3DUtil_InitMaterial(mtrl, (FLOAT)nRed/255.0f, (FLOAT)nGreen/255.0f, (FLOAT)nBlue/255.0f);
		mtrl.Diffuse.a = 100.0f/255.0f;
		DrawBillBoard(g_xMainWnd.Get3DDevice(), &vecTrans, &vecScale, &mtrl, NULL);
		g_xMainWnd.DrawWithGDI(&rc, NULL, RGB(nRedLine, nGreenLine, nBlueLine), 1);
/// ���� ��ġ

/*
		D3DUtil_InitMaterial(mtrl, (FLOAT)75/255.0f, (FLOAT)50/255.0f, (FLOAT)0/255.0f);
		mtrl.Diffuse.a = 100.0f/255.0f;
		DrawBillBoard(g_xMainWnd.Get3DDevice(), &vecTrans, &vecScale, &mtrl, NULL);
		g_xMainWnd.DrawWithGDI(&rc, NULL, RGB(100, 75, 50), 1);
*/

		static INT nScrlY = 0;
		static INT nSum   = -1500;

		if ( nLoopTime < 10000 )
		{
			nSum += nLoopTime;
		}

		if ( nSum > 100 )
		{
			nSum = 0;

			if ( m_xNoticeList.GetCount()*14 > nH )
			{
				nScrlY--;

				if ( abs(nScrlY) >= (m_xNoticeList.GetCount()-1)*14 )
				{
					nScrlY = nH-14;
				}
			}
		}

		for ( pNode = m_xNoticeList.GetHead(), nCnt = 0; pNode; pNode = pNode->GetNext(), nCnt++ )
		{
			pstNotice = pNode->GetData();

			if ( pstNotice )
			{
				if ( m_rcNotice.top+(nCnt*14)+nScrlY >= m_rcNotice.top && m_rcNotice.top+((nCnt+1)*14)+nScrlY < m_rcNotice.bottom )
				{
					g_xMainWnd.PutsHan(NULL, m_rcNotice.left, m_rcNotice.top+(nCnt*14)+nScrlY, 
									   GetChatColor(pstNotice->nFC), GetChatColor(pstNotice->nBC), 
									   pstNotice->pszMsg, g_xMainWnd.CreateGameFont(NULL, pstNotice->nSize, 0, pstNotice->nWeight));
				}
			}
		}
	}
}


VOID CLoginProcess::ReleaseNoticeText()
{
	// �������׺κ� �ʱ�ȭ.
}



/******************************************************************************************************************
�ʿ��� ����Ÿ�� �ʱ�ȭ�ϰ� �ε��Ѵ�.
Prcedure�� ���۵Ǳ����� ȣ��Ǿ�� �Ѵ�.
Load()->RenderScene()->SetNextProc()�� �����̴�.
*******************************************************************************************************************/
VOID CLoginProcess::Load()
{
	Init();

	SetSvrBtn();

	ZeroMemory(g_xChatEditBox.m_szInputMsg, sizeof(g_xChatEditBox.m_szInputMsg));
	SetWindowText(g_xChatEditBox.GetSafehWnd(), "");
	SendMessage(g_xChatEditBox.GetSafehWnd(), EM_SETPASSWORDCHAR, NULL, 0);
	g_xChatEditBox.SetLimitText(10);
	g_xChatEditBox.SetEnterUse(FALSE);
	MoveWindow(g_xChatEditBox.GetSafehWnd(), g_xMainWnd.m_rcWindow.left+288, g_xMainWnd.m_rcWindow.top+405, 75, 13, TRUE);
	ShowWindow(g_xChatEditBox.GetSafehWnd(), SW_HIDE);


	if ( g_execCmds.szServerIP[0] )
	{
		SetSvrIP(g_execCmds.szServerIP);
	}
	if ( g_execCmds.nPort != -1 )
	{
		SetSvrPort(g_execCmds.nPort);
	}

	if ( g_stInfoShop.bSetted )
	{
		SetSvrIP(g_stInfoShop.pszAddr);
		SetSvrPort(g_stInfoShop.nPort);
	}

	strcpy(g_szServerIP, m_pszSvrIP);

	m_xImageInter.NewLoad(".\\Data\\gameinter.wil");
	m_xImageLogin.NewLoad(".\\Data\\Interface1c.wil");

	ZeroMemory(g_xChatEditBox.m_szInputMsg, sizeof(g_xChatEditBox.m_szInputMsg));

	if ( m_hBrush != NULL )
	{
		DeleteObject(m_hBrush);
		m_hBrush = NULL;
	}
	m_hBrush = CreateSolidBrush(RGB(0, 0, 0));

	// [DX9][B] 登录背景动画：Login.dat 纯内存解码（lpDD=NULL 无音频、无 DX7 表面）
	m_pLoginBg = NULL;
	BOOL bAviOk = m_xAvi.Create(NULL, NULL, ".\\Data\\Login_mjpeg.avi", TRUE, FALSE);
	if ( bAviOk )
	{
		m_xAvi.SetDestRect(0, 60, 640, 420);
		m_xAvi.Start();
	}
	else
	{
		// [DX9][B] 解码器缺失时回退：加载动画帧静态背景（LoginBg.bmp）
		m_pLoginBg = g_xMainWnd.LoadBmp565(".\\Data\\LoginBg.bmp", &m_nLoginBgW, &m_nLoginBgH);
	}
	
	m_xLoginBtn[_LOGIN_BTN_CONN]. CreateGameBtn(&m_xImageLogin, 11, 11, 459, 436, NULL, _BTN_TYPE_FOCUS, 10, TRUE);
	m_xLoginBtn[_LOGIN_BTN_REG].  CreateGameBtn(&m_xImageLogin, 13, 13, 139, 379, NULL, _BTN_TYPE_FOCUS, 12, TRUE);
	m_xLoginBtn[_LOGIN_BTN_CHGPW].CreateGameBtn(&m_xImageLogin, 15, 15, 279, 379, NULL, _BTN_TYPE_FOCUS, 14, TRUE);
	m_xLoginBtn[_LOGIN_BTN_QUIT]. CreateGameBtn(&m_xImageLogin, 17, 17, 439, 379, NULL, _BTN_TYPE_FOCUS, 16, TRUE);

	SetRect(&m_rcEditID, 128, 440, 227, 454);
	SetRect(&m_rcEditPW, 326, 440, 425, 454);

	g_xChatEditBox.SetLimitText(24);
	// Enter Key�Է¿� ���ؼ� Game Procedure���� ���� ������ �����Ѵ�.
	g_xChatEditBox.SetEnterUse(FALSE);

	LoadNoticeText();

	g_xClientSocket.ConnectToServer(g_xMainWnd.GetSafehWnd(), m_pszSvrIP, m_nSvrPort);

	SndObjDestroy(m_stOpenSnd);
	m_stOpenSnd = SndObjCreate(g_SoundManager.mSound_lpds, ".\\Sound\\Opening.wav", 10);

	// ���ȭ�� �����ϰ� �׷��� �׸� �ε���
	m_nBackIdx = GetRandomNum(20,33);
}


/******************************************************************************************************************
�Ҹ��� �ʿ��� ��ü�� ����.
Procedure�� ����ǰų�, �����¿��� ���α׷��� �����ų�� ȣ��ȴ�.
*******************************************************************************************************************/
VOID CLoginProcess::DeleteProc()
{
	m_xImageInter.Destroy();
	m_xImageLogin.Destroy();

	g_xClientSocket.DisconnectToServer();

	if ( m_hBrush != NULL )
	{
		DeleteObject(m_hBrush);
		m_hBrush = NULL;
	}

	m_stSvrList.ClearAll();

	m_xAvi.Release();
	g_xMainWnd.FreeBmp565(m_pLoginBg);
	m_pLoginBg = NULL;

	// Edit Box������ ������� �ǵ�����.
	g_xChatEditBox.SetEnterUse(TRUE);

//	m_xClientSysMsg.Destroy();

	m_dwBGMPlayTime = 0;
	m_bBGMPlay		= TRUE;
}


/******************************************************************************************************************
���� Procedure�� ����.
ȣ��Ǹ� �����������ʹ� Procedure�� �ٲ��.
*******************************************************************************************************************/
VOID CLoginProcess::SetNextProc()
{
	DeleteProc();
	g_xClientSocket.m_pxDefProc = g_xMainWnd.m_pxDefProcess = &g_xChrSelProc;
	g_xChrSelProc.Load();
	g_bProcState = _CHAR_SEL_PROC;

	SndObjStop(m_stOpenSnd);
	SndObjDestroy(m_stOpenSnd);
}


//-----------------------------------------------------------------------------------------------------------------
// ȭ�� Render���� �Լ�.
//-----------------------------------------------------------------------------------------------------------------
/******************************************************************************************************************
3D ����ǿ� �ؽ��縦 �÷��� �׸���.
lpDevice   : �ʱ� DX3D���ý� ���� 3D��ġDevice.
pvTrans    : ������ǥ�� ������ ������ ������.(�ȼ���ǥ)
pvScale    : �̹����� ũ�⸦ ������ ������ ������.
pmtrl	   : ����� Matrial�� ������.
lpddsTextr : �ؽ���� ���� Surface.
bBlendType : ȭ�鿡 ������ ����.
*******************************************************************************************************************/
HRESULT CLoginProcess::DrawBillBoard(LPDIRECT3DDEVICE9 lpDevice, D3DVECTOR* pvTrans, D3DVECTOR* pvScale, /*D3DVECTOR vRot, */D3DMATERIAL9* pmtrl, /*FLOAT fRotRad, */IDirect3DTexture9* lpddsTextr, BYTE bBlendType)
{
	if ( lpDevice )
	{
		D3DMATRIX matTrans;
		D3DMATRIX matScale;
		D3DMATRIX matRot;
		D3DMATRIX matWorld;
		D3DMATRIX matWorldOriginal;

		pvTrans->x =  pvTrans->x+pvScale->x/2-320;
		pvTrans->y = -pvTrans->y-pvScale->y/2+240;
		lpDevice->SetMaterial(pmtrl);

		if( SUCCEEDED(lpDevice->BeginScene()) )
		{
			lpDevice->GetTransform(D3DTS_WORLD, &matWorldOriginal);

			D3DUtil_SetTranslateMatrix(matTrans, *pvTrans);
			D3DUtil_SetScaleMatrix(matScale, pvScale->x, pvScale->y, pvScale->z);
			D3DMath_MatrixMultiply(matWorld, matScale, matTrans);
			lpDevice->SetTransform(D3DTS_WORLD, &matWorld);

			SetBlendRenderState(lpDevice, bBlendType, pmtrl);

			lpDevice->SetTexture(0, lpddsTextr);

			lpDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, m_avBillBoard, sizeof(D3DVERTEX));

			ResetBlendenderState(lpDevice);

			lpDevice->SetTransform(D3DTS_WORLD, &matWorldOriginal);

			lpDevice->EndScene();

			return S_OK;
		}
	}
	return E_FAIL;
}


/******************************************************************************************************************
Procedure�� ���η���.
*******************************************************************************************************************/
VOID CLoginProcess::RenderScene(INT nLoopTime)
{	
	Clear(RGB(0, 0, 0));

	// [DX9][T67] 状态诊断日志: 每30帧输出当前状态(定位SVRSEL黑屏)

	RenderBackground(nLoopTime);

	switch ( m_bProcState )
	{
		case _LOGINPROC_LOGIN:
		{
//			g_xClientSocket.OnLogin("mir3pro", "78771267");

//			sprintf( m_pszID, "mir3pro" );
//			sprintf( m_pszPW, "78771267" );

			RenderLogin();
			break;
		}
		case _LOGINPROC_SVRSEL:
		{
			RenderSvrSel();
			break;
		}
		case _LOGINPROC_FADE:
		{
			RenderFade(nLoopTime);
			break;
		}
	}

	ShowNotice(nLoopTime);

	m_xMsgBox.RenderMsgBox(m_ptMousePos);
	// [DX9][B] 黑块根因已修复(CDX9SoftSurface::GetDC 同步软缓冲进 DIB, ReleaseDC 不再用全黑覆盖), 原生 RenderLogin 绘制保留背景立绘, 移除 re-blit 兜底
	// [DX9][AUDIT] 独立审计确认同一修复方向, 详见 AUDIT_REPORT.md



	//Ŭ���̾�Ʈ�޽���(���ʻ��).
	//m_xClientSysMsg.ShowSysMsg(nLoopTime, 30, 340);

}


/******************************************************************************************************************
����� �׸���.
����� Logo Avi -> Login Avi -> Still Image �� ������ �׷�����.
*******************************************************************************************************************/
VOID CLoginProcess::RenderBackground(INT nLoopTime)
{
	switch ( m_bBackState )
	{
		case _LOGINBACK_LOGO:
		{
			if ( TRUE )  // DX9 迁移期 AVI 禁用
			{
				m_bProcState = _LOGINPROC_LOGIN;
				m_bBackState = _LOGINBACK_LOGIN;

				m_bLoginInputState = _LOGININPUT_ID;

				SetEditFocus();
/*
				BOOL	bLoaded = FALSE;
				bLoaded = MyLoadLibrary();

				if (bLoaded == TRUE)
				{
					if (!MyInitialize())
					{
						MessageBox(g_xMainWnd.GetSafehWnd(), g_xMsg.GetMsg(4), "Information", MB_OK);
					}
					
					MyRegisterHWNDForEdit(KEYCRYPTION_EDIT_HWND, g_xChatEditBox.GetSafehWnd());

					if (!MyStartEncryption())
					{
						MessageBox(g_xMainWnd.GetSafehWnd(), g_xMsg.GetMsg(4), "Information", MB_OK);
					}
				}
				else
				{
					MessageBox(g_xMainWnd.GetSafehWnd(), g_xMsg.GetMsg(4), "Information", MB_OK);
				}
*/
/*				m_xAvi.Create(g_xMainWnd.GetDirectDraw(), g_SoundManager.mSound_lpds, ".\\Data\\ei_login.dat", FALSE);
				m_xAvi.SetDestRect(0, 60, 640, 420);
				m_xAvi.Start();*/
				
				m_bBackState = _LOGINBACK_STILL;

				Sleep(100);

				m_xAvi.Release();
			}
			break;
		}
		case _LOGINBACK_LOGIN:
		{
			if ( TRUE )  // DX9 迁移期 AVI 禁用
			{		
				m_bBackState = _LOGINBACK_STILL;
				Sleep(100);
				m_xAvi.Release();
			}
			break;
		}
		case _LOGINBACK_STILL:
		{
			// 1.2���Ŀ� BGM Play.
			if ( m_bBGMPlay )
			{
				m_dwBGMPlayTime += nLoopTime;

				if ( m_dwBGMPlayTime > 1200 )
				{
					SndObjPlay(0, 0, m_stOpenSnd, DSBPLAY_LOOPING);
					m_bBGMPlay     = FALSE;
					m_dwBGMPlayTime = 0;
				}
			}

			// [DX9][B] AVI 背景(Login_mjpeg.avi)优先，失败回退静态帧/Interface1c
			BOOL bAvi = FALSE;
			if ( m_xAvi.m_bPlaying && m_xAvi.m_lpOutput )
			{
				bAvi = g_xMainWnd.DrawAviFrame(&m_xAvi, 0, 60, 640, 420);
			}
			// [DX9][T25] 修复2: 背景随机化 - 优先使用WIL随机背景，m_pLoginBg仅作为最终回退
			// 原逻辑bug: m_pLoginBg总是非NULL导致永远走静态路径，随机背景代码永不执行
			if ( !bAvi && m_xImageLogin.NewSetIndex(m_nBackIdx) )
			{
				BOOL bScaled = g_xMainWnd.DrawWithImageForCompScaled(0, 0, m_xImageLogin.m_lpstNewCurrWilImageInfo->shWidth, m_xImageLogin.m_lpstNewCurrWilImageInfo->shHeight, (WORD*)m_xImageLogin.m_pbCurrImage, g_xMainWnd.m_stDisplayInfo.wWidth, g_xMainWnd.m_stDisplayInfo.wHeight);
			}
			else if ( !bAvi && m_pLoginBg )
			{
				// [DX9][B] 最终回退：静态背景帧铺满显示区
				g_xMainWnd.Draw565Scaled(0, 0, m_nLoginBgW, m_nLoginBgH, m_pLoginBg, g_xMainWnd.m_stDisplayInfo.wWidth, g_xMainWnd.m_stDisplayInfo.wHeight);
			}
			break;
		}	
	}
}


/******************************************************************************************************************
Login ȭ�鿡 �ʿ��� �κ��� �׷��ش�.
����� ID, PW, ��ư, ���.
*******************************************************************************************************************/
VOID CLoginProcess::RenderLogin()
{
	D3DVECTOR	 vecTrans(0.0f, 372.0f, 0);
	D3DVECTOR	 vecScale(640.0f, 48.0f, 1);

	D3DMATERIAL9 mtrl;
	D3DUtil_InitMaterial(mtrl, (FLOAT)50/255.0f, (FLOAT)50/255.0f, (FLOAT)60/255.0f);
	mtrl.Diffuse.a = 100.0f/255.0f;

	// [DX9][A] 修复按钮区域背景条：使用半透明白色替代深色，与原版一致
	// DrawWithGDI case 5: dwColor 格式 0xAARRGGBB (低字节alpha, 高3字节RGB)
	// 半透明白色: alpha=120, RGB=白色 => 0xFFFFFF78
	RECT rc = {0, 372, 640, 420};
	g_xMainWnd.DrawWithGDI(&rc, NULL, 0xFFFFFF78, 5);

	for ( INT nCnt = 0; nCnt < _MAX_LOGIN_BTN; nCnt++ )
	{
		m_xLoginBtn[nCnt].ShowGameBtn();
	}





	// �����ϱ� ��ư�̹����� ���°����...
	RECT* prcBtn = &m_xLoginBtn[_LOGIN_BTN_CONN].m_rcBtn;
	if ( PtInRect(prcBtn, m_ptMousePos) )
	{
		vecTrans = D3DVECTOR((FLOAT)prcBtn->left, (FLOAT)prcBtn->top, 0);
		vecScale = D3DVECTOR((FLOAT)prcBtn->right-prcBtn->left, (FLOAT)prcBtn->bottom-prcBtn->top, 1);

		D3DUtil_InitMaterial(mtrl, (FLOAT)150/255.0f, (FLOAT)100/255.0f, (FLOAT)50/255.0f);
		mtrl.Diffuse.a = 50.0f/255.0f;
		// [DX9][B] 清源修复: 悬停高亮 billboard(BeginScene 清软缓冲) 禁用, 用 FrameRect 边框替代
		//DrawBillBoard(g_xMainWnd.Get3DDevice(), &vecTrans, &vecScale, &mtrl, NULL);

		g_xMainWnd.DrawWithGDI(prcBtn, NULL, RGB(200, 150, 100), 1);
	}


	// [DX9][T82] 删除T25加的帧1绘制: Interface1c.wil 帧1=640x48全不透明单色条(0x30718), 画在(100,410)覆盖背景成深色横条;
	// 正常版(国服源码 LoginProc.cpp:737)只画 _LOGINIMG_IDPW(帧2=328x20半透明输入框+白字ID/PASSWORD), 帧1绘制为画蛇添足。

	// [DX9][T82d] 按用户要求恢复原版: 帧2(_LOGINIMG_IDPW 328x20半透明输入框+白字ID/PASSWORD)原始绘制。
	// 用户反馈淡蓝方案"框内黑+黑字看不清", 明确要求恢复原版按原始代码来。
	if ( m_xImageLogin.NewSetIndex(_LOGINIMG_IDPW) )
	{
		g_xMainWnd.DrawWithImageForComp(100, 436, m_xImageLogin.m_lpstNewCurrWilImageInfo->shWidth, m_xImageLogin.m_lpstNewCurrWilImageInfo->shHeight,
										(WORD*)(m_xImageLogin.m_pbCurrImage));
	}

	// ���ڿ��� ���̰� ĭ(100)���� ũ�� �����ϰ� �߶� �����ش�.
	CHAR pszTmp[MAX_PATH];
	INT  nLen = 0; 
	ZeroMemory(pszTmp, MAX_PATH);

	if ( m_bLoginInputState != _LOGININPUT_PW )
	{
		nLen = strlen(m_pszPW);
		memset(pszTmp, '*', nLen);

		SIZE sizeLen = g_xMainWnd.GetStrLength(NULL, NULL, pszTmp);
		if ( sizeLen.cx > 100 )
		{
			for ( INT nCnt = nLen; nCnt > 0; nCnt-- )
			{
				pszTmp[nCnt] = '\0';

				sizeLen = g_xMainWnd.GetStrLength(NULL, NULL, pszTmp);

				if ( sizeLen.cx < 100 )
				{
					break;
				}
			}
		}
		// [DX9][T82d] 按官方原版(D:\Mir3China LoginProc.cpp:768) 输入文字=白色 RGB(255,255,255), 帧2半透明暗青绿底上白字清晰
		g_xMainWnd.PutsHan(NULL, 326, 440, RGB(255, 255, 255), RGB(0,0,0), pszTmp);
	}

	if ( m_bLoginInputState != _LOGININPUT_ID )
	{
		nLen = strlen(m_pszID);
		strcpy(pszTmp, m_pszID);

		// ���ڿ��� ���̰� ĭ(100)���� ũ�� �����ϰ� �ڸ���.
		SIZE sizeLen = g_xMainWnd.GetStrLength(NULL, NULL, pszTmp);
		if ( sizeLen.cx > 100 )
		{
			for ( INT nCnt = nLen; nCnt > 0; nCnt-- )
			{
				pszTmp[nCnt] = '\0';

				sizeLen = g_xMainWnd.GetStrLength(NULL, NULL, pszTmp);

				if ( sizeLen.cx < 100 )
				{
					break;
				}
			}
		}
		// [DX9][T82d] 按官方原版(D:\Mir3China LoginProc.cpp:792) 输入文字=白色 RGB(255,255,255)
		g_xMainWnd.PutsHan(NULL, 128, 440, RGB(255, 255, 255), RGB(0,0,0), pszTmp);
	}

//	DrawEffect(5, 450, 345);
}


/******************************************************************************************************************
���� ���ÿ� �ʿ��� �κ��� �ο��ش�.
�����̸��� ���ÿ��ε�.
*******************************************************************************************************************/
VOID CLoginProcess::RenderSvrSel()
{
	// [DX9][T67] SVRSEL 渲染日志: 每30帧输出列表数量与区域(定位黑屏)

	INT			 nSvrListWidth  = m_rcSvrListRgn.right-m_rcSvrListRgn.left;
	INT			 nSvrListHeight = m_rcSvrListRgn.bottom-m_rcSvrListRgn.top;
	D3DVECTOR	 vecTrans((FLOAT)m_rcSvrListRgn.left, (FLOAT)m_rcSvrListRgn.top, 0);
	D3DVECTOR	 vecScale((FLOAT)nSvrListWidth, (FLOAT)nSvrListHeight, 1);
	POINT		 ptLoc;

	ptLoc.x = 40;
	ptLoc.y = 440;

	g_xMainWnd.PutsHan(NULL, ptLoc.x, ptLoc.y, RGB(200, 200, 50), RGB(0, 0, 0), g_xMsg.GetMsg(129));
	
	ptLoc.x += 200;

	g_xMainWnd.PutsHan(NULL, ptLoc.x, ptLoc.y, RGB(200, 200, 50), RGB(0, 0, 0), g_xMsg.GetMsg(130));
	
	ptLoc.x += 200;

	g_xMainWnd.PutsHan(NULL, ptLoc.x, ptLoc.y, RGB(200, 200, 50), RGB(0, 0, 0), g_xMsg.GetMsg(131));

	// [DX9][T24] 服务器列表整体背景：使用DrawWithGDI case 5 Alpha填充替代DrawBillBoard
	// dwColor格式: 0xBBGGRR_AA (低字节alpha, 高3字节BGR)
	// 灰色(100,100,100) alpha=120 => 0x64646478
	g_xMainWnd.DrawWithGDI(&m_rcSvrListRgn, NULL, 0x64646478, 5);
	g_xMainWnd.DrawWithGDI(&m_rcSvrListRgn, NULL, RGB(150, 150, 150), 1);

	INT						nCnt;
	CListNode < SVRLIST >	*pNode;
	LPSVRLIST				pstSvrList = NULL;
	CHAR					szTemp[MAX_PATH];

	for ( pNode = m_stSvrList.GetHead(), nCnt = 0; pNode; pNode = pNode->GetNext(), nCnt++ )
	{
		pstSvrList = pNode->GetData();

		if ( pstSvrList )
		{
			if ( m_nClickedSvr == -1 )
			{
				if ( PtInRect(&pstSvrList->rcRgn, m_ptMousePos) )
				{
					nSvrListWidth  = pstSvrList->rcRgn.right-pstSvrList->rcRgn.left;
					nSvrListHeight = pstSvrList->rcRgn.bottom-pstSvrList->rcRgn.top;
					vecTrans = D3DVECTOR((FLOAT)pstSvrList->rcRgn.left, (FLOAT)pstSvrList->rcRgn.top, 0);
					vecScale = D3DVECTOR((FLOAT)nSvrListWidth, (FLOAT)nSvrListHeight, 1);

					// [DX9][T24] 服务器项悬停背景：半透明灰色
					// 灰色(100,100,100) alpha=50 => 0x64646432
					g_xMainWnd.DrawWithGDI(&pstSvrList->rcRgn, NULL, 0x64646432, 5);

					if ( pstSvrList->nNum == 1)
						sprintf(szTemp, "%s (%s)", pstSvrList->pszCaption, g_xMsg.GetMsg(126));
					else if ( pstSvrList->nNum == 2)
						sprintf(szTemp, "%s (%s)", pstSvrList->pszCaption, g_xMsg.GetMsg(127));
					else if ( pstSvrList->nNum == 3)
						sprintf(szTemp, "%s (%s)", pstSvrList->pszCaption, g_xMsg.GetMsg(128));
					else
						sprintf(szTemp, "%s", pstSvrList->pszCaption);

					g_xMainWnd.PutsHan(NULL, pstSvrList->rcRgn.left+_SVRLIST_GAP_X, pstSvrList->rcRgn.top+_SVRLIST_GAP_Y/2, RGB(0, 0, 255), RGB(0, 0, 0), szTemp);

					g_xMainWnd.DrawWithGDI(&pstSvrList->rcRgn, NULL, RGB(100, 100, 200), 1);
				}
				else
				{
					if ( pstSvrList->nNum == 1)
						sprintf(szTemp, "%s (%s)", pstSvrList->pszCaption, g_xMsg.GetMsg(126));
					else if ( pstSvrList->nNum == 2)
						sprintf(szTemp, "%s (%s)", pstSvrList->pszCaption, g_xMsg.GetMsg(127));
					else if ( pstSvrList->nNum == 3)
						sprintf(szTemp, "%s (%s)", pstSvrList->pszCaption, g_xMsg.GetMsg(128));
					else
						sprintf(szTemp, "%s", pstSvrList->pszCaption);


					g_xMainWnd.PutsHan(NULL, pstSvrList->rcRgn.left+_SVRLIST_GAP_X, pstSvrList->rcRgn.top+_SVRLIST_GAP_Y/2, RGB(30, 30, 30), RGB(0, 0, 0), szTemp);
					
				}
			}
			else
			{
				if ( m_nClickedSvr == nCnt )
				{
					nSvrListWidth  = pstSvrList->rcRgn.right-pstSvrList->rcRgn.left;
					nSvrListHeight = pstSvrList->rcRgn.bottom-pstSvrList->rcRgn.top;
					vecTrans = D3DVECTOR((FLOAT)pstSvrList->rcRgn.left, (FLOAT)pstSvrList->rcRgn.top, 0);
					vecScale = D3DVECTOR((FLOAT)nSvrListWidth, (FLOAT)nSvrListHeight, 1);

					// [DX9][T24] 服务器项选中背景：半透明红色
					// 红色(255,100,100) alpha=50 => 0x6464FF32
					g_xMainWnd.DrawWithGDI(&pstSvrList->rcRgn, NULL, 0x6464FF32, 5);

					if ( pstSvrList->nNum == 1)
						sprintf(szTemp, "%s (%s)", pstSvrList->pszCaption, g_xMsg.GetMsg(126));
					else if ( pstSvrList->nNum == 2)
						sprintf(szTemp, "%s (%s)", pstSvrList->pszCaption, g_xMsg.GetMsg(127));
					else if ( pstSvrList->nNum == 3)
						sprintf(szTemp, "%s (%s)", pstSvrList->pszCaption, g_xMsg.GetMsg(128));
					else
						sprintf(szTemp, "%s", pstSvrList->pszCaption);

					g_xMainWnd.PutsHan(NULL, pstSvrList->rcRgn.left+_SVRLIST_GAP_X, pstSvrList->rcRgn.top+_SVRLIST_GAP_Y/2, RGB(255, 100, 100), RGB(0, 0, 0), szTemp);

					g_xMainWnd.DrawWithGDI(&pstSvrList->rcRgn, NULL, RGB(255, 100, 100), 1);
				}
				else
				{
					if ( pstSvrList->nNum == 1)
						sprintf(szTemp, "%s (%s)", pstSvrList->pszCaption, g_xMsg.GetMsg(126));
					else if ( pstSvrList->nNum == 2)
						sprintf(szTemp, "%s (%s)", pstSvrList->pszCaption, g_xMsg.GetMsg(127));
					else if ( pstSvrList->nNum == 3)
						sprintf(szTemp, "%s (%s)", pstSvrList->pszCaption, g_xMsg.GetMsg(128));
					else
						sprintf(szTemp, "%s", pstSvrList->pszCaption);

					g_xMainWnd.PutsHan(NULL, pstSvrList->rcRgn.left+_SVRLIST_GAP_X, pstSvrList->rcRgn.top+_SVRLIST_GAP_Y/2, RGB(30, 30, 30), RGB(0, 0, 0), szTemp);
				}
			}
		}
	}
}


/******************************************************************************************************************
������ ���õǾ����� ���� Procedure�� ȭ���� ��ȯ�ϱ����� ������ο����� �κ��� ó���Ѵ�.
*******************************************************************************************************************/
VOID CLoginProcess::RenderFade(INT nLoopTime)
{
	m_nFadeProcTime += nLoopTime;

	if ( m_nFadeProcTime < 2000 )
	{
		FLOAT		 fRate = 255.0f - (FLOAT)((FLOAT)m_nFadeProcTime*255.0f)/2000;

		D3DVECTOR	 vecTrans(0.0f, 0.0f, 0);
		D3DVECTOR	 vecScale(640.0f, 480.0f, 1);

		D3DMATERIAL9 mtrl;
		D3DUtil_InitMaterial(mtrl, fRate/255.0f, fRate/255.0f, fRate/255.0f);
		mtrl.Diffuse.a = 100/255.0f;

		DrawBillBoard(g_xMainWnd.Get3DDevice(), &vecTrans, &vecScale, &mtrl, NULL, 6);
	}
	else
	{
		Clear(RGB(0, 0, 0));
		SetNextProc();			// ���� ó�� ���ӽ�
	}	
}


//-----------------------------------------------------------------------------------------------------------------
// �α��� �κ� �Լ�.
//-----------------------------------------------------------------------------------------------------------------
/******************************************************************************************************************
�α��� ȭ�鿡�� ID, PW�� ��ġ�� ���� Edit Box�� �Ӽ��� ���Ѵ�.
*******************************************************************************************************************/
VOID CLoginProcess::SetEditFocus()
{
	if ( m_bLoginInputState == _LOGININPUT_ID )
	{
		GetWindowText(g_xChatEditBox.GetSafehWnd(), m_pszPW, MAX_PATH);
		SetWindowText(g_xChatEditBox.GetSafehWnd(), m_pszID);

		MoveWindow(g_xChatEditBox.GetSafehWnd(), 
				   g_xMainWnd.m_rcWindow.left+m_rcEditID.left, g_xMainWnd.m_rcWindow.top+m_rcEditID.top, m_rcEditID.right-m_rcEditID.left, m_rcEditID.bottom-m_rcEditID.top, TRUE);

		SendMessage(g_xChatEditBox.GetSafehWnd(), EM_SETPASSWORDCHAR, NULL, 0);
	}
	else
	{
		GetWindowText(g_xChatEditBox.GetSafehWnd(), m_pszID, MAX_PATH);
		SetWindowText(g_xChatEditBox.GetSafehWnd(), m_pszPW);

		MoveWindow(g_xChatEditBox.GetSafehWnd(), 
				   g_xMainWnd.m_rcWindow.left+m_rcEditPW.left, g_xMainWnd.m_rcWindow.top+m_rcEditPW.top, m_rcEditPW.right-m_rcEditPW.left, m_rcEditPW.bottom-m_rcEditPW.top, TRUE);

		SendMessage(g_xChatEditBox.GetSafehWnd(), EM_SETPASSWORDCHAR, (WPARAM)'*', 0);
	}
	if ( g_xMainWnd.m_bIsWindowActive )
	{
		g_xChatEditBox.SetSelectAll();
		SetFocus(g_xChatEditBox.GetSafehWnd());
		ShowWindow(g_xChatEditBox.GetSafehWnd(), SW_SHOW);
	}
}


//-----------------------------------------------------------------------------------------------------------------
// �������� �κ� �Լ�.
//-----------------------------------------------------------------------------------------------------------------
/******************************************************************************************************************
�������ù�ư�� ���� ������ Mir3.Ini���Ϸ� �����Է�.
*******************************************************************************************************************/
VOID CLoginProcess::SetSvrBtn()
{
	CHAR	pszFullPath[MAX_PATH];
	CHAR	pszTemp[MAX_PATH];
	INT		nSvrCount;

	GetCurrentDirectory(MAX_PATH, pszFullPath);

	strcat(pszFullPath+strlen(pszFullPath), "\\\0");

#ifdef _TEST_SERVER
 	strcat(pszFullPath, _MIR3EI_TSINI_FILE_NAME);
#else
 	strcat(pszFullPath, _MIR3EI_INI_FILE_NAME);
#endif

	GetPrivateProfileString(_MIR3EI_INI_SECTION, _LOGIN_SERVER_NAME, _LOGIN_GATE_SERVER_IP, m_pszSvrIP, 16, pszFullPath);

	m_nSvrPort = GetPrivateProfileInt(_MIR3EI_INI_SECTION, _LOGIN_SERVER_PORT, _LOGIN_SERVER_PORT_DATA, pszFullPath);
	nSvrCount  = GetPrivateProfileInt(_MIR3EI_SRV_SECTION, _SERVER_COUNT, 0, pszFullPath);							

	INT	 nCnt;
	SIZE sizeLenSave  = {0, 0};
	SIZE sizeLenCurr  = {0, 0};
	m_stSvrList.ClearAll();

	for ( INT nCnt = 0; nCnt < nSvrCount; nCnt++ )
	{
		LPSVRLIST pstSvrList = new SVRLIST;

		if ( pstSvrList )
		{
			ZeroMemory(pstSvrList, sizeof(SVRLIST));

			sprintf(pszTemp, "%s%d%s", "server", nCnt+1, "caption");
			GetPrivateProfileString(_MIR3EI_SRV_SECTION, (LPCTSTR)pszTemp, NULL, pstSvrList->pszCaption, MAX_PATH, pszFullPath);

			sprintf(pszTemp, "%s%d%s", "server", nCnt+1, "name");
			GetPrivateProfileString(_MIR3EI_SRV_SECTION, (LPCTSTR)pszTemp, NULL, pstSvrList->pszName, MAX_PATH, pszFullPath);

			sizeLenCurr = g_xMainWnd.GetStrLength(NULL, NULL, pstSvrList->pszCaption);

			if ( sizeLenCurr.cx > sizeLenSave.cx )
			{
				sizeLenSave = sizeLenCurr;
			}

			m_stSvrList.Insert(pstSvrList);
		}
	}

	INT nListCnt = m_stSvrList.GetCount();

	// �¿쿩�� 20, ���Ͽ��� 10�ȼ�, ������ ���� 10�ȼ�.
	SetRect(&m_rcSvrListRgn, _SVRLIST_SX-_SVRLIST_GAP_X, _SVRLIST_SY-_SVRLIST_GAP_Y, _SVRLIST_SX+sizeLenSave.cx+_SVRLIST_GAP_X+50, _SVRLIST_SY+(sizeLenSave.cy+_SVRLIST_GAP_Y)*nListCnt);

	CListNode < SVRLIST >	*pNode;
	LPSVRLIST				pstSvrList = NULL;

	for ( pNode = m_stSvrList.GetHead(), nCnt = 0; pNode; pNode = pNode->GetNext(), nCnt++ )
	{
		pstSvrList = pNode->GetData();

		if ( pstSvrList )
		{
			SetRect(&pstSvrList->rcRgn, _SVRLIST_SX-_SVRLIST_GAP_X, _SVRLIST_SY+(sizeLenSave.cy+_SVRLIST_GAP_Y)*nCnt-_SVRLIST_GAP_Y/2,
				    _SVRLIST_SX+sizeLenSave.cx+_SVRLIST_GAP_X+50, _SVRLIST_SY+(sizeLenSave.cy+_SVRLIST_GAP_Y)*(nCnt+1)-_SVRLIST_GAP_Y/2);
		}
	}
}


/******************************************************************************************************************
���콺��ǥ�� �������ù�ư�� ����Ʈ��ȣ�� ���´�.
*******************************************************************************************************************/
INT CLoginProcess::GetSvrNum(POINT ptMouse)
{
	INT						nCnt;
	CListNode < SVRLIST >	*pNode;
	LPSVRLIST				pstSvrList = NULL;

	for ( pNode = m_stSvrList.GetHead(), nCnt = 0; pNode; pNode = pNode->GetNext(), nCnt++ )
	{
		pstSvrList = pNode->GetData();

		if ( pstSvrList )
		{
			if ( PtInRect(&pstSvrList->rcRgn, ptMouse) )
			{
				return nCnt;
			}
		}
	}

	return -1;
}


/******************************************************************************************************************
����Ʈ��ȣ�� �������ù�ư�� ������ ���´�.
*******************************************************************************************************************/
LPSVRLIST CLoginProcess::GetSvrList(INT nNum)
{
	INT						nCnt;
	CListNode < SVRLIST >	*pNode;
	LPSVRLIST				pstSvrList = NULL;

	for ( pNode = m_stSvrList.GetHead(), nCnt = 0; pNode; pNode = pNode->GetNext(), nCnt++ )
	{
		pstSvrList = pNode->GetData();

		if ( pstSvrList )
		{
			if ( nNum == nCnt )
			{
				return pstSvrList;
			}
		}
	}

	return NULL;
}










//-----------------------------------------------------------------------------------------------------------------
// ��Ŷ���� �Լ�����.
//-----------------------------------------------------------------------------------------------------------------
/******************************************************************************************************************
�α伭���� ���Ӽ���.
*******************************************************************************************************************/
VOID CLoginProcess::OnConnectToServer()
{
}


/******************************************************************************************************************
dll Error Msg ���Ϸκ��� ���������� ���´�.
*******************************************************************************************************************/
/*VOID CLoginProcess::GetErrorMsg(INT nMsg, CHAR* pszMsg)
{
	HINSTANCE hLib;
	hLib = LoadLibrary(_LOGINMSG);

	if ( hLib )
	{
		LoadString(hLib, nMsg, pszMsg, MAX_PATH);
		FreeLibrary(hLib);
	}
}*/


/******************************************************************************************************************
��Ŷ�� �ڸ���.
*******************************************************************************************************************/
CHAR* CLoginProcess::OnMessageReceive(CHAR* pszBuf, INT nLen)
{
	CHAR* pszStart = pszBuf;
	CHAR* pszEnd;

	while ( pszEnd = (CHAR*)memchr(pszStart, '!', nLen) )
	{
		*pszEnd++ = '\0';

		OnSocketMessageReceive(pszStart+1);

		nLen -= pszEnd - pszStart;
		
		if ( !(pszStart = (CHAR*)memchr(pszEnd, '#', nLen)) )
		{
			return NULL;
		}
	}

	return pszStart;
}


/******************************************************************************************************************
#...!�� �߶��� ��Ŷ�� �м��Ѵ�.
*******************************************************************************************************************/
VOID CLoginProcess::OnSocketMessageReceive(CHAR* pszMsg)
{
	INT					nMsgNum;

	_TDEFAULTMESSAGE	tdm;

	fnDecodeMessage(&tdm, pszMsg);

	switch ( tdm.wIdent )
	{
		case SM_ID_NOTFOUND:
		{
			m_xMsgBox.CreateMsgBox(&m_xImageInter, LOGIN_WNDID_INPUT, _OK, g_xMsg.GetMsg(112), FALSE, 140, 110);
			break;
		}
		case SM_PASSWD_FAIL:
		{ 
			switch ( tdm.nRecog )
			{ 
			case -1:
				nMsgNum = 113;	// -1	�н����� Ʋ��
				break;
			case -2:
				nMsgNum = 114;	// �н����带 5ȸ ���� �� 10���� ������ �ʾ���
				break;
			case -3:
				nMsgNum = 116;	// ���� ���̵� ����
				break;
			case -5:
				nMsgNum = 117;	// ���̵� ������
				break;
			case -6:
				nMsgNum = 112;	// ���̵� ����
				break;
			case -7:			
				nMsgNum = 118;	// Mir2 ID�� Ȩ���������� �Ǹ��� �ؾ߸��մϴ�.
				return;
			case -9:
				nMsgNum = 133;	// ī������ 3ȸ ����
				break;
			default:
				nMsgNum = 116;	// ������ �˼� ����.
				break;
			}
			m_xMsgBox.CreateMsgBox(&m_xImageInter, LOGIN_WNDID_INPUT, _OK, g_xMsg.GetMsg(nMsgNum), FALSE, 140, 110);
			break;
		}
		case SM_CERTIFICATION_FAIL:
		{
			m_xMsgBox.CreateMsgBox(&m_xImageInter, LOGIN_WNDID_INPUT, _OK, g_xMsg.GetMsg(119), FALSE, 140, 110);
			break;
		}
		case SM_PASSOK_WRONGSSN:
		{
			INT nID = LOGIN_WNDID_GAMEOUT;

#ifdef _DEBUG
			nID = LOGIN_WNDID_INPUT;
#endif

			switch ( tdm.nRecog )
			{
				// �ùٸ��� �ʴ� �ֹε�Ϲ�ȣ �Դϴ�.
				case 1:
				{
					m_xMsgBox.CreateMsgBox(&m_xImageInter, nID, _OK, g_xMsg.GetMsg(124), FALSE, 140, 110);
					break;
				}
				// �� 15�� �̸��� ����ڴ� �̿��Ҽ� �����ϴ�. 
				case 2:
				{
					m_xMsgBox.CreateMsgBox(&m_xImageInter, nID, _OK, g_xMsg.GetMsg(125), FALSE, 140, 110);
					break;
				}
				default:
				{
					m_xMsgBox.CreateMsgBox(&m_xImageInter, nID, _OK, g_xMsg.GetMsg(122), FALSE, 140, 110);
					break;
				}
			}
			break;
		}
		case SM_NOT_IN_SERVICE:
		{
			m_xMsgBox.CreateMsgBox(&m_xImageInter, LOGIN_WNDID_GAMEOUT, _OK, g_xMsg.GetMsg(123), FALSE, 140, 110);
			break;
		}
		case SM_CARDIDPASSWORD:
		{
			INT nCnt = tdm.nRecog;
			CHAR	pszDecodeMsg[256];
			CHAR	pszMessage[256];

			ZeroMemory(pszDecodeMsg, sizeof(pszDecodeMsg));
			ZeroMemory(pszMsg, sizeof(pszMsg));
			fnDecode6BitBuf((pszMsg + _DEFBLOCKSIZE), pszDecodeMsg, sizeof(pszDecodeMsg));
			//fnDecode6BitBufOriginal((pszMsg + _DEFBLOCKSIZE), pszDecodeMsg, sizeof(pszDecodeMsg));

			sprintf(pszDecodeMsg, "%s", pszDecodeMsg);

			switch ( tdm.nRecog )
			{
			case 0:
				sprintf(pszMessage, "%s\n\n                 ", g_xMsg.GetMsg(134));	// 1ȸ �Է½�
				strcat(pszMessage, pszDecodeMsg);
				m_xMsgBox.CreateMsgBox(&m_xImageInter, LOGON_WNDID_CARDKEY, _OK, pszMessage, TRUE, 140, 110, 1);				
				break;
			case 1:
				sprintf(pszMessage, "%s\n\n                 ", g_xMsg.GetMsg(135));	// 2ȸ �Է½�
				strcat(pszMessage, pszDecodeMsg);
				m_xMsgBox.CreateMsgBox(&m_xImageInter, LOGON_WNDID_CARDKEY, _OK, pszMessage, TRUE, 140, 110, 1);				
				break;
			case 2:
				sprintf(pszMessage, "%s\n\n                 ", g_xMsg.GetMsg(136));	// 3ȸ �Է½�
				strcat(pszMessage, pszDecodeMsg);
				m_xMsgBox.CreateMsgBox(&m_xImageInter, LOGON_WNDID_CARDKEY, _OK, pszMessage, TRUE, 140, 110, 1);				
				break;
			}
			break;
		}
		case SM_PASSOK_SELECTSERVER:
		{
			CHAR	pszDecodeMsg[256];
			char	*pszIndex = NULL;
			char	*pszNext = NULL;
			char	*pszTemp = NULL;
			INT		nPos = 0, nCount = 0;
			char	szTemp[1024];

			ZeroMemory(pszDecodeMsg, sizeof(pszDecodeMsg));
			//nPos = fnDecode6BitBuf((pszMsg + _DEFBLOCKSIZE), pszDecodeMsg, sizeof(pszDecodeMsg));
			nPos = fnDecode6BitBufOriginal((pszMsg + _DEFBLOCKSIZE), pszDecodeMsg, sizeof(pszDecodeMsg));  //20051031
			pszDecodeMsg[nPos] = '\0';
			pszIndex = pszDecodeMsg;

			CListNode < SVRLIST >	*pNode;
			LPSVRLIST				pstSvrList = NULL;

			do
			{
				if ( pszNext = strchr(pszIndex, ';'))
				{
					*pszNext++ = '\0';

					pszTemp = strchr(pszIndex, '(');
					*pszTemp++ = '\0';

					while( *pszTemp != ')' )
					{
						szTemp[nCount] = *pszTemp;
						nCount++;
						*pszTemp++;
					}
					szTemp[nCount] = '\0';

					for ( pNode = m_stSvrList.GetHead(); pNode; pNode = pNode->GetNext())
					{
						pstSvrList = pNode->GetData();

						if ( pstSvrList )
						{
							if ( strcmp(pstSvrList->pszName, pszIndex) == 0 )
							{
								pstSvrList->nNum = atoi(szTemp);
							}
						}
					}

					pszIndex = pszNext;
					nCount = 0;
				}
				
			} while (pszNext);

//			ShowWindow(g_xChatEditBox.GetSafehWnd(), SW_HIDE);
//			SetFocus(g_xMainWnd.GetSafehWnd());

			strcpy(g_szUserID, m_pszID);

			m_bProcState = _LOGINPROC_SVRSEL;

//			Sleep(1000);
//			g_xClientSocket.OnSelectServer("MIR3�׽�Ʈ");
//			strcpy(g_szServerName, "MIR3�׽�Ʈ");

			int m_nAvailIDDay  = tdm.wTag;
			int m_nAvailIPDay  = tdm.wSeries;
			int m_nAvailIPHour = tdm.wParam;
			int m_nAvailIDHour = tdm.nRecog;

			CHAR pszTmpMsg[MAX_PATH];

			if ( m_nAvailIDDay > 0 )
			{
				g_nPaySystem = _KIND_PRIVATE_FIXED_MONEY;
				//����������
				if ( m_nAvailIDDay == 1 )
				{
					m_xMsgBox.CreateMsgBox(&m_xImageInter, LOGIN_WNDID_INPUT, _OK, g_xMsg.GetMsg(102), FALSE, 140, 110);
				}
				else if ( m_nAvailIDDay > 1 )
				{
					sprintf(pszTmpMsg, g_xMsg.GetMsg(103), m_nAvailIDDay - 1 );
					m_xMsgBox.CreateMsgBox(&m_xImageInter, LOGIN_WNDID_INPUT, _OK, pszTmpMsg, FALSE, 140, 110);
				}
			}
			else if ( m_nAvailIPDay > 0 )
			{
				g_nPaySystem = _KIND_IP_FIXED_MONEY;
				//IP������
				if ( m_nAvailIPDay == 1 )
				{
					m_xMsgBox.CreateMsgBox(&m_xImageInter, LOGIN_WNDID_INPUT, _OK, g_xMsg.GetMsg(105), FALSE, 140, 110);
				}
				else if ( m_nAvailIPDay > 1 )
				{
					sprintf(pszTmpMsg, g_xMsg.GetMsg(104), m_nAvailIPDay - 1 );
					m_xMsgBox.CreateMsgBox(&m_xImageInter, LOGIN_WNDID_INPUT, _OK, pszTmpMsg, FALSE, 140, 110);						
				}
			}
			else if ( m_nAvailIPHour > 0 )
			{
				g_nPaySystem = _KIND_IP_FIXED_TIME;
				//IP������
				if ( m_nAvailIPHour <= 100 )
				{
					sprintf(pszTmpMsg, g_xMsg.GetMsg(106), m_nAvailIPHour);
					m_xMsgBox.CreateMsgBox(&m_xImageInter, LOGIN_WNDID_INPUT, _OK, pszTmpMsg, FALSE, 140, 110);						
				}
			}
			else if ( m_nAvailIDHour > 0 )
			{
				g_nPaySystem = _KIND_PRIVATE_FIXED_TIME;
				//����������
				sprintf(pszTmpMsg, g_xMsg.GetMsg(107), m_nAvailIDHour);
				m_xMsgBox.CreateMsgBox(&m_xImageInter, LOGIN_WNDID_INPUT, _OK, pszTmpMsg, FALSE, 140, 110);						
			}
/*			else
			{
				sprintf(pszTmpMsg, "IDDAY:%d IPDAY:%d IDHOUR:%d IPHOUR:%d", m_nAvailIDDay, m_nAvailIPDay, m_nAvailIDHour, m_nAvailIPHour);
				m_xMsgBox.CreateMsgBox(&m_xImageInter, LOGIN_WNDID_INPUT, _OK, pszTmpMsg, FALSE, 140, 110);						
			}*/

//			sprintf(pszTmpMsg, "IDDAY:[%d]  IPDAY:[%d]  IDHOUR:[%d]  IPHOUR:[%d]", m_nAvailIDDay, m_nAvailIPDay, m_nAvailIDHour, m_nAvailIPHour);
//			m_xClientSysMsg.AddSysMsg(pszTmpMsg);

			break;
		}
		case SM_CHGPASSWD_SUCCESS:
		{
			break;
		}
		case SM_CHGPASSWD_FAIL:
		{
			break;	
		}
		case SM_SELECTSERVER_OK:
		{
			CHAR  pszDecodeMsg[256];
			CHAR* pszIP		= NULL;
			CHAR* pszPort	= NULL;
			CHAR* pszIdenty	= NULL;

			ZeroMemory(pszDecodeMsg, sizeof(pszDecodeMsg));
			//fnDecode6BitBuf((pszMsg + _DEFBLOCKSIZE), pszDecodeMsg, sizeof(pszDecodeMsg));
			fnDecode6BitBufOriginal((pszMsg + _DEFBLOCKSIZE), pszDecodeMsg, sizeof(pszDecodeMsg));  //20051031

			pszIP = pszDecodeMsg;

			if ( pszPort = strchr(pszIP, '/') )
			{
				*pszPort = '\0';
				pszPort++;

				if ( pszIdenty = strchr(pszPort, '/') )
				{
					*pszIdenty = '\0';
					pszIdenty++;

					strcpy(g_szPreServerIP, g_szServerIP);
					g_nPreServerPort = g_nServerPort;

					strcpy(g_szServerIP, pszDecodeMsg);
					g_nServerPort = atoi(pszPort);
					g_nCertifyCode = tdm.nRecog;

					m_bProcState = _LOGINPROC_FADE;
				}
			}
			break;
		}
		case SM_OUTOFCONNECTION:
		{
			m_xMsgBox.CreateMsgBox(&m_xImageInter, LOGIN_WNDID_QUIT, _OK, g_xMsg.GetMsg(121), FALSE, 140, 110);
			break;
		}
	}
}










//-----------------------------------------------------------------------------------------------------------------
// ������ �޽��� ���� �Լ�����.
//-----------------------------------------------------------------------------------------------------------------
/******************************************************************************************************************
�޽��� Procedure.
*******************************************************************************************************************/
LRESULT CLoginProcess::DefMainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch ( uMsg )
	{
		case WM_CTLCOLOREDIT:
		{
			if ( (HWND)lParam == g_xChatEditBox.GetSafehWnd() )
			{
				if ( m_hBrush != NULL )
				{
					SetBkColor((HDC)wParam, RGB(0, 0, 0));
					SetTextColor((HDC)wParam, RGB(255, 255, 255));
					UnrealizeObject(m_hBrush);
					SetBrushOrgEx((HDC)wParam, 0, 0, NULL);
					return (LRESULT)m_hBrush;
				}
			}
			break;
		}
		case WM_DESTROY:
			OnDestroy(wParam, lParam);
			break;
		case WM_SYSKEYDOWN:
#ifndef _DEBUG
			return 0L;
#endif
			break;
		case WM_KEYDOWN:
			OnKeyDown(wParam, lParam);
			break;
		case ID_SOCKCLIENT_EVENT_MSG:
			g_xClientSocket.OnSocketMessage(wParam, lParam);
			break;
		case WM_LBUTTONDOWN:
			OnLButtonDown(wParam, lParam);
			break;
		case WM_LBUTTONUP:
			OnLButtonUp(wParam, lParam);
			break;
		case WM_MOUSEMOVE:
			OnMouseMove(wParam, lParam);
			break;
		case WM_MOVE:
			OnMove(wParam, lParam);
			break;
		case _WM_MSGBOX_RETURN:
			OnMsgBoxReturn(wParam, lParam);
			break;
	}
	return CWHDefProcess::DefMainWndProc(hWnd, uMsg, wParam, lParam);
}


/******************************************************************************************************************
���α׷� �����.
*******************************************************************************************************************/
LRESULT CLoginProcess::OnDestroy(WPARAM wParam, LPARAM lParam)
{
//	DeleteProc();
	WSACleanup();
//	ReleaseFontList();
	return 0L;
}


HRESULT	CLoginProcess::OnKeyDown(WPARAM wParam, LPARAM lParam)
{
	if ( m_xMsgBox.OnKeyDown(wParam, lParam) )
	{
		return 0L;
	}

	switch ( wParam )
	{
		case VK_TAB:
		{
			if ( m_bProcState == _LOGINPROC_LOGIN )
			{
				if ( m_bLoginInputState == _LOGININPUT_ID )
				{
					m_bLoginInputState = _LOGININPUT_PW;
					SetEditFocus();
				}
				else
				{
					m_bLoginInputState = _LOGININPUT_ID;
					SetEditFocus();
				}
			}
			break;
		}
		case VK_RETURN:
		{
			if ( m_bProcState == _LOGINPROC_LOGIN )
			{
				// TABŰ�� �����ϴ�.
				if ( m_bLoginInputState == _LOGININPUT_ID )
				{
					m_bLoginInputState = _LOGININPUT_PW;
					SetEditFocus();
				}
				else
				{
					m_bLoginInputState = _LOGININPUT_ID;
					SetEditFocus();
				}

				if ( m_bLoginInputState == _LOGININPUT_ID && m_pszID[0] != '\0' && m_pszPW[0] != '\0' )
				{
					// ID�� PW�� Ʋ���� ��쿡 PW�Է»��·� �����Ѵ�.
					m_bLoginInputState = _LOGININPUT_SETTED;

					// �����ϱ� ��ư�� �����Ͱ� ����.
					g_xClientSocket.OnLogin(m_pszID, m_pszPW);
					SetFocus(g_xMainWnd.GetSafehWnd());
					ShowWindow(g_xChatEditBox.GetSafehWnd(), SW_HIDE);
					return 0L;
				}
			}
			break;
		}
	}

	return 0L;
}


LRESULT	CLoginProcess::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
	m_ptMousePos.x = LOWORD(lParam);
	m_ptMousePos.y = HIWORD(lParam);

	if ( m_xMsgBox.OnLButtonDown(m_ptMousePos) )
	{
		return 0L;
	}

	if ( m_bProcState == _LOGINPROC_LOGIN )
	{
		if ( PtInRect(&m_rcEditID, m_ptMousePos) )
		{
			m_bLoginInputState = _LOGININPUT_ID;
			SetEditFocus();
		}
		else if ( PtInRect(&m_rcEditPW, m_ptMousePos) )
		{
			m_bLoginInputState = _LOGININPUT_PW;
			SetEditFocus();
		}
		else
		{
			m_xLoginBtn[_LOGIN_BTN_CONN] .OnLButtonDown(m_ptMousePos);
			m_xLoginBtn[_LOGIN_BTN_REG]  .OnLButtonDown(m_ptMousePos);
			m_xLoginBtn[_LOGIN_BTN_CHGPW].OnLButtonDown(m_ptMousePos);
			m_xLoginBtn[_LOGIN_BTN_QUIT] .OnLButtonDown(m_ptMousePos);
		}
	}
	else if ( m_bProcState == _LOGINPROC_SVRSEL )
	{
		m_nClickedSvr = GetSvrNum(m_ptMousePos);
	}

	return 0L;
}


LRESULT	CLoginProcess::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
	m_ptMousePos.x = LOWORD(lParam);
	m_ptMousePos.y = HIWORD(lParam);

	if ( m_xMsgBox.OnLButtonUp(m_ptMousePos) )
	{
		return 0L;
	}

	if ( m_bProcState == _LOGINPROC_LOGIN )
	{
		CHAR	pszFullPath[MAX_PATH];
		CHAR	pszURL[MAX_PATH];

		ZeroMemory(pszFullPath, MAX_PATH);
		ZeroMemory(pszURL, MAX_PATH);

		GetCurrentDirectory(MAX_PATH, pszFullPath);
		strcat(pszFullPath + strlen(pszFullPath), "\\\0");

#ifdef _TEST_SERVER
 		strcat(pszFullPath, _MIR3EI_TSINI_FILE_NAME);
#else
 		strcat(pszFullPath, _MIR3EI_INI_FILE_NAME);
#endif

		if ( m_xLoginBtn[_LOGIN_BTN_CONN].OnLButtonUp(m_ptMousePos) )
		{
			if ( m_bLoginInputState == _LOGININPUT_ID )
			{
				m_bLoginInputState = _LOGININPUT_PW;
				SetEditFocus();
			}
			else
			{
				m_bLoginInputState = _LOGININPUT_ID;
				SetEditFocus();
			}

			g_xClientSocket.OnLogin(m_pszID, m_pszPW);
			SetFocus(g_xMainWnd.GetSafehWnd());
			ShowWindow(g_xChatEditBox.GetSafehWnd(), SW_HIDE);
		}
		else if (m_xLoginBtn[_LOGIN_BTN_REG].OnLButtonUp(m_ptMousePos) )
		{
			// [FIX-R2] restore original behaviour: open the account-registration
			// web page from Mir3.ini (Param2), then close the client.
				GetPrivateProfileString(_MIR3EI_INI_SECTION, _URL_ACCOUNT, _URL_NEW_ACCOUNT, pszURL, MAX_PATH, pszFullPath);
				ShellExecute(NULL, NULL, pszURL, NULL, NULL, SW_SHOWNORMAL);
			
				SendMessage(g_xMainWnd.GetSafehWnd(), WM_DESTROY, NULL, NULL);
		}
else if ( m_xLoginBtn[_LOGIN_BTN_CHGPW].OnLButtonUp(m_ptMousePos) )
		{
			GetPrivateProfileString(_MIR3EI_INI_SECTION, _URL_CHANGEPWD, _URL_CHANGE_PASSWORD, pszURL, MAX_PATH, pszFullPath);
			ShellExecute(NULL, NULL, pszURL, NULL, NULL, SW_SHOWNORMAL);

			SendMessage(g_xMainWnd.GetSafehWnd(), WM_DESTROY, NULL, NULL);
		}
		else if ( m_xLoginBtn[_LOGIN_BTN_QUIT].OnLButtonUp(m_ptMousePos) )
		{
			DeleteProc();
			g_xMainWnd.m_pxDefProcess = &g_xGameProc;
			g_OriginWndProc = GetWindowLong(g_xMainWnd.GetSafehWnd(), GWL_WNDPROC);
			g_bProcState = _GAME_PROC;	
			g_xGameProc.m_bCreditShow = TRUE;
			g_xGameProc.m_xImage.m_xImageList[70].NewLoad(".\\Data\\GameInter.wil");
			g_xGameProc.LoadCredit();
			ShowWindow(g_xChatEditBox.GetSafehWnd(), SW_HIDE);
//			SendMessage(g_xMainWnd.GetSafehWnd(), WM_DESTROY, NULL, NULL);
		}
	}
	else if ( m_bProcState == _LOGINPROC_SVRSEL )
	{
		if ( m_nClickedSvr != -1 )
		{
			INT nClickedSvr = GetSvrNum(m_ptMousePos);

			if ( m_nClickedSvr == nClickedSvr )
			{
				LPSVRLIST pstSvrList = GetSvrList(m_nClickedSvr);

				if ( pstSvrList ) 
				{
					g_xClientSocket.OnSelectServer(pstSvrList->pszName);
					strcpy(g_szServerName, pstSvrList->pszName);
				}
			}

			m_nClickedSvr = -1;
		}
	}

	return 0L;
}


LRESULT	CLoginProcess::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
	m_ptMousePos.x = LOWORD(lParam);
	m_ptMousePos.y = HIWORD(lParam);

	m_xMsgBox.OnMouseMove(m_ptMousePos);

	if ( m_bProcState == _LOGINPROC_LOGIN )
	{
		m_xLoginBtn[_LOGIN_BTN_CONN] .OnMouseMove(m_ptMousePos);
		m_xLoginBtn[_LOGIN_BTN_REG]  .OnMouseMove(m_ptMousePos);
		m_xLoginBtn[_LOGIN_BTN_CHGPW].OnMouseMove(m_ptMousePos);
		m_xLoginBtn[_LOGIN_BTN_QUIT] .OnMouseMove(m_ptMousePos);
	}

	return 0L;
}


LRESULT	CLoginProcess::OnMsgBoxReturn(WPARAM wParam, LPARAM lParam)
{
	BYTE bWndID		= HIBYTE(HIWORD(wParam));
	BYTE bMsgReturn	= LOBYTE(HIWORD(wParam));
	WORD wPacket	= LOWORD(wParam);

	switch ( bWndID )
	{
		case LOGIN_WNDID_QUIT:
		{
			if ( bMsgReturn == _BTN_OK)
			{
				SendMessage(g_xMainWnd.GetSafehWnd(), WM_DESTROY, NULL, NULL);
				break;
			}
			break;
		}
		case LOGIN_WNDID_INPUT:
		{
			if ( m_bProcState == _LOGINPROC_LOGIN )
			{
				// �Է»��¸� �缳���Ѵ�. ->PW�� ����� ID�� ���¸� �����Ѵ�.
				ZeroMemory(m_pszPW, MAX_PATH);
				SetWindowText(g_xChatEditBox.GetSafehWnd(), m_pszID);

				m_bLoginInputState = _LOGININPUT_PW;
				SetEditFocus();
				g_xChatEditBox.SetEnterUse(FALSE);
			}
			break;
		}
		case LOGIN_WNDID_GAMEOUT:
		{
#ifdef _RELEASE
			CHAR szUrl[MAX_PATH];
			sprintf( szUrl, "%s", g_xMsg.GetMsg(1077) );
			ShellExecute(NULL, NULL, szUrl, NULL, NULL, SW_SHOWNORMAL);

			SendMessage(g_xMainWnd.GetSafehWnd(), WM_DESTROY, NULL, NULL);
#endif
			break;
		}
		case LOGON_WNDID_CARDKEY:
		{
			if ( bMsgReturn == _BTN_OK)
			{
				CHAR szTemp[MAX_PATH];

				sprintf( szTemp, "%s", lParam);
				
//				if ( strlen(szTemp) != 0 )
				g_xClientSocket.SendCardNum(szTemp);
			}
			break;
		}
	}

	return 0L;
}


LRESULT	CLoginProcess::OnMove(WPARAM wParam, LPARAM lParam)
{
	if ( m_bProcState == _LOGINPROC_LOGIN )
	{
		SetFocus(g_xMainWnd.GetSafehWnd());
		ShowWindow(g_xChatEditBox.GetSafehWnd(), SW_HIDE);
	}

	return 0L;
}


VOID CLoginProcess::DrawEffect(INT nIdx, INT nX, INT nY)
{
	if ( g_xMainWnd.Get3DDevice() )
	{
		if ( m_xImageLogin.NewSetIndex(nIdx) )
		{
			D3DWILTextr_CreateEmptyTexture(m_xImageLogin.m_szWilFileName, nIdx, 
										   m_xImageLogin.m_lpstNewCurrWilImageInfo->shWidth,
										   m_xImageLogin.m_lpstNewCurrWilImageInfo->shHeight,
										   (WORD*)m_xImageLogin.m_pbCurrImage, D3DTEXTR_TRANSPARENTBLACK);
			D3DWILTextr_Restore(m_xImageLogin.m_szWilFileName, nIdx, g_xMainWnd.Get3DDevice());

			D3DVECTOR vTrans;
			D3DMATRIX matTrans;
			D3DMATRIX matScale;
			D3DMATRIX matWorld;
			D3DMATRIX matWorldOriginal;
			D3DMATERIAL9 mtrl;

			D3DUtil_InitMaterial(mtrl, (FLOAT)255.0f/255.0f, (FLOAT)255.0f/255.0f, (FLOAT)255.0f/255.0f);
			mtrl.Diffuse.a = 255/255.0f;
			g_xMainWnd.Get3DDevice()->SetMaterial(&mtrl);

			vTrans.x = (FLOAT) nX+(FLOAT)m_xImageLogin.m_lpstNewCurrWilImageInfo->shWidth /2+m_xImageLogin.m_lpstNewCurrWilImageInfo->shPX-320; 
			vTrans.y = (FLOAT)-nY-(FLOAT)m_xImageLogin.m_lpstNewCurrWilImageInfo->shHeight/2-m_xImageLogin.m_lpstNewCurrWilImageInfo->shPY+240; 
			vTrans.z = 0.0f;

			if( SUCCEEDED(g_xMainWnd.Get3DDevice()->BeginScene()))
			{
				g_xMainWnd.Get3DDevice()->GetTransform(D3DTS_WORLD, &matWorldOriginal);

				D3DUtil_SetTranslateMatrix(matTrans, vTrans);
				D3DUtil_SetScaleMatrix(matScale, (FLOAT)m_xImageLogin.m_lpstNewCurrWilImageInfo->shWidth, (FLOAT)m_xImageLogin.m_lpstNewCurrWilImageInfo->shHeight, 1.0f);
				D3DMath_MatrixMultiply(matWorld, matScale, matTrans);
				g_xMainWnd.Get3DDevice()->SetTransform(D3DTS_WORLD, &matWorld);
				
				SetBlendRenderState(g_xMainWnd.Get3DDevice(), _BLEND_LIGHT, &mtrl);

				g_xMainWnd.Get3DDevice()->SetTexture(0, D3DWILTextr_GetSurface(m_xImageLogin.m_szWilFileName, nIdx));

				g_xMainWnd.Get3DDevice()->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, m_avBillBoard, sizeof(D3DVERTEX));

				ResetBlendenderState(g_xMainWnd.Get3DDevice());

				g_xMainWnd.Get3DDevice()->SetTransform(D3DTS_WORLD, &matWorldOriginal);

				g_xMainWnd.Get3DDevice()->EndScene();
			}

			D3DWILTextr_Invalidate(m_xImageLogin.m_szWilFileName, nIdx);
			D3DWILTextr_DestroyTexture(m_xImageLogin.m_szWilFileName, nIdx);
		}
	}
}
