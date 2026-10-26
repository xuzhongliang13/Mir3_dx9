#include "StdAfx.h"

extern LONG	g_OriginWndProc;

//-----------------------------------------------------------------------------------------------------------------
// Procedure �����Լ�.
//-----------------------------------------------------------------------------------------------------------------
/******************************************************************************************************************
[������]
*******************************************************************************************************************/
CChrProcess::CChrProcess()
{
	Init();
}


/******************************************************************************************************************
[�Ҹ���]
*******************************************************************************************************************/
CChrProcess::~CChrProcess()
{
	DeleteProc();
}


/******************************************************************************************************************
������ ��簴ü��?�ʱ�ȭ.
*******************************************************************************************************************/
VOID CChrProcess::Init()
{
	m_xMsgBox.InitMsgBox();
	m_xImageInter.Init();
	m_xImageChr.Init();
	m_xAvi.Init();

	ZeroMemory(&m_ptMousePos, sizeof(POINT));

	m_hBrush = NULL;

 	D3DVECTOR vNorm(0, 0, -1);
	m_avBillBoard[0]  = D3DVERTEX(D3DVECTOR(-0.5f, 0.5f, 0), vNorm, 0, 0);
	m_avBillBoard[1]  = D3DVERTEX(D3DVECTOR(-0.5f,-0.5f, 0), vNorm, 0, 1);
	m_avBillBoard[2]  = D3DVERTEX(D3DVECTOR( 0.5f, 0.5f, 0), vNorm, 1, 0);
	m_avBillBoard[3]  = D3DVERTEX(D3DVECTOR( 0.5f,-0.5f, 0), vNorm, 1, 1);

	ZeroMemory(m_stSelectChrInfo, _MAX_CHAR*sizeof(CHARINFO));
	ZeroMemory(m_stCreateChrInfo, _MAX_CHAR*sizeof(CHARINFO));
	ZeroMemory(m_stChrSprInfo   , _MAX_SPR_KIND*sizeof(CHARSPRINFO));
	m_bChrProcState = _CHR_PROC_SELECT;

	m_bBGMPlay     = FALSE;
	m_bBGMPlayTime = 0;			
	
	m_nSelectedChr = _SELECTED_NONE;
	SetRect(&m_rcSelectChrInfo, 0, 0, 0, 0);
	ZeroMemory(&m_stSelShowInfo, sizeof(SELCHRSHOWINFO));
	
	m_nCreatedChr  = _SELECTED_FST_CHR;
	m_nDividedExplain = 0;
	SetRect(&m_rcChrExplain, 0, 0, 0, 0);
	ZeroMemory(&m_pszChrExplain, 15*MAX_PATH);

	m_bFrontSetofSelChar = TRUE;
	m_bMoreTwoChar = FALSE;

	INT nCnt;

	for ( nCnt = 0; nCnt < _MAX_CHR_SELECT_BTN; nCnt++ )
	{
		m_xSelectBtn[nCnt].Init();
	}

	for ( nCnt = 0; nCnt < _CHR_CHR_CREATE_BTN; nCnt++ )
	{
		m_xCreateBtn[nCnt].Init();
	}

	m_dwWaitTick = 0;
}


/******************************************************************************************************************
�ʿ��� ����Ÿ�� �ʱ�ȭ�ϰ� �ε��Ѵ�.
Prcedure�� ���۵Ǳ����� ȣ��Ǿ�� �Ѵ�.
Load()->RenderScene()->SetNextProc()�� �����̴�.
*******************************************************************************************************************/
VOID CChrProcess::Load()
{
	Init();

	m_xImageInter.NewLoad(".\\Data\\gameinter.wil");
	m_xImageChr.NewLoad(".\\Data\\Interface1c.wil");

	// EditBox����(Password��������).
	ZeroMemory(g_xChatEditBox.m_szInputMsg, sizeof(g_xChatEditBox.m_szInputMsg));
	SetWindowText(g_xChatEditBox.GetSafehWnd(), "");
	SendMessage(g_xChatEditBox.GetSafehWnd(), EM_SETPASSWORDCHAR, NULL, 0);
	g_xChatEditBox.SetLimitText(13);
	g_xChatEditBox.SetEnterUse(FALSE);
	MoveWindow(g_xChatEditBox.GetSafehWnd(), g_xMainWnd.m_rcWindow.left+288, g_xMainWnd.m_rcWindow.top+405, 75, 13, TRUE);
	ShowWindow(g_xChatEditBox.GetSafehWnd(), SW_HIDE);

	if ( m_hBrush != NULL )
	{
		DeleteObject(m_hBrush);
		m_hBrush = NULL;
	}
	m_hBrush = CreateSolidBrush(RGB(0, 0, 0));

	g_xClientSocket.ConnectToServer(g_xMainWnd.GetSafehWnd(), g_szServerIP, g_nServerPort);

	m_xSelectBtn[_MAX_CHR_SELECT_CREATE].CreateGameBtn(&m_xImageChr, 51, 51, 440,  93, NULL, _BTN_TYPE_FOCUS, 52, TRUE);
	m_xSelectBtn[_MAX_CHR_SELECT_DELETE].CreateGameBtn(&m_xImageChr, 53, 53,  79, 243, NULL, _BTN_TYPE_FOCUS, 54, TRUE);
	m_xSelectBtn[_MAX_CHR_SELECT_START]. CreateGameBtn(&m_xImageChr, 55, 55, 259,  49, NULL, _BTN_TYPE_FOCUS, 56, TRUE);
	m_xSelectBtn[_MAX_CHR_SELECT_QUIT].  CreateGameBtn(&m_xImageChr, 57, 57,  28, 438, NULL, _BTN_TYPE_FOCUS, 58, TRUE);
	m_xSelectBtn[_MAX_CHR_SELECT_NEXT].  CreateGameBtn(&m_xImageInter, 1155, 1154, 550, 400, NULL, _BTN_TYPE_FOCUS, 1153, FALSE);
	m_xSelectBtn[_MAX_CHR_SELECT_BEFORE].CreateGameBtn(&m_xImageInter, 1152, 1152, 500, 400, NULL, _BTN_TYPE_FOCUS, 1152, FALSE);
	

	m_xCreateBtn[_CHR_CHR_CREATE_WARIOR].  CreateGameBtn(&m_xImageChr, 92, 93, 266, 419, NULL, _BTN_TYPE_FOCUS, 91, TRUE);
	m_xCreateBtn[_CHR_CHR_CREATE_MAGICIAN].CreateGameBtn(&m_xImageChr, 95, 96, 308, 419, NULL, _BTN_TYPE_FOCUS, 94, TRUE);
	m_xCreateBtn[_CHR_CHR_CREATE_CLERIC].  CreateGameBtn(&m_xImageChr, 98, 98, 352, 419, NULL, _BTN_TYPE_FOCUS, 97, TRUE);
	m_xCreateBtn[_CHR_CHR_CREATE_MAKE].    CreateGameBtn(&m_xImageChr, 86, 87, 450, 444, NULL, _BTN_TYPE_FOCUS, 85, TRUE);
	m_xCreateBtn[_CHR_CHR_CREATE_CANCEL].  CreateGameBtn(&m_xImageChr, 89, 90, 491, 444, NULL, _BTN_TYPE_FOCUS, 88, TRUE);
	m_xCreateBtn[_CHR_CHR_CREATE_ARCHER].  CreateGameBtn(&m_xImageChr, 101, 102, 390, 419, NULL, _BTN_TYPE_FOCUS, 100, TRUE);

	SetChrSelSprite();

	m_bBGMPlay	   = TRUE;
	m_bBGMPlayTime = 0;

	SndObjDestroy(m_stEffctWav[0]);
	SndObjDestroy(m_stEffctWav[1]);
	SndObjDestroy(m_stEffctWav[2]);
	m_stEffctWav[0] = SndObjCreate(g_SoundManager.mSound_lpds, ".\\Sound\\CreateChr.wav", 10);
	m_stEffctWav[1] = SndObjCreate(g_SoundManager.mSound_lpds, ".\\Sound\\SelChr.wav"   , 10);
	m_stEffctWav[2] = SndObjCreate(g_SoundManager.mSound_lpds, ".\\Sound\\StartGame.wav", 10);

	SndObjDestroy(m_stCreateWav[0][0]);
	SndObjDestroy(m_stCreateWav[0][1]);
	SndObjDestroy(m_stCreateWav[0][2]);
	SndObjDestroy(m_stCreateWav[1][0]);
	SndObjDestroy(m_stCreateWav[1][1]);
	SndObjDestroy(m_stCreateWav[1][2]);
	m_stCreateWav[0][0] = SndObjCreate(g_SoundManager.mSound_lpds, ".\\Sound\\JMCre.wav", 10);
	m_stCreateWav[0][1] = SndObjCreate(g_SoundManager.mSound_lpds, ".\\Sound\\SMCre.wav", 10);
	m_stCreateWav[0][2] = SndObjCreate(g_SoundManager.mSound_lpds, ".\\Sound\\DMCre.wav", 10);
	m_stCreateWav[1][0] = SndObjCreate(g_SoundManager.mSound_lpds, ".\\Sound\\JWCre.wav", 10);
	m_stCreateWav[1][1] = SndObjCreate(g_SoundManager.mSound_lpds, ".\\Sound\\SWCre.wav", 10);
	m_stCreateWav[1][2] = SndObjCreate(g_SoundManager.mSound_lpds, ".\\Sound\\DWCre.wav", 10);

	// �迭�� ���� -> ����, ����.
	SndObjDestroy(m_stSelectWav[0][0]);
	SndObjDestroy(m_stSelectWav[0][1]);
	SndObjDestroy(m_stSelectWav[0][2]);
	SndObjDestroy(m_stSelectWav[1][0]);
	SndObjDestroy(m_stSelectWav[1][1]);
	SndObjDestroy(m_stSelectWav[1][2]);

	// ����, ����.
	m_stSelectWav[0][0] = SndObjCreate(g_SoundManager.mSound_lpds, ".\\Sound\\JMMSel.wav", 10);
	// ����, ����.
	m_stSelectWav[1][0] = SndObjCreate(g_SoundManager.mSound_lpds, ".\\Sound\\JWMSel.wav", 10);
	// ����, ����.
	m_stSelectWav[0][1] = SndObjCreate(g_SoundManager.mSound_lpds, ".\\Sound\\SMMSel.wav", 10);
	// ����, ����.
	m_stSelectWav[1][1] = SndObjCreate(g_SoundManager.mSound_lpds, ".\\Sound\\SWMSel.wav", 10);
	// ����, ����.
	m_stSelectWav[0][2] = SndObjCreate(g_SoundManager.mSound_lpds, ".\\Sound\\DMMSel.wav", 10);
	// ����, ����.
	m_stSelectWav[1][2] = SndObjCreate(g_SoundManager.mSound_lpds, ".\\Sound\\DWMSel.wav", 10);

#ifdef _NEWINTERFACE
	m_xMsgBox.CreateMsgBox(&m_xImageInter, 0, _NONE, g_xMsg.GetMsg(200), FALSE, 155, 150);
#else //_NEWINTERFACE
	m_xMsgBox.CreateMsgBox(&m_xImageInter, 0, _NONE, g_xMsg.GetMsg(200), FALSE, 140, 110);
#endif //_NEWINTERFACE 
	m_dwWaitTick = timeGetTime();
}


/******************************************************************************************************************
�Ҹ��� �ʿ��� ��ü�� ����.
Procedure�� ����ǰų�? �����¿��� ���α׷��� �����ų��?ȣ��ȴ�?
*******************************************************************************************************************/
VOID CChrProcess::DeleteProc()
{
	m_xImageInter.Destroy();
	m_xImageChr.Destroy();

	g_xClientSocket.DisconnectToServer();

	if ( m_hBrush != NULL )
	{
		DeleteObject(m_hBrush);
		m_hBrush = NULL;
	}

	m_xAvi.Release();

	// Edit Box������ �������?�ǵ�����.
	g_xChatEditBox.SetEnterUse(TRUE);
}


/******************************************************************************************************************
���� Procedure�� ����.
ȣ��Ǹ�?�����������ʹ� Procedure�� �ٲ��?
*******************************************************************************************************************/
VOID CChrProcess::SetNextProc()
{
	DeleteProc();
 	g_xClientSocket.m_pxDefProc = g_xMainWnd.m_pxDefProcess = &g_xGameProc;
	g_OriginWndProc = GetWindowLong(g_xMainWnd.GetSafehWnd(), GWL_WNDPROC);
	g_xGameProc.Load();
	g_bProcState = _GAME_PROC;
	g_xGameProc.m_dwScreenModeTimer = g_dwCurrTime;
}


