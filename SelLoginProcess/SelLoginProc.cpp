#include "StdAfx.h"

#define _PROFILE_FILENAME			"/SelLogin.ini"

//-----------------------------------------------------------------------------------------------------------------
// Procedure �����Լ�.
//-----------------------------------------------------------------------------------------------------------------
/******************************************************************************************************************
[������]
*******************************************************************************************************************/
CSelLoginProcess::CSelLoginProcess()
{
	Init();
}


/******************************************************************************************************************
[�Ҹ���]
*******************************************************************************************************************/
CSelLoginProcess::~CSelLoginProcess()
{
	DeleteProc();
}


/******************************************************************************************************************
������ ��簴ü�� �ʱ�ȭ.
*******************************************************************************************************************/
VOID CSelLoginProcess::Init()
{
	m_nSvrBtnCount = 0;
	m_nSelectedSvrBtn = -1;
	m_nMenuStart = 0;

	m_nAreaBtnCount = 0;
	m_nSelectedAreaBtn = -1;
	m_xScrSelectMenu.Init();

	m_nSerNameCnt = 0;

	m_pObj = NULL;

	for ( INT nI=0; nI<40; nI++ )
	{
		ZeroMemory(&m_ptAreaBtn[nI], sizeof(POINT));
		ZeroMemory(&m_rcAreaBtn[nI], sizeof(RECT));
		ZeroMemory(&m_rcSvrBtn[nI], sizeof(RECT));
//		ZeroMemory(&m_xServerInfo[nI], sizeof(SERVERINFO));
		ZeroMemory(&m_szServerName[nI], sizeof(MAX_PATH));
		ZeroMemory(&m_szScreenName[nI], sizeof(MAX_PATH));
	}

	m_xServerInfo.ClearAll();

	ZeroMemory(&m_ptMousePos, sizeof(POINT));

	m_xImageLogin.Init();

	m_lpddsTextr = NULL;

}




/******************************************************************************************************************
�ʿ��� ����Ÿ�� �ʱ�ȭ�ϰ� �ε��Ѵ�.
Prcedure�� ���۵Ǳ����� ȣ��Ǿ�� �Ѵ�.
Load()->RenderScene()->SetNextProc()�� �����̴�.
*******************************************************************************************************************/
VOID CSelLoginProcess::Load()
{
	Init();
	m_xImageLogin.NewLoad(".\\Data\\LoginInterface.wil");

	GetCurrentDirectory( sizeof( m_szProfileFullName ), m_szProfileFullName );
	lstrcat( m_szProfileFullName, _PROFILE_FILENAME );

	INT nCount = GetPrivateProfileInt( "ServerGroup", "Count", 0, m_szProfileFullName );

	m_xCloseBtn.CreateGameBtn(&m_xImageLogin, 3, 4, 760, 13, g_xMsg.GetMsg(1052), _BTN_TYPE_FOCUS, 3, TRUE);
	m_xScrSelectMenu.CreateScrlBar(&m_xImageLogin, 15, 10, 20, 447, 31);

	for ( INT nI=0; nI<nCount; nI++ )
	{
		CHAR szTemp[MAX_PATH], szData[MAX_PATH];
		char *pszLocation;
		sprintf( szTemp, "Server%d", nI+1);		
		GetPrivateProfileString( "ServerGroup", szTemp, "", szData, MAX_PATH, m_szProfileFullName );

		// ��ư ��ǥ ���
		if ( pszLocation = strchr(szData, ',') )
		{
			*pszLocation ++ = '\0';
			if ( szData[0] == NULL )
			{
			}
			else
			{
				strcpy(m_szScreenName[nI], szData);
			}
			strcpy(szData, pszLocation);

			pszLocation =  strchr(szData, ',');
			*pszLocation ++ = '\0';
			
			strcpy(m_szServerName[nI], szData);
			strcpy(szData, pszLocation);

			if ( pszLocation = strchr(szData, ',') )
				*pszLocation ++ = '\0';

			POINT ptLoc;
			m_ptAreaBtn[nI].x = atoi(szData);
			m_ptAreaBtn[nI].y = atoi(pszLocation);
			SetRect(&m_rcAreaBtn[nI], m_ptAreaBtn[nI].x, m_ptAreaBtn[nI].y, m_ptAreaBtn[nI].x + 40, m_ptAreaBtn[nI].y + 40);
			m_nAreaBtnCount++;
		}
	}
}