//-----------------------------------------------------------------------------------------------------------------
// ȭ�� Render���� �Լ�.
//-----------------------------------------------------------------------------------------------------------------
/******************************************************************************************************************
3D ����ǿ�?�ؽ��縦 �÷��� �׸���.
lpDevice   : �ʱ� DX3D���ý� ���� 3D��ġDevice.
pvTrans    : ������ǥ�� ������ ������ ������.(�ȼ���ǥ)
pvScale    : �̹����� ũ�⸦ ������ ������ ������.
pmtrl	   : �����?Matrial�� ������.
lpddsTextr : �ؽ����?���� Surface.
bBlendType : ȭ�鿡 ������ ����.
*******************************************************************************************************************/
HRESULT CChrProcess::DrawBillBoard(LPDIRECT3DDEVICE9 lpDevice, D3DVECTOR* pvTrans, D3DVECTOR* pvScale, /*D3DVECTOR vRot, */D3DMATERIAL9* pmtrl, /*FLOAT fRotRad, */IDirect3DTexture9* lpddsTextr, BYTE bBlendType)
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
ĳ������ ȿ��.
*******************************************************************************************************************/
VOID CChrProcess::DrawCharEffect(INT nIdx, INT nX, INT nY)
{
	// [DX9][B] 魔法特效修复: 原用 D3D 直绘(BeginScene/DrawPrimitiveUP �?swapchain),
	// �?Present 只上传软缓冲(m_pSoftBackBuffer->纹理->D3DXSprite->Present), 直绘特效被软缓冲覆盖不可见�?
	// 改为把特效帧半透明混合到软缓冲, Present 后特效可见�?
	if ( m_xImageChr.NewSetIndex(nIdx) )
	{
		// [DX9][AB01] 位置+黑影一次性修�?
		//   1) 补加特效�?shPX/shPY (原漏加致法师 frm900 shPY=-53 画低53px 压到脚部影子=黑框; 原版位置数学 nX+shWidth/2+shPX).
		//   2) 魔法火焰�?frm900-911)绝大多数像素是黑�?帧内部背�? frm900 96%�? frm910 �?363). 原版 D3D �?
		//      D3DTEXTR_TRANSPARENTBLACK(�?全透明)+_BLEND_LIGHT(加色混合) 使黑色帧背景不可见、只显橙�?
		//      本端�?alpha 混合(bOpa=50) 把黑像素画成黑色 �?黑色放射状黑�? 改为软件加色(additive)混合:
		//      黑像�?0x0000)�?=不变=透明, 亮像素加�? 完全复刻原版. select/create 共用本函�?
		INT nw = m_xImageChr.m_lpstNewCurrWilImageInfo->shWidth;
		INT nh = m_xImageChr.m_lpstNewCurrWilImageInfo->shHeight;
		INT nEx = nX + m_xImageChr.m_lpstNewCurrWilImageInfo->shPX;
		INT nEy = nY + m_xImageChr.m_lpstNewCurrWilImageInfo->shPY;

		CDX9SoftSurface* pBB = g_xMainWnd.GetBackBuffer();
		if ( pBB == NULL || pBB->m_pBits == NULL ) return;
		WORD* pDstBase = pBB->m_pBits;
		LONG lPitch = pBB->m_lPitch >> 1;		// WORDS per row
		LONG bw = pBB->m_dwWidth, bh = pBB->m_dwHeight;

		WORD* pwSrc = (WORD*)m_xImageChr.m_pbCurrImage;
		INT pos = 0;
		for ( INT row = 0; row < nh; row++ )
		{
			INT rowcnt = (INT)pwSrc[pos++];
			INT rowend = pos + rowcnt;
			INT col = 0;
			while ( pos < rowend )
			{
				WORD code = pwSrc[pos++];
				if ( code == 0xC0 )
				{
					INT cnt = (INT)pwSrc[pos++]; col += cnt;	// transparent skip
				}
				else if ( code == 0xC1 || code == 0xC2 || code == 0xC3 )
				{
					INT cnt = (INT)pwSrc[pos++];
					for ( INT k = 0; k < cnt; k++ )
					{
						WORD px = pwSrc[pos + k];
						INT dx = nEx + col, dy = nEy + row;
						if ( dx >= 0 && dx < bw && dy >= 0 && dy < bh )
						{
							WORD* pDst = pDstBase + (LONG)dy * lPitch + dx;
							INT dr = ((*pDst) >> 11) & 0x1F, dg = ((*pDst) >> 5) & 0x3F, db = (*pDst) & 0x1F;
							dr += (px >> 11) & 0x1F; if ( dr > 0x1F ) dr = 0x1F;
							dg += (px >> 5) & 0x3F;  if ( dg > 0x3F ) dg = 0x3F;
							db += px & 0x1F;         if ( db > 0x1F ) db = 0x1F;
							*pDst = (WORD)((dr << 11) | (dg << 5) | db);
						}
						col++;
					}
					pos += cnt;
				}
			}
		}
	}
}


/******************************************************************************************************************
Procedure�� ���η���.
*******************************************************************************************************************/
VOID CChrProcess::RenderScene(INT nLoopTime)
{	
	Clear(RGB(0, 0, 0));

	switch ( m_bChrProcState )
	{
		case _CHR_PROC_SELECT:
		{
			RenderSelect(nLoopTime);
			m_xMsgBox.RenderMsgBox(m_ptMousePos);
			break;
		}
		case _CHR_PROC_CREATE:
		{
			RenderCreate(nLoopTime);
			m_xMsgBox.RenderMsgBox(m_ptMousePos);
			break;
		}
		case _CHR_PROC_CREATEIN:
		{
			if ( TRUE )
			{		
				Sleep(100);
				m_xAvi.Release();
				m_bChrProcState = _CHR_PROC_CREATE;

				m_bBGMPlay	   = TRUE;
				m_bBGMPlayTime = 0;

				// ���������� ����â �ε�.
				SetCharExplain(_GENDER_MAN, _JOB_JUNSA);

//				g_SoundManager.SndmngrPlayMp3(".\\Sound\\CreateChr.mp3", TRUE);

				SetWindowText(g_xChatEditBox.GetSafehWnd(), "");
				SetFocus(g_xChatEditBox.GetSafehWnd());
				MoveWindow(g_xChatEditBox.GetSafehWnd(), g_xMainWnd.m_rcWindow.left+288, g_xMainWnd.m_rcWindow.top+405, 75, 13, TRUE);
				ShowWindow(g_xChatEditBox.GetSafehWnd(), SW_SHOW);

				SetChrInfo(&m_stCreateChrInfo[0], _GENDER_MAN, _JOB_JUNSA, 0);
				SetMotion(0, _CHR_MT_CC);

				SetChrInfo(&m_stCreateChrInfo[1], _GENDER_WOMAN, _JOB_JUNSA, 0);
				SetMotion(1, _CHR_MT_CC);
			}
			break;
		}
		case _CHR_PROC_CREATEOUT:
		{
			if ( TRUE )
			{		
//				m_xAvi.Release();
				m_bChrProcState = _CHR_PROC_SELECT;

				m_bBGMPlay	   = TRUE;
				m_bBGMPlayTime = 0;

				if ( m_nSelectedChr != _SELECTED_NONE )
				{
					SetMotion(m_nSelectedChr, _CHR_MT_SM);
				}
			}
			break;
		}
		case _CHR_PROC_START:
		{
			if ( TRUE )
			{		
				SetNextProc();
			}
			break;
		}
	}
}


/******************************************************************************************************************
Character Select �κ��� ���η���.
*******************************************************************************************************************/
VOID CChrProcess::RenderSelect(INT nLoopTime)
{
	INT nCnt;

	// 1���Ŀ� BGM Play.
	if ( m_bBGMPlay )
	{
		m_bBGMPlayTime += nLoopTime;

		if ( m_bBGMPlayTime > 1000 )
		{
			g_SoundManager.SndmngrPlayMp3(".\\Sound\\SelChr.mp3", TRUE);
			m_bBGMPlay     = FALSE;
			m_bBGMPlayTime = 0;
		}
	}

	// ���?
	if ( m_xImageChr.NewSetIndex(_CHR_PROC_IMG_SELECT) )
	{
		// [DX9][T22] 背景 640x480 等比缩放铺满当前显示尺寸�?40x480 退�?1:1 / 800x600 铺满�?
		g_xMainWnd.DrawWithImageForCompScaled(0, 0, m_xImageChr.m_lpstNewCurrWilImageInfo->shWidth, m_xImageChr.m_lpstNewCurrWilImageInfo->shHeight,
										(WORD*)(m_xImageChr.m_pbCurrImage),
										g_xMainWnd.m_stDisplayInfo.wWidth, g_xMainWnd.m_stDisplayInfo.wHeight);
	}

	int	nSelectIdx;
	// ĳ����.
	for ( nCnt=0 ; nCnt < _MAX_SHOW_CHAR; nCnt++ )
	{

		if (m_bFrontSetofSelChar == FALSE)
			nSelectIdx = nCnt + 2;
		else
			nSelectIdx = nCnt;



		if ( m_stSelectChrInfo[nSelectIdx].bSetted )
		{
			LPCHARINFO		pstChrInfo = &m_stSelectChrInfo[nSelectIdx];
			LPCHARSPRINFO	pstSprInfo = m_stSelectChrInfo[nSelectIdx].stCurrFrmInfo.pstCurrSprInfo;

			if ( pstSprInfo )
			{
				pstChrInfo->stCurrFrmInfo.wCurrDelay += nLoopTime;

				if ( pstChrInfo->stCurrFrmInfo.wCurrDelay > pstSprInfo->wDelay )
				{
					pstChrInfo->stCurrFrmInfo.wCurrDelay = 0;
					pstChrInfo->stCurrFrmInfo.wCurrFrm++;

					// ���� �÷���.
					if ( pstChrInfo->stCurrFrmInfo.wCurrFrm ==  pstSprInfo->wFstFrm+1 )
					{
						if ( pstChrInfo->stCurrFrmInfo.wCurrMtn == _CHR_MT_SM )
						{
							SndObjPlay(0, 0, m_stSelectWav[pstChrInfo->bGender][pstChrInfo->bClass], 0);
						}
					}

					SetCharRect(nSelectIdx, pstChrInfo->stCurrFrmInfo.wCurrMtn);

					if ( pstChrInfo->stCurrFrmInfo.wCurrFrm > pstSprInfo->wEndFrm )
					{
						// �ִϸ��̼��� ������ �ٸ����۰� ����Ǵ�?���?�ִ�.
						if ( pstChrInfo->stCurrFrmInfo.wCurrMtn == _CHR_MT_SM )
						{
							SetMotion(nSelectIdx, _CHR_MT_SS);
						}
						else if ( pstChrInfo->stCurrFrmInfo.wCurrMtn == _CHR_MT_R )
						{
							SetMotion(nSelectIdx, _CHR_MT_NS);
						}
						else
						{
							pstChrInfo->stCurrFrmInfo.wCurrFrm = pstSprInfo->wFstFrm;
						}
					}
				}

				INT nPosX, nPosY;

				// �׸���.
				if ( m_xImageChr.NewSetIndex(pstChrInfo->stCurrFrmInfo.wCurrFrm+20) )
				{
					nPosX = pstChrInfo->stCurrFrmInfo.nPosX+m_xImageChr.m_lpstNewCurrWilImageInfo->shPX;
					nPosY = pstChrInfo->stCurrFrmInfo.nPosY+m_xImageChr.m_lpstNewCurrWilImageInfo->shPY;
					g_xMainWnd.DrawWithABlendCompDataWithBackBuffer(nPosX, nPosY, 
																	m_xImageChr.m_lpstNewCurrWilImageInfo->shWidth, m_xImageChr.m_lpstNewCurrWilImageInfo->shHeight,
																	(WORD*)(m_xImageChr.m_pbCurrImage), 640, 480);
				}

				// ĳ����.
				if ( m_xImageChr.NewSetIndex(pstChrInfo->stCurrFrmInfo.wCurrFrm) )
				{
					nPosX = pstChrInfo->stCurrFrmInfo.nPosX+m_xImageChr.m_lpstNewCurrWilImageInfo->shPX;
					nPosY = pstChrInfo->stCurrFrmInfo.nPosY+m_xImageChr.m_lpstNewCurrWilImageInfo->shPY;

					if ( pstChrInfo->stCurrFrmInfo.bBlend )
					{
						g_xMainWnd.DrawWithImageForComp(nPosX, nPosY, 
														m_xImageChr.m_lpstNewCurrWilImageInfo->shWidth, m_xImageChr.m_lpstNewCurrWilImageInfo->shHeight,
														(WORD*)(m_xImageChr.m_pbCurrImage));
					}
					else
					{
						g_xMainWnd.DrawWithImageForCompClipRgnColor(nPosX, nPosY, m_xImageChr.m_lpstNewCurrWilImageInfo->shWidth, m_xImageChr.m_lpstNewCurrWilImageInfo->shHeight,
																	(WORD*)(m_xImageChr.m_pbCurrImage), 640, 480, _STATE_GRAY, FALSE, FALSE);
					}
				}

				// ĳ���� ȿ��.
				if ( m_xImageChr.NewSetIndex(pstChrInfo->stCurrFrmInfo.wCurrFrm+40) )
				{
					nPosX = pstChrInfo->stCurrFrmInfo.nPosX;
					nPosY = pstChrInfo->stCurrFrmInfo.nPosY;
					DrawCharEffect(pstChrInfo->stCurrFrmInfo.wCurrFrm+40, nPosX, nPosY);
				}
			}
		}
	}

	for ( nCnt = 0; nCnt < _MAX_CHR_SELECT_BTN; nCnt++ )
	{
		if (m_bMoreTwoChar == FALSE)
		{
			if ( nCnt == _MAX_CHR_SELECT_BEFORE || nCnt == _MAX_CHR_SELECT_NEXT )
				continue;
/*			if (m_bFrontSetofSelChar == TRUE)
			{
				if (nCnt == _MAX_CHR_SELECT_NEXT)
				{
					m_xSelectBtn[nCnt].ShowGameBtn();
				}
			}
			else
			{
				if (nCnt == _MAX_CHR_SELECT_BEFORE)
				{
					m_xSelectBtn[nCnt].ShowGameBtn();
				}
			}
*/
		}


//		if ( nCnt != _MAX_CHR_SELECT_BEFORE && nCnt != _MAX_CHR_SELECT_NEXT )
			m_xSelectBtn[nCnt].ShowGameBtn();

		// ��ư�̹���(Ŭ��)�� ���°����?..
		RECT* prcBtn = &m_xSelectBtn[nCnt].m_rcBtn;
		if ( PtInRect(prcBtn, m_ptMousePos) )
		{
			if ( nCnt == _MAX_CHR_SELECT_BEFORE || nCnt == _MAX_CHR_SELECT_NEXT )
				continue;

			D3DVECTOR	 vecTrans((FLOAT)prcBtn->left, (FLOAT)prcBtn->top, 0);
			D3DVECTOR	 vecScale((FLOAT)prcBtn->right-prcBtn->left, (FLOAT)prcBtn->bottom-prcBtn->top, 1);

			D3DMATERIAL9 mtrl;
			D3DUtil_InitMaterial(mtrl, (FLOAT)150/255.0f, (FLOAT)100/255.0f, (FLOAT)50/255.0f);
			mtrl.Diffuse.a = 50.0f/255.0f;
			DrawBillBoard(g_xMainWnd.Get3DDevice(), &vecTrans, &vecScale, &mtrl, NULL);

			g_xMainWnd.DrawWithGDI(prcBtn, NULL, RGB(200, 150, 100), 1);
		}
	}

	ShowSelectedChrInfo();
}


/******************************************************************************************************************
Character Create �κ��� ���η���.
*******************************************************************************************************************/
VOID CChrProcess::RenderCreate(INT nLoopTime)
{
	INT nCnt;

	// 1���Ŀ� BGM Play.
	if ( m_bBGMPlay )
	{
		m_bBGMPlayTime += nLoopTime;

		if ( m_bBGMPlayTime > 1000 )
		{
			g_SoundManager.SndmngrPlayMp3(".\\Sound\\CreateChr.mp3", TRUE);
			m_bBGMPlay	   = FALSE;
			m_bBGMPlayTime = 0;
		}
	}

	// ���?
	if ( m_xImageChr.NewSetIndex(_CHR_PROC_IMG_CREATE) )
	{
		// [DX9][T22] 建角色背�?640x480 等比缩放铺满当前显示尺寸
		g_xMainWnd.DrawWithImageForCompScaled(0, 0, m_xImageChr.m_lpstNewCurrWilImageInfo->shWidth, m_xImageChr.m_lpstNewCurrWilImageInfo->shHeight,
										(WORD*)(m_xImageChr.m_pbCurrImage),
										g_xMainWnd.m_stDisplayInfo.wWidth, g_xMainWnd.m_stDisplayInfo.wHeight);
	}

	ShowCharExplain();

	// ĳ����.
	for ( nCnt = 0; nCnt < _MAX_CHAR; nCnt++ )
	{
		if ( m_stCreateChrInfo[nCnt].bSetted )
		{
			LPCHARINFO		pstChrInfo = &m_stCreateChrInfo[nCnt];
			LPCHARSPRINFO	pstSprInfo = m_stCreateChrInfo[nCnt].stCurrFrmInfo.pstCurrSprInfo;

			if ( pstSprInfo )
			{
				pstChrInfo->stCurrFrmInfo.wCurrDelay += nLoopTime;

				if ( pstChrInfo->stCurrFrmInfo.wCurrDelay > pstSprInfo->wDelay )
				{
					if ( nCnt == m_nCreatedChr )
					{
						if ( pstChrInfo->stCurrFrmInfo.wCurrFrm == pstSprInfo->wFstFrm )
						{
							SndObjPlay(0, 0, m_stCreateWav[pstChrInfo->bGender][pstChrInfo->bClass], 0);
						}
					}

					pstChrInfo->stCurrFrmInfo.wCurrDelay = 0;
					pstChrInfo->stCurrFrmInfo.wCurrFrm++;

					SetCharRect(nCnt, pstChrInfo->stCurrFrmInfo.wCurrMtn);

					if ( pstChrInfo->stCurrFrmInfo.wCurrFrm > pstSprInfo->wEndFrm )
					{
						pstChrInfo->stCurrFrmInfo.wCurrFrm = pstSprInfo->wFstFrm;
					}
				}

				INT nPosX, nPosY;

				// �׸���.
				if ( m_xImageChr.NewSetIndex(pstChrInfo->stCurrFrmInfo.wCurrFrm+20) )
				{
					nPosX = pstChrInfo->stCurrFrmInfo.nPosX+m_xImageChr.m_lpstNewCurrWilImageInfo->shPX;
					nPosY = pstChrInfo->stCurrFrmInfo.nPosY+m_xImageChr.m_lpstNewCurrWilImageInfo->shPY;
					g_xMainWnd.DrawWithABlendCompDataWithBackBuffer(nPosX, nPosY, 
																	m_xImageChr.m_lpstNewCurrWilImageInfo->shWidth, m_xImageChr.m_lpstNewCurrWilImageInfo->shHeight,
																	(WORD*)(m_xImageChr.m_pbCurrImage), 640, 480);
				}

				//ĳ����.
				if ( m_nCreatedChr == nCnt )
				{
					if ( m_xImageChr.NewSetIndex(pstChrInfo->stCurrFrmInfo.wCurrFrm) )
					{
						nPosX = pstChrInfo->stCurrFrmInfo.nPosX+m_xImageChr.m_lpstNewCurrWilImageInfo->shPX;
						nPosY = pstChrInfo->stCurrFrmInfo.nPosY+m_xImageChr.m_lpstNewCurrWilImageInfo->shPY;
						g_xMainWnd.DrawWithImageForComp(nPosX, nPosY, 
														m_xImageChr.m_lpstNewCurrWilImageInfo->shWidth, m_xImageChr.m_lpstNewCurrWilImageInfo->shHeight,
														(WORD*)(m_xImageChr.m_pbCurrImage));
					}
				}
				else
				{
					if ( m_xImageChr.NewSetIndex(pstChrInfo->stCurrFrmInfo.pstCurrSprInfo->wFstFrm) )
					{
						nPosX = pstChrInfo->stCurrFrmInfo.nPosX+m_xImageChr.m_lpstNewCurrWilImageInfo->shPX;
						nPosY = pstChrInfo->stCurrFrmInfo.nPosY+m_xImageChr.m_lpstNewCurrWilImageInfo->shPY;
						g_xMainWnd.DrawWithImageForCompClipRgnColor(nPosX, nPosY, m_xImageChr.m_lpstNewCurrWilImageInfo->shWidth, m_xImageChr.m_lpstNewCurrWilImageInfo->shHeight,
																	(WORD*)(m_xImageChr.m_pbCurrImage), 640, 480, _STATE_GRAY, FALSE, FALSE);
					}
				}

				// ĳ���� ȿ��.
				if ( m_nCreatedChr == nCnt )
				{
					if ( m_xImageChr.NewSetIndex(pstChrInfo->stCurrFrmInfo.wCurrFrm+40) )
					{
						nPosX = pstChrInfo->stCurrFrmInfo.nPosX;
						nPosY = pstChrInfo->stCurrFrmInfo.nPosY;
						DrawCharEffect(pstChrInfo->stCurrFrmInfo.wCurrFrm+40, nPosX, nPosY);
					}
				}
			}
		}
	}

	// ���׸���.
	if ( m_xImageChr.NewSetIndex(_CHR_PROC_IMG_CREATE_STONE_S) )
	{
		g_xMainWnd.DrawWithABlendCompDataWithBackBuffer(201, 434, 
													    m_xImageChr.m_lpstNewCurrWilImageInfo->shWidth, m_xImageChr.m_lpstNewCurrWilImageInfo->shHeight,
														(WORD*)(m_xImageChr.m_pbCurrImage), 640, 480);
	}
	// ��.
	if ( m_xImageChr.NewSetIndex(_CHR_PROC_IMG_CREATE_STONE) )
	{
		g_xMainWnd.DrawWithImageForComp(247, 384, m_xImageChr.m_lpstNewCurrWilImageInfo->shWidth, m_xImageChr.m_lpstNewCurrWilImageInfo->shHeight,
										(WORD*)(m_xImageChr.m_pbCurrImage));
	}

	for ( nCnt = 0; nCnt < _CHR_CHR_CREATE_BTN; nCnt++ )
	{
		m_xCreateBtn[nCnt].ShowGameBtn();
		m_xCreateBtn[_CHR_CHR_CREATE_WARIOR].ChangeRect(260, 420);
		m_xCreateBtn[_CHR_CHR_CREATE_MAGICIAN].ChangeRect(296, 420);
		m_xCreateBtn[_CHR_CHR_CREATE_CLERIC].ChangeRect(333, 420);
		m_xCreateBtn[_CHR_CHR_CREATE_ARCHER].ChangeRect(369, 420);
	}

	for ( nCnt = 0; nCnt < _CHR_CHR_CREATE_BTN; nCnt++ )
	{
		m_xCreateBtn[nCnt].ShowGameBtn();
	}

	// Tool Tip.
	RECT	rcTemp;

	for ( nCnt = 0; nCnt < _CHR_CHR_CREATE_BTN; nCnt++ )
	{
		RECT* prcBtn = &m_xCreateBtn[nCnt].m_rcBtn;
		if ( PtInRect(prcBtn, m_ptMousePos) )
		{
			CHAR	szTemp[MAX_PATH];
			POINT	ptShow;

			switch ( nCnt )
			{
			case 0:
				strcpy(szTemp, g_xMsg.GetMsg(201));
				ptShow.x = 289;
				ptShow.y = 438;
				break;
			case 1:
				strcpy(szTemp, g_xMsg.GetMsg(202));
				ptShow.x = 332;
				ptShow.y = 438;
				break;
			case 2:
				strcpy(szTemp, g_xMsg.GetMsg(203));
				ptShow.x = 376;
				ptShow.y = 438;
				break;
			case 3:
				strcpy(szTemp, g_xMsg.GetMsg(204));
				ptShow.x = 469;
				ptShow.y = 458;
				break;
			case 4:
				strcpy(szTemp, g_xMsg.GetMsg(205));
				ptShow.x = 510;
				ptShow.y = 458;
				break;
			}
			SetRect(&rcTemp, ptShow.x+10, ptShow.y+2, ptShow.x+10+strlen(szTemp)*6, ptShow.y+17);

			D3DVECTOR	 vecTrans((FLOAT)rcTemp.left, (FLOAT)rcTemp.top, 0);
			D3DVECTOR	 vecScale((FLOAT)rcTemp.right-rcTemp.left, (FLOAT)rcTemp.bottom-rcTemp.top, 1);

			D3DMATERIAL9 mtrl;
			D3DUtil_InitMaterial(mtrl, (FLOAT)125/255.0f, (FLOAT)125/255.0f, (FLOAT)75/255.0f);
			mtrl.Diffuse.a = 50.0f/255.0f;
			DrawBillBoard(g_xMainWnd.Get3DDevice(), &vecTrans, &vecScale, &mtrl, NULL);

			g_xMainWnd.DrawWithGDI(&rcTemp, NULL, RGB(180, 180, 120), 1);

			g_xMainWnd.PutsHan(NULL,rcTemp.left+1, rcTemp.top+1, RGB( 25,  25,  25),RGB(0, 0, 0), szTemp);
			g_xMainWnd.PutsHan(NULL,rcTemp.left+3, rcTemp.top+1, RGB( 25,  25,  25),RGB(0, 0, 0), szTemp);
			g_xMainWnd.PutsHan(NULL,rcTemp.left+1, rcTemp.top+3, RGB( 25,  25,  25),RGB(0, 0, 0), szTemp);
			g_xMainWnd.PutsHan(NULL,rcTemp.left+3, rcTemp.top+3, RGB( 25,  25,  25),RGB(0, 0, 0), szTemp);
			g_xMainWnd.PutsHan(NULL,rcTemp.left+2, rcTemp.top+2, RGB(255, 255, 180),RGB(0, 0, 0), szTemp);
		}
	}

	SetRect(&rcTemp, 287, 404, 364, 419);
	g_xMainWnd.DrawWithGDI(&rcTemp, NULL, RGB(120, 90, 60), 1);
}