/******************************************************************************************************************
�Ҹ��� �ʿ��� ��ü�� ����.
Procedure�� ����ǰų�, �����¿��� ���α׷��� �����ų�� ȣ��ȴ�.
*******************************************************************************************************************/
VOID CSelLoginProcess::DeleteProc()
{
	m_xImageLogin.Destroy();
	
	g_xClientSocket.DisconnectToServer();
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
HRESULT CSelLoginProcess::DrawBillBoard(LPDIRECT3DDEVICE9 lpDevice, D3DVECTOR* pvTrans, D3DVECTOR* pvScale, /*D3DVECTOR vRot, */D3DMATERIAL9* pmtrl, /*FLOAT fRotRad, */IDirect3DTexture9* lpddsTextr, BYTE bBlendType)
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

//			lpDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, m_avBillBoard, sizeof(D3DVERTEX));

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
VOID CSelLoginProcess::RenderScene(INT nLoopTime)
{
	Clear(RGB(0, 0, 0));

	BOOL bBgOK = m_xImageLogin.NewSetIndex(0);
	if ( bBgOK )
	{
		// [DX9][T21] 背景 640x480 等比缩放铺满当前显示尺寸(800x600)，消除四周黑边
		g_xMainWnd.DrawWithImageForCompScaled(0, 0, m_xImageLogin.m_lpstNewCurrWilImageInfo->shWidth, m_xImageLogin.m_lpstNewCurrWilImageInfo->shHeight,
										(WORD*)(m_xImageLogin.m_pbCurrImage),
										g_xMainWnd.m_stDisplayInfo.wWidth, g_xMainWnd.m_stDisplayInfo.wHeight);
	}

	m_xCloseBtn.ShowGameBtn();

	for ( INT nX=0; nX<m_nAreaBtnCount; nX++ )
	{
		if ( m_ptAreaBtn[nX].x !=0 || m_ptAreaBtn[nX].y != 0 )
		{
			if ( m_szScreenName[nX][0] != NULL )
			{
				RECT rcTitle = { m_ptAreaBtn[nX].x + 5, m_ptAreaBtn[nX].y + 25, m_ptAreaBtn[nX].x + 50, m_ptAreaBtn[nX].y + 40 };
				DWORD dwFontColor = RGB(0, 0, 0);
				g_xMainWnd.PutsHan(NULL, rcTitle.left - 1, rcTitle.top, dwFontColor, RGB(0, 0, 0), m_szScreenName[nX]);
				g_xMainWnd.PutsHan(NULL, rcTitle.left + 1, rcTitle.top, dwFontColor, RGB(0, 0, 0), m_szScreenName[nX]);
				g_xMainWnd.PutsHan(NULL, rcTitle.left , rcTitle.top + 1, dwFontColor, RGB(0, 0, 0), m_szScreenName[nX]);
				g_xMainWnd.PutsHan(NULL, rcTitle.left , rcTitle.top - 1, dwFontColor, RGB(0, 0, 0), m_szScreenName[nX]);
				g_xMainWnd.PutsHan(NULL, rcTitle.left , rcTitle.top, RGB(240, 240, 240), RGB(0, 0, 0), m_szScreenName[nX]);
			}

			RECT rcBtn = { m_ptAreaBtn[nX].x, m_ptAreaBtn[nX].y, m_ptAreaBtn[nX].x + 33, m_ptAreaBtn[nX].y + 21 };
			if ( nX == m_nSelectedAreaBtn )
			{
				if ( m_xImageLogin.NewSetIndex(13) )
				{
					g_xMainWnd.DrawWithImageForComp(m_ptAreaBtn[nX].x - 3, m_ptAreaBtn[nX].y - 3, m_xImageLogin.m_lpstNewCurrWilImageInfo->shWidth, m_xImageLogin.m_lpstNewCurrWilImageInfo->shHeight,
													(WORD*)(m_xImageLogin.m_pbCurrImage));
				}
			}
			else if ( PtInRect(&rcBtn, m_ptMousePos) )
			{
				if ( m_xImageLogin.NewSetIndex(12) )
				{
					g_xMainWnd.DrawWithImageForComp(m_ptAreaBtn[nX].x, m_ptAreaBtn[nX].y, m_xImageLogin.m_lpstNewCurrWilImageInfo->shWidth, m_xImageLogin.m_lpstNewCurrWilImageInfo->shHeight,
													(WORD*)(m_xImageLogin.m_pbCurrImage));
				}
			}
			else
			{
				if ( m_xImageLogin.NewSetIndex(11) )
				{
					g_xMainWnd.DrawWithImageForComp(m_ptAreaBtn[nX].x, m_ptAreaBtn[nX].y, m_xImageLogin.m_lpstNewCurrWilImageInfo->shWidth, m_xImageLogin.m_lpstNewCurrWilImageInfo->shHeight,
													(WORD*)(m_xImageLogin.m_pbCurrImage));
				}
			}
		}

		if ( nX != m_nSelectedAreaBtn )
			continue; 

		if ( m_xImageLogin.NewSetIndex(1) )
		{
			if ( g_xMainWnd.Get3DDevice() )
			{
				D3DVECTOR vTrans;
				D3DMATRIX matTrans;
				D3DMATRIX matScale;
				D3DMATRIX matWorld;
				D3DMATRIX matWorldOriginal;
				D3DMATERIAL9 mtrl;				
				D3DVECTOR		vecTrans;
				D3DVECTOR		vecScale;

				vecTrans = D3DVECTOR(m_ptAreaBtn[nX].x - 45 , m_ptAreaBtn[nX].y - 45, 0);
				vecScale = D3DVECTOR((FLOAT)m_xImageLogin.m_lpstNewCurrWilImageInfo->shWidth, (FLOAT)m_xImageLogin.m_lpstNewCurrWilImageInfo->shHeight, 1);

				INT nFileType = 3, nFileIdx = 1, nImgIdx = 1;

				if ( !m_pObj )
				{
					m_pObj = new sTextureNode;

					sprintf( m_pObj->szKey, "%05d%05d%05d", 3, 1, 1);
					m_pObj->nFileType	= 3;
					m_pObj->nFileIdx	= 1;
					m_pObj->nImgIdx	= 1;
					m_pObj->pImg		= &m_xImageLogin;
					m_pObj->pSurface	= NULL;

					if ( m_pObj->pSurface == NULL )
					{
						if ( RestoreImg(m_pObj) == E_FAIL )
						{
							SAFE_DELETE(m_pObj);
						}
					}
				}

				if ( m_lpddsTextr == NULL )
					m_lpddsTextr = m_pObj->pSurface;

				D3DUtil_InitMaterial(mtrl, (FLOAT)((FLOAT)255/255.0f), (FLOAT)((FLOAT)255/255.0f), (FLOAT)((FLOAT)255/255.0f));
				mtrl.Diffuse.a = (0)/255.0f;
				g_xMainWnd.Get3DDevice()->SetMaterial(&mtrl);

				vTrans.x = (FLOAT) m_ptAreaBtn[nX].x-55+(FLOAT)((FLOAT)m_xImageLogin.m_lpstNewCurrWilImageInfo->shWidth /2.0f)+(FLOAT)m_xImageLogin.m_lpstNewCurrWilImageInfo->shPX-400.0f; 
				vTrans.y = (FLOAT)-(m_ptAreaBtn[nX].y-120)-(FLOAT)((FLOAT)m_xImageLogin.m_lpstNewCurrWilImageInfo->shHeight/2.0f)-(FLOAT)m_xImageLogin.m_lpstNewCurrWilImageInfo->shPY+300.0f; 
				vTrans.z = 0;

 				D3DVECTOR vNorm(0, 0, -1);
				D3DVERTEX			m_avMagic[4];
				m_avMagic[0]  = D3DVERTEX(D3DVECTOR(-0.5f, 0.5f, 0), vNorm, 0, 0);
				m_avMagic[1]  = D3DVERTEX(D3DVECTOR(-0.5f,-0.5f, 0), vNorm, 0, 1);
				m_avMagic[2]  = D3DVERTEX(D3DVECTOR( 0.5f, 0.5f, 0), vNorm, 1, 0);
				m_avMagic[3]  = D3DVERTEX(D3DVECTOR( 0.5f,-0.5f, 0), vNorm, 1, 1);
				
				if ( SUCCEEDED(g_xMainWnd.Get3DDevice()->BeginScene()) )
				{
					g_xMainWnd.Get3DDevice()->GetTransform(D3DTS_WORLD, &matWorldOriginal);

					D3DUtil_SetTranslateMatrix(matTrans, vTrans);
					D3DUtil_SetScaleMatrix(matScale, (FLOAT)m_xImageLogin.m_lpstNewCurrWilImageInfo->shWidth, (FLOAT)m_xImageLogin.m_lpstNewCurrWilImageInfo->shHeight, 1.0f);
					D3DMath_MatrixMultiply(matWorld, matScale, matTrans);
					g_xMainWnd.Get3DDevice()->SetTransform(D3DTS_WORLD, &matWorld);

					SetBlendRenderState(g_xMainWnd.Get3DDevice(), 2, &mtrl);

					g_xMainWnd.Get3DDevice()->SetTexture(0, m_lpddsTextr);
					g_xMainWnd.Get3DDevice()->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, m_avMagic, sizeof(D3DVERTEX));

					ResetBlendenderState(g_xMainWnd.Get3DDevice());

					g_xMainWnd.Get3DDevice()->SetTransform(D3DTS_WORLD, &matWorldOriginal);

					g_xMainWnd.Get3DDevice()->EndScene();
				}
			}
		}
	}

	m_xScrSelectMenu.ShowScrlBar(204, 84, m_nMenuStart, m_nSvrBtnCount);

	if ( m_nSvrBtnCount > 0 )
	{
		RECT rcTitle = { 28, 20, 200, 35 };
		g_xMainWnd.PutsHan(NULL, rcTitle, RGB(250, 250, 250), RGB(0, 0, 0), m_szServerName[m_nSelectedAreaBtn]);
		for ( INT nX =0; nX<m_nSvrBtnCount && nX<20; nX++ )
		{
			INT  nLine, nListCnt;
			CListNode < SERVERINFO > *pNode;
			LPSERVERINFO pstServerInfo = NULL;

			for ( pNode = m_xServerInfo.GetHead(), nListCnt = 0; pNode ; pNode = pNode->GetNext(), nListCnt++ )
			{
				if ( nListCnt == m_nMenuStart + nX )
				{
					pstServerInfo = pNode->GetData();
					break;
				}
			}
			

/*			if ( m_nSvrBtnCount > 20 && PtInRect(&m_rcSvrBtn[nX], m_ptMousePos) )
			{
				if ( nX < 20 )
				{
					if ( m_xImageLogin.NewSetIndex(9) )
					{
						g_xMainWnd.DrawWithImageForComp(m_rcSvrBtn[nX].left - 15, m_rcSvrBtn[nX].top - 5, m_xImageLogin.m_lpstNewCurrWilImageInfo->shWidth, m_xImageLogin.m_lpstNewCurrWilImageInfo->shHeight,
														(WORD*)(m_xImageLogin.m_pbCurrImage));
					}
				}
				else
				{
					if ( m_xImageLogin.NewSetIndex(10) )
					{
						g_xMainWnd.DrawWithImageForComp(m_rcSvrBtn[nX].left - 2, m_rcSvrBtn[nX].top - 5, m_xImageLogin.m_lpstNewCurrWilImageInfo->shWidth, m_xImageLogin.m_lpstNewCurrWilImageInfo->shHeight,
														(WORD*)(m_xImageLogin.m_pbCurrImage));
					}
				}

				g_xMainWnd.PutsHan(NULL, m_rcSvrBtn[nX], RGB(250, 250, 250), RGB(0, 0, 0), m_xServerInfo[nX].szServerName);
			}
			else
*/			if ( PtInRect(&m_rcSvrBtn[nX], m_ptMousePos) )
			{
				if ( pstServerInfo != NULL )
				{
					if ( strcmp(pstServerInfo->szServerName, "empty") != 0 )
					{
						if ( m_xImageLogin.NewSetIndex(8) )
						{
							g_xMainWnd.DrawWithImageForComp(m_rcSvrBtn[nX].left + 7, m_rcSvrBtn[nX].top - 5, m_xImageLogin.m_lpstNewCurrWilImageInfo->shWidth, m_xImageLogin.m_lpstNewCurrWilImageInfo->shHeight,
															(WORD*)(m_xImageLogin.m_pbCurrImage));
						}
						
						
						g_xMainWnd.PutsHan(NULL, m_rcSvrBtn[nX], RGB(250, 250, 250), RGB(0, 0, 0), pstServerInfo->szServerName);
					}
				}
//				g_xMainWnd.PutsHan(NULL, m_rcSvrBtn[nX], RGB(250, 250, 250), RGB(0, 0, 0), m_xServerInfo[nX].szServerName);
			}
			else
			{
/*				if ( nX < 20 && m_nSvrBtnCount > 20 )
				{
					if ( m_xImageLogin.NewSetIndex(5) )
					{
						g_xMainWnd.DrawWithImageForComp(m_rcSvrBtn[nX].left - 15, m_rcSvrBtn[nX].top - 5, m_xImageLogin.m_lpstNewCurrWilImageInfo->shWidth, m_xImageLogin.m_lpstNewCurrWilImageInfo->shHeight,
														(WORD*)(m_xImageLogin.m_pbCurrImage));
					}
				}
				else if ( m_nSvrBtnCount > 20 )
				{
					if ( m_xImageLogin.NewSetIndex(6) )
					{
						g_xMainWnd.DrawWithImageForComp(m_rcSvrBtn[nX].left - 2, m_rcSvrBtn[nX].top - 5, m_xImageLogin.m_lpstNewCurrWilImageInfo->shWidth, m_xImageLogin.m_lpstNewCurrWilImageInfo->shHeight,
														(WORD*)(m_xImageLogin.m_pbCurrImage));
					}
				}
				else
				{
*/
				if ( pstServerInfo != NULL )
				{
					if ( strcmp(pstServerInfo->szServerName, "empty") != 0 )
					{
						if ( m_xImageLogin.NewSetIndex(7) )
						{
							g_xMainWnd.DrawWithImageForComp(m_rcSvrBtn[nX].left + 7, m_rcSvrBtn[nX].top - 5, m_xImageLogin.m_lpstNewCurrWilImageInfo->shWidth, m_xImageLogin.m_lpstNewCurrWilImageInfo->shHeight,
															(WORD*)(m_xImageLogin.m_pbCurrImage));
						}
	//				}
	//				g_xMainWnd.DrawWithGDI(&m_rcSvrBtn[nX], NULL, RGB(200, 200, 200), 1);
					
						g_xMainWnd.PutsHan(NULL, m_rcSvrBtn[nX], RGB(250, 250, 250), RGB(0, 0, 0), pstServerInfo->szServerName);
					}
				}
//				g_xMainWnd.PutsHan(NULL, m_rcSvrBtn[nX], RGB(250, 250, 250), RGB(0, 0, 0), m_xServerInfo[nX].szServerName);			
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------------------------
// ������ �޽��� ���� �Լ�����.
//-----------------------------------------------------------------------------------------------------------------
/******************************************************************************************************************
�޽��� Procedure.
*******************************************************************************************************************/
LRESULT CSelLoginProcess::DefMainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch ( uMsg )
	{
		case WM_DESTROY:
			OnDestroy(wParam, lParam);
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
		case WM_MOUSEWHEEL:
			OnWheel(wParam, lParam);
	}
	return CWHDefProcess::DefMainWndProc(hWnd, uMsg, wParam, lParam);
}


/******************************************************************************************************************
���α׷� �����.
*******************************************************************************************************************/
LRESULT CSelLoginProcess::OnDestroy(WPARAM wParam, LPARAM lParam)
{
	WSACleanup();
	return 0L;
}


LRESULT	CSelLoginProcess::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
	POINT ptMouse;

	ptMouse.x = LOWORD(lParam);
	ptMouse.y = HIWORD(lParam);

	for ( INT nI=0; nI<m_nAreaBtnCount; nI++ )
	{
		if ( PtInRect(&m_rcAreaBtn[nI], ptMouse) )
		{
			m_nSelectedAreaBtn = nI;
			LoadServerList();
		}
	}

	for ( INT nI =0; nI<m_nSvrBtnCount; nI++ )
	{
		if ( PtInRect(&m_rcSvrBtn[nI], ptMouse) )
		{
			m_nSelectedSvrBtn = nI + m_nMenuStart;

			if ( m_nSelectedSvrBtn < m_nSerNameCnt )
				MakeIniFile();
		}
	}

	if ( m_xScrSelectMenu.OnLButtonDown(ptMouse) )
	{
		FLOAT	fScrlRate;

		fScrlRate	= m_xScrSelectMenu.GetScrlRate();
		m_nMenuStart = (INT)((m_nSvrBtnCount)*fScrlRate);

		if ( m_nMenuStart > m_nSvrBtnCount - 1 )
			m_nMenuStart = m_nSvrBtnCount - 1 ;

		return TRUE;
	}


	if ( m_xCloseBtn.OnLButtonDown(ptMouse) )		
		return TRUE;

	return 0L;
}


LRESULT	CSelLoginProcess::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
	POINT ptMouse;

	ptMouse.x = LOWORD(lParam);
	ptMouse.y = HIWORD(lParam);

	if ( m_xCloseBtn.OnLButtonUp(ptMouse) )
	{
		BOOL bRestart = FALSE;
		QuitProgram(bRestart);
	}
		
	if ( m_xScrSelectMenu.OnLButtonUp(ptMouse) )
		return FALSE;

	return 0L;
}


LRESULT	CSelLoginProcess::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
	m_ptMousePos.x = LOWORD(lParam);
	m_ptMousePos.y = HIWORD(lParam);

	if ( m_xScrSelectMenu.OnMouseMove(m_ptMousePos) )
	{
		FLOAT	fScrlRate;
		fScrlRate	= m_xScrSelectMenu.GetScrlRate();

		m_nMenuStart = (INT)((m_nSvrBtnCount)*fScrlRate);
		return TRUE;
	}

	return 0L;
}