//-----------------------------------------------------------------------------------------------------------------
// Character Select�κ�.
//-----------------------------------------------------------------------------------------------------------------
/******************************************************************************************************************
���õ� Character�� ������ ���?
*******************************************************************************************************************/
VOID CChrProcess::ShowSelectedChrInfo()
{
	if ( m_nSelectedChr != _SELECTED_NONE && m_nSelectedChr < _MAX_CHAR )
	{
		D3DVECTOR vecTrans((FLOAT)m_rcSelectChrInfo.left, (FLOAT)m_rcSelectChrInfo.top, 0);
		D3DVECTOR vecScale((FLOAT)m_rcSelectChrInfo.right-m_rcSelectChrInfo.left, (FLOAT)m_rcSelectChrInfo.bottom-m_rcSelectChrInfo.top, 1);

		D3DMATERIAL9 mtrl;
		D3DUtil_InitMaterial(mtrl, (FLOAT)50/255.0f, (FLOAT)50/255.0f, (FLOAT)25/255.0f);
		mtrl.Diffuse.a = 100.0f/255.0f;
		DrawBillBoard(g_xMainWnd.Get3DDevice(), &vecTrans, &vecScale, &mtrl, NULL);
		g_xMainWnd.DrawWithGDI(&m_rcSelectChrInfo, NULL, RGB(100, 100, 80), 1);

		g_xMainWnd.PutsHan(NULL, m_rcSelectChrInfo.left+10, m_rcSelectChrInfo.top+10, RGB(255, 200, 150), RGB(0, 0, 0), m_stSelShowInfo.pszName , g_xMainWnd.CreateGameFont(g_xMsg.GetMsg(1074), 9, 0, FW_BOLD));
		g_xMainWnd.PutsHan(NULL, m_rcSelectChrInfo.left+10, m_rcSelectChrInfo.top+30, RGB(150, 240, 150), RGB(0, 0, 0), m_stSelShowInfo.pszLevel, g_xMainWnd.CreateGameFont(g_xMsg.GetMsg(1074), 9, 0, FW_BOLD));
		g_xMainWnd.PutsHan(NULL, m_rcSelectChrInfo.left+10, m_rcSelectChrInfo.top+50, RGB(255, 255, 175), RGB(0, 0, 0), m_stSelShowInfo.pszJob  , g_xMainWnd.CreateGameFont(g_xMsg.GetMsg(1074), 9, 0, FW_BOLD));

		if ( m_stSelectChrInfo[m_nSelectedChr].bUpgradeLevel == 1 )
		{
			g_xMainWnd.PutsHan(NULL, m_rcSelectChrInfo.left+10, m_rcSelectChrInfo.top+70, RGB(255, 255, 175), RGB(0, 0, 0), m_stSelShowInfo.pszHighLevel  , g_xMainWnd.CreateGameFont(g_xMsg.GetMsg(1074), 9, 0, FW_BOLD));
		}
	}
}


/******************************************************************************************************************
���õ� Character�� ������ �Է�.
*******************************************************************************************************************/
VOID CChrProcess::SetSelectChrShowInfo()
{
	SIZE	sizeLen1;
	SIZE	sizeLen2;
	POINT	ptPos = {79, 108};
	INT		nHeight = 0;
	
	if ( m_nSelectedChr != _SELECTED_NONE && m_nSelectedChr < _MAX_CHAR )
	{
		sprintf(m_stSelShowInfo.pszName, "%s   %s", g_xMsg.GetMsg(206), m_stSelectChrInfo[m_nSelectedChr].pszChrName);  //lee wun gyo
		sizeLen1 = g_xMainWnd.GetStrLength(NULL, g_xMainWnd.CreateGameFont(g_xMsg.GetMsg(1074), 9, 0, FW_BOLD), m_stSelShowInfo.pszName);

		if ( m_stSelectChrInfo[m_nSelectedChr].bUpgradeLevel == 1 )
			SetRect(&m_rcSelectChrInfo, ptPos.x, ptPos.y, ptPos.x+sizeLen1.cx+20, ptPos.y+20*4+10);
		else
			SetRect(&m_rcSelectChrInfo, ptPos.x, ptPos.y, ptPos.x+sizeLen1.cx+20, ptPos.y+20*3+10);

		sprintf(m_stSelShowInfo.pszLevel, "%s   %d", g_xMsg.GetMsg(207), m_stSelectChrInfo[m_nSelectedChr].bLevel);
		
		switch ( m_stSelectChrInfo[m_nSelectedChr].bClass )
		{
			case _JOB_JUNSA:
				sprintf(m_stSelShowInfo.pszJob, g_xMsg.GetMsg(208));
				break;
			case _JOB_SULSA:
				sprintf(m_stSelShowInfo.pszJob, g_xMsg.GetMsg(209));
				break;
			case _JOB_DOSA:
				sprintf(m_stSelShowInfo.pszJob, g_xMsg.GetMsg(210));
				break;
		}

		if ( m_stSelectChrInfo[m_nSelectedChr].bUpgradeLevel == 1 )
		{
			sprintf(m_stSelShowInfo.pszHighLevel, g_xMsg.GetMsg(238));
			nHeight = ptPos.y+20*4+10;
		}
		else
		{
			nHeight = ptPos.y+20*3+10;
		}

		sizeLen2 = g_xMainWnd.GetStrLength(NULL, g_xMainWnd.CreateGameFont(g_xMsg.GetMsg(1074), 9, 0, FW_BOLD), m_stSelShowInfo.pszJob);

		if ( sizeLen2.cx > sizeLen1.cx )
		{
			SetRect(&m_rcSelectChrInfo, ptPos.x, ptPos.y, ptPos.x+sizeLen2.cx+20, nHeight);
		}
	}
}


/******************************************************************************************************************
CHARINFO ����ü�� ������ ����.
*******************************************************************************************************************/
VOID CChrProcess::SetChrInfo(LPCHARINFO pstChrInfo, BYTE bGender, BYTE bJob, BYTE bLevel, CHAR* pszName)
{
	pstChrInfo->bSetted = TRUE;
	pstChrInfo->bGender = bGender;
	pstChrInfo->bClass  = bJob;
	pstChrInfo->bLevel  = bLevel;

	POINT ptPos = GetChrCreatePos(bGender, bJob);
	pstChrInfo->stCurrFrmInfo.nPosX = ptPos.x;
	pstChrInfo->stCurrFrmInfo.nPosY = ptPos.y;

	ZeroMemory(pstChrInfo->pszChrName, 25);

	if ( pszName )
	{
		strcpy(pstChrInfo->pszChrName, pszName);
	}
}









//-----------------------------------------------------------------------------------------------------------------
// Character Create�κ�.
//-----------------------------------------------------------------------------------------------------------------
/******************************************************************************************************************
���õ� ������ ���� ���������� ���´�.
*******************************************************************************************************************/
BOOL CChrProcess::SetCharExplain(BYTE bGender, BYTE bJob)
{
	if ( (bGender >= 0 && bGender < 2) && (bJob >= 0 && bJob < 4) )
	{
		INT nCnt;
		CHAR pszJobExplain[1024];
		CHAR pszJobExplainEx[2048];

		ZeroMemory(pszJobExplain, 1024);
		ZeroMemory(pszJobExplainEx, 2048);
		ZeroMemory(&m_pszChrExplain, 15*MAX_PATH);

		switch ( bGender )
		{
			case _GENDER_MAN:
			{
				strcpy(m_pszChrExplain[0], g_xMsg.GetMsg(211));
				break;
			}
			case _GENDER_WOMAN:
			{
				strcpy(m_pszChrExplain[0], g_xMsg.GetMsg(212));
				break;
			}			
		}

		switch ( bJob )
		{
			case _JOB_JUNSA:
			{
				strcat(m_pszChrExplain[0], g_xMsg.GetMsg(213));
				g_xMainWnd.StringDivide(_CHR_EXPLAIN_WIDTH, m_nDividedExplain, g_xMsg.GetMsg(216+bJob), pszJobExplainEx);
				break;
			}
			case _JOB_SULSA:
			{
				strcat(m_pszChrExplain[0], g_xMsg.GetMsg(214));
				g_xMainWnd.StringDivide(_CHR_EXPLAIN_WIDTH, m_nDividedExplain, g_xMsg.GetMsg(216+bJob), pszJobExplainEx);
				break;
			}
			case _JOB_DOSA:
			{
				strcat(m_pszChrExplain[0], g_xMsg.GetMsg(215));
				g_xMainWnd.StringDivide(_CHR_EXPLAIN_WIDTH, m_nDividedExplain, g_xMsg.GetMsg(216+bJob), pszJobExplainEx);
				break;
			}
			case _JOB_GUNGSA:
			{
				strcpy(m_pszChrExplain[0], g_xMsg.GetMsg(237));
				g_xMainWnd.StringDivide(_CHR_EXPLAIN_WIDTH, m_nDividedExplain, g_xMsg.GetMsg(236), pszJobExplainEx);
				break;
			}
		}

		

		// ù�ٱ��� �����ش�.
		m_nDividedExplain += 1;
		
		sscanf(pszJobExplainEx, "%[^`]%*c %[^`]%*c %[^`]%*c %[^`]%*c %[^`]%*c %[^`]%*c %[^`]%*c %[^`]%*c %[^`]%*c %[^`]%*c %[^`]%*c %[^`]%*c %[^`]%*c %[^`]%*c %[^`]%*c", 
			   m_pszChrExplain[1], m_pszChrExplain[ 2], m_pszChrExplain[ 3], m_pszChrExplain[ 4], m_pszChrExplain[ 5], m_pszChrExplain[ 6], m_pszChrExplain[ 7], m_pszChrExplain[ 8], 
			   m_pszChrExplain[9], m_pszChrExplain[10], m_pszChrExplain[11], m_pszChrExplain[12], m_pszChrExplain[13], m_pszChrExplain[14]);

		return TRUE;
	}
	return FALSE;
}


/******************************************************************************************************************
���õ� ������ ���� ���������� ȭ���?���?
*******************************************************************************************************************/
VOID CChrProcess::ShowCharExplain()
{
	if ( m_nCreatedChr != _SELECTED_NONE && m_nCreatedChr < _MAX_CHAR )
	{
		RECT rcShow = {100, 15, 100+_CHR_EXPLAIN_WIDTH+20, 15+m_nDividedExplain*18+20};

		D3DVECTOR vecTrans((FLOAT)rcShow.left, (FLOAT)rcShow.top, 0);
		D3DVECTOR vecScale((FLOAT)rcShow.right-rcShow.left, (FLOAT)rcShow.bottom-rcShow.top, 1);

		D3DMATERIAL9 mtrl;
		D3DUtil_InitMaterial(mtrl, (FLOAT)50/255.0f, (FLOAT)50/255.0f, (FLOAT)50/255.0f);
		mtrl.Diffuse.a = 50.0f/255.0f;
		DrawBillBoard(g_xMainWnd.Get3DDevice(), &vecTrans, &vecScale, &mtrl, NULL);
		g_xMainWnd.DrawWithGDI(&rcShow, NULL, RGB(150, 150, 150), 1);

		switch ( m_stCreateChrInfo[m_nSelectedChr].bClass )
		{
			case _JOB_JUNSA:
				g_xMainWnd.PutsHan(NULL, rcShow.left + 10, rcShow.top + 10, RGB(250, 200, 150), RGB(0,0,0), m_pszChrExplain[0], g_xMainWnd.CreateGameFont(g_xMsg.GetMsg(1074), 10, 0, FW_BOLD));
				break;
			case _JOB_SULSA:
				g_xMainWnd.PutsHan(NULL, rcShow.left + 10, rcShow.top + 10, RGB(250, 170, 170), RGB(0,0,0), m_pszChrExplain[0], g_xMainWnd.CreateGameFont(g_xMsg.GetMsg(1074), 10, 0, FW_BOLD));
				break;
			case _JOB_DOSA:
				g_xMainWnd.PutsHan(NULL, rcShow.left + 10, rcShow.top + 10, RGB(150, 220, 150), RGB(0,0,0), m_pszChrExplain[0], g_xMainWnd.CreateGameFont(g_xMsg.GetMsg(1074), 10, 0, FW_BOLD));
				break;
			case _JOB_GUNGSA:
				g_xMainWnd.PutsHan(NULL, rcShow.left + 10, rcShow.top + 10, RGB(150, 220, 250), RGB(0,0,0), m_pszChrExplain[0], g_xMainWnd.CreateGameFont(g_xMsg.GetMsg(1074), 10, 0, FW_BOLD));
				break;
		}
		
		for ( INT nCnt = 1; nCnt < m_nDividedExplain; nCnt++ )
		{
			g_xMainWnd.PutsHan(NULL, rcShow.left+10, rcShow.top+35+18*(nCnt-1), RGB(250, 250, 255), RGB(0, 0, 0), m_pszChrExplain[nCnt]);
		}
	}
}


/******************************************************************************************************************
ĳ������ ȭ�����?��ġ�� ����.(�̹��������� �߽����� ���� ��ġ���� �ʱ� ����.)
*******************************************************************************************************************/
POINT CChrProcess::GetChrCreatePos(BYTE bGender, BYTE bJob)
{
	POINT ptRet		  = { 110, 100 };
	POINT ptPos[2][4] = { { {110, 110}, {110, 120}, {110, 120}, {110, 120} }, { {400, 160}, {420, 115}, {425, 118}, {420, 118} } };

	if ( (bGender >= 0 && bGender < 2) && (bJob >= 0 && bJob < 4) )
	{
		ptRet = ptPos[bGender][bJob];
	}

	return ptRet;
}


/******************************************************************************************************************
������ų Character�� �̸��˻�(����).
*******************************************************************************************************************/
BOOL CChrProcess::IsAlphabet(CHAR cChar)
{
	if ( ( (cChar >= 'a') && (cChar <= 'z') ) || ( (cChar >= 'A') && (cChar <= 'Z') ) )
	{
		return TRUE;
	}

	return FALSE;
}


/******************************************************************************************************************
������ų Character�� �̸��˻�(����).
*******************************************************************************************************************/
BOOL CChrProcess::IsNumeric(CHAR cChar)
{
	INT nNum = cChar-'0';

	if ( nNum >= 0 && nNum <= 9 )
	{
		return TRUE;
	}

	return FALSE;
}


/******************************************************************************************************************
������ų Character�� �̸��˻�(�ѱ�).
*******************************************************************************************************************/
BOOL CChrProcess::IsHangul(CHAR* pszChar)
{
	CHAR	cChar;
	CHAR	cNextChar;
	WORD	wchDBCS;

	cChar	  = *pszChar;
	cNextChar = *(pszChar+1);

	if ( cChar & 0x800 )
	{
		if ( (wchDBCS = (cChar << 8) + cNextChar) > 0xCAA0 )
		{
			// ����.
			return FALSE;
		}
		else
		{
			if ( (wchDBCS >= 0xafa1) && (wchDBCS < 0xc8f0) )
			{
				return TRUE;	// Hangul	�� KS�ϼ����� ����.
			}
			else
			{
				return FALSE;
			}
		}
	}

	return FALSE;
}


/******************************************************************************************************************
������ų Character�� �̸��� Ÿ�������� ���θ� �Ǻ��Ѵ�.
*******************************************************************************************************************/
BOOL CChrProcess::CreateNameValidate(CHAR* pszName)
{
	if ( pszName != NULL )
	{
		CHAR* pszTemp = pszName;
		// ���͸�.
		while ( *pszName )
		{
			if ( (*pszName == ' ') ||
				 (*pszName == '/') || (*pszName == '@') || (*pszName == '?') ||
				 (*pszName == '\'')|| (*pszName == '"') || (*pszName == '\\')||
				 (*pszName == '.') || (*pszName == ',') || (*pszName == ':') ||
				 (*pszName == ';') || (*pszName == '`') || (*pszName == '~') ||
				 (*pszName == '!') || (*pszName == '#') || (*pszName == '$') ||
				 (*pszName == '%') || (*pszName == '^') || (*pszName == '&') ||
				 (*pszName == '*') || (*pszName == '(') || (*pszName == ')') ||
				 (*pszName == '-') || (*pszName == '_') || (*pszName == '+') ||
				 (*pszName == '=') || (*pszName == '|') || (*pszName == '[') ||
				 (*pszName == '{') || (*pszName == ']') || (*pszName == '}') ||
				 (*pszName == '<') || (*pszName == '>'))
			{
				return FALSE;
			}

			if ( !IsAlphabet(*pszName) )	
			{
				if ( !IsNumeric(*pszName) )
				{
//					if ( !IsHangul(pszName) )
//					{
//						return	FALSE;
//					}
//					else
//					{
						// �ѱ��ϰ��?2����Ʈ ������ ���ȴ�.
						pszName++;
//					}
				}
			}

			pszName++;
		}

		return TRUE;
	}

	return FALSE;
}









//-----------------------------------------------------------------------------------------------------------------
// Character ���� ���úκ�.
//-----------------------------------------------------------------------------------------------------------------
/******************************************************************************************************************
ĳ������ ���ÿ����� �����Ѵ�.(���콺 Ŭ�������� ���õ� ĳ������ ���� �� �ְ� �ϱ� ����)
*******************************************************************************************************************/
VOID CChrProcess::SetCharRect(INT nNum, BYTE bMtn)
{
	if ( nNum > _MAX_CHAR || bMtn >= _CHR_MAX_MT)
	{
		return;
	}

	LPCHARINFO pstChrInfo = NULL;

	if ( m_bChrProcState == _CHR_PROC_SELECT )
	{
		pstChrInfo = &m_stSelectChrInfo[nNum];
	}
	else if ( m_bChrProcState == _CHR_PROC_CREATE )
	{
		pstChrInfo = &m_stCreateChrInfo[nNum];
	}

	if ( pstChrInfo && pstChrInfo->bSetted )
	{
		pstChrInfo->stCurrFrmInfo.pstCurrSprInfo = GetCharSprInfo(pstChrInfo->bGender, pstChrInfo->bClass, bMtn);

		if ( pstChrInfo->stCurrFrmInfo.pstCurrSprInfo )
		{			
			//ĳ����.
			if ( m_xImageChr.NewSetIndex(pstChrInfo->stCurrFrmInfo.wCurrFrm) )
			{
				INT nPosX = pstChrInfo->stCurrFrmInfo.nPosX+m_xImageChr.m_lpstNewCurrWilImageInfo->shPX;
				INT nPosY = pstChrInfo->stCurrFrmInfo.nPosY+m_xImageChr.m_lpstNewCurrWilImageInfo->shPY;

				SetRect(&pstChrInfo->stCurrFrmInfo.rcChrRgn, nPosX, nPosY, 
						nPosX + m_xImageChr.m_lpstNewCurrWilImageInfo->shWidth, 
						nPosY + m_xImageChr.m_lpstNewCurrWilImageInfo->shHeight);
			}
		}
	}
}


/******************************************************************************************************************
ĳ������ ���� ���ۿ� ���� ������ �Է�.
*******************************************************************************************************************/
VOID CChrProcess::SetMotion(INT nNum, BYTE bMtn)
{
	if ( nNum > _MAX_CHAR || bMtn >= _CHR_MAX_MT)
	{
		return;
	}

	LPCHARINFO pstChrInfo = NULL;

	if ( m_bChrProcState == _CHR_PROC_SELECT || m_bChrProcState == _CHR_PROC_CREATEOUT )
	{
		pstChrInfo = &m_stSelectChrInfo[nNum];
	}
	else if ( m_bChrProcState == _CHR_PROC_CREATE )
	{
		pstChrInfo = &m_stCreateChrInfo[nNum];
	}

	if ( pstChrInfo && pstChrInfo->bSetted )
	{
		pstChrInfo->stCurrFrmInfo.pstCurrSprInfo = GetCharSprInfo(pstChrInfo->bGender, pstChrInfo->bClass, bMtn);

		if ( pstChrInfo->stCurrFrmInfo.pstCurrSprInfo )
		{
			pstChrInfo->stCurrFrmInfo.wCurrMtn   = bMtn;
			pstChrInfo->stCurrFrmInfo.wCurrFrm   = pstChrInfo->stCurrFrmInfo.pstCurrSprInfo->wFstFrm;
			pstChrInfo->stCurrFrmInfo.wCurrDelay = 0;

			if ( pstChrInfo->stCurrFrmInfo.wCurrMtn == _CHR_MT_NS )
			{
				pstChrInfo->stCurrFrmInfo.bBlend = 0;
			}
			else
			{
				pstChrInfo->stCurrFrmInfo.bBlend = 255;
			}
		}
	}
}


/******************************************************************************************************************
CHARSPRINFO ����ü�� ���������Լ�.
*******************************************************************************************************************/
VOID CChrProcess::SetCharSprInfo(LPCHARSPRINFO pstChrSpr, WORD wFstFrm, WORD wEndFrm, WORD wDelay)
{
	pstChrSpr->wFstFrm = wFstFrm;
	pstChrSpr->wEndFrm = wEndFrm;
	pstChrSpr->wDelay  = wDelay;
}


/******************************************************************************************************************
ĳ������ ���������� �Է�.
*******************************************************************************************************************/
VOID CChrProcess::SetChrSelSprite()
{
	// �������� ����.

	// ��������.
	SetCharSprInfo(&m_stChrSprInfo[0],  200,  210, 120);	// _CHR_MT_NS (Normal Stand ) ���� �ȵɽ��� ����.
	SetCharSprInfo(&m_stChrSprInfo[1],  260,  279, 120);	// _CHR_MT_SM (Select Motion) ���� ���� ����.
	SetCharSprInfo(&m_stChrSprInfo[2],  320,  331, 120);	// _CHR_MT_SS (Select Stand ) ���� ���� ����.
	SetCharSprInfo(&m_stChrSprInfo[3],  380,  387, 120);	// _CHR_MT_R  (Return       ) ���� �ȵɶ��� ���ư���.
	SetCharSprInfo(&m_stChrSprInfo[4],  440,  457, 120);	// _CHR_MT_CC (Char Create  ) ĳ�� �������� ����.

	// ��������.
	SetCharSprInfo(&m_stChrSprInfo[5],  500,  510, 120);
	SetCharSprInfo(&m_stChrSprInfo[6],  560,  570, 120);
	SetCharSprInfo(&m_stChrSprInfo[7],  620,  630, 120);
	SetCharSprInfo(&m_stChrSprInfo[8],  680,  691, 120);
	SetCharSprInfo(&m_stChrSprInfo[9],  740,  755, 120);

	// ���ڼ���.
	SetCharSprInfo(&m_stChrSprInfo[10],  800,  810, 120);
	SetCharSprInfo(&m_stChrSprInfo[11],  860,  871, 120);
	SetCharSprInfo(&m_stChrSprInfo[12],  920,  930, 120);
	SetCharSprInfo(&m_stChrSprInfo[13],  980,  990, 120);
	SetCharSprInfo(&m_stChrSprInfo[14], 1040, 1054, 120);

	// ���ڼ���.
	SetCharSprInfo(&m_stChrSprInfo[15], 1100, 1110, 120);
	SetCharSprInfo(&m_stChrSprInfo[16], 1160, 1177, 120);
	SetCharSprInfo(&m_stChrSprInfo[17], 1220, 1230, 120);
	SetCharSprInfo(&m_stChrSprInfo[18], 1280, 1288, 120);
	SetCharSprInfo(&m_stChrSprInfo[19], 1340, 1356, 120);

	// ���ڵ���.
	// [DX9][T57] 道士帧号表改�?1400-1954(与韩�?国服原版源码一�?�?
	//   反汇编正常客户端 Mir3_patched_v3_backup.exe 实测(0x4097e9-0x409921 区段):
	//   道士�?NS=1400 SM=1460 SS=1520 R=1580 CC=1640 / 道士�?NS=1700 SM=1760 SS=1820 R=1880 CC=1940
	//   T44 �?440-757 是把道士错映射到战士帧区(440=战士男CC/500=战士女NS/560=战士女SM)导致立绘错配�?
	SetCharSprInfo(&m_stChrSprInfo[20], 1400, 1410, 120); // 道士�?NS
	SetCharSprInfo(&m_stChrSprInfo[21], 1460, 1476, 120); // 道士�?SM
	SetCharSprInfo(&m_stChrSprInfo[22], 1520, 1539, 120); // 道士�?SS
	SetCharSprInfo(&m_stChrSprInfo[23], 1580, 1591, 120); // 道士�?R
	SetCharSprInfo(&m_stChrSprInfo[24], 1640, 1656, 120); // 道士�?CC

	SetCharSprInfo(&m_stChrSprInfo[25], 1700, 1710, 120); // 道士�?NS
	SetCharSprInfo(&m_stChrSprInfo[26], 1760, 1776, 120); // 道士�?SM
	SetCharSprInfo(&m_stChrSprInfo[27], 1820, 1830, 120); // 道士�?SS
	SetCharSprInfo(&m_stChrSprInfo[28], 1880, 1889, 120); // 道士�?R
	SetCharSprInfo(&m_stChrSprInfo[29], 1940, 1954, 120); // 道士�?CC

	// ���ڱû�.
	SetCharSprInfo(&m_stChrSprInfo[30],  0, 0, 0);	// _CHR_MT_NS (Normal Stand ) ���� �ȵɽ��� ����.
	SetCharSprInfo(&m_stChrSprInfo[31],  0, 0, 0);	// _CHR_MT_SM (Select Motion) ���� ���� ����.
	SetCharSprInfo(&m_stChrSprInfo[32],  0, 0, 0);	// _CHR_MT_SS (Select Stand ) ���� ���� ����.
	SetCharSprInfo(&m_stChrSprInfo[33],  0, 0, 0);	// _CHR_MT_R  (Return       ) ���� �ȵɶ��� ���ư���.
	SetCharSprInfo(&m_stChrSprInfo[34],  2030,  2030, 120);	// _CHR_MT_CC (Char Create  ) ĳ�� �������� ����.

	// ���ڱû�.
	SetCharSprInfo(&m_stChrSprInfo[35],  0, 0, 0);
	SetCharSprInfo(&m_stChrSprInfo[36],  0, 0, 0);
	SetCharSprInfo(&m_stChrSprInfo[37],  0, 0, 0);
	SetCharSprInfo(&m_stChrSprInfo[38],  0, 0, 0);
	SetCharSprInfo(&m_stChrSprInfo[39],  2040,  2040, 120);
}