LRESULT	CSelLoginProcess::OnWheel(WPARAM wParam, LPARAM lParam)
{
	INT nDelta;

	nDelta = ( (INT)wParam < 0 ) ? -1 : 1;
	if ( nDelta > 0 )
	{
		if ( m_nMenuStart > 0 )
			m_nMenuStart--;
	}
	else
	{
		if ( m_nMenuStart < m_nSerNameCnt - 1 )
			m_nMenuStart++;
	}

	return 0L;
}



VOID CSelLoginProcess::OnConnectToServer()
{
}


CHAR* CSelLoginProcess::OnMessageReceive(CHAR* pszBuf, INT nLen)
{
	return pszBuf;
}

HRESULT CSelLoginProcess::RestoreImg(sTextureNode* pstTextrNode)
{
    LPDIRECT3DDEVICE9	pd3dDevice = g_xMainWnd.Get3DDevice();
    DWORD				dwWidth, dwHeight;
    WORD*				pwSrc;

    if ( pstTextrNode == NULL || pd3dDevice == NULL )
        return E_FAIL;

    SAFE_RELEASE(pstTextrNode->pSurface);

    dwWidth = dwHeight = 0;
    pwSrc	= NULL;
    if ( pstTextrNode->pImg->NewSetIndex(pstTextrNode->nImgIdx) )
    {
        dwWidth  = pstTextrNode->pImg->m_lpstNewCurrWilImageInfo->shWidth;
        dwHeight = pstTextrNode->pImg->m_lpstNewCurrWilImageInfo->shHeight;
        pwSrc	 = (WORD*)pstTextrNode->pImg->m_pbCurrImage;
    }
    else
        return E_FAIL;

    if ( FAILED(pd3dDevice->CreateTexture(dwWidth, dwHeight, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &pstTextrNode->pSurface, NULL)) )
        return E_FAIL;

    if ( CopyDToS(dwWidth, dwHeight, pwSrc, pstTextrNode->pSurface) == E_FAIL )
    {
        SAFE_RELEASE(pstTextrNode->pSurface);
        return E_FAIL;
    }
    return S_OK;
}