/******************************************************************************************************************
����, ����, �������� ĳ������ ������������ ���´�.
*******************************************************************************************************************/
LPCHARSPRINFO CChrProcess::GetCharSprInfo(BYTE bGender, BYTE bJob, BYTE bMtn)
{
	INT nSprArray = bJob*10 + bGender*5 + bMtn;

	if ( nSprArray >= 0 && nSprArray < _MAX_SPR_KIND )
	{
		return 	(&m_stChrSprInfo[nSprArray]);
	}

	return NULL;
}










//-----------------------------------------------------------------------------------------------------------------
// ��Ŷ���� �Լ�����.
//-----------------------------------------------------------------------------------------------------------------
/******************************************************************************************************************
�α伭���� ���Ӽ���.
*******************************************************************************************************************/
VOID CChrProcess::OnConnectToServer()
{
	g_xClientSocket.OnQueryChar();
}


/******************************************************************************************************************
dll Error Msg ���Ϸκ��� ���������� ���´�.
*******************************************************************************************************************/
/*VOID CChrProcess::GetErrorMsg(INT nMsg, CHAR* pszMsg)
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
CHAR* CChrProcess::OnMessageReceive(CHAR* pszBuf, INT nLen)
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
VOID CChrProcess::OnSocketMessageReceive(CHAR* pszMsg)
{
	_TDEFAULTMESSAGE	tdm;

	fnDecodeMessage(&tdm, pszMsg);

	switch ( tdm.wIdent )
	{
		case SM_QUERYCHR:
		{
			DWORD	dwTime;
			dwTime = timeGetTime();
			
			while(dwTime - m_dwWaitTick < 500)
			{
				dwTime = timeGetTime();
			}
			m_xMsgBox.InitMsgBox();
			ZeroMemory(m_stSelectChrInfo, _MAX_CHAR*sizeof(CHARINFO));
			m_nSelectedChr = _SELECTED_NONE;

			if ( tdm.nRecog )
			{
				INT		nPos = 0;

				CHAR	pszDecodeMsg[256];

				CHAR*   pszID	 = NULL;
				CHAR*   pszJob	 = NULL;
				CHAR*   pszFace	 = NULL;
				CHAR*   pszLevel = NULL;
				CHAR*   pszSex	 = NULL;
				CHAR*   pszNext	 = NULL;

				CHAR*   pszUpgrade = NULL; //lee wun gyo

				ZeroMemory(pszDecodeMsg, sizeof(pszDecodeMsg));

				//nPos = fnDecode6BitBuf((pszMsg + _DEFBLOCKSIZE), pszDecodeMsg, sizeof(pszDecodeMsg));
				nPos = fnDecode6BitBufOriginal((pszMsg + _DEFBLOCKSIZE), pszDecodeMsg, sizeof(pszDecodeMsg)); //20051031
				pszDecodeMsg[nPos] = '\0';

				// [DX9][DIAG-QCHR] 临�诊断: dump SM_QUERYCHR 原始字符�?
				//   用于确定本服务器性别字段的真实含�?
    #if MIR3_DIAG_LOG
				{
					FILE* fpQ = fopen("D:\\querychr_dump.txt", "a");
					if ( fpQ )
					{
						fprintf(fpQ, "\n=== SM_QUERYCHR nChrCnt=%d ===\n", tdm.nRecog);
						fprintf(fpQ, "decoded: [%s]\n", pszDecodeMsg);
						fclose(fpQ);
					}
				}
    #endif // MIR3_DIAG_LOG

				INT nChrCnt = tdm.nRecog;

				if ( nChrCnt > _MAX_CHAR )
				{
					nChrCnt = _MAX_CHAR;
				}

				if (nChrCnt > 2)
					m_bMoreTwoChar = TRUE;
				else
					m_bMoreTwoChar = FALSE;


pszNext = &pszDecodeMsg[0];

								for ( INT nCnt = 0; nCnt < nChrCnt; nCnt++ )
				{
					memset(&m_stSelectChrInfo[nCnt], 0, sizeof(CHARINFO));

					if ( *pszNext == '*' )
					{
						pszID = ++pszNext;
					}
					else
					{
						pszID = pszNext;
					}


					if ( pszJob = strchr(pszID, '/') )
					{
						*pszJob = '\0';
						pszJob++;

						strcpy(m_stSelectChrInfo[nCnt].pszChrName, pszID);

						m_stSelectChrInfo[nCnt].bClass = atoi(pszJob);
						
						if ( pszFace = strchr(pszJob, '/') )
						{
							*pszFace = '\0';
							pszFace++;

							// [DX9][T26] 修复字段解析顺序：服务器格式�?class/gender/level/upgradeLevel
							// 原代码错误地�?class/level/gender/upgradeLevel 解析，导致等级和性别搞反
							

							if ( pszLevel = strchr(pszFace, '/') )
							{
								*pszLevel = '\0';
								pszLevel++;

								m_stSelectChrInfo[nCnt].bLevel = atoi(pszLevel);

								if ( pszSex = strchr(pszLevel, '/') )
								{
									*pszSex = '\0';
									pszSex++;

									m_stSelectChrInfo[nCnt].bGender = atoi(pszSex); // D field: 0=male, 1=female
																																				m_stSelectChrInfo[nCnt].bUpgradeLevel = 0; // D field is gender, not upgrade flag
									pszUpgrade = strchr(pszSex, '/');
												if ( pszUpgrade ) pszUpgrade++;
												else pszUpgrade = pszSex + strlen(pszSex);
								}
								else
								{
									// LOG: ȱ�����ֶ�, ��Ĭ�� 0
									m_stSelectChrInfo[nCnt].bUpgradeLevel = 0;
									pszUpgrade = strchr(pszLevel, '/');
												if ( pszUpgrade ) pszUpgrade++;
												else pszUpgrade = pszLevel + strlen(pszLevel);
								}
							}
							else
							{
								// LOG: ȱ�Ա��ֶ�, ��Ĭ�� 1(��)
								m_stSelectChrInfo[nCnt].bGender = 1;
								m_stSelectChrInfo[nCnt].bUpgradeLevel = 0;
								pszUpgrade = strchr(pszFace, '/');
												if ( pszUpgrade ) pszUpgrade++;
												else pszUpgrade = pszFace + strlen(pszFace);
							}
						}
						else
						{
							// LOG: ȱ�ȼ��ֶ�, ��Ĭ�� 1
							m_stSelectChrInfo[nCnt].bLevel = 1;
							m_stSelectChrInfo[nCnt].bGender = 1;
							m_stSelectChrInfo[nCnt].bUpgradeLevel = 0;
							pszUpgrade = strchr(pszJob, '/');
												if ( pszUpgrade ) pszUpgrade++;
												else pszUpgrade = pszJob + strlen(pszJob);
						}

						// ǯ���Ա�/ְҵ����Ч��Χ, ��ֹ��������Խ�絼�½�ɫ����ʾ
						if ( m_stSelectChrInfo[nCnt].bGender > 1 ) m_stSelectChrInfo[nCnt].bGender = 1;
						if ( m_stSelectChrInfo[nCnt].bClass > 3 ) m_stSelectChrInfo[nCnt].bClass = 3;
						// [DX9][DIAG-QCHR] 记录解析出的性别/职业/等级
      #if MIR3_DIAG_LOG
						{
							FILE* fpQ = fopen("D:\\querychr_dump.txt", "a");
							if ( fpQ )
							{
								fprintf(fpQ, "chr[%d] name=%s class=%d gender=%d level=%d up=%d\n",
									nCnt, m_stSelectChrInfo[nCnt].pszChrName,
									(int)m_stSelectChrInfo[nCnt].bClass,
									(int)m_stSelectChrInfo[nCnt].bGender,
									(int)m_stSelectChrInfo[nCnt].bLevel,
									(int)m_stSelectChrInfo[nCnt].bUpgradeLevel);
								fclose(fpQ);
							}
						}
      #endif // MIR3_DIAG_LOG
						m_stSelectChrInfo[nCnt].bSetted = TRUE;
						pszNext = pszUpgrade;


						BYTE bMtn = _CHR_MT_NS;

						if ( nChrCnt == 1 )
						{
							bMtn = _CHR_MT_SM;
							m_stSelectChrInfo[nCnt].stCurrFrmInfo.bBlend = 255;
							m_stSelectChrInfo[nCnt].stCurrFrmInfo.nPosX  = 300;
							m_stSelectChrInfo[nCnt].stCurrFrmInfo.nPosY  = 210;
						}
						else
						{
							if ( nCnt == 0 || nCnt == 2)
							{
								bMtn = _CHR_MT_SM;
								m_stSelectChrInfo[nCnt].stCurrFrmInfo.bBlend = 255;
								m_stSelectChrInfo[nCnt].stCurrFrmInfo.nPosX  = 250;
								m_stSelectChrInfo[nCnt].stCurrFrmInfo.nPosY  = 210;
							}
							else if (nCnt == 1 || nCnt == 3)
							{
								m_stSelectChrInfo[nCnt].stCurrFrmInfo.nPosX = 350;
								m_stSelectChrInfo[nCnt].stCurrFrmInfo.nPosY = 250;
							}
						}

						m_nSelectedChr = _SELECTED_FST_CHR;

						m_bFrontSetofSelChar = TRUE;

						SetSelectChrShowInfo();

						SetMotion(nCnt, bMtn);
					}
					else
					{
						pszNext = pszID + strlen(pszID);
					}
				}

			}
			else
			{
				m_xMsgBox.CreateMsgBox(&m_xImageInter, 0, _OK, g_xMsg.GetMsg(219), FALSE, 140, 110);
			}
			break;
		}
		case SM_QUERYCHR_FAIL:
		{
			m_xMsgBox.CreateMsgBox(&m_xImageInter, 0, _OK, g_xMsg.GetMsg(220), FALSE, 140, 110);
			break;
		}
		case SM_NEWCHR_SUCCESS:
		{
			g_SoundManager.SndmngrStopMp3();
			m_bChrProcState = _CHR_PROC_CREATEOUT;
			m_bBGMPlay		= FALSE;

//			SndObjPlay(0, 0, m_stEffctWav[1], 0);

//			m_xAvi.Create(g_xMainWnd.GetDirectDraw(), g_SoundManager.mSound_lpds, ".\\Data\\CreateChr.dat", FALSE, TRUE);
//			m_xAvi.Start();

			SetFocus(g_xMainWnd.GetSafehWnd());
			ShowWindow(g_xChatEditBox.GetSafehWnd(), SW_HIDE);

			g_xClientSocket.OnQueryChar();
#ifdef _NEWINTERFACE
			m_xMsgBox.CreateMsgBox(&m_xImageInter, 0, _NONE, g_xMsg.GetMsg(200), FALSE, 155, 150);
#else //_NEWINTERFACE
			m_xMsgBox.CreateMsgBox(&m_xImageInter, 0, _NONE, g_xMsg.GetMsg(200), FALSE, 140, 110);
#endif //_NEWINTERFACE 
			m_dwWaitTick = timeGetTime();
			break;
		}
		case SM_NEWCHR_FAIL:
		{
			INT nMsgNum = -1;

			switch ( tdm.nRecog )
			{
				case 2:
				{
					nMsgNum = 224;
					break;
				}
				case 3:
				{
					nMsgNum = 226;
					break;
				}
				case 4:
				{
					nMsgNum = 232;
					break;
				}
				default:
				{
					nMsgNum = 232;
					break;
				}
			}
			m_xMsgBox.CreateMsgBox(&m_xImageInter, 0, _OK, g_xMsg.GetMsg(nMsgNum), FALSE, 140, 110);
			break;
		}
		case SM_DELCHR_SUCCESS:
		{
			g_xClientSocket.OnQueryChar();
#ifdef _NEWINTERFACE
			m_xMsgBox.CreateMsgBox(&m_xImageInter, 0, _NONE, g_xMsg.GetMsg(200), FALSE, 155, 150);
#else //_NEWINTERFACE
			m_xMsgBox.CreateMsgBox(&m_xImageInter, 0, _NONE, g_xMsg.GetMsg(200), FALSE, 140, 110);
#endif //_NEWINTERFACE 
			m_dwWaitTick = timeGetTime();

			break;
		}
		case SM_DELCHR_FAIL:
		{
			m_xMsgBox.CreateMsgBox(&m_xImageInter, 0, _OK, g_xMsg.GetMsg(227), FALSE, 140, 110);

			g_xClientSocket.OnQueryChar();
#ifdef _NEWINTERFACE
			m_xMsgBox.CreateMsgBox(&m_xImageInter, 0, _NONE, g_xMsg.GetMsg(200), FALSE, 155, 150);
#else //_NEWINTERFACE
			m_xMsgBox.CreateMsgBox(&m_xImageInter, 0, _NONE, g_xMsg.GetMsg(200), FALSE, 140, 110);
#endif //_NEWINTERFACE 
			m_dwWaitTick = timeGetTime();

			break;
		}
		case SM_STARTPLAY:
		{
			INT		nPos = 0;
			CHAR	pszDecodeMsg[256];

			ZeroMemory(pszDecodeMsg, sizeof(pszDecodeMsg));

			//nPos = fnDecode6BitBuf((pszMsg + _DEFBLOCKSIZE), pszDecodeMsg, sizeof(pszDecodeMsg));
			nPos = fnDecode6BitBufOriginal((pszMsg + _DEFBLOCKSIZE), pszDecodeMsg, sizeof(pszDecodeMsg)); //20051031
			pszDecodeMsg[nPos] = '\0';

			CHAR* pszIP   = NULL;
			CHAR* pszPort = NULL;

			pszIP = &pszDecodeMsg[0];

			if ( pszPort = strchr(pszIP, '/') )
			{
				*pszPort = '\0';
				pszPort++;

				strcpy(g_szPreServerIP, g_szServerIP);
				g_nPreServerPort = g_nServerPort;

				strcpy(g_szServerIP, pszIP);
				g_nServerPort = atoi(pszPort);

//				INT nTemp = rand()%2;

//				if (nTemp == 0)
//					g_nServerPort++;

				if ( m_nSelectedChr != _SELECTED_NONE && m_nSelectedChr < _MAX_CHAR )
				{
					strcpy(g_szCharName, m_stSelectChrInfo[m_nSelectedChr].pszChrName);
					g_nUpgradeLevel = m_stSelectChrInfo[m_nSelectedChr].bUpgradeLevel;
				}

				g_SoundManager.SndmngrStopMp3();
				m_bChrProcState = _CHR_PROC_START;
				m_bBGMPlay	    = FALSE;

				SndObjPlay(0, 0, m_stEffctWav[2], 0);

				//				m_xAvi.Create(g_xMainWnd.GetDirectDraw(), g_SoundManager.mSound_lpds, ".\\Data\\StartGame.dat", FALSE);
				//				m_xAvi.Start();
			}
			break;
		}
		case SM_STARTFAIL:
		{
			m_xMsgBox.CreateMsgBox(&m_xImageInter, 0, _OK, g_xMsg.GetMsg(221), FALSE, 140, 110);
			break;
		}
		case SM_OUTOFCONNECTION:
		{
			m_xMsgBox.CreateMsgBox(&m_xImageInter, 0, _OK, g_xMsg.GetMsg(222), FALSE, 140, 110);
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
LRESULT CChrProcess::DefMainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
		case WM_KEYDOWN:
			OnKeyDown(wParam, lParam);
			break;
		case WM_SYSKEYDOWN:
#ifndef _DEBUG
			return 0L;
#endif
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
		case WM_RBUTTONDOWN:
			SetFocus(g_xChatEditBox.GetSafehWnd());
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

	g_SoundManager.SndmngrMessageProc(hWnd, uMsg, wParam, lParam);

	return CWHDefProcess::DefMainWndProc(hWnd, uMsg, wParam, lParam);
}


/******************************************************************************************************************
���α׷� �����?
*******************************************************************************************************************/
LRESULT CChrProcess::OnDestroy(WPARAM wParam, LPARAM lParam)
{
//	DeleteProc();
	WSACleanup();
//	ReleaseFontList();
	return 0L;
}