HRESULT CSelLoginProcess::CopyDToSToS(DWORD dwWidth, DWORD dwHeight, WORD* pwSrc, IDirect3DTexture9* lpddsDst)
{
    return CopyDToS(dwWidth, dwHeight, pwSrc, lpddsDst);
}


HRESULT CSelLoginProcess::CopyDToS(DWORD dwWidth, DWORD dwHeight, WORD* pwSrc, IDirect3DTexture9* lpddsDst)
{
    if ( lpddsDst == NULL ) return E_FAIL;
    D3DLOCKED_RECT lr;
    if ( FAILED(lpddsDst->LockRect(0, &lr, NULL, 0)) ) return E_FAIL;

    for ( LONG y=0; y<(LONG)dwHeight; y++ ){
        DWORD* row=(DWORD*)((BYTE*)lr.pBits+(size_t)y*lr.Pitch);
        for ( LONG x=0; x<(LONG)dwWidth; x++ ) row[x]=0;
    }

    INT nWidthStart=0, nWidthEnd=0, nCurrWidth=0, nCntCopyWord=0, nYCnt=0, nLastWidth=0;
    for ( nYCnt = 0 ; nYCnt < (LONG)dwHeight ; nYCnt++ )
    {
        nWidthEnd += pwSrc[nWidthStart]; nWidthStart++;
        for ( INT x = nWidthStart; x < nWidthEnd ; )
        {
            if ( pwSrc[x] == 0xC0 )
            {
                x++; nCntCopyWord = pwSrc[x]; x++;
                nLastWidth = nCurrWidth; nCurrWidth += nCntCopyWord;
                DWORD* row=(DWORD*)((BYTE*)lr.pBits+(size_t)nYCnt*lr.Pitch);
                for ( INT k=0;k<nCntCopyWord;k++ ){ INT sx=nLastWidth+k; if(sx<(LONG)dwWidth) row[sx]=0; }
            }
            else if ( pwSrc[x] == 0xC1 || pwSrc[x] == 0xC2 || pwSrc[x] == 0xC3 )
            {
                x++; nCntCopyWord = pwSrc[x]; x++;
                nLastWidth = nCurrWidth; nCurrWidth += nCntCopyWord;
                DWORD* row=(DWORD*)((BYTE*)lr.pBits+(size_t)nYCnt*lr.Pitch);
                for ( INT k=0;k<nCntCopyWord;k++ ){
                    INT sx=nLastWidth+k; if(sx>=(LONG)dwWidth) break;
                    WORD px=pwSrc[x+k]; DWORD c=0;
                    if(px!=0x0000){ WORD r=(px>>11)&31,g=(px>>5)&63,b=px&31; BYTE r8=(BYTE)((r<<3)|(r>>2)),g8=(BYTE)((g<<2)|(g>>4)),b8=(BYTE)((b<<3)|(b>>2)); c=0xFF000000|(r8<<16)|(g8<<8)|b8; }
                    row[sx]=c;
                }
                x += nCntCopyWord;
            }
        }
        nWidthEnd++; nWidthStart=nWidthEnd; nCurrWidth=0;
    }
    lpddsDst->UnlockRect(0);
    return S_OK;
}

VOID CSelLoginProcess::LoadServerList()
{
	CHAR szGroup[MAX_PATH], szData[MAX_PATH], szKey[MAX_PATH];
	char *pszLocation;
	INT nLineLen = 0, nI = 0;

	m_nSerNameCnt = 0;
	m_nSelectedSvrBtn = -1;
	m_nMenuStart = 0;

	sprintf(szGroup, "ServerGroup%d", m_nSelectedAreaBtn+1);

//	for ( INT nI=0; nI<m_nSvrBtnCount; nI++ )
//		ZeroMemory(&m_xServerInfo[nI], sizeof(SERVERINFO));
	m_xServerInfo.ClearAll();
	
	m_nSvrBtnCount = GetPrivateProfileInt( szGroup, "Count", 0, m_szProfileFullName );

	if ( ( m_nSvrBtnCount == 0 ) )
		return;

//	if ( m_nSvrBtnCount >= 20 )
//	{
		for ( INT nI =0; nI<m_nSvrBtnCount; nI++ )
		{
			sprintf(szKey, "Server%d", nI+1);

			if ( GetPrivateProfileString ( szGroup, szKey, "", szData, sizeof(szData), m_szProfileFullName ) )
			{
				CHAR* pszServer = NULL;
				CHAR* pszLogin = NULL;
				CHAR* pszPatch = NULL;

				SERVERINFO *pObj = new SERVERINFO;

				// �����׷� �̸��� �̾Ƴ��� �κ�
				if ( pszLocation = strchr(szData, ',') )
				{
					*pszLocation ++ = '\0';		//szData; -> �̸�
//					strcpy( m_xServerInfo[nI].szServerName, szData );

					nLineLen = strlen(szData)+1;
					pszServer = new CHAR[nLineLen];
					ZeroMemory(pszServer, nLineLen);

					sprintf(pszServer, szData);
					sprintf(pObj->szServerName, pszServer);
//					pObj->szServerName = pszServer;
					
//					if ( nI>=20 )
//						SetRect(&m_rcSvrBtn[nI], 115, 70+25*(nI-20), 200, 85+25*(nI-20));
//					else
//						SetRect(&m_rcSvrBtn[nI], 10, 70+25*nI, 200, 85+25*nI);

					if ( nI<20 )
						SetRect(&m_rcSvrBtn[nI], 10, 70+25*nI, 200, 85+25*nI);

					strcpy( szData, pszLocation );
				}

				if ( pszLocation = strchr(szData, ',') )
				{
					*pszLocation ++ = '\0';		//szData; -> �̸�

//					strcpy( m_xServerInfo[nI].szLoginSectionName, szData );

					nLineLen = strlen(szData)+1;
					pszLogin = new CHAR[nLineLen];
					ZeroMemory(pszLogin, nLineLen);

					sprintf(pszLogin, szData);

					
					sprintf(pObj->szLoginSectionName , pszLogin);
//					pObj->szLoginSectionName = pszLogin;
					
//					strcpy( m_xServerInfo[nI].szPatchSectionName, pszLocation );

					nLineLen = strlen(pszLocation)+1;
					pszPatch = new CHAR[nLineLen];
					ZeroMemory(pszPatch, nLineLen);

					sprintf(pszPatch, pszLocation);

					sprintf(pObj->szPatchSectionName, pszPatch);
//					pObj->szLoginSectionName = pszPatch;
				}

				m_nSerNameCnt++;
				if ( !m_xServerInfo.Insert(pObj) )
				{
					delete[] pszServer;
					delete[] pszLogin;
					delete[] pszPatch;
					CHAR pszError[MAX_PATH];
					sprintf(pszError, "SelLogin.txt File Error");
					MessageBox(NULL, (char*)pszError, "[Error] - Legend of Mir III", MB_ICONERROR);
	//				DelAllMIF();
					return;
				}
			}
		}
//	}
//	else
//	{
/*		for ( INT nI =0; nI<m_nSvrBtnCount; nI++ )
		{
			sprintf(szKey, "Server%d", nI+1);

			if ( GetPrivateProfileString ( szGroup, szKey, "", szData, sizeof(szData), m_szProfileFullName ) )
			{
				// �����׷� �̸��� �̾Ƴ��� �κ�
				if ( pszLocation = strchr(szData, ',') )
				{
					*pszLocation ++ = '\0';		//szData; -> �̸�
					strcpy( m_xServerInfo[nI].szServerName, szData );

					SetRect(&m_rcSvrBtn[nI], 50, 70+25*nI, 165, 85+25*nI);

					strcpy( szData, pszLocation );
				}

				if ( pszLocation = strchr(szData, ',') )
				{
					*pszLocation ++ = '\0';		//szData; -> �̸�
					strcpy( m_xServerInfo[nI].szLoginSectionName, szData );
					strcpy( m_xServerInfo[nI].szPatchSectionName, pszLocation );
				}

			}
		}
*/
//	}
}