HRESULT	CChrProcess::OnKeyDown(WPARAM wParam, LPARAM lParam)
{
	if ( m_xMsgBox.OnKeyDown(wParam, lParam) )
	{
		return 0L;
	}

	switch ( wParam )
	{
		case VK_RETURN:
		{
			if ( m_bChrProcState == _CHR_PROC_CREATE )
			{
				if ( m_nCreatedChr != _SELECTED_NONE && m_nCreatedChr < _MAX_CHAR )
				{
					CHAR pszCreateChr[MAX_PATH];
					ZeroMemory(pszCreateChr, MAX_PATH);
					GetWindowText(g_xChatEditBox.GetSafehWnd(), pszCreateChr, MAX_PATH);
					SetWindowText(g_xChatEditBox.GetSafehWnd(), "");

					INT nLen = strlen(pszCreateChr);

					if ( pszCreateChr[0] != '\0' )
					{
						if ( nLen > 13 )
						{
							m_xMsgBox.CreateMsgBox(&m_xImageInter, 0, _OK, g_xMsg.GetMsg(223), FALSE, 140, 110);
						}
						else if ( CreateNameValidate(pszCreateChr) )
						{
							g_xClientSocket.OnNewChar(pszCreateChr, 1, m_stCreateChrInfo[m_nCreatedChr].bClass, m_stCreateChrInfo[m_nCreatedChr].bGender);
						}
						else
						{
							m_xMsgBox.CreateMsgBox(&m_xImageInter, 0, _OK, g_xMsg.GetMsg(225), FALSE, 140, 110);
						}
					}
				}

				SetFocus(g_xChatEditBox.GetSafehWnd());
				MoveWindow(g_xChatEditBox.GetSafehWnd(), g_xMainWnd.m_rcWindow.left+288, g_xMainWnd.m_rcWindow.top+405, 75, 13, TRUE);
				ShowWindow(g_xChatEditBox.GetSafehWnd(), SW_SHOW);
			}
			break;
		}
	}

	if ( HIBYTE(GetKeyState('S')) && m_bChrProcState == _CHR_PROC_SELECT )
	{
		if ( m_nSelectedChr != _SELECTED_NONE && m_nSelectedChr < _MAX_CHAR )
		{
//     ���� ����
			g_xClientSocket.OnSelChar(m_stSelectChrInfo[m_nSelectedChr].pszChrName);
//     ���� ����
/* ���� ����
			g_xClientSocket.OnSelChar(m_stSelectChrInfo[m_nSelectedChr].pszChrName);
*/
		}
		else
		{
			m_xMsgBox.CreateMsgBox(&m_xImageInter, 0, _OK, g_xMsg.GetMsg(230), FALSE, 140, 110);
		}


	}




	return 0L;
}


LRESULT	CChrProcess::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
	INT nCnt;

	m_ptMousePos.x = LOWORD(lParam);
	m_ptMousePos.y = HIWORD(lParam);

	if ( m_xMsgBox.OnLButtonDown(m_ptMousePos) )
	{
		return 0L;
	}

	if ( m_bChrProcState == _CHR_PROC_SELECT )
	{
		INT		nSelectIdx;

		for ( nCnt = 0; nCnt < _MAX_CHAR; nCnt++ )
		{
			if (m_bFrontSetofSelChar == FALSE)
				nSelectIdx = nCnt + 2;
			else
				nSelectIdx = nCnt;

			if ( m_stSelectChrInfo[nSelectIdx].bSetted )
			{
				if ( PtInRect(&m_stSelectChrInfo[nSelectIdx].stCurrFrmInfo.rcChrRgn, m_ptMousePos) )
				{
					if ( m_nSelectedChr != nSelectIdx )
					{
						SetMotion(nSelectIdx, _CHR_MT_SM);

						if ( nSelectIdx == 0 )
						{
							if ( m_stSelectChrInfo[1].bSetted )
							{
								SetMotion(1, _CHR_MT_R);
							}
						}
						else if ( nSelectIdx == 1 )
						{
							if ( m_stSelectChrInfo[0].bSetted )
							{
								SetMotion(0, _CHR_MT_R);
							}
						}
						else if ( nSelectIdx == 2 )
						{
							if ( m_stSelectChrInfo[3].bSetted )
							{
								SetMotion(3, _CHR_MT_R);
							}
						}
						else if ( nSelectIdx == 3 )
						{
							if ( m_stSelectChrInfo[2].bSetted )
							{
								SetMotion(2, _CHR_MT_R);
							}
						}
					}

					m_nSelectedChr = nSelectIdx;

					SetSelectChrShowInfo();
					break;
				}
			}
		}

		for ( nCnt = 0; nCnt < _MAX_CHR_SELECT_BTN; nCnt++ )
		{
			m_xSelectBtn[nCnt].OnLButtonDown(m_ptMousePos);
		}
	}
	else if ( m_bChrProcState == _CHR_PROC_CREATE )
	{
		SetFocus(g_xChatEditBox.GetSafehWnd());

		for ( nCnt = 0; nCnt < _MAX_CHAR; nCnt++ )
		{
			if ( m_stCreateChrInfo[nCnt].bSetted )
			{
				if ( PtInRect(&m_stCreateChrInfo[nCnt].stCurrFrmInfo.rcChrRgn, m_ptMousePos) )
				{
					if ( m_nCreatedChr != nCnt )
					{
						SetMotion(nCnt, _CHR_MT_CC);
					}

					m_nCreatedChr = nCnt;

					if ( m_nCreatedChr != _SELECTED_NONE )
					{
						SetCharExplain(m_nCreatedChr, m_stCreateChrInfo[nCnt].bClass);
					}
				}
			}
		}

		for ( INT nCnt = 0; nCnt < _CHR_CHR_CREATE_BTN; nCnt++ )
		{
			m_xCreateBtn[nCnt].OnLButtonDown(m_ptMousePos);
		}
	}

	return 0L;
}


LRESULT	CChrProcess::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
	INT nCnt;

	m_ptMousePos.x = LOWORD(lParam);
	m_ptMousePos.y = HIWORD(lParam);

	if ( m_xMsgBox.OnLButtonUp(m_ptMousePos) )
	{
		return 0L;
	}

	if ( m_bChrProcState == _CHR_PROC_SELECT )
	{
		if ( m_xSelectBtn[_MAX_CHR_SELECT_CREATE].OnLButtonUp(m_ptMousePos) )
		{
			BOOL bGoInCreate = FALSE;
			
			for ( nCnt = 0; nCnt < _MAX_SHOW_CHAR; nCnt++ )
			{
				if ( !m_stSelectChrInfo[nCnt].bSetted )
				{
					bGoInCreate = TRUE;
					break;
				}
			}

			if ( bGoInCreate )
			{
				g_SoundManager.SndmngrStopMp3();
				m_bChrProcState = _CHR_PROC_CREATEIN;
				m_bBGMPlay	    = FALSE;

				SndObjPlay(0, 0, m_stEffctWav[0], 0);

				//				m_xAvi.Create(g_xMainWnd.GetDirectDraw(), g_SoundManager.mSound_lpds, ".\\Data\\CreateChr.dat", FALSE);
				//				m_xAvi.Start();
			}
			else
			{
				m_xMsgBox.CreateMsgBox(&m_xImageInter, 0, _OK, g_xMsg.GetMsg(226), FALSE, 140, 110);
			}
		}
		else if ( m_xSelectBtn[_MAX_CHR_SELECT_START].OnLButtonUp(m_ptMousePos) )
		{
			if ( m_nSelectedChr != _SELECTED_NONE && m_nSelectedChr < _MAX_CHAR )
			{
				g_xClientSocket.OnSelChar(m_stSelectChrInfo[m_nSelectedChr].pszChrName);
			}
			else
			{
				m_xMsgBox.CreateMsgBox(&m_xImageInter, 0, _OK, g_xMsg.GetMsg(230), FALSE, 140, 110);
			}
		}
		else if ( m_xSelectBtn[_MAX_CHR_SELECT_QUIT].OnLButtonUp(m_ptMousePos) )
		{
			DeleteProc();
			g_xMainWnd.m_pxDefProcess = &g_xGameProc;
			g_bProcState = _GAME_PROC;	
			g_xGameProc.m_bCreditShow = TRUE;
			g_xGameProc.m_xImage.m_xImageList[70].NewLoad(".\\Data\\GameInter.wil");
			g_xGameProc.LoadCredit();
			ShowWindow(g_xChatEditBox.GetSafehWnd(), SW_HIDE);
		}
		else if ( m_xSelectBtn[_MAX_CHR_SELECT_DELETE].OnLButtonUp(m_ptMousePos) )
		{
			if ( m_nSelectedChr != _SELECTED_NONE )
			{
				m_xMsgBox.CreateMsgBox(&m_xImageInter, 1, _YESNO, g_xMsg.GetMsg(228), TRUE, 140, 110, 1, TRUE);
			}
			else
			{
				m_xMsgBox.CreateMsgBox(&m_xImageInter, 0, _OK, g_xMsg.GetMsg(227), FALSE, 140, 110);
			}
		}
		else if ( m_xSelectBtn[_MAX_CHR_SELECT_NEXT].OnLButtonUp(m_ptMousePos, FALSE) )
		{
			BYTE	bMtn;
			INT		nCnt;

			if (m_bMoreTwoChar == FALSE)
				return 0L;

			if (m_bFrontSetofSelChar ==  TRUE)
			{
				m_xSelectBtn[_MAX_CHR_SELECT_NEXT].OnLButtonUp(m_ptMousePos);
				bMtn = _CHR_MT_SM;
				nCnt = _SELECTED_TRD_CHR;
				SetMotion(m_nSelectedChr, _CHR_MT_NS);
				m_nSelectedChr = _SELECTED_TRD_CHR;
				SetSelectChrShowInfo();
				SetMotion(nCnt, bMtn);

				m_bFrontSetofSelChar = FALSE;
				m_xSelectBtn[_MAX_CHR_SELECT_NEXT].ResetGameBtn(1155, 1155, 1155);
				m_xSelectBtn[_MAX_CHR_SELECT_BEFORE].ResetGameBtn(1152, 1151, 1150);
			}
			else
				return 0L;
		}
		else if ( m_xSelectBtn[_MAX_CHR_SELECT_BEFORE].OnLButtonUp(m_ptMousePos, FALSE) )
		{
			BYTE	bMtn;
			INT		nCnt;

			if (m_bMoreTwoChar == FALSE)
				return 0L;

			if (m_bFrontSetofSelChar ==  FALSE)
			{
				m_xSelectBtn[_MAX_CHR_SELECT_BEFORE].OnLButtonUp(m_ptMousePos);
				bMtn = _CHR_MT_SM;
				nCnt = _SELECTED_FST_CHR;
				SetMotion(m_nSelectedChr, _CHR_MT_NS);
				m_nSelectedChr = _SELECTED_FST_CHR;
				SetSelectChrShowInfo();
				SetMotion(nCnt, bMtn);

				m_bFrontSetofSelChar = TRUE;
				m_xSelectBtn[_MAX_CHR_SELECT_BEFORE].ResetGameBtn(1152, 1152, 1152);
				m_xSelectBtn[_MAX_CHR_SELECT_NEXT].ResetGameBtn(1155, 1154, 1153);
			}
			else
				return 0L;
		}
	}
	else if ( m_bChrProcState == _CHR_PROC_CREATE )
	{
		RECT rcTemp;
		SetRect(&rcTemp, 294, 404, 371, 419);

		if ( PtInRect(&rcTemp, m_ptMousePos) )
		{
			SetFocus(g_xChatEditBox.GetSafehWnd());
			MoveWindow(g_xChatEditBox.GetSafehWnd(), g_xMainWnd.m_rcWindow.left+288, g_xMainWnd.m_rcWindow.top+405, 75, 13, TRUE);
			ShowWindow(g_xChatEditBox.GetSafehWnd(), SW_SHOW);
		}
		else if ( m_xCreateBtn[_CHR_CHR_CREATE_CANCEL].OnLButtonUp(m_ptMousePos) )
		{
			g_SoundManager.SndmngrStopMp3();
			m_bChrProcState = _CHR_PROC_CREATEOUT;
			m_bBGMPlay		= FALSE;

//			SndObjPlay(0, 0, m_stEffctWav[1], 0);

//			m_xAvi.Create(g_xMainWnd.GetDirectDraw(), g_SoundManager.mSound_lpds, ".\\Data\\CreateChr.dat", FALSE, TRUE);
//			m_xAvi.Start();

			SetFocus(g_xMainWnd.GetSafehWnd());
			ShowWindow(g_xChatEditBox.GetSafehWnd(), SW_HIDE);

			g_xClientSocket.OnQueryChar();
//			m_xMsgBox.CreateMsgBox(&m_xImageInter, 0, _NONE, g_xMsg.GetMsg(200), FALSE, 140, 110);
//			m_dwWaitTick = timeGetTime();
		}
		else if ( m_xCreateBtn[_CHR_CHR_CREATE_WARIOR].OnLButtonUp(m_ptMousePos) )
		{
			SetChrInfo(&m_stCreateChrInfo[0], _GENDER_MAN, _JOB_JUNSA, 0);
			SetMotion(0, _CHR_MT_CC);

			SetChrInfo(&m_stCreateChrInfo[1], _GENDER_WOMAN, _JOB_JUNSA, 0);
			SetMotion(1, _CHR_MT_CC);

			if ( m_nCreatedChr !=_SELECTED_NONE )
			{
				SetCharExplain(m_nCreatedChr, _JOB_JUNSA);
			}
		}
		else if ( m_xCreateBtn[_CHR_CHR_CREATE_MAGICIAN].OnLButtonUp(m_ptMousePos) )
		{
			SetChrInfo(&m_stCreateChrInfo[0], _GENDER_MAN, _JOB_SULSA, 0);
			SetMotion(0, _CHR_MT_CC);

			SetChrInfo(&m_stCreateChrInfo[1], _GENDER_WOMAN, _JOB_SULSA, 0);
			SetMotion(1, _CHR_MT_CC);

			if ( m_nCreatedChr != _SELECTED_NONE )
			{
				SetCharExplain(m_nCreatedChr, _JOB_SULSA);
			}
		}
		else if ( m_xCreateBtn[_CHR_CHR_CREATE_CLERIC].OnLButtonUp(m_ptMousePos) )
		{
			SetChrInfo(&m_stCreateChrInfo[0], _GENDER_MAN, _JOB_DOSA, 0);
			SetMotion(0, _CHR_MT_CC);

			SetChrInfo(&m_stCreateChrInfo[1], _GENDER_WOMAN, _JOB_DOSA, 0);
			SetMotion(1, _CHR_MT_CC);

			if ( m_nCreatedChr != _SELECTED_NONE )
			{
				SetCharExplain(m_nCreatedChr, _JOB_DOSA);
			}
		}
		else if ( m_xCreateBtn[_CHR_CHR_CREATE_ARCHER].OnLButtonUp(m_ptMousePos) )
		{
			SetChrInfo(&m_stCreateChrInfo[0], _GENDER_MAN, _JOB_GUNGSA, 0);
			SetMotion(0, _CHR_MT_CC);

			SetChrInfo(&m_stCreateChrInfo[1], _GENDER_WOMAN, _JOB_GUNGSA, 0);
			SetMotion(1, _CHR_MT_CC);

			if ( m_nCreatedChr != _SELECTED_NONE )
			{
				SetCharExplain(m_nCreatedChr, _JOB_GUNGSA);
			}
		}
		else if ( m_xCreateBtn[_CHR_CHR_CREATE_MAKE].OnLButtonUp(m_ptMousePos) )
		{
			if ( m_nCreatedChr != _SELECTED_NONE && m_nCreatedChr < _MAX_CHAR && m_stCreateChrInfo[m_nCreatedChr].bClass != _JOB_GUNGSA )
			{
				CHAR pszCreateChr[MAX_PATH];
				ZeroMemory(pszCreateChr, MAX_PATH);
				GetWindowText(g_xChatEditBox.GetSafehWnd(), pszCreateChr, MAX_PATH);
				SetWindowText(g_xChatEditBox.GetSafehWnd(), "");

				INT nLen = strlen(pszCreateChr);

				if ( pszCreateChr[0] != '\0' )
				{
					if ( nLen > 13 )
					{
						m_xMsgBox.CreateMsgBox(&m_xImageInter, 0, _OK, g_xMsg.GetMsg(223), FALSE, 140, 110);
					}
					else if ( CreateNameValidate(pszCreateChr) )
					{
						g_xClientSocket.OnNewChar(pszCreateChr, 1, m_stCreateChrInfo[m_nCreatedChr].bClass, m_stCreateChrInfo[m_nCreatedChr].bGender);
					}
					else
					{
						m_xMsgBox.CreateMsgBox(&m_xImageInter, 0, _OK, g_xMsg.GetMsg(225), FALSE, 140, 110);
					}
				}
			}
		}
	}

	return 0L;
}


LRESULT	CChrProcess::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
	m_ptMousePos.x = LOWORD(lParam);
	m_ptMousePos.y = HIWORD(lParam);

	m_xMsgBox.OnMouseMove(m_ptMousePos);

	if ( m_bChrProcState == _CHR_PROC_SELECT )
	{
		for ( INT nCnt = 0; nCnt < _MAX_SHOW_CHAR; nCnt++ )
		{
			m_xSelectBtn[nCnt].OnMouseMove(m_ptMousePos);
		}
	}
	else if ( m_bChrProcState == _CHR_PROC_CREATE )
	{
		for ( INT nCnt = 0; nCnt < _CHR_CHR_CREATE_BTN; nCnt++ )
		{
			m_xCreateBtn[nCnt].OnMouseMove(m_ptMousePos);
		}
	}

	return 0L;
}


LRESULT	CChrProcess::OnMsgBoxReturn(WPARAM wParam, LPARAM lParam)
{
	if ( HIBYTE(HIWORD(wParam))== 1 && LOBYTE(HIWORD(wParam)) == _BTN_YES )
	{
		if ( m_nSelectedChr != _SELECTED_NONE && m_nSelectedChr < _MAX_CHAR )
		{
			CHAR szTemp[MAX_PATH];
			sprintf(szTemp, "%s/%s/%d",  m_stSelectChrInfo[m_nSelectedChr].pszChrName, lParam, m_stSelectChrInfo[m_nSelectedChr].bUpgradeLevel);
			g_xClientSocket.OnDelChar(szTemp);
			g_xClientSocket.OnQueryChar();
#ifdef _NEWINTERFACE
			m_xMsgBox.CreateMsgBox(&m_xImageInter, 0, _NONE, g_xMsg.GetMsg(200), FALSE, 155, 150);
#else //_NEWINTERFACE
			m_xMsgBox.CreateMsgBox(&m_xImageInter, 0, _NONE, g_xMsg.GetMsg(200), FALSE, 140, 110);
#endif //_NEWINTERFACE 
			m_dwWaitTick = timeGetTime();
		}
	}

	if ( m_bChrProcState == _CHR_PROC_CREATE )
	{
		MoveWindow(g_xChatEditBox.GetSafehWnd(), g_xMainWnd.m_rcWindow.left+288, g_xMainWnd.m_rcWindow.top+405, 75, 13, TRUE);
		ShowWindow(g_xChatEditBox.GetSafehWnd(), SW_SHOW);
		SetFocus(g_xChatEditBox.GetSafehWnd());
	}

	g_xChatEditBox.SetEnterUse(FALSE);

	return 0L;
}


LRESULT	CChrProcess::OnMove(WPARAM wParam, LPARAM lParam)
{
	if ( m_bChrProcState == _CHR_PROC_CREATE )
	{
		SetFocus(g_xMainWnd.GetSafehWnd());
		ShowWindow(g_xChatEditBox.GetSafehWnd(), SW_HIDE);
	}

	return 0L;
}