VOID CSelLoginProcess::MakeIniFile()
{
	INT  nLine, nListCnt;
	CListNode < SERVERINFO > *pNode;
	LPSERVERINFO pstServerInfo = NULL;

	for ( pNode = m_xServerInfo.GetHead(), nListCnt = 0; pNode ; pNode = pNode->GetNext(), nListCnt++ )
	{
		if ( nListCnt < m_nSelectedSvrBtn )
			continue;

		pstServerInfo = pNode->GetData();
		break;
	}

	if ( strcmp(pstServerInfo->szLoginSectionName, "empty") == 0 )
		return; 

	GetPrivateProfileString( pstServerInfo->szLoginSectionName, "ServerAddr", "", 
									m_MirInfo.szLoginSvrAddr, sizeof( m_MirInfo.szLoginSvrAddr ), m_szProfileFullName );
	GetPrivateProfileString( pstServerInfo->szLoginSectionName, "Param1", "", 
									m_MirInfo.szParam1, sizeof( m_MirInfo.szParam1 ), m_szProfileFullName );
	GetPrivateProfileString( pstServerInfo->szLoginSectionName, "Param2", "", 
									m_MirInfo.szParam2, sizeof( m_MirInfo.szParam2 ), m_szProfileFullName );
	GetPrivateProfileString( pstServerInfo->szLoginSectionName, "Param3", "", 
									m_MirInfo.szParam3, sizeof( m_MirInfo.szParam3 ), m_szProfileFullName );
	GetPrivateProfileString( pstServerInfo->szLoginSectionName, "ServerList", "", 
									m_MirInfo.szServerList, sizeof( m_MirInfo.szServerList ), m_szProfileFullName );

	WriteMirProfile();

	GetPrivateProfileString( pstServerInfo->szPatchSectionName, "ServerIP", "", 
									m_MirPatchInfo.szServerIP, sizeof( m_MirPatchInfo.szServerIP ), m_szProfileFullName );
	GetPrivateProfileString( pstServerInfo->szPatchSectionName, "BaseDirectory", "", 
									m_MirPatchInfo.szBaseDirectory, sizeof( m_MirPatchInfo.szBaseDirectory ), m_szProfileFullName );
/*
	GetPrivateProfileString( m_xServerInfo[m_nSelectedSvrBtn].szLoginSectionName, "ServerAddr", "", 
									m_MirInfo.szLoginSvrAddr, sizeof( m_MirInfo.szLoginSvrAddr ), m_szProfileFullName );
	GetPrivateProfileString( m_xServerInfo[m_nSelectedSvrBtn].szLoginSectionName, "Param1", "", 
									m_MirInfo.szParam1, sizeof( m_MirInfo.szParam1 ), m_szProfileFullName );
	GetPrivateProfileString( m_xServerInfo[m_nSelectedSvrBtn].szLoginSectionName, "Param2", "", 
									m_MirInfo.szParam2, sizeof( m_MirInfo.szParam2 ), m_szProfileFullName );
	GetPrivateProfileString( m_xServerInfo[m_nSelectedSvrBtn].szLoginSectionName, "Param3", "", 
									m_MirInfo.szParam3, sizeof( m_MirInfo.szParam3 ), m_szProfileFullName );
	GetPrivateProfileString( m_xServerInfo[m_nSelectedSvrBtn].szLoginSectionName, "ServerList", "", 
									m_MirInfo.szServerList, sizeof( m_MirInfo.szServerList ), m_szProfileFullName );

	WriteMirProfile();

	GetPrivateProfileString( m_xServerInfo[m_nSelectedSvrBtn].szPatchSectionName, "ServerIP", "", 
									m_MirPatchInfo.szServerIP, sizeof( m_MirPatchInfo.szServerIP ), m_szProfileFullName );
	GetPrivateProfileString( m_xServerInfo[m_nSelectedSvrBtn].szPatchSectionName, "BaseDirectory", "", 
									m_MirPatchInfo.szBaseDirectory, sizeof( m_MirPatchInfo.szBaseDirectory ), m_szProfileFullName );
*/
	WriteMirPatchProfile();

	BOOL bRestart = TRUE;
	QuitProgram(bRestart);
}

VOID CSelLoginProcess::WriteMirProfile()
{
	char		szMirProfileName[256];
	char		*pszServer = m_MirInfo.szServerList;
	UINT		nIndex = 1;
	char		szCount[8];

	GetCurrentDirectory( sizeof( szMirProfileName ), szMirProfileName );
	lstrcat( szMirProfileName, _MIRPROFILE_FILENAME );

	WritePrivateProfileString( "Initial", "ServerAddr", m_MirInfo.szLoginSvrAddr, szMirProfileName );
	WritePrivateProfileString( "Initial", "Param1", m_MirInfo.szParam1, szMirProfileName );
	WritePrivateProfileString( "Initial", "Param2", m_MirInfo.szParam2, szMirProfileName );
	WritePrivateProfileString( "Initial", "Param3", m_MirInfo.szParam3, szMirProfileName );

	while ( pszServer = GetCaptionName( nIndex, pszServer, szMirProfileName ) )
		nIndex++;
	
	_itoa( nIndex, szCount, 10 );

	WritePrivateProfileString( "Server", "ServerCount", szCount, szMirProfileName );
}

VOID CSelLoginProcess::WriteMirPatchProfile()
{
	char szMirProfileName[256];

	GetCurrentDirectory( sizeof( szMirProfileName ), szMirProfileName );
	lstrcat( szMirProfileName, _MIRPATCHPROFILE_FILENAME );

	WritePrivateProfileString( "PATCH_FTP", "ServerIP", m_MirPatchInfo.szServerIP, szMirProfileName );
	WritePrivateProfileString( "PATCH_FTP", "BaseDirectory", m_MirPatchInfo.szBaseDirectory, szMirProfileName );
	WritePrivateProfileString( "PATCH_FTP", "PORT", "21", szMirProfileName );
	WritePrivateProfileString( "PATCH_FTP", "userid", "anonymous", szMirProfileName );
	WritePrivateProfileString( "PATCH_FTP", "passwd", "user@host.com", szMirProfileName );
	WritePrivateProfileString( "PATCH_FTP", "ListFile", "!plist.lst", szMirProfileName );
}


char *CSelLoginProcess::GetCaptionName( int nIndex, char *pszText, char *pszFileName )
{
	char *pszStart	= strchr( pszText, '(' );
	char *pszEnd	= NULL;
	char *pszDevide	= NULL;
	char szKey[32];

	if ( !pszStart ) return NULL;

	pszEnd = strchr( ++pszStart, ')' );

	if ( pszEnd )
	{
		pszDevide = strchr( pszStart, '/' );

		if ( !pszDevide ) return NULL;

		*pszDevide++	= '\0';
		*pszEnd++		= '\0';

		sprintf( szKey, "server%dcaption", nIndex);
		WritePrivateProfileString( "Server", szKey, pszStart, pszFileName );
		sprintf( szKey, "server%dname", nIndex);
		WritePrivateProfileString( "Server", szKey, pszDevide, pszFileName );

		return strchr( pszEnd, '(' );
	}

	return NULL;
}

VOID CSelLoginProcess::QuitProgram(BOOL bRestart)
{
	if ( bRestart )
	{
#ifdef _TEST_SERVER
#else
	if ( g_execCmds.bNotPatchd == TRUE )
	{
		char szFileName[MAX_PATH];

		GetCurrentDirectory(MAX_PATH, szFileName);

		strcat(szFileName, ".\\Mir3Patch.exe");
	
		STARTUPINFO			si = {sizeof( STARTUPINFO ), 0,};
		PROCESS_INFORMATION	pi = {0,};

		if ( !CreateProcess( szFileName, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi ) )
		{
			MessageBox(NULL, "Patch Error", "Legend Of Mir 3", MB_OK|MB_ICONSTOP);
		}

		SendMessage(g_xMainWnd.GetSafehWnd(), WM_DESTROY, NULL, NULL);
	}
	else
	{
		SetNextProc();
	}
#endif
	}
	else
	{
		DeleteProc();
		SendMessage(g_xMainWnd.GetSafehWnd(), WM_DESTROY, NULL, NULL);
	}
}

VOID CSelLoginProcess::SetNextProc()
{
	DeleteProc();
	g_xClientSocket.m_pxDefProc = g_xMainWnd.m_pxDefProcess = &g_xLoginProc;
	g_xLoginProc.Load();
	g_bProcState = _LOGIN_PROC;
	if ( g_execCmds.bFullScreen )
		g_xMainWnd.ResetDXG(640, 480, 16, _DXG_SCREENMODE_FULLSCREEN, _DXG_DEVICEMODE_PRIMARY|_DXG_DEVICEMODE_D3D);
	else
		g_xMainWnd.ResetDXG(640, 480, 16, _DXG_SCREENMODE_WINDOW, _DXG_DEVICEMODE_PRIMARY|_DXG_DEVICEMODE_D3D);
}
