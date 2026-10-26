/******************************************************************************************************************
                                                                                                                   
	????:																											
																													
	?????:																											
	?????:																											
																													
	[????][??????] : ???? ????																						
                                                                                                                   
*******************************************************************************************************************/


#include "stdafx.h"
#include "WHEngine.h"
#include "EffectLayer.h"
#include "../AVI/Avi.h"
#include <windows.h>


/*********************************************************************************************

	// ORZ: ???? ?????? ?????

 *********************************************************************************************/
#define ALPHA_565_MASK1		0xf81f07e0
#define ALPHA_565_MASK2		0x07e0f81f


/*********************************************************************************************

	// ORZ: ?????? ????? ??? ????

	- ??? ???1?71?1?771?1?71?1?777!

 *********************************************************************************************/
#define IS_OUTSIDE( nX, nY, nXSize, nYSize )\
	( nX > m_stDisplayInfo.wWidth || nX + nXSize < 0 || nY > m_stDisplayInfo.wHeight || nY + nYSize < 0 )

#define CREATE_CLIP_REGION( nX, nY, nXSize, nYSize )\
	if ( nX + nXSize > m_stDisplayInfo.wWidth )	{ rc.right	= m_stDisplayInfo.wWidth -nX; }\
	else						{ rc.right	= nXSize; }\
	if ( nX < 0 )				{ rc.left	= -nX; nX = 0; }\
	else						{ rc.left	= 0; }\
	if ( nY + nYSize > m_stDisplayInfo.wHeight )	{ rc.bottom	= m_stDisplayInfo.wHeight - nY; }\
	else						{ rc.bottom	= nYSize; }\
	if ( nY < 0 )				{ rc.top	= -nY; nY = 0; }\
	else						{ rc.top	= 0; }\
\
	imgW = rc.right - rc.left;\
	imgH = rc.bottom - rc.top;

#define SURFACE_LOCK( surface )\
	ddsd.dwSize = sizeof( DDSURFACEDESC2 );\
	if ( FAILED( surface->Lock( NULL, &ddsd, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL ) ) ) return FALSE;\
	ddsd.lPitch >>= 1;

#define SURFACE_UNLOCK( surface )\
	surface->Unlock( NULL );



/*********************************************************************************************

	// ORZ: ?????? ?��??? ???

 *********************************************************************************************/
__forceinline void putAlpha32Pixel( WORD *dst, WORD src, byte srclevel )
{
	static DWORD src1, src2, dst1, dst2;

	int dstlevel = 32 - srclevel;

	src1 = src;
	src2 = src1;
	
	src1 &=	 ALPHA_565_MASK1;
	src1 >>= 5;
	src1 *=  srclevel;
	src1 &=  ALPHA_565_MASK1;
	
	src2 &=  ALPHA_565_MASK2;
	src2 *=  srclevel;
	src2 >>= 5;
	src2 &=  ALPHA_565_MASK2;
	src2 |=  src1;
	
	dst1 = *dst;
	dst2 = dst1;
	
	dst1 &=  ALPHA_565_MASK1;
	dst1 >>= 5;
	dst1 *=  dstlevel;
	dst1 &=  ALPHA_565_MASK1;
	
	dst2 &=  ALPHA_565_MASK2;		
	dst2 *=  dstlevel;
	dst2 >>= 5;
	dst2 &=  ALPHA_565_MASK2;
	dst2 |=  dst1;
	
	*dst++ = (WORD) (dst2 + src2);
}


__forceinline void putAlpha32( WORD *dst, WORD *src, int len, byte srclevel )
{
	static DWORD src1, src2, dst1, dst2;

	int dstlevel = 32 - srclevel;

	if ( len & 1 )
	{
		src1 = *src++;
		src2 = src1;
		
		src1 &=	 ALPHA_565_MASK1;
		src1 >>= 5;
		src1 *=  srclevel;
		src1 &=  ALPHA_565_MASK1;

		src2 &=  ALPHA_565_MASK2;
		src2 *=  srclevel;
		src2 >>= 5;
		src2 &=  ALPHA_565_MASK2;
		src2 |=  src1;
		
		dst1 = *dst;
		dst2 = dst1;

		dst1 &=  ALPHA_565_MASK1;
		dst1 >>= 5;
		dst1 *=  dstlevel;
		dst1 &=  ALPHA_565_MASK1;

		dst2 &=  ALPHA_565_MASK2;		
		dst2 *=  dstlevel;
		dst2 >>= 5;
		dst2 &=  ALPHA_565_MASK2;
		dst2 |=  dst1;
		
		*dst++ = (WORD) (dst2 + src2);
	}

	for ( int i = 0; i < len >> 1; i++ )
	{
		src1 = *((DWORD *) src);
		src2 = src1;
		
		src1 &=	 ALPHA_565_MASK1;
		src1 >>= 5;
		src1 *=  srclevel;
		src1 &=  ALPHA_565_MASK1;

		src2 &=  ALPHA_565_MASK2;
		src2 *=  srclevel;
		src2 >>= 5;
		src2 &=  ALPHA_565_MASK2;
		src2 |=  src1;
		
		dst1 = *((DWORD *) dst);
		dst2 = dst1;

		dst1 &=  ALPHA_565_MASK1;
		dst1 >>= 5;
		dst1 *=  dstlevel;
		dst1 &=  ALPHA_565_MASK1;

		dst2 &=  ALPHA_565_MASK2;			
		dst2 *=  dstlevel;
		dst2 >>= 5;
		dst2 &=  ALPHA_565_MASK2;
		dst2 |=  dst1;
		
		*((DWORD *) dst) = dst2 + src2;

		dst += 2;
		src += 2;
	}
}


/******************************************************************************************************************

	CWHDXGraphicWindow Functions

*******************************************************************************************************************/


static BYTE						g_bNumDevices = 0;
static DXG_ENUM_DEVICEINFO		g_stDXGEnumDeviceInfo[_MAX_DEVICES];

static BOOL		WINAPI			DXGDriverEnumCallbackEx(GUID* pGUID, LPSTR szDesc, LPSTR szName, LPVOID pContext, HMONITOR hm);
static HRESULT	WINAPI			DXGEnumZBufferFormatsCallback(DDPIXELFORMAT* pddpf, VOID* pContext);


// DX ???? ��???.
extern DWORD GetDXVersion();



CWHDXGraphicWindow* CWHDXGraphicWindow::m_pxDXGWnd = NULL;

CWHDXGraphicWindow* GetDXGWindowPtr()
{
	return CWHDXGraphicWindow::m_pxDXGWnd;
}


/******************************************************************************************************************

	????? : CWHDXGraphicWindow::CWHDXGraphicWindow()

	????? : 
	????? : 

	????   : 
	???   : WORD wWidth
	         WORD wHeight
	         WORD wBpp
	???1?71?1?771?1?71?1?777   : 

	[????][??????] : ????????

*******************************************************************************************************************/
CWHDXGraphicWindow::CWHDXGraphicWindow(WORD wWidth, WORD wHeight, WORD wBpp)
{
	m_pD3D				= NULL;
	m_pd3dDevice		= NULL;
	m_pd3dBackBuffer	= NULL;
	m_pd3dPresentTex	= NULL;
	m_pSprite			= NULL;
	m_pFont				= NULL;
	m_pddsBackBuffer	= NULL;
	m_pSoftBackBuffer	= NULL;
	m_dwSoftBufSize		= 0;
	m_bSoftBufLocked	= FALSE;
	m_hDefGameFont		= NULL;
	ZeroMemory(m_pszDefFont, MAX_PATH);
	m_stDisplayInfo.wWidth	= wWidth;
	m_stDisplayInfo.wHeight	= wHeight;
	m_stDisplayInfo.wBPP	= wBpp;
	m_bScreenModeFlag		= _DXG_SCREENMODE_WINDOW;
	m_bDeviceModeFlag		= _DXG_DEVICEMODE_PRIMARY;
	SetRect(&m_rcWindow, 0, 0, 0, 0);
	ZeroMemory(&m_stBitsMaskInfo, sizeof(DXG_MASKINFO));
	m_pxDXGWnd = this;
	m_bIsWindowActive = FALSE;
	m_bIsWindowReady = FALSE;
	m_bIsActiveChanged = FALSE;
	m_dwTextureTotal	= 0;
	m_dwTextureFree		= 0;
	m_dwVideoTotal		= 0;
	m_dwVideoFree		= 0;
	m_pxDefProcess = NULL;
}


VOID CWHDXGraphicWindow::UsedAndFreeMemoryCheck()
{
	// DX9: ������Ⱦ�����ѯ�Դ�
	m_dwTextureTotal	= 0;
	m_dwTextureFree		= 0;
	m_dwVideoTotal		= 0;
	m_dwVideoFree		= 0;
} 


/******************************************************************************************************************

	????? : CWHDXGraphicWindow::~CWHDXGraphicWindow()

	????? : 
	????? : 

	????   : 
	???1?71?1?771?1?71?1?777   : 

	[????][??????] : ????????

*******************************************************************************************************************/
CWHDXGraphicWindow::~CWHDXGraphicWindow()
{
	FreeDXGEnumModeResources();
    DestroyDXGObjects();

	m_pxDefProcess = NULL;
	m_pxDXGWnd = NULL;
}


/******************************************************************************************************************

	????? : CWHDXGraphicWindow::Create()

	????? : 
	????? : 

	????   : 
	???   : LPTSTR lpCaption
	         CHAR *pszMenuName
	         BYTE bScreenModeFlag
	         BYTE bDeviceModeFlag
	???1?71?1?771?1?71?1?777   : BOOL 

	[????][??????] : ????????

*******************************************************************************************************************/
BOOL CWHDXGraphicWindow::Create(HINSTANCE hInst, LPTSTR lpCaption, CHAR *pszMenuName, CHAR* pszIconName, BYTE bScreenModeFlag, BYTE bDeviceModeFlag, BOOL bLarge)
{
	DWORD	dwStyle;
	if ( !CWHWindow::Create(hInst, lpCaption, pszMenuName, pszIconName, bLarge) )
	{
		MessageBox(m_hWnd, TEXT("[CWHDXGraphicWindow::Create]") TEXT("Window create failed."), "MirDXG", MB_ICONERROR | MB_OK);
		return FALSE;
	}
	m_bScreenModeFlag		= bScreenModeFlag;
	m_bDeviceModeFlag		= bDeviceModeFlag;
	if ( m_bScreenModeFlag & _DXG_SCREENMODE_WINDOW )
	{
		dwStyle  = GetWindowStyle(m_hWnd);
		dwStyle &= ~WS_POPUP;
		dwStyle |= WS_OVERLAPPED | WS_CAPTION | WS_MINIMIZEBOX;
		SetWindowLong(m_hWnd, GWL_STYLE, dwStyle);
	}
	else
	{
		dwStyle  = GetWindowStyle(m_hWnd);
		dwStyle &= ~dwStyle;
		dwStyle |= WS_POPUP | WS_SYSMENU;
		SetWindowLong(m_hWnd, GWL_STYLE, dwStyle);
		ShowWindow(m_hWnd, SW_HIDE);
	}
	if ( SUCCEEDED(CreateDXG()) )
	{
		m_bIsWindowReady = TRUE;
		SetDefFont();
		return TRUE;
	}
	MessageBox(m_hWnd, TEXT("[CWHDXGraphicWindow::Create]") TEXT("DirectGraphic create failed."), "MirDXG", MB_ICONERROR | MB_OK);
	return FALSE;
}


/******************************************************************************************************************

	????? : CWHDXGraphicWindow::MainWndProcDXG()

	????? : 
	????? : 

	????   : 
	???   : HWND hWnd
	         UINT uMsg
	         WPARAM wParam
	         LPARAM lParam
	???1?71?1?771?1?71?1?777   : LRESULT 

	[????][??????] : ????????

*******************************************************************************************************************/
LRESULT CWHDXGraphicWindow::MainWndProcDXG(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch (uMsg)
	{
		case WM_CREATE:
		{
			RegisterHotKey( hWnd, 0, MOD_ALT, VK_F4 );
//			RegisterHotKey( hWnd, 1, MOD_ALT, VK_TAB );
			break;
		}

		case WM_HOTKEY:
		{
			switch ( wParam )
			{
			case 0:		// F4
				SendMessage(hWnd, WM_DESTROY, 0, 0);
				break;
//			case 1:		// TAB
//				break;
			}
			break;
		}

		case WM_SYSKEYDOWN:
			return OnSysKeyDown(wParam, lParam);

		case WM_MOVE:
			return OnMove(wParam, lParam);

		case WM_SIZE:
			return OnSize(wParam, lParam);

		case WM_DESTROY:
		{
			UnregisterHotKey( hWnd, 0 );
			UnregisterHotKey( hWnd, 1 );
			return OnDestroy();
		}

        case WM_SETCURSOR:
			return OnSetCursor();   

 //      case WM_GETMINMAXINFO:			
//			return OnGetMinMaxInfo(wParam, lParam);

		default:
			break;
	}

    return CWHWindow::MainWndProc(hWnd, uMsg, wParam, lParam);
}


LRESULT CWHDXGraphicWindow::OnGetMinMaxInfo(WPARAM wParam, LPARAM lParam)
{
	if ( m_hWnd )
	{
		MINMAXINFO* pMinMax = (MINMAXINFO*) lParam;

		DWORD dwFrameWidth    = GetSystemMetrics( SM_CXSIZEFRAME );
		DWORD dwFrameHeight   = GetSystemMetrics( SM_CYSIZEFRAME );
		DWORD dwMenuHeight;
		if( GetMenu( m_hWnd ) )
			dwMenuHeight    = GetSystemMetrics( SM_CYMENU );
		else
			dwMenuHeight    = 0;					

		DWORD dwCaptionHeight = GetSystemMetrics( SM_CYCAPTION );

		pMinMax->ptMinTrackSize.x = m_stDisplayInfo.wWidth  + dwFrameWidth * 2;
		pMinMax->ptMinTrackSize.y = m_stDisplayInfo.wHeight + dwFrameHeight * 2 + 
									dwMenuHeight + dwCaptionHeight;

		pMinMax->ptMaxTrackSize.x = pMinMax->ptMinTrackSize.x;
		pMinMax->ptMaxTrackSize.y = pMinMax->ptMinTrackSize.y;
	}

    return 0L;
}


LRESULT CWHDXGraphicWindow::OnSetCursor()
{
    SetCursor(LoadCursor(NULL, IDC_ARROW));
    return 0L;
}



/******************************************************************************************************************

	????? : CWHDXGraphicWindow::OnSize()

	????? : 
	????? : 

	????   : 
	???   : WPARAM wParam
	         LPARAM lParam
	???1?71?1?771?1?71?1?777   : LRESULT 

	[????][??????] : ????????

*******************************************************************************************************************/
LRESULT CWHDXGraphicWindow::OnSize(WPARAM wParam, LPARAM lParam)
{
    if ( SIZE_MAXHIDE == wParam || SIZE_MINIMIZED == wParam )
        m_bIsWindowActive = FALSE;
    else
	{
		if ( m_bIsWindowActive == FALSE )
		{
			m_bIsActiveChanged = TRUE;
		}
        m_bIsWindowActive = TRUE;
	}

	UpdateBoundsWnd();
 	return 0L;
}



/******************************************************************************************************************

	????? : CWHDXGraphicWindow::OnMove()

	????? : 
	????? : 

	????   : 
	???   : WPARAM wParam
	         LPARAM lParam
	???1?71?1?771?1?71?1?777   : LRESULT 

	[????][??????] : ????????

*******************************************************************************************************************/
LRESULT CWHDXGraphicWindow::OnMove(WPARAM wParam, LPARAM lParam)
{
	UpdateBoundsWnd();
	return 0L;
}


/******************************************************************************************************************

	????? : CWHDXGraphicWindow::OnDestroy()

	????? : 
	????? : 

	????   : 
	???1?71?1?771?1?71?1?777   : BOOL 

	[????][??????] : ????????

*******************************************************************************************************************/
LRESULT CWHDXGraphicWindow::OnDestroy()
{
	return CWHWindow::OnDestroy();
}


/******************************************************************************************************************

	????? : CWHDXGraphicWindow::OnSysKeyDown()

	????? : 
	????? : 

	????   : 
	???   : WPARAM wParam
	         LPARAM lParam
	???1?71?1?771?1?71?1?777   : LRESULT 

	[????][??????] : ????????

*******************************************************************************************************************/
LRESULT CWHDXGraphicWindow::OnSysKeyDown(WPARAM wParam, LPARAM lParam)
{
	if ( wParam == VK_RETURN )
	{
		m_bIsWindowReady = FALSE;

		if ( m_bScreenModeFlag & _DXG_SCREENMODE_WINDOW )
		{
			if ( FAILED(ResetDXG(_SCREEN_WIDTH, _SCREEN_HEIGHT, _SCREEN_BPP, _DXG_SCREENMODE_FULLSCREEN, _DXG_DEVICEMODE_PRIMARY|_DXG_DEVICEMODE_D3D)) )
			{
				MessageBox(m_hWnd, TEXT("DirectDraw RESETUP failed!!!") TEXT("The Program will now exit."), TEXT("MirDXG"), MB_ICONERROR | MB_OK);
			}
		}
		else
		{
			if ( FAILED(ResetDXG(_SCREEN_WIDTH, _SCREEN_HEIGHT, _SCREEN_BPP, _DXG_SCREENMODE_WINDOW, _DXG_DEVICEMODE_PRIMARY|_DXG_DEVICEMODE_D3D)) )
			{
				MessageBox(m_hWnd, TEXT("DirectDraw RESETUP failed!!!") TEXT("The Program will now exit."), TEXT("MirDXG"), MB_ICONERROR | MB_OK);
			}
		}

		m_bIsWindowReady = TRUE;
	}

	return 0L;
}


/*******************************************************************************************************************/

BYTE CWHDXGraphicWindow::FindDriverAndDevice()
{
	// DX9: ���� DirectDraw ö��, ֱ��ʹ����������
	return 0;
}


/*******************************************************************************************************************/

HRESULT CWHDXGraphicWindow::DestroyDXGObjects()
{
	if ( m_pd3dDevice ) { m_pd3dDevice->Release(); m_pd3dDevice = NULL; }
	if ( m_pd3dBackBuffer ) { m_pd3dBackBuffer->Release(); m_pd3dBackBuffer = NULL; }
	if ( m_pd3dPresentTex ) { m_pd3dPresentTex->Release(); m_pd3dPresentTex = NULL; }
	if ( m_pSprite ) { m_pSprite->Release(); m_pSprite = NULL; }
	if ( m_pFont ) { m_pFont->Release(); m_pFont = NULL; }
	if ( m_pddsBackBuffer ) { delete m_pddsBackBuffer; m_pddsBackBuffer = NULL; }
	if ( m_pSoftBackBuffer ) { free(m_pSoftBackBuffer); m_pSoftBackBuffer = NULL; }
	m_dwSoftBufSize = 0;
	m_bSoftBufLocked = FALSE;
	if ( m_pD3D ) { m_pD3D->Release(); m_pD3D = NULL; }
	if ( m_hDefGameFont != NULL )
	{
		DeleteObject(m_hDefGameFont);
		m_hDefGameFont = NULL;
	}
	return S_OK;
}


/*******************************************************************************************************************/

VOID CWHDXGraphicWindow::FreeDXGEnumModeResources()
{
	// DX9: �� DirectDraw ö����Դ
}


/*******************************************************************************************************************/

HRESULT CWHDXGraphicWindow::CreateDXG()
{
    HRESULT hr;
	// ===== DX9 Ǩ��: Direct3D9 �豸 + ���� backbuffer(CDX9SoftSurface) =====
	m_pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	if ( !m_pD3D )
	{
		MessageBox(m_hWnd, TEXT("[CWHDXGraphicWindow::CreateDXG]") TEXT("Direct3DCreate9 failed."), TEXT("MirDXG"), MB_ICONERROR | MB_OK);
		return E_FAIL;
	}
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed          = TRUE;
	d3dpp.SwapEffect        = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow     = m_hWnd;
	d3dpp.BackBufferFormat  = D3DFMT_UNKNOWN;
	d3dpp.BackBufferCount   = 1;
	// [DX9][T17] ��ʽ���� swapchain backbuffer = ��ʾ�ֱ��ʣ����ⴰ��ģʽ��
	// backbuffer ����������ߴ粻һ�µ��� Present ����ʧ�档
	d3dpp.BackBufferWidth   = m_stDisplayInfo.wWidth;
	d3dpp.BackBufferHeight  = m_stDisplayInfo.wHeight;
	hr = m_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_hWnd,
							  D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &m_pd3dDevice);
	if ( FAILED(hr) )
		hr = m_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_SW, m_hWnd,
							  D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &m_pd3dDevice);
	if ( FAILED(hr) || !m_pd3dDevice )
	{
		MessageBox(m_hWnd, TEXT("[CWHDXGraphicWindow::CreateDXG]") TEXT("Direct3D Device create failed."), TEXT("MirDXG"), MB_ICONERROR | MB_OK);
		return E_FAIL;
	}
	// ���� backbuffer
	DWORD w = m_stDisplayInfo.wWidth, h = m_stDisplayInfo.wHeight;
	m_dwSoftBufSize = w * h * sizeof(WORD);
	m_pSoftBackBuffer = (WORD*)malloc(m_dwSoftBufSize);
	if ( !m_pSoftBackBuffer ) return E_FAIL;
	m_pddsBackBuffer = new CDX9SoftSurface;
	m_pddsBackBuffer->m_pBits      = m_pSoftBackBuffer;
	m_pddsBackBuffer->m_dwWidth    = w;
	m_pddsBackBuffer->m_dwHeight   = h;
	m_pddsBackBuffer->m_lPitch     = w * sizeof(WORD);
	// ��������
	// [FIX-R5] 首选 R5G6B5: 软缓冲本身就是 565, 可以直接整行 memcpy 上传,
	// 省掉每帧 800x600 像素的标量 565->8888 展开(这是客户端最大的固定 CPU 开销)。
	// 驱动不支持时退回 32 位纹理, Present() 里按实际格式决定走哪条路。
	hr = m_pd3dDevice->CreateTexture(w, h, 1, D3DUSAGE_DYNAMIC, D3DFMT_R5G6B5, D3DPOOL_DEFAULT, &m_pd3dPresentTex, NULL);
	if ( FAILED(hr) )
		hr = m_pd3dDevice->CreateTexture(w, h, 1, D3DUSAGE_DYNAMIC, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &m_pd3dPresentTex, NULL);
	if ( FAILED(hr) )
		hr = m_pd3dDevice->CreateTexture(w, h, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pd3dPresentTex, NULL);
	if ( FAILED(hr) || !m_pd3dPresentTex ) return E_FAIL;
	// D3DX9 Sprite / Font
	if ( FAILED(hr = D3DXCreateSprite(m_pd3dDevice, &m_pSprite)) ) return E_FAIL;
	if ( FAILED(D3DXCreateFontA(m_pd3dDevice, 14, 0, FW_NORMAL, 1, FALSE,
							   134, 0, 0, 0, "System", &m_pFont)) )  // [#38-FIX] GB2312_CHARSET
		m_pFont = NULL;
	UpdateBoundsWnd();
	m_stBitsMaskInfo = GetRGBMaskInfoIN16Bits(NULL);
	Init3DDeviceObjects();
	return S_OK;
}


/******************************************************************************************************************

	????? : CDXGraphic::ResetDXG()

	????? : 
	????? : 

	????   : 
	???   : WORD wWidth
	         WORD wHeight
	         WORD wBPP
	         BYTE bScreenModeFlag
	         BYTE bDeviceModeFlag
	???1?71?1?771?1?71?1?777   : HRESULT 

	[????][??????] : ????????

*******************************************************************************************************************/
HRESULT CWHDXGraphicWindow::ResetDXG(WORD wWidth, WORD wHeight, WORD wBPP, BYTE bScreenModeFlag, BYTE bDeviceModeFlag)
{
    HRESULT hr;
    DWORD	dwStyle;

//	if ( m_bDeviceModeFlag == bDeviceModeFlag && m_bScreenModeFlag == bScreenModeFlag && 
//		 m_stDisplayInfo.wWidth == wWidth && m_stDisplayInfo.wHeight == wHeight && m_stDisplayInfo.wBPP == wBPP )
//		return E_FAIL;													    

    if ( FAILED(DestroyDXGObjects()) )
	{
		MessageBox(m_hWnd, TEXT("[CWHDXGraphicWindow::ResetDXG]") TEXT("DirectGraphic reset failed."), "MirDXG", MB_ICONERROR | MB_OK);
		return E_FAIL;
	}

	m_stDisplayInfo.wWidth	= wWidth;
	m_stDisplayInfo.wHeight	= wHeight;
	m_stDisplayInfo.wBPP	= wBPP;
	m_bScreenModeFlag		= bScreenModeFlag;
	m_bDeviceModeFlag		= bDeviceModeFlag;

	if ( m_bScreenModeFlag & _DXG_SCREENMODE_WINDOW )
	{
		dwStyle  = GetWindowStyle(m_hWnd);
		dwStyle &= ~WS_POPUP;
		dwStyle |= WS_OVERLAPPED | WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU;
		SetWindowLong(m_hWnd, GWL_STYLE, dwStyle);
		// [DX9][FIX] �ָ����ڴ�С������ȷ�� client ��ƥ����ʾ�ֱ���
		RECT rcClient;
		SetRect(&rcClient, 0, 0, wWidth, wHeight);
		AdjustWindowRectEx(&rcClient, dwStyle, FALSE, GetWindowLong(m_hWnd, GWL_EXSTYLE));
		SetWindowPos(m_hWnd, NULL, 0, 0,
				rcClient.right - rcClient.left, rcClient.bottom - rcClient.top,
				SWP_NOMOVE | SWP_NOZORDER | SWP_SHOWWINDOW);
	}
	else
	{
		dwStyle  = GetWindowStyle(m_hWnd);
		dwStyle &= ~dwStyle;
		dwStyle |= WS_POPUP | WS_SYSMENU;
		SetWindowLong(m_hWnd, GWL_STYLE, dwStyle);
	}

	if ( hr = FAILED(CreateDXG()) )
	{
		MessageBox(m_hWnd, TEXT("[CWHDXGraphicWindow::ResetDXG]") TEXT("DirectGraphic create failed."), "MirDXG", MB_ICONERROR | MB_OK);
		return E_FAIL;
	}

	SetDefFont();

	return S_OK;
}


HRESULT CWHDXGraphicWindow::Clear(DWORD dwColor)
{
    DDBLTFX ddbltfx;
	HRESULT hr;

    if ( NULL == m_pddsBackBuffer )
	{
        return E_POINTER;
	}

    ZeroMemory(&ddbltfx, sizeof(ddbltfx));
    ddbltfx.dwSize      = sizeof(ddbltfx);
    ddbltfx.dwFillColor = dwColor;

	while ( TRUE )
	{
		hr = m_pddsBackBuffer->Blt(NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx);

		if ( hr == DDERR_SURFACELOST )
		{
			m_pddsBackBuffer->Restore();
		}

//		if ( hr == DD_OK )
		if ( hr != DDERR_WASSTILLDRAWING )
		{
		    return S_OK;
		}
	} 
}


/******************************************************************************************************************

	????? : CWHDXGraphicWindow:: marySurface()

	????? : 
	????? : 

	????   : 
	???1?71?1?771?1?71?1?777   : HRESULT 

	[????][??????] : ????????

*******************************************************************************************************************/
HRESULT CWHDXGraphicWindow::CreatePrimarySurface()
{
	// DX9: ���� backbuffer ���� CreateDXG ����
	return S_OK;
}


/******************************************************************************************************************

	????? : CWHDXGraphicWindow::UpdateBoundsWnd()

	????? : 
	????? : 

	????   : 
	???1?71?1?771?1?71?1?777   : VOID 

	[????][??????] : ????????

*******************************************************************************************************************/
VOID CWHDXGraphicWindow::UpdateBoundsWnd()
{
	if ( m_bScreenModeFlag & _DXG_SCREENMODE_WINDOW )
	{
        GetClientRect(m_hWnd, &m_rcWindow);
        ClientToScreen(m_hWnd, (POINT*)&m_rcWindow);
        ClientToScreen(m_hWnd, (POINT*)&m_rcWindow+1);
    }
    else
    {
        SetRect(&m_rcWindow, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
    }
}


/******************************************************************************************************************

	????? : CWHDXGraphicWindow::CreateZBuffer()

	????? : 
	????? : 

	????   : 
	???   : GUID* pDeviceGUID
	???1?71?1?771?1?71?1?777   : HRESULT 

	[????][??????] : ????????

*******************************************************************************************************************/
HRESULT CWHDXGraphicWindow::CreateZBuffer(const GUID* pDeviceGUID)
{
	// DX9: ����ģʽ������Ⱦ, ������� ZBuffer
	return S_OK;
}


HRESULT CWHDXGraphicWindow::Init3DDeviceObjects()
{
	if ( !m_pd3dDevice ) return E_FAIL;
	FLOAT fHalfWidth  = (FLOAT)((FLOAT)m_stDisplayInfo.wWidth /2.0f);
	FLOAT fHalfHeight = (FLOAT)((FLOAT)m_stDisplayInfo.wHeight/2.0f);
	D3DXMATRIX matWorld, matView, matProj;
	D3DXVECTOR3 vEyePt(0, 0, -fHalfHeight);
	D3DXVECTOR3 vLookatPt(0, 0, 0);
	D3DXVECTOR3 vUpVec(0, 1, 0);
	D3DXMatrixIdentity(&matWorld);
	D3DXMatrixLookAtLH(&matView, &vEyePt, &vLookatPt, &vUpVec);
	D3DXMatrixPerspectiveFovLH(&matProj, g_PI/2, fHalfHeight/fHalfWidth, -0.1f, 1.0f);
	m_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
	m_pd3dDevice->SetTransform(D3DTS_VIEW, &matView);
	m_pd3dDevice->SetTransform(D3DTS_PROJECTION, &matProj);
	m_pd3dDevice->SetRenderState(D3DRS_AMBIENT, 0xFFFFFFFF);
	m_pd3dDevice->SetRenderState(D3DRS_DITHERENABLE, FALSE);
	m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
	m_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	return S_OK;
}


/*******************************************************************************************************************/

HFONT CWHDXGraphicWindow::CreateGameFont(LPCSTR szFontName, INT nHeight, INT nWidth, INT nWeight, BOOL bItalic, BOOL bULine, BOOL bStrikeOut, DWORD dwCharSet)
{
	INT nYsize = nHeight;
	HDC hDC = GetDC(m_hWnd);
	if ( hDC )
	{
		nYsize = -MulDiv(nHeight, GetDeviceCaps(hDC, LOGPIXELSY), 72);
		ReleaseDC(m_hWnd, hDC);
	}
	if ( szFontName )
		return CreateFont(nYsize, nWidth, 0, 0, nWeight, bItalic, bULine, bStrikeOut, dwCharSet, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, szFontName);
	else if ( m_pszDefFont[0] != '\0' )
		return CreateFont(nYsize, nWidth, 0, 0, nWeight, bItalic, bULine, bStrikeOut, dwCharSet, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, m_pszDefFont);
	else
		return CreateFont(nYsize, nWidth, 0, 0, nWeight, bItalic, bULine, bStrikeOut, dwCharSet, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "System");
}

VOID CWHDXGraphicWindow::SetDefFont(CHAR* pszFont, INT nFontSize)
{
	// ??? ??????? ??????? ?????? ???? ?????1?71?1?771?1?71?1?777 ????? Clear.
	if ( m_hDefGameFont != NULL )
	{
		DeleteObject(m_hDefGameFont); 
		m_hDefGameFont = NULL;
	}

	if ( pszFont == NULL )
	{
		if ( m_pszDefFont[0] != '\0' )
		{
			m_hDefGameFont	= CreateGameFont(m_pszDefFont, 9);
		}
		else
		{
			m_hDefGameFont	= CreateGameFont("????", 9);
			ZeroMemory(m_pszDefFont, MAX_PATH);
		}
	}
	else
	{
		m_hDefGameFont	= CreateGameFont(pszFont, 9);
		strcpy(m_pszDefFont, pszFont);
	}	
}



VOID CWHDXGraphicWindow::StringPlus(CHAR* szResult, CHAR* szSrc, ...)
{
	INT nCnt = 0;

	va_list vaMarker;
	CHAR*	pszString;


//	ZeroMemory(szResult, strlen(szResult));
	va_start(vaMarker, szSrc);
	pszString = szSrc;
	
	while ( TRUE )
	{		
		strcpy(&szResult[strlen(szResult)], pszString);
		if ( !strcmp(pszString,  "") )		break;
		pszString = (CHAR*)va_arg(vaMarker, CHAR*);
	}

	va_end(vaMarker);
}


BOOL CWHDXGraphicWindow::BelongStr(CHAR* pszSrc, CHAR* pszCmp)
{
	INT nCmpLen = strlen(pszCmp);
	INT nSrcLen = strlen(pszSrc);

	if ( nCmpLen > nSrcLen ) 
	{
		return FALSE;
	}

	CHAR* pszCheck = NULL;
	while ( pszCheck = strchr(pszSrc, pszCmp[0]) )
	{
		if ( !memcmp(pszCheck, pszCmp, nCmpLen) )
		{
			return TRUE;
		}
		pszSrc = pszCheck+1;
	}

	return FALSE;
}

CHAR* CWHDXGraphicWindow::IntToStr(INT nNum)
{
	static CHAR szResult[MAX_PATH];
	ZeroMemory(szResult, MAX_PATH);
	_itoa(nNum, szResult, 10);
	return szResult;
}




/******************************************************************************************************************

	????? : CWHDXGraphicWindow::PutsHan()

	????? : 
	????? : 

	????   : 
	???   : CDX9SoftSurface* pSurface
	         INT nX
	         INT nY
	         COLORREF foreColor
	         COLORREF backColor
	         CHAR* szFormat
	         ...
	???1?71?1?771?1?71?1?777   : VOID 

	[????][??????] : ????????

*******************************************************************************************************************/
#if MIR3_DIAG_LOG
// ============================================================================
// [DX9][T60-TEXT] GDI �ı�·��ȡ֤: ��¼ szText ָ����Ч�� / HDC / ����״̬��
//   Ŀ��: ��λ"��ʿ��ħ������"(msvcrt �ڲ��ַ�������)�Ƿ�Դ�ڴ˴���
//   Լ��: ֻ��¼, ���ı���Ⱦ��Ϊ; ֻ��¼���ɵ���, ��������֡ˢ����
// ============================================================================
static void Dx9TextDiag(const char* pszApi, CDX9SoftSurface* pSurface, CHAR* szText,
                        HFONT hFont, HFONT hDefFont, HDC hDC, HRESULT hrGetDC)
{
	static INT s_nLogged = 0;
	if ( s_nLogged > 200 ) return;

	BOOL bNullText = ( szText == NULL );
	// [FIX-R4] IsBadStringPtrA 是 SEH 探测, 每次 PutsHan 都跑会把聊天/怪物名/血条
	// 的成本放大; strnlen(4096) 同理。只有真正要写盘时才需要长度, 所以先做廉价判定。
	BOOL bBadText  = FALSE;
	size_t nLen    = 0;
	BOOL bHuge     = FALSE;

	BOOL bSuspect = bNullText || FAILED(hrGetDC) || hDC == NULL;
	if ( !bSuspect ) return;

	if ( !bNullText )
	{
		nLen  = strnlen(szText, 4096);
		bHuge = ( nLen >= 4096 );
	}

	s_nLogged++;
	FILE* fp = fopen("D:\\text_diag.txt", "a");
	if ( !fp ) return;
	fprintf(fp, "[%s] surface=%p hDC=%p hr=0x%08X hFont=%p defFont=%p text=%p nullText=%d badText=%d len=%u huge=%d\r\n",
		pszApi, (void*)pSurface, (void*)hDC, (unsigned)hrGetDC,
		(void*)hFont, (void*)hDefFont, (void*)szText,
		(int)bNullText, (int)bBadText, (unsigned)nLen, (int)bHuge);
	if ( !bNullText && !bBadText )
	{
		INT nShow = ( nLen < 96 ) ? (INT)nLen : 96;
		fprintf(fp, "        text[0..%d]=", nShow);
		for ( INT i = 0; i < nShow; i++ ) fprintf(fp, "%02X ", (BYTE)szText[i]);
		fprintf(fp, "\r\n");
	}
	fclose(fp);
}
#endif // MIR3_DIAG_LOG
VOID CWHDXGraphicWindow::PutsHan(CDX9SoftSurface* pSurface, INT nX, INT nY, COLORREF foreColor, COLORREF backColor, CHAR* szText, HFONT hFont)
{
	HDC		hDC;
	HFONT	hOldFont;

    if ( NULL == pSurface )
	{
		pSurface = m_pddsBackBuffer;
	}
			
    if ( NULL == pSurface )
	{
		return;
	}

	if ( NULL == hFont )
	{
		hFont = m_hDefGameFont;
	}

	HRESULT hr = pSurface->GetDC(&hDC);

	#if MIR3_DIAG_LOG

	Dx9TextDiag("PutsHanXY", pSurface, szText, hFont, m_hDefGameFont, hDC, hr);

	#endif // MIR3_DIAG_LOG

	if ( FAILED( hr ) )
	{
		return;
	}

	if (backColor == RGB( 0, 0, 0 ))
		SetBkMode(hDC, TRANSPARENT);	
	else
		SetBkColor(hDC, backColor);

	INT ntet = GetLastError();

	hOldFont = (HFONT)SelectObject(hDC, hFont);

	SetTextColor(hDC, foreColor);

	TextOut(hDC, nX, nY, szText, strlen(szText));

	SelectObject(hDC, hOldFont);

	if ( hFont != m_hDefGameFont )
	{
		DeleteObject(hFont);
		hFont = NULL;
	}
	
	pSurface->ReleaseDC(hDC);
}

VOID CWHDXGraphicWindow::PutsHanA(CDX9SoftSurface* pSurface, INT nX, INT nY, COLORREF foreColor, COLORREF backColor, CHAR* szText, HFONT hFont)
{
	HDC		hDC;
	HFONT	hOldFont;

    if (NULL == pSurface)
	{
		pSurface = m_pddsBackBuffer;
	}
			
    if ( NULL == pSurface )
	{
		return;
	}

	if ( NULL == hFont )
	{
		hFont = m_hDefGameFont;
	}

	HRESULT hr = pSurface->GetDC(&hDC);

	if ( FAILED( hr ) )
	{
		return;
	}

	if (backColor == RGB( 0, 0, 0 ))		SetBkMode(hDC, TRANSPARENT);	
	else									SetBkColor(hDC, backColor);

	hOldFont = (HFONT)SelectObject(hDC, hFont);

	SetTextColor(hDC, foreColor);

	TextOut(hDC, nX, nY, szText, strlen(szText));

	SelectObject(hDC, hOldFont);

	if ( hFont != m_hDefGameFont )
	{
		DeleteObject(hFont);
		hFont = NULL;
	}

	pSurface->ReleaseDC(hDC);
}

/******************************************************************************************************************

	????? : CWHDXGraphicWindow::PutsHan()

	????? : 
	????? : 

	????   : 
	???   : CDX9SoftSurface* pSurface
	         INT nX
	         INT nY
	         COLORREF foreColor
	         COLORREF backColor
	         CHAR* szFormat
	         ...
	???1?71?1?771?1?71?1?777   : VOID 

	[????][??????] : ????????

*******************************************************************************************************************/
VOID CWHDXGraphicWindow::PutsHan(CDX9SoftSurface* pSurface, RECT rc, COLORREF foreColor, COLORREF backColor, CHAR* szText, HFONT hFont)
{
	HDC		hDC;
	HFONT	hOldFont;

    if (NULL == pSurface)
	{
		pSurface = m_pddsBackBuffer;
	}
			
    if ( NULL == pSurface )
	{
		return;
	}

	if ( NULL == hFont )
	{
		hFont = m_hDefGameFont;
	}

	HRESULT hr = pSurface->GetDC(&hDC);

	if ( FAILED( hr ) )
	{
		return;
	}

	if (backColor == RGB( 0, 0, 0 ))		SetBkMode(hDC, TRANSPARENT);	
	else									SetBkColor(hDC, backColor);

	hOldFont = (HFONT)SelectObject(hDC, hFont);

	SetTextColor(hDC, foreColor);

	#if MIR3_DIAG_LOG

	Dx9TextDiag("PutsHanRC", pSurface, szText, hFont, m_hDefGameFont, hDC, hr);

	#endif // MIR3_DIAG_LOG

	DrawText(hDC, szText, -1, &rc, DT_CENTER|DT_VCENTER|DT_SINGLELINE);

	SelectObject(hDC, hOldFont);

	if ( hFont != m_hDefGameFont )
	{
		DeleteObject(hFont);
		hFont = NULL;
	}
	
	pSurface->ReleaseDC(hDC);
}


VOID CWHDXGraphicWindow::PutsHanLeft(CDX9SoftSurface* pSurface, RECT rc, COLORREF foreColor, COLORREF backColor, CHAR* szText, HFONT hFont)
{
	HDC		hDC;
	HFONT	hOldFont;

    if (NULL == pSurface)
	{
		pSurface = m_pddsBackBuffer;
	}
			
    if ( NULL == pSurface )
	{
		return;
	}

	if ( NULL == hFont )
	{
		hFont = m_hDefGameFont;
	}

	HRESULT hr = pSurface->GetDC(&hDC);

	if ( FAILED( hr ) )
	{
		return;
	}

	if (backColor == RGB( 0, 0, 0 ))		SetBkMode(hDC, TRANSPARENT);	
	else									SetBkColor(hDC, backColor);

	hOldFont = (HFONT)SelectObject(hDC, hFont);

	SetTextColor(hDC, foreColor);

	DrawText(hDC, szText, -1, &rc, DT_LEFT|DT_VCENTER|DT_SINGLELINE);

	SelectObject(hDC, hOldFont);

	if ( hFont != m_hDefGameFont )
	{
		DeleteObject(hFont);
		hFont = NULL;
	}
	
	pSurface->ReleaseDC(hDC);
}

VOID CWHDXGraphicWindow::PutsHanEx(CDX9SoftSurface* pSurface, RECT rc, COLORREF foreColor, COLORREF backColor, CHAR* szText, HFONT hFont)
{
	HDC		hDC;
	HFONT	hOldFont;
	

    if (NULL == pSurface)
		pSurface = m_pddsBackBuffer;
			
    if ( NULL == pSurface )
		return;

	if ( NULL == hFont )
		hFont = m_hDefGameFont;

	HRESULT hr = pSurface->GetDC(&hDC);

	if ( FAILED( hr ) )
	{
		return;
	}

	if (backColor == RGB( 0, 0, 0 ))		SetBkMode(hDC, TRANSPARENT);	
	else									SetBkColor(hDC, backColor);

	hOldFont = (HFONT)SelectObject(hDC, hFont);

	SetTextColor(hDC, foreColor);

	// ????? ??????? ??????? ?????.
	SIZE sizeTmp;
	GetTextExtentPoint32(hDC, szText, strlen(szText), &sizeTmp);

	// ???????.
	if ( sizeTmp.cx < rc.right-rc.left )
		DrawText(hDC, szText, strlen(szText), &rc, DT_CENTER|DT_VCENTER|DT_SINGLELINE);
	else
	{
		INT nHeight = DrawTextEx(hDC, szText, strlen(szText), &rc, DT_LEFT|DT_WORDBREAK|DT_CALCRECT, NULL);
		RECT rcTmp;
		rcTmp.top	 = rc.top+(rc.bottom-rc.top)/2 - nHeight/2;
		rcTmp.bottom = rcTmp.top + nHeight;
		rcTmp.left   = rc.left;
		rcTmp.right  = rc.right;
		DrawText(hDC, szText, strlen(szText), &rcTmp, DT_LEFT|DT_WORDBREAK);
	}

	SelectObject(hDC, hOldFont);

 	if ( hFont != m_hDefGameFont )
	{
		DeleteObject(hFont);
		hFont = NULL;
	}

	pSurface->ReleaseDC(hDC);
}


SIZE CWHDXGraphicWindow::GetStrLength(CDX9SoftSurface* pSurface, HFONT hFont, CHAR* szFormat)
{
	HDC		hDC;
	SIZE	sSize;
	
	sSize.cx = 0;
	sSize.cy = 0;

	HFONT	hOldFont;

	if ( NULL == pSurface )
	{
		pSurface = m_pddsBackBuffer;
	}
			
    if ( NULL == pSurface )	
	{
		return sSize;
	}

	if ( NULL == hFont )
	{
		hFont = m_hDefGameFont;
	}

	HRESULT hr = pSurface->GetDC(&hDC);

	if ( FAILED( hr ) )
	{
		return sSize;
	}

	hOldFont = (HFONT)SelectObject(hDC, hFont);

	GetTextExtentPoint32(hDC,szFormat,strlen(szFormat),&sSize);

	SelectObject(hDC, hOldFont);
	
	if ( hFont != m_hDefGameFont )
	{
		DeleteObject(hFont);
		hFont = NULL;
	}

	pSurface->ReleaseDC(hDC);

	return sSize;
}


/*BOOL CWHDXGraphicWindow::StringDivideLen(INT nDivideLen, INT& nDividedLine, CHAR* szSrc,CDLList<CHAR*>* m_pxpStr)
{
	CHAR szTmpCheck[MAX_PATH];
	CHAR szResult[MAX_PATH];
	
	nDividedLine = 1;

	INT nStartLen	= 0;
	INT nEndLen		= 0;
	INT nResultLen	= 0;
	INT nWordCheck	= 0;

	if ( szSrc[0] != NULL )
	{
		ZeroMemory(szResult,MAX_PATH);

		for ( INT nCnt = 0; nCnt < (INT)strlen(szSrc); nCnt++)
		{
			ZeroMemory(szTmpCheck, MAX_PATH);
			nEndLen = nCnt+1;

			// ????�ʧ�?????1?71?1?771?1?71?1?777 ??????? ?�ա�?.
			memcpy(&szTmpCheck, &szSrc[nStartLen], nEndLen-nStartLen);
			INT nsLen;
			nsLen = strlen(szTmpCheck);

			// ???????1?71?1?771?1?71?1?777 ?��??1?71?1?771?1?71?1?777 ?��??? ??????? ???????.
			if (nsLen > nDivideLen )
			{
				// ???? 2????? ??????1?71?1?771?1?71?1?777. 
				if ( szSrc[nEndLen-1] < 0 )
				{
					// ???? ???????? 2????? ????? ?????1?71?1?771?1?71?1?777
					if ( !(nWordCheck%2) )
					{
						nStartLen += strlen(szTmpCheck)-1;
						nCnt--;
						CHAR* pszNewLine;
						pszNewLine = new CHAR[nResultLen+1];
						memcpy(pszNewLine,szResult,nResultLen);
						pszNewLine[nResultLen]=NULL;
						m_pxpStr->AddNode(pszNewLine);
						nResultLen = 0;
						nDividedLine++;
					}
					else
					{
						nStartLen += strlen(szTmpCheck)-2;
						nCnt -= 2;
						CHAR* pszNewLine;
						pszNewLine = new CHAR[nResultLen];
						memcpy(pszNewLine,szResult,nResultLen-1);
						pszNewLine[nResultLen-1]=NULL;
						m_pxpStr->AddNode(pszNewLine);
						nResultLen = 0;
						nDividedLine++;
						nWordCheck--;
					}
				}
				// 1????? ????. 
				// ???ֽ?? ????????�ʧ�????? ????????? ???.
				else
				{
					nStartLen += strlen(szTmpCheck)-1;
					nCnt--;
					CHAR* pszNewLine;
					pszNewLine = new CHAR[nResultLen+1];
					memcpy(pszNewLine,szResult,nResultLen);
					pszNewLine[nResultLen]=NULL;
					m_pxpStr->AddNode(pszNewLine);
					nResultLen=0;
					nDividedLine++;
				}
			}
			else if(szSrc[nEndLen-1]=='\n')	// ????????     \n???????  Error ????
			{
				nStartLen += strlen(szTmpCheck)-1;
				CHAR* pszNewLine;
				pszNewLine = new CHAR[nResultLen+1];
				memcpy(pszNewLine,szResult,nResultLen);
				pszNewLine[nResultLen]=NULL;
				m_pxpStr->AddNode(pszNewLine);
				nResultLen=0;
				nDividedLine++;
			}
			else
			{
				if ( szSrc[nEndLen-1] < 0 )
					nWordCheck++;

				szResult[nResultLen] = szSrc[nEndLen-1];
				nResultLen++;
			}
		}
		if(nResultLen!=0)
		{
			CHAR* pszNewLine;
			pszNewLine = new CHAR[nResultLen+1];
			memcpy(pszNewLine,szResult,nResultLen);
			pszNewLine[nResultLen]=NULL;
			m_pxpStr->AddNode(pszNewLine);
			nDividedLine++;
			nResultLen=0;
		}
		return TRUE;
	}
	return FALSE;
}
*/


BOOL CWHDXGraphicWindow::StringDivide(INT nDivideWidth, INT& nDividedLine, CHAR* szSrc, CHAR* szResult, HFONT hFont)
{
	CHAR szTmpCheck[MAX_PATH];
	
	INT nStartLen, nEndLen, nResultLen, nWordCheck;

	nStartLen = nEndLen = nResultLen = nWordCheck = 0;
	nDividedLine = 1;


	HDC		hDC;
	HFONT	hOldFont;
	
	if ( NULL == hFont || m_pddsBackBuffer == NULL )
	{
		hFont = m_hDefGameFont;
	}

	m_pddsBackBuffer->GetDC(&hDC);
//	if ( FAILED( m_pddsBackBuffer->GetDC(&hDC) ) )
//	{
//		return FALSE;
//	}

	hOldFont = (HFONT)SelectObject(hDC, hFont);	

	if ( szSrc[0] != '\0' )
	{
		for ( INT nCnt = 0; nCnt < (INT)strlen(szSrc); nCnt++)
		{
			ZeroMemory(szTmpCheck, MAX_PATH);

			nEndLen = nCnt+1;

/*			// ???? ??????? ??? ??????? ????? ?????? ???????.
			if ( nEndLen >= MAX_PATH )
			{
				szResult[MAX_PATH-1] = NULL;
				break;
			}
*/
			// ????�ʧ�?????1?71?1?771?1?71?1?777 ??????? ?�ա�?.
			memcpy(&szTmpCheck, &szSrc[nStartLen], nEndLen-nStartLen);

			SIZE sizeLen = {0, 0};

			GetTextExtentPoint32(hDC, szTmpCheck, strlen(szTmpCheck), &sizeLen);

			// ???????1?71?1?771?1?71?1?777 ?��??1?71?1?771?1?71?1?777 ?��??? ??????? ???????.
			if ( sizeLen.cx > nDivideWidth )
			{
				// ???? 2????? ??????1?71?1?771?1?71?1?777. 
				if ( szSrc[nEndLen-1] < 0 )
				{
					// ???? ???????? 2????? ????? ?????1?71?1?771?1?71?1?777
					if ( !(nWordCheck%2) )
					{
						nStartLen += strlen(szTmpCheck)-1;
						nCnt--;
						szResult[nResultLen] = '`';
						nResultLen++;
						nDividedLine++;
					}
					else
					{
						nStartLen += strlen(szTmpCheck)-2;
						nCnt -= 2;
						szResult[nResultLen-1] = '`';
						nDividedLine++;
						nWordCheck--;
					}
				}
				// 1????? ????. 
				// ???ֽ?? ????????�ʧ�????? ????????? ???.
				else
				{
					nStartLen += strlen(szTmpCheck)-1;
					nCnt--;
					szResult[nResultLen] = '`';
					nResultLen++;
					nDividedLine++;
				}
			}
			else
			{
				if ( szSrc[nEndLen-1] < 0 )
				{
					nWordCheck++;


					szResult[nResultLen] = szSrc[nEndLen-1];
					nResultLen++;

					nWordCheck++;

					nCnt++;
					nEndLen = nCnt+1;
					szResult[nResultLen] = szSrc[nEndLen-1];
					nResultLen++;
				}
				else
				{
					szResult[nResultLen] = szSrc[nEndLen-1];
					nResultLen++;
				}
			}
		}
	}

	if ( szSrc[0] == '\0' )
	{
		*szResult = '\0';
	}

	SelectObject(hDC, hOldFont);
	
	if ( hFont != m_hDefGameFont )
	{
		DeleteObject(hFont);
		hFont = NULL;
	}

	m_pddsBackBuffer->ReleaseDC(hDC);

	return TRUE;

}


/*******************************************************************************************************************/

DXG_MASKINFO CWHDXGraphicWindow::GetRGBMaskInfoIN16Bits(LPVOID pSurface)
{
	DXG_MASKINFO mi;
	ZeroMemory(&mi, sizeof(mi));
	mi.bRCnt=5; mi.bGCnt=6; mi.bBCnt=5;
	mi.bRShift=11; mi.bGShift=5; mi.bBShift=0;
	// [DX9][T56] 565 �����޸���ԭ 0x7C00/0x03E0 �� 555 ������룬
	// �� bGCnt=6/bRShift=11 �� 565 ����ì�ܣ����� R/G ��ȡ���롢����ƫ��(�ڼ�Ӱ)��
	mi.dwRMask=0xF800; mi.dwGMask=0x07E0; mi.dwBMask=0x001F;
	return mi;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ????? ????? ????.

WORD CWHDXGraphicWindow::ConvertColor24To16(COLORREF dwColor)
{
	WORD wRet;

	INT nRed  = RGB_GETRED(dwColor);
	INT nGreen = RGB_GETGREEN(dwColor);
	INT nBlue   = RGB_GETBLUE(dwColor);
	
	nRed = (nRed*(1<<m_stBitsMaskInfo.bRCnt))/(1<<8);
	nGreen = (nGreen*(1<<m_stBitsMaskInfo.bGCnt))/(1<<8);
	nBlue = (nBlue*(1<<m_stBitsMaskInfo.bBCnt))/(1<<8);

	wRet = (nRed << m_stBitsMaskInfo.bRShift) | (nGreen << m_stBitsMaskInfo.bGShift) | (nBlue << m_stBitsMaskInfo.bBShift); 
	return wRet;
}


/******************************************************************************************************************

	????? : CWHDXGraphicWindow::DrawWithGDI()

	????? : 
	????? : 

	????   : 
	???   : RECT rc
	         CDX9SoftSurface* pSurface
	         DWORD dwColor
	         DWORD dwPenStyleFlg
	         BYTE bKind
	???1?71?1?771?1?71?1?777   : HRESULT 

	[????][??????] : ????????

*******************************************************************************************************************/
HRESULT CWHDXGraphicWindow::DrawWithGDI(RECT* prc, CDX9SoftSurface* pSurface, DWORD dwColor, BYTE bKind)
{
	POINT	pt;
	HDC		hDC;
	HPEN	hPen, hOldPen;
	HBRUSH	hBrush, hOldBrush;
	HRESULT hr;

	if ( pSurface == NULL )
	{
		pSurface = m_pddsBackBuffer;
	}

	if ( pSurface == NULL )
	{
		return E_FAIL;
	}

	if ( FAILED(hr = pSurface->GetDC(&hDC)) )
	{
		return E_FAIL;
	}

	switch ( bKind )
	{
	case 0:
		{
			hPen = CreatePen(PS_SOLID, NULL, dwColor);
		
			hOldPen = (HPEN)SelectObject(hDC, hPen);

			MoveToEx(hDC, prc->left, prc->top, &pt);
			LineTo(hDC, prc->right, prc->bottom);

			SelectObject(hDC, hOldPen);
			DeleteObject(hPen);
			hPen = NULL;
		}
		break;
	case 1:
		{
			hBrush = CreateSolidBrush(dwColor);

			hOldBrush = (HBRUSH)SelectObject(hDC, hBrush);

			FrameRect(hDC, prc, hBrush);

			SelectObject(hDC, hOldBrush);
			DeleteObject(hBrush);
			hBrush = NULL;
		}
		break;
	case 2:
		{
			hBrush = CreateSolidBrush(dwColor);

			hOldBrush = (HBRUSH)SelectObject(hDC, hBrush);

			FillRect(hDC, prc, hBrush);

			SelectObject(hDC, hOldBrush);
			DeleteObject(hBrush);
			hBrush = NULL;
		}
		break;
	case 5:
		{
			// Alpha ��䣺dwColor �ĵ��ֽ�Ϊ alpha ֵ (0-255)���� 3 �ֽ�Ϊ RGB
			// ��ʽ��0xAARRGGBB
			BYTE bAlpha = (BYTE)(dwColor & 0xFF);
			DWORD dwRGB = (dwColor >> 8) & 0xFFFFFF;
			BYTE bR = (BYTE)(dwRGB & 0xFF);
			BYTE bG = (BYTE)((dwRGB >> 8) & 0xFF);
			BYTE bB = (BYTE)((dwRGB >> 16) & 0xFF);

			// �����ڴ� DC ��λͼ���� AlphaBlend
			HDC hMemDC = CreateCompatibleDC(hDC);
			int nWidth = prc->right - prc->left;
			int nHeight = prc->bottom - prc->top;
			HBITMAP hBmp = CreateCompatibleBitmap(hDC, nWidth, nHeight);
			HBITMAP hOldBmp = (HBITMAP)SelectObject(hMemDC, hBmp);

			// ��䴿ɫ
			HBRUSH hBrush2 = CreateSolidBrush(RGB(bR, bG, bB));
			RECT rcMem = {0, 0, nWidth, nHeight};
			FillRect(hMemDC, &rcMem, hBrush2);
			DeleteObject(hBrush2);

			// AlphaBlend ��Ŀ�� DC
			BLENDFUNCTION bf;
			bf.BlendOp = AC_SRC_OVER;
			bf.BlendFlags = 0;
			bf.SourceConstantAlpha = bAlpha;
			bf.AlphaFormat = 0;
			AlphaBlend(hDC, prc->left, prc->top, nWidth, nHeight,
			           hMemDC, 0, 0, nWidth, nHeight, bf);

			SelectObject(hMemDC, hOldBmp);
			DeleteObject(hBmp);
			DeleteDC(hMemDC);
		}
		break;
	case 3:
		{
			hBrush = CreateSolidBrush(dwColor);

			hOldBrush = (HBRUSH)SelectObject(hDC, hBrush);

			Ellipse(hDC, prc->left, prc->top, prc->right, prc->bottom);

			SelectObject(hDC, hOldBrush);
			DeleteObject(hBrush);
			hBrush = NULL;
		}
		break;
	case 4:
		{
			hPen = CreatePen(PS_SOLID, NULL, dwColor);
		
			hOldPen = (HPEN)SelectObject(hDC, hPen);

			Ellipse(hDC, prc->left, prc->top, prc->right, prc->bottom);

			SelectObject(hDC, hOldPen);
			DeleteObject(hPen);
			hPen = NULL;
		}
		break;
	default:
		break;
	}

	pSurface->ReleaseDC(hDC);

	return S_OK;
}


/******************************************************************************************************************

	????? : CWHDXGraphicWindow::DrawSprite()

	????? : 
	????? : 

	????   : ??????? ?????. ??????? ?????. ????????? ?????, ??????? ?????
	???   : INT nX
	         INT nY
	         INT nXSize
	         INT nYSize
	         WORD* pwSrc
	???1?71?1?771?1?71?1?777   : VOID 

	[????][??????] : ????????

*******************************************************************************************************************/
BOOL CWHDXGraphicWindow::DrawWithImage(INT nX, INT nY, INT nXSize, INT nYSize, WORD* pwSrc)
{
	RECT			rc;
	DDSURFACEDESC2	ddsd;

	INT	nWidth		= nXSize;
	INT	nHeight		= nYSize;
	INT	nXOffset	= 0;
	INT	nYOffset	= 0;
	INT	nStartX		= 0;
	INT	nStartY		= 0;
	INT	nEndX		= m_stDisplayInfo.wWidth -1;
	INT	nEndY		= m_stDisplayInfo.wHeight-1;

	if ( m_pddsBackBuffer != NULL )
	{
		if (nX < nStartX )
		{ 
			nXOffset = nStartX - nX;
			nWidth	 = nXSize - nXOffset;
		}
		if ( (nX+nXSize-1) > nEndX )
			nWidth	 = nEndX - nX - nXOffset + 1;		
		if ( nY < nStartY )
		{ 
			nYOffset	= nStartY - nY;
			nHeight		= nYSize - nYOffset;
		}
		if ( (nY+nYSize-1) > nEndY )
			nHeight	= nEndY - nY - nYOffset + 1;

		if ( (nWidth > 0) && (nHeight > 0) )
		{
			rc.left		= nXOffset;
			rc.right	= nXOffset + nWidth;
			rc.top		= nYOffset;
			rc.bottom	= nYOffset + nHeight;
 
			LONG	lWidth;
			ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
			ddsd.dwSize = sizeof(DDSURFACEDESC2);
			if ( FAILED(m_pddsBackBuffer->Lock(NULL, &ddsd, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL)) )
			{
				return FALSE;
			}

			WORD* pwdDst = (WORD*)ddsd.lpSurface;
			lWidth		 = ddsd.lPitch >> 1;

			for ( INT y=rc.top ; y < rc.bottom ; y++ )
			{
				for ( INT x=rc.left ; x < rc.right ; x++ )
				{
					if ( pwSrc[y * nXSize + x] != 0 )
					{
						pwdDst[((y+nY) * lWidth) + (nX+x)] = pwSrc[y * nXSize + x];
					}
				}
			}

			m_pddsBackBuffer->Unlock(NULL);
		}
		return TRUE;
	}
	return FALSE;
}


BOOL CWHDXGraphicWindow::DrawWithImagePerLineClipRgn(INT nX, INT nY, INT nXSize, INT nYSize, WORD* pwSrc, WORD wClipWidth, WORD wClipHeight)
{
	RECT			rc;
	DDSURFACEDESC2	ddsd;

	INT	nWidth		= nXSize;
	INT	nHeight		= nYSize;
	INT	nXOffset	= 0;
	INT	nYOffset	= 0;
	INT	nStartX		= 0;
	INT	nStartY		= 0;
	INT	nEndX		= wClipWidth  - 1;
	INT	nEndY		= wClipHeight - 1;

	if ( m_pddsBackBuffer != NULL )
	{
		if (nX < nStartX )
		{ 
			nXOffset = nStartX - nX;
			nWidth	 = nXSize - nXOffset;
		}
		if ( (nX+nXSize-1) > nEndX )
			nWidth	 = nEndX - nX - nXOffset + 1;		
		if ( nY < nStartY )
		{ 
			nYOffset	= nStartY - nY;
			nHeight		= nYSize - nYOffset;
		}
		if ( (nY+nYSize-1) > nEndY )
			nHeight	= nEndY - nY - nYOffset + 1;

		if ( (nWidth > 0) && (nHeight > 0) )
		{
			rc.left		= nXOffset;
			rc.right	= nXOffset + nWidth;
			rc.top		= nYOffset;
			rc.bottom	= nYOffset + nHeight;

			LONG	lWidth;
			ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
			ddsd.dwSize = sizeof(DDSURFACEDESC2);
			if ( FAILED(m_pddsBackBuffer->Lock(NULL, &ddsd, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL)) )
			{
				return FALSE;
			}

			WORD* pwdDst = (WORD*)ddsd.lpSurface;
			lWidth		 = ddsd.lPitch >> 1;

			for ( INT y=rc.top ; y < rc.bottom ; y++ )
			{
				memcpy(&pwdDst[((y+nY) * lWidth) + (nX + rc.left)], &pwSrc[y * nXSize + rc.left], (rc.right-rc.left)*2);
			}

			m_pddsBackBuffer->Unlock(NULL);
		}
		return TRUE;
	}
	return FALSE;
}



/******************************************************************************************************************

	????? : CWHDXGraphicWindow::DrawWithSurface()

	????? : 
	????? : 

	????   : 
	???   : CDX9SoftSurface* pddsSrc
	         RECT* prcSrc
	         RECT* prcDst
	???1?71?1?771?1?71?1?777   : BOOL 

	[????][??????] : ????????

*******************************************************************************************************************/
BOOL CWHDXGraphicWindow::DrawWithSurface(INT nX, INT nY, INT nXSize, INT nYSize, CDX9SoftSurface* pddsSrc)
{
	RECT	rc;
	RECT	rcDst;
	INT		nXOffset	= 0;
	INT		nYOffset	= 0;
	INT		nWidth		= nXSize;
	INT		nHeight		= nYSize;
	INT		nStartX		= 0;
	INT		nStartY		= 0;

	INT		nEndX		= m_stDisplayInfo.wWidth -1;
	INT		nEndY		= m_stDisplayInfo.wHeight-1;;


	if ( m_pddsBackBuffer != NULL )
	{
		if (nX < nStartX )
		{ 
			nXOffset	= nStartX - nX;
			nWidth		= nXSize - nXOffset;
		}
		if ( (nX+nXSize-1) > nEndX )
			nWidth		= nEndX - nX - nXOffset + 1;		
		if ( nY < nStartY )
		{ 
			nYOffset	= nStartY - nY;
			nHeight		= nYSize - nYOffset;
		}		
		if ( (nY+nYSize-1) > nEndY )
			nHeight	= nEndY - nY - nYOffset + 1;

		if ( (nWidth > 0) && (nHeight > 0) )
		{
			rc.left		 = nXOffset;
			rc.right	 = nXOffset+nWidth;
			rc.top		 = nYOffset;
			rc.bottom	 = nYOffset+nHeight;

			rcDst.left	 = rc.left+nX;
			rcDst.top	 = rc.top+nY;
			rcDst.right  = rc.right+nX;
			rcDst.bottom = rc.bottom+nY;

			m_pddsBackBuffer->Blt(&rcDst, pddsSrc, &rc, DDBLT_WAIT|DDBLT_KEYSRC, NULL);
		}
		return TRUE;
	}
	return FALSE;
}


BOOL CWHDXGraphicWindow::DrawWithImageForCompMemToMem(INT nX, INT nY, INT nXSize, INT nYSize, WORD* pwSrc, INT nDstXSize, INT nDstYSize, WORD* pwDst, WORD wChooseColor1, WORD wChooseColor2)
{
	RECT			rc;

	INT	nWidth		= nXSize;
	INT	nHeight		= nYSize;
	INT	nXOffset	= 0;
	INT	nYOffset	= 0;
	INT	nStartX		= 0;
	INT	nStartY		= 0;
	INT	nEndX		= nDstXSize - 1;
	INT	nEndY		= nDstYSize - 1;

	if ( pwDst != NULL )
	{
		if (nX < nStartX )
		{ 
			nXOffset	= nStartX - nX;
			nWidth		= nXSize - nXOffset;
		}
		if ( (nX+nXSize-1) > nEndX )
			nWidth		= nEndX - nX - nXOffset + 1;		
		if ( nY < nStartY )		{ 
			nYOffset	= nStartY - nY;
			nHeight		= nYSize - nYOffset;
		}		
		if ( (nY+nYSize-1) > nEndY )
			nHeight	= nEndY - nY - nYOffset + 1;

		if ( (nWidth > 0) && (nHeight > 0) )
		{
			rc.left		= nXOffset;
			rc.right	= nXOffset+nWidth;
			rc.top		= nYOffset;
			rc.bottom	= nYOffset+nHeight;

			INT nWidthStart	= 0;
			INT nWidthEnd	= 0;
			INT nCurrWidth  = 0;
			INT nCntCopyWord = 0;
			INT nYCnt =0;
			INT nLastWidth = 0;

			FLOAT rBlueRate, rGreenRate, bRedRate;
			BYTE  bRedSrc, bGreenSrc, bBlueSrc;
			BYTE  bRedWantedColor, bGreenWantedColor, bBlueWantedColor;
			WORD  wPixel;

			// y?? ?????.
			for ( nYCnt=0 ; nYCnt < rc.top ; nYCnt++ )
			{
				nWidthEnd += pwSrc[nWidthStart];
				nWidthStart++;
				nWidthEnd++;
				nWidthStart	= nWidthEnd;
			}

			// y?? ????? ?????? ???��?? Count???? ?????.
			for ( nYCnt=rc.top ; nYCnt < rc.bottom ; nYCnt++ )
			{
				// ??????? ????? ???��?.(?????? ????)
				nWidthEnd += pwSrc[nWidthStart];
				nWidthStart++;

				// ?????? ?????? ??? ??????.
				for ( INT x = nWidthStart; x < nWidthEnd ; )
				{
					if ( pwSrc[x] == 0xC0 )
					{
						x++;
						nCntCopyWord = pwSrc[x];
						x++;
						nCurrWidth += nCntCopyWord;
					}
					else if ( pwSrc[x] == 0xC1 )
					{
						x++;
						nCntCopyWord = pwSrc[x];
						x++;

						nLastWidth = nCurrWidth;
						nCurrWidth += nCntCopyWord;

						if ( rc.left > nCurrWidth || rc.right < nLastWidth )
						{
							x += nCntCopyWord;
						}
						else
						{
							// rc.left???? ???????? ��???? ?????? ??? ???1?71?1?771?1?71?1?777.
							if ( nLastWidth < rc.left && rc.left <= nCurrWidth )
							{
								if ( ((nYCnt+nY) * nDstXSize) + (rc.left+nX) < 0 )
									return FALSE;

								x += (rc.left-nLastWidth);
								memcpy(&pwDst[((nYCnt+nY) * nDstXSize) + (rc.left+nX)], &pwSrc[x], sizeof(WORD)*(nCurrWidth-rc.left));
								x += (nCurrWidth-rc.left);
							}
							// rc.right???? ???????? ��???? ?????? ??? ???1?71?1?771?1?71?1?777.
							else if ( nLastWidth <= rc.right && rc.right < nCurrWidth )
							{
								memcpy(&pwDst[((nYCnt+nY) * nDstXSize) + (nLastWidth+nX)], &pwSrc[x], sizeof(WORD)*(rc.right-nLastWidth));
								x += nCntCopyWord;
							}
							else
							{
								memcpy(&pwDst[((nYCnt+nY) * nDstXSize) + (nLastWidth+nX)], &pwSrc[x], sizeof(WORD)*nCntCopyWord);
								x += nCntCopyWord;
							}
						}
					}
					else if ( pwSrc[x] == 0xC2 || pwSrc[x] == 0xC3 )
					{
						WORD wDyingKind, wChooseColor;
						wDyingKind = pwSrc[x];
						switch ( wDyingKind )
						{
						case 0xC2: 
							wChooseColor = wChooseColor1;
							break;
						case 0xC3: 
							wChooseColor = wChooseColor2;
							break;
						}
						x++;
						nCntCopyWord = pwSrc[x];
						x++;

						nLastWidth = nCurrWidth;
						nCurrWidth += nCntCopyWord;

						if ( rc.left > nCurrWidth || rc.right < nLastWidth )
						{
							x += nCntCopyWord;
						}
						else
						{
							// rc.left???? ???????? ��???? ?????? ??? ???1?71?1?771?1?71?1?777.
							if ( nLastWidth < rc.left && rc.left <= nCurrWidth )
							{
								x += (rc.left-nLastWidth);
								for ( INT nCheck = 0; nCheck < nCurrWidth-rc.left; nCheck++ )
								{
									wPixel	  = wChooseColor;
									bBlueWantedColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenWantedColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedWantedColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									wPixel	  = pwSrc[x+nCheck];
									bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
									rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
									bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

									bBlueWantedColor = (BYTE)(((FLOAT)bBlueWantedColor*rBlueRate));
									bGreenWantedColor = (BYTE)(((FLOAT)bGreenWantedColor*rGreenRate));
									bRedWantedColor = (BYTE)(((FLOAT)bRedWantedColor*bRedRate));

									if ( bBlueWantedColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
										bBlueWantedColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
									if ( bGreenWantedColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
										bGreenWantedColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
									if ( bRedWantedColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
										bRedWantedColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);

									if ( ((nYCnt+nY) * nDstXSize) + (nLastWidth+nX+nCheck) < 0 )
										return FALSE;

									pwDst[((nYCnt+nY) * nDstXSize) + (nLastWidth+nX+nCheck)] = ((bRedWantedColor  <<m_stBitsMaskInfo.bRShift) |
																							    (bGreenWantedColor<<m_stBitsMaskInfo.bGShift) |
																								(bBlueWantedColor <<m_stBitsMaskInfo.bBShift));			
								}
								x += (nCurrWidth-rc.left);
							}
							// rc.right???? ???????? ��???? ?????? ??? ???1?71?1?771?1?71?1?777.
							else if ( nLastWidth <= rc.right && rc.right < nCurrWidth )
							{
								for ( INT nCheck = 0; nCheck < rc.right-nLastWidth; nCheck++ )
								{
									wPixel	  = wChooseColor;
									bBlueWantedColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenWantedColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedWantedColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									wPixel	  = pwSrc[x+nCheck];
									bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
									rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
									bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

									bBlueWantedColor = (BYTE)(((FLOAT)bBlueWantedColor*rBlueRate));
									bGreenWantedColor = (BYTE)(((FLOAT)bGreenWantedColor*rGreenRate));
									bRedWantedColor = (BYTE)(((FLOAT)bRedWantedColor*bRedRate));

									if ( bBlueWantedColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
										bBlueWantedColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
									if ( bGreenWantedColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
										bGreenWantedColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
									if ( bRedWantedColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
										bRedWantedColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);

									pwDst[((nYCnt+nY) * nDstXSize) + (nLastWidth+nX+nCheck)] = ((bRedWantedColor  <<m_stBitsMaskInfo.bRShift) |
																								(bGreenWantedColor<<m_stBitsMaskInfo.bGShift) |
																								(bBlueWantedColor <<m_stBitsMaskInfo.bBShift));			
								}
								x += nCntCopyWord;
							}
							else
							{
								for ( INT nCheck = 0; nCheck < nCntCopyWord; nCheck++ )
								{
									wPixel	  = wChooseColor;
									bBlueWantedColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenWantedColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedWantedColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									wPixel	  = pwSrc[x+nCheck];
									bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
									rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
									bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

									bBlueWantedColor = (BYTE)(((FLOAT)bBlueWantedColor*rBlueRate));
									bGreenWantedColor = (BYTE)(((FLOAT)bGreenWantedColor*rGreenRate));
									bRedWantedColor = (BYTE)(((FLOAT)bRedWantedColor*bRedRate));

									if ( bBlueWantedColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
										bBlueWantedColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
									if ( bGreenWantedColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
										bGreenWantedColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
									if ( bRedWantedColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
										bRedWantedColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);

									pwDst[((nYCnt+nY) * nDstXSize) + (nLastWidth+nX+nCheck)] = ((bRedWantedColor  <<m_stBitsMaskInfo.bRShift) |
																								(bGreenWantedColor<<m_stBitsMaskInfo.bGShift) |
																								(bBlueWantedColor <<m_stBitsMaskInfo.bBShift));			
								}
								x += nCntCopyWord;
							}
						}
					}	
					else
					{
						return FALSE;
					}
				}
				// ?????? ???? ???? ?????? ???????? ??????.
				nWidthEnd++;

				nWidthStart	= nWidthEnd;
				nCurrWidth = 0;
			}
		}

		return TRUE;
	}
	return FALSE;
}


BOOL CWHDXGraphicWindow::DrawWithImageForComp(INT nX, INT nY, RECT* prcSrc, WORD* pwSrc, WORD wChooseColor1, WORD wChooseColor2, CDX9SoftSurface* pddsDst)
{
	RECT			rc;
	DDSURFACEDESC2	ddsd;
	CDX9SoftSurface* ddsDst;

	if ( !prcSrc || !pwSrc )
	{
		return FALSE;
	}

	if ( pddsDst )
	{
		ddsDst = pddsDst;
	}
	else
	{
		ddsDst = m_pddsBackBuffer;
	}

	INT	nWidth		= prcSrc->right-prcSrc->left;
	INT	nHeight		= prcSrc->bottom-prcSrc->top;
	INT	nXOffset	= prcSrc->left;
	INT	nYOffset	= prcSrc->top;
	INT	nStartX		= 0;
	INT	nStartY		= 0;
	INT	nEndX		= m_stDisplayInfo.wWidth -1;
	INT	nEndY		= m_stDisplayInfo.wHeight-1;

	if ( ddsDst != NULL )
	{
		if (nX < nStartX )
		{ 
			if ( nXOffset < -nX )
			{
				nXOffset = -nX;
			}
			nWidth	 = prcSrc->right - nXOffset;
		}
		if ( (nX+prcSrc->right-prcSrc->left-1) > nEndX )
		{
			nWidth	= nEndX - nX - nXOffset + 1;		
		}
		if ( nY < nStartY )	
		{ 
			if ( nYOffset < -nY )
			{
				nYOffset = -nY;
			}
			nHeight	 = prcSrc->bottom - nYOffset;
		}		
		if ( (nY+prcSrc->bottom-prcSrc->top-1) > nEndY )
		{
			nHeight	= nEndY - nY - nYOffset + 1;
		}

		if ( (nWidth > 0) && (nHeight > 0) )
		{
			rc.left		= nXOffset;
			rc.right	= nXOffset+nWidth;
			rc.top		= nYOffset;
			rc.bottom	= nYOffset+nHeight;

			LONG	lWidth;
			ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
			ddsd.dwSize = sizeof(DDSURFACEDESC2);
			if ( FAILED(ddsDst->Lock(NULL, &ddsd, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL)) )
			{
				return FALSE;
			}

			WORD* pwdDst = (WORD*)ddsd.lpSurface;
			lWidth		 = ddsd.lPitch >> 1;

			INT nWidthStart	= 0;
			INT nWidthEnd	= 0;
			INT nCurrWidth  = 0;
			INT nCntCopyWord = 0;
			INT nYCnt =0;
			INT nLastWidth = 0;

			FLOAT rBlueRate, rGreenRate, bRedRate;
			BYTE  bRedSrc, bGreenSrc, bBlueSrc;
			BYTE  bRedWantedColor, bGreenWantedColor, bBlueWantedColor;
			WORD  wPixel;

			// y?? ?????.
			for ( nYCnt=0 ; nYCnt < rc.top ; nYCnt++ )
			{
				nWidthEnd += pwSrc[nWidthStart];
				nWidthStart++;
				nWidthEnd++;
				nWidthStart	= nWidthEnd;
			}

			// y?? ????? ?????? ???��?? Count???? ?????.
			for ( nYCnt=rc.top ; nYCnt < rc.bottom ; nYCnt++ )
			{
				// ??????? ????? ???��?.(?????? ????)
				nWidthEnd += pwSrc[nWidthStart];
				nWidthStart++;

				// ?????? ?????? ??? ??????.
				for ( INT x = nWidthStart; x < nWidthEnd ; )
				{
					if ( pwSrc[x] == 0xC0 )
					{
						x++;
						nCntCopyWord = pwSrc[x];
						x++;
						nCurrWidth += nCntCopyWord;
					}
					else if ( pwSrc[x] == 0xC1 )
					{
						x++;
						nCntCopyWord = pwSrc[x];
						x++;

						nLastWidth = nCurrWidth;
						nCurrWidth += nCntCopyWord;

						if ( rc.left > nCurrWidth || rc.right < nLastWidth )
						{
							x += nCntCopyWord;
						}
						else
						{
							// rc.left???? ???????? ��???? ?????? ??? ???1?71?1?771?1?71?1?777.
							if ( nLastWidth < rc.left && rc.left <= nCurrWidth )
							{
								if ( ((nYCnt+nY) * lWidth) + (rc.left+nX) < 0 )
									return FALSE;

								x += (rc.left-nLastWidth);
								memcpy(&pwdDst[((nYCnt+nY) * lWidth) + (rc.left+nX)], &pwSrc[x], sizeof(WORD)*(nCurrWidth-rc.left));
								x += (nCurrWidth-rc.left);
							}
							// rc.right???? ???????? ��???? ?????? ??? ???1?71?1?771?1?71?1?777.
							else if ( nLastWidth <= rc.right && rc.right < nCurrWidth )
							{
								memcpy(&pwdDst[((nYCnt+nY) * lWidth) + (nLastWidth+nX)], &pwSrc[x], sizeof(WORD)*(rc.right-nLastWidth));
								x += nCntCopyWord;
							}
							else
							{
								memcpy(&pwdDst[((nYCnt+nY) * lWidth) + (nLastWidth+nX)], &pwSrc[x], sizeof(WORD)*nCntCopyWord);
								x += nCntCopyWord;
							}
						}
					}
					else if ( pwSrc[x] == 0xC2 || pwSrc[x] == 0xC3 )
					{
						WORD wDyingKind, wChooseColor;
						wDyingKind = pwSrc[x];
						switch ( wDyingKind )
						{
						case 0xC2: 
							wChooseColor = wChooseColor1;
							break;
						case 0xC3: 
							wChooseColor = wChooseColor2;
							break;
						}
						x++;
						nCntCopyWord = pwSrc[x];
						x++;

						nLastWidth = nCurrWidth;
						nCurrWidth += nCntCopyWord;

						if ( rc.left > nCurrWidth || rc.right < nLastWidth )
						{
							x += nCntCopyWord;
						}
						else
						{
							// rc.left???? ???????? ��???? ?????? ??? ???1?71?1?771?1?71?1?777.
							if ( nLastWidth < rc.left && rc.left <= nCurrWidth )
							{
								x += (rc.left-nLastWidth);
								for ( INT nCheck = 0; nCheck < nCurrWidth-rc.left; nCheck++ )
								{
									wPixel	  = wChooseColor;
									bBlueWantedColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenWantedColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedWantedColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									wPixel	  = pwSrc[x+nCheck];
									bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
									rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
									bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

									bBlueWantedColor = (BYTE)(((FLOAT)bBlueWantedColor*rBlueRate));
									bGreenWantedColor = (BYTE)(((FLOAT)bGreenWantedColor*rGreenRate));
									bRedWantedColor = (BYTE)(((FLOAT)bRedWantedColor*bRedRate));

									if ( bBlueWantedColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
										bBlueWantedColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
									if ( bGreenWantedColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
										bGreenWantedColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
									if ( bRedWantedColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
										bRedWantedColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);

									if ( ((nYCnt+nY) * lWidth) + (nLastWidth+nX+nCheck) < 0 )
										return FALSE;

									pwdDst[((nYCnt+nY) * lWidth) + (nLastWidth+nX+nCheck)] = ((bRedWantedColor  <<m_stBitsMaskInfo.bRShift) |
																										  (bGreenWantedColor<<m_stBitsMaskInfo.bGShift) |
																										  (bBlueWantedColor <<m_stBitsMaskInfo.bBShift));			
								}
								x += (nCurrWidth-rc.left);
							}
							// rc.right???? ???????? ��???? ?????? ??? ???1?71?1?771?1?71?1?777.
							else if ( nLastWidth <= rc.right && rc.right < nCurrWidth )
							{
								for ( INT nCheck = 0; nCheck < rc.right-nLastWidth; nCheck++ )
								{
									wPixel	  = wChooseColor;
									bBlueWantedColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenWantedColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedWantedColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									wPixel	  = pwSrc[x+nCheck];
									bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
									rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
									bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

									bBlueWantedColor = (BYTE)(((FLOAT)bBlueWantedColor*rBlueRate));
									bGreenWantedColor = (BYTE)(((FLOAT)bGreenWantedColor*rGreenRate));
									bRedWantedColor = (BYTE)(((FLOAT)bRedWantedColor*bRedRate));

									if ( bBlueWantedColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
										bBlueWantedColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
									if ( bGreenWantedColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
										bGreenWantedColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
									if ( bRedWantedColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
										bRedWantedColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);

									pwdDst[((nYCnt+nY) * lWidth) + (nLastWidth+nX+nCheck)] = ((bRedWantedColor  <<m_stBitsMaskInfo.bRShift) |
																										  (bGreenWantedColor<<m_stBitsMaskInfo.bGShift) |
																										  (bBlueWantedColor <<m_stBitsMaskInfo.bBShift));			
								}
								x += nCntCopyWord;
							}
							else
							{
								for ( INT nCheck = 0; nCheck < nCntCopyWord; nCheck++ )
								{
									wPixel	  = wChooseColor;
									bBlueWantedColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenWantedColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedWantedColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									wPixel	  = pwSrc[x+nCheck];
									bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
									rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
									bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

									bBlueWantedColor = (BYTE)(((FLOAT)bBlueWantedColor*rBlueRate));
									bGreenWantedColor = (BYTE)(((FLOAT)bGreenWantedColor*rGreenRate));
									bRedWantedColor = (BYTE)(((FLOAT)bRedWantedColor*bRedRate));

									if ( bBlueWantedColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
										bBlueWantedColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
									if ( bGreenWantedColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
										bGreenWantedColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
									if ( bRedWantedColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
										bRedWantedColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);

									pwdDst[((nYCnt+nY) * lWidth) + (nLastWidth+nX+nCheck)] = ((bRedWantedColor  <<m_stBitsMaskInfo.bRShift) |
																										  (bGreenWantedColor<<m_stBitsMaskInfo.bGShift) |
																									 	  (bBlueWantedColor <<m_stBitsMaskInfo.bBShift));			
								}
								x += nCntCopyWord;
							}
						}
					}	
					else
					{
						ddsDst->Unlock(NULL);
						return FALSE;
					}
				}
				// ?????? ???? ???? ?????? ???????? ??????.
				nWidthEnd++;

				nWidthStart	= nWidthEnd;
				nCurrWidth = 0;
			}
		}

		m_pddsBackBuffer->Unlock(NULL);
		return TRUE;
	}
	return FALSE;
}


BOOL CWHDXGraphicWindow::DrawWithImageForComp(
	INT nX, INT nY, INT nXSize, INT nYSize, WORD* pwSrc, WORD wChooseColor1, WORD wChooseColor2)
{
	RECT			rc;
	int				imgW;
	int				imgH;
	DDSURFACEDESC2	ddsd;


	if ( IS_OUTSIDE( nX, nY, nXSize, nYSize ) )
		return TRUE;

	CREATE_CLIP_REGION( nX, nY, nXSize, nYSize );

	LONG	lWidth;
	ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
	ddsd.dwSize = sizeof(DDSURFACEDESC2);
	if ( FAILED(m_pddsBackBuffer->Lock(NULL, &ddsd, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL)) )
	{
		return FALSE;
	}

	lWidth = ddsd.lPitch >> 1;


	WORD *dst = (WORD *) ddsd.lpSurface + (nY * lWidth) + nX;
	WORD *src = (WORD *) pwSrc;
	WORD *nxt, *tmp;
	
	int   y, pixelCnt, pixelPos, curPos;
	WORD  cc;         // choosecolor	
	BYTE  Rc, Gc, Bc; // choosecolor's RGB
	BYTE  Rs, Gs, Bs; // sourcecolor's RGB
	float Rr, Gr, Br; // sourcecolor's RGB Rate

	// Y ??? ?????
	for ( y = 0; y < rc.top; y++ )
		src	+= *src + 1;

	for ( y = nY; y < nY + imgH; y++ )
	{
		// [DX9][T77] �޸�: nxt Ӧ=�����ݽ���(ǰ׺��+�п�), ԭ +1 ����1��WORD
		// �����һ����ǰ׺��RLE���BADCODE����֡����(������/����/��������ȫ��ʧ����)
		nxt		= src + *src++;
		curPos	= 0;

		while ( src < nxt )
		{
			if ( *src == 0xC0 )
			{
				++src;
				curPos += *src++;
			}
			else if ( *src == 0xC1 )
			{
				++src;
				pixelCnt = *src++;

				if ( curPos + pixelCnt > rc.left && curPos < rc.right )
				{
					// ???? ?????
					if ( curPos < rc.left )
					{
						src		 += (rc.left - curPos);
						pixelCnt -= (rc.left - curPos);
						curPos	  = rc.left;
					}
								
					// ???? ?????
					if ( curPos + pixelCnt > rc.right )
						memcpy( dst + curPos - rc.left, src, (rc.right - curPos) << 1 );
					else
						memcpy( dst + curPos - rc.left, src, pixelCnt << 1 );
				}

				src    += pixelCnt;
				curPos += pixelCnt;
			}
			else if ( *src == 0xC2 || *src == 0xC3 )				
			{
				cc = *src == 0xC2 ? wChooseColor1 : wChooseColor2;

				// ???? RGB
				Rc = (BYTE)((cc & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);
				Gc = (BYTE)((cc & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
				Bc = (BYTE)((cc & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);

				++src;
				pixelCnt = *src++;


				if ( curPos + pixelCnt > rc.left && curPos < rc.right )
				{				
					// ???? ?????
					if ( curPos < rc.left )
					{
						src		 += (rc.left - curPos);
						pixelCnt -= (rc.left - curPos);
						curPos	  = rc.left;
					}

					tmp = dst + curPos - rc.left;

					for ( pixelPos = 0; pixelPos < pixelCnt; pixelPos++ )
					{
						// ???? ?????
						if ( curPos + pixelPos > rc.right )
							break;

						cc = src[pixelPos];
						
						// ???? RGB
						Rs = (BYTE)((cc & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);
						Gs = (BYTE)((cc & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
						Bs = (BYTE)((cc & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
						
						// ???? RGB Rate
						Rr = (float)((float) Rs / (float)(m_stBitsMaskInfo.dwRMask >> m_stBitsMaskInfo.bRShift));
						Gr = (float)((float) Gs / (float)(m_stBitsMaskInfo.dwGMask >> m_stBitsMaskInfo.bGShift));
						Br = (float)((float) Bs / (float)(m_stBitsMaskInfo.dwBMask >> m_stBitsMaskInfo.bBShift));
						
						// ???? ???? RGB
						Rs = (BYTE)((float) Rc * Rr);
						Gs = (BYTE)((float) Gc * Gr);
						Bs = (BYTE)((float) Bc * Br);
						
						*tmp++ = Rs << m_stBitsMaskInfo.bRShift |
								 Gs << m_stBitsMaskInfo.bGShift |
								 Bs << m_stBitsMaskInfo.bBShift;
					}
				}

				src	   += pixelCnt;
				curPos += pixelCnt;
			}
			else
			{
				m_pddsBackBuffer->Unlock(NULL);
				return FALSE;
			}
		}

		dst += lWidth;
	}
	
	m_pddsBackBuffer->Unlock(NULL);


	return TRUE;
}



// [DX9][T21] RLE ѹ�� wil ͼ�������������� blit ���� backbuffer��
// ��;���� 640x480 ԭ���¼/ѡ�������ȱȷŴ����� 800x600��ͬΪ 4:3����ʧ�棩��
BOOL CWHDXGraphicWindow::DrawWithImageForCompScaled(
	INT nX, INT nY, INT nSrcXSize, INT nSrcYSize, WORD* pwSrc,
	INT nDstXSize, INT nDstYSize, WORD wChooseColor1, WORD wChooseColor2)
{
	if ( !pwSrc || nSrcXSize <= 0 || nSrcYSize <= 0 || nDstXSize <= 0 || nDstYSize <= 0 )
		return FALSE;

	// [DX9][T26] �ߴ���ͬҲ�� ToMem+blit���ɸ�ʽ RLE(�ް汾ħ��/800����) �� DrawWithImageForComp ������ƥ��ʱ������ʧ
	// 1) RLE ���뵽��ʱ 565 ����
	WORD* pwTemp = (WORD*)malloc((size_t)nSrcXSize * (size_t)nSrcYSize * sizeof(WORD));
	if ( !pwTemp )
		return FALSE;
	RECT rcWanted;
	SetRect(&rcWanted, 0, 0, nSrcXSize, nSrcYSize);
	if ( !DrawWithImageForCompToMem(rcWanted, pwSrc, wChooseColor1, wChooseColor2, pwTemp) )
	{
		free(pwTemp);
		return FALSE;
	}

	// 2) ���� backbuffer����������� blit
	DDSURFACEDESC2 ddsd;
	ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
	ddsd.dwSize = sizeof(DDSURFACEDESC2);
	if ( FAILED(m_pddsBackBuffer->Lock(NULL, &ddsd, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL)) )
	{
		free(pwTemp);
		return FALSE;
	}
	LONG lWidth = ddsd.lPitch >> 1;
	WORD* pDst = (WORD*)ddsd.lpSurface;

	for ( INT dy = 0; dy < nDstYSize; dy++ )
	{
		INT sy = (INT)(((FLOAT)dy * (FLOAT)nSrcYSize) / (FLOAT)nDstYSize);
		if ( sy >= nSrcYSize ) sy = nSrcYSize - 1;
		if ( sy < 0 ) sy = 0;
		WORD* dRow = pDst + ((nY + dy) * lWidth) + nX;
		WORD* sRow = pwTemp + sy * nSrcXSize;
		for ( INT dx = 0; dx < nDstXSize; dx++ )
		{
			INT sx = (INT)(((FLOAT)dx * (FLOAT)nSrcXSize) / (FLOAT)nDstXSize);
			if ( sx >= nSrcXSize ) sx = nSrcXSize - 1;
			if ( sx < 0 ) sx = 0;
			dRow[dx] = sRow[sx];
		}
	}
	m_pddsBackBuffer->Unlock(NULL);
	free(pwTemp);
	return TRUE;
}


// [DX9][B] AVI ��ǰ֡���Ƶ��� backbuffer�����ţ�
BOOL CWHDXGraphicWindow::DrawAviFrame(CAvi* pAvi, INT nX, INT nY, INT nDstW, INT nDstH)
{
	if ( !pAvi || !pAvi->m_lpOutput || !m_pddsBackBuffer ) return FALSE;
	DDSURFACEDESC2 ddsd;
	ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
	ddsd.dwSize = sizeof(DDSURFACEDESC2);
	if ( FAILED(m_pddsBackBuffer->Lock(NULL, &ddsd, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL)) ) return FALSE;
	LONG lWidth = ddsd.lPitch >> 1;
	WORD* pDst = (WORD*)ddsd.lpSurface + (LONG)nY * lWidth + nX;
	pAvi->DrawFrame565(pDst, ddsd.lPitch, nDstW, nDstH);
	m_pddsBackBuffer->Unlock(NULL);
	return TRUE;
}


// [DX9][B] ���� BMP(24bit) ת 565 ���壨�Ե����� DIB �� ������ 565��
WORD* CWHDXGraphicWindow::LoadBmp565(LPCSTR szPath, INT* pW, INT* pH)
{
	*pW = *pH = 0;
	HBITMAP hbm = (HBITMAP)LoadImageA(NULL, szPath, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	if ( !hbm ) return NULL;
	BITMAP bm;
	GetObjectA(hbm, sizeof(BITMAP), &bm);
	INT w = bm.bmWidth, h = bm.bmHeight;
	BITMAPINFOHEADER bi;
	ZeroMemory(&bi, sizeof(bi));
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = w;
	bi.biHeight = h;
	bi.biPlanes = 1;
	bi.biBitCount = 24;
	bi.biCompression = BI_RGB;
	BYTE* pPix = (BYTE*)malloc((size_t)w * (size_t)h * 3);
	HDC hdc = GetDC(NULL);
	INT nGot = GetDIBits(hdc, hbm, 0, h, pPix, (BITMAPINFO*)&bi, DIB_RGB_COLORS);
	ReleaseDC(NULL, hdc);
	DeleteObject(hbm);
	if ( nGot == 0 ) { free(pPix); return NULL; }
	WORD* pBuf = (WORD*)malloc((size_t)w * (size_t)h * 2);
	BYTE* src = pPix;
	for ( INT y = 0; y < h; y++ )
	{
		WORD* dRow = pBuf + (LONG)(h - 1 - y) * (LONG)w;   // DIB �Ե����� �� 565 ������
		BYTE* sRow = src + (LONG)y * (LONG)w * 3;
		for ( INT x = 0; x < w; x++ )
		{
			BYTE* p = sRow + (LONG)x * 3;   // BGR
			dRow[x] = (WORD)(((p[2] >> 3) << 11) | ((p[1] >> 2) << 5) | (p[0] >> 3));
		}
	}
	free(pPix);
	*pW = w; *pH = h;
	return pBuf;
}

void CWHDXGraphicWindow::FreeBmp565(WORD* pBuf)
{
	if ( pBuf ) free(pBuf);
}

// [DX9][B] ԭʼ565������������Ż��ƣ������ⲿͼ�񱳾����� wil RLE ���ݣ�
BOOL CWHDXGraphicWindow::Draw565Scaled(INT nX, INT nY, INT nSrcW, INT nSrcH, WORD* pwSrc, INT nDstW, INT nDstH)
{
	if ( !pwSrc || nSrcW <= 0 || nSrcH <= 0 || nDstW <= 0 || nDstH <= 0 ) return FALSE;
	DDSURFACEDESC2 ddsd;
	ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
	ddsd.dwSize = sizeof(DDSURFACEDESC2);
	if ( FAILED(m_pddsBackBuffer->Lock(NULL, &ddsd, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL)) ) return FALSE;
	LONG lWidth = ddsd.lPitch >> 1;
	WORD* pDst = (WORD*)ddsd.lpSurface;
	for ( INT dy = 0; dy < nDstH; dy++ )
	{
		INT sy = (INT)(((FLOAT)dy * (FLOAT)nSrcH) / (FLOAT)nDstH);
		if ( sy >= nSrcH ) sy = nSrcH - 1;
		if ( sy < 0 ) sy = 0;
		WORD* dRow = pDst + ((nY + dy) * lWidth) + nX;
		WORD* sRow = pwSrc + (LONG)sy * nSrcW;
		for ( INT dx = 0; dx < nDstW; dx++ )
		{
			INT sx = (INT)(((FLOAT)dx * (FLOAT)nSrcW) / (FLOAT)nDstW);
			if ( sx >= nSrcW ) sx = nSrcW - 1;
			if ( sx < 0 ) sx = 0;
			dRow[dx] = sRow[sx];
		}
	}
	m_pddsBackBuffer->Unlock(NULL);
	return TRUE;
}

// [DX9][T26] ��ϣ������� backbuffer(16bit 565) dump Ϊ 24bit BMP �ļ�
void CWHDXGraphicWindow::DebugDumpBackBuffer(const char* szPath)
{
	if ( !m_pSoftBackBuffer || !m_dwSoftBufSize || !szPath )
		return;

	INT nW = m_stDisplayInfo.wWidth;
	INT nH = m_stDisplayInfo.wHeight;
	if ( nW <= 0 || nH <= 0 )
		return;

	INT nRowBytes = nW * 3;
	INT nPad = (4 - (nRowBytes % 4)) % 4;
	DWORD dwDataSize = (DWORD)(nRowBytes + nPad) * (DWORD)nH;
	DWORD dwFileSize = 54 + dwDataSize;

	FILE* fp = fopen(szPath, "wb");
	if ( !fp )
		return;

	BITMAPFILEHEADER bf;
	BITMAPINFOHEADER bi;
	ZeroMemory(&bf, sizeof(bf));
	ZeroMemory(&bi, sizeof(bi));
	bf.bfType = 0x4D42; // 'BM'
	bf.bfSize = dwFileSize;
	bf.bfOffBits = 54;
	bi.biSize = 40;
	bi.biWidth = nW;
	bi.biHeight = nH;
	bi.biPlanes = 1;
	bi.biBitCount = 24;
	bi.biSizeImage = dwDataSize;
	fwrite(&bf, sizeof(bf), 1, fp);
	fwrite(&bi, sizeof(bi), 1, fp);

	BYTE* pLine = new BYTE[nRowBytes + nPad];
	if ( !pLine )
	{
		fclose(fp);
		return;
	}
	for ( INT y = 0; y < nH; y++ )
	{
		const WORD* pSrc = m_pSoftBackBuffer + (INT)((nH - 1 - y) * m_stDisplayInfo.wWidth); // BMP �Ե�����
		BYTE* pD = pLine;
		for ( INT x = 0; x < nW; x++ )
		{
			WORD c = pSrc[x];
			BYTE r = (BYTE)(((c >> 11) & 0x1F) << 3);
			BYTE g = (BYTE)(((c >> 5) & 0x3F) << 2);
			BYTE b = (BYTE)((c & 0x1F) << 3);
			*pD++ = b;
			*pD++ = g;
			*pD++ = r;
		}
		fwrite(pLine, 1, nRowBytes + nPad, fp);
	}
	delete[] pLine;
	fclose(fp);
}



BOOL CWHDXGraphicWindow::DrawWithImageForCompClipRgnBase(INT nX, INT nY, INT nXSize, INT nYSize, WORD* pwSrc, WORD wClipWidth, WORD wClipHeight, WORD wChooseColor1, WORD wChooseColor2)
{
	RECT			rc;
	DDSURFACEDESC2	ddsd;

	INT	nWidth		= nXSize;
	INT	nHeight		= nYSize;
	INT	nXOffset	= 0;
	INT	nYOffset	= 0;
	INT	nStartX		= 0;
	INT	nStartY		= 0;
	INT	nEndX		= wClipWidth -1;
	INT	nEndY		= wClipHeight-1;

	if ( m_pddsBackBuffer != NULL )
	{
		if (nX < nStartX )
		{ 
			nXOffset	= nStartX - nX;
			nWidth		= nXSize - nXOffset;
		}
		if ( (nX+nXSize-1) > nEndX )
			nWidth		= nEndX - nX - nXOffset + 1;		
		if ( nY < nStartY )	
		{ 
			nYOffset	= nStartY - nY;
			nHeight		= nYSize - nYOffset;
		}		
		if ( (nY+nYSize-1) > nEndY )
			nHeight	= nEndY - nY - nYOffset + 1;

		if ( (nWidth > 0) && (nHeight > 0) )
		{
			rc.left		= nXOffset;
			rc.right	= nXOffset+nWidth;
			rc.top		= nYOffset;
			rc.bottom	= nYOffset+nHeight;

			LONG	lWidth;
			ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
			ddsd.dwSize = sizeof(DDSURFACEDESC2);
			if ( FAILED(m_pddsBackBuffer->Lock(NULL, &ddsd, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL)) )
			{
				m_pddsBackBuffer->Unlock(NULL);
				return FALSE;
			}

			WORD* pwdDst = (WORD*)ddsd.lpSurface;
			lWidth		 = ddsd.lPitch >> 1;

			INT nWidthStart	= 0;
			INT nWidthEnd	= 0;
			INT nCurrWidth  = 0;
			INT nCntCopyWord = 0;
			INT nYCnt =0;
			INT nLastWidth = 0;

			FLOAT rBlueRate, rGreenRate, bRedRate;
			BYTE  bRedSrc, bGreenSrc, bBlueSrc;
			BYTE  bRedWantedColor, bGreenWantedColor, bBlueWantedColor;
			WORD  wPixel;

			// y?? ?????.
  			for ( nYCnt=0 ; nYCnt < rc.top ; nYCnt++ )
			{
				nWidthEnd += pwSrc[nWidthStart];
				nWidthStart++;
				nWidthEnd++;
				nWidthStart	= nWidthEnd;
			}

			// y?? ????? ?????? ???��?? Count???? ?????.
			for ( nYCnt=rc.top ; nYCnt < rc.bottom ; nYCnt++ )
			{
				// ??????? ????? ???��?.(?????? ????)
				nWidthEnd += pwSrc[nWidthStart];
				nWidthStart++;

				// ?????? ?????? ??? ??????.
				for ( INT x = nWidthStart; x < nWidthEnd ; )
				{
					if ( pwSrc[x] == 0xC0 )
					{
						x++;
						nCntCopyWord = pwSrc[x];
						x++;
						nCurrWidth += nCntCopyWord;
					}
					else if ( pwSrc[x] == 0xC1 )
					{
						x++;
						nCntCopyWord = pwSrc[x];
						x++;

						nLastWidth = nCurrWidth;
						nCurrWidth += nCntCopyWord;

						if ( rc.left > nCurrWidth || rc.right < nLastWidth )
						{
							x += nCntCopyWord;
						}
						else
						{
							// rc.left???? ???????? ��???? ?????? ??? ???1?71?1?771?1?71?1?777.
							if ( nLastWidth < rc.left && rc.left <= nCurrWidth )
							{
								if ( ((nYCnt+nY) * lWidth) + (rc.left+nX) < 0 )
									return FALSE;

								x += (rc.left-nLastWidth);
								memcpy(&pwdDst[((nYCnt+nY) * lWidth) + (rc.left+nX)], &pwSrc[x], sizeof(WORD)*(nCurrWidth-rc.left));
								x += (nCurrWidth-rc.left);
							}
							// rc.right???? ???????? ��???? ?????? ??? ???1?71?1?771?1?71?1?777.
							else if ( nLastWidth <= rc.right && rc.right < nCurrWidth )
							{
								memcpy(&pwdDst[((nYCnt+nY) * lWidth) + (nLastWidth+nX)], &pwSrc[x], sizeof(WORD)*(rc.right-nLastWidth));
								x += nCntCopyWord;
							}
							else
							{
								memcpy(&pwdDst[((nYCnt+nY) * lWidth) + (nLastWidth+nX)], &pwSrc[x], sizeof(WORD)*nCntCopyWord);
								x += nCntCopyWord;
							}
						}
					}
					else if ( pwSrc[x] == 0xC2 || pwSrc[x] == 0xC3 )
					{
						WORD wDyingKind, wChooseColor;
						wDyingKind = pwSrc[x];
						switch ( wDyingKind )
						{
						case 0xC2: 
							wChooseColor = wChooseColor1;
							break;
						case 0xC3: 
							wChooseColor = wChooseColor2;
							break;
						}
						x++;
						nCntCopyWord = pwSrc[x];
						x++;

						nLastWidth = nCurrWidth;
						nCurrWidth += nCntCopyWord;

						if ( rc.left > nCurrWidth || rc.right < nLastWidth )
						{
							x += nCntCopyWord;
						}
						else
						{
							// rc.left???? ???????? ��???? ?????? ??? ???1?71?1?771?1?71?1?777.
							if ( nLastWidth < rc.left && rc.left <= nCurrWidth )
							{
								x += (rc.left-nLastWidth);
								for ( INT nCheck = 0; nCheck < nCurrWidth-rc.left; nCheck++ )
								{
									wPixel	  = wChooseColor;
									bBlueWantedColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenWantedColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedWantedColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									wPixel	  = pwSrc[x+nCheck];
									bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
									rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
									bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

									bBlueWantedColor = (BYTE)(((FLOAT)bBlueWantedColor*rBlueRate));
									bGreenWantedColor = (BYTE)(((FLOAT)bGreenWantedColor*rGreenRate));
									bRedWantedColor = (BYTE)(((FLOAT)bRedWantedColor*bRedRate));

									if ( bBlueWantedColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
										bBlueWantedColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
									if ( bGreenWantedColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
										bGreenWantedColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
									if ( bRedWantedColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
										bRedWantedColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);

									if ( ((nYCnt+nY) * lWidth) + (nLastWidth+nX+nCheck) < 0 )
										return FALSE;

									pwdDst[((nYCnt+nY) * lWidth) + (nLastWidth+nX+nCheck)] = ((bRedWantedColor  <<m_stBitsMaskInfo.bRShift) |
																										  (bGreenWantedColor<<m_stBitsMaskInfo.bGShift) |
																									 	  (bBlueWantedColor <<m_stBitsMaskInfo.bBShift));			
								}
								x += (nCurrWidth-rc.left);
							}
							// rc.right???? ???????? ��???? ?????? ??? ???1?71?1?771?1?71?1?777.
							else if ( nLastWidth <= rc.right && rc.right < nCurrWidth )
							{
								for ( INT nCheck = 0; nCheck < rc.right-nLastWidth; nCheck++ )
								{
									wPixel	  = wChooseColor;
									bBlueWantedColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenWantedColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedWantedColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									wPixel	  = pwSrc[x+nCheck];
									bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
									rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
									bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

									bBlueWantedColor = (BYTE)(((FLOAT)bBlueWantedColor*rBlueRate));
									bGreenWantedColor = (BYTE)(((FLOAT)bGreenWantedColor*rGreenRate));
									bRedWantedColor = (BYTE)(((FLOAT)bRedWantedColor*bRedRate));

									if ( bBlueWantedColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
										bBlueWantedColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
									if ( bGreenWantedColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
										bGreenWantedColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
									if ( bRedWantedColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
										bRedWantedColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);

									pwdDst[((nYCnt+nY) * lWidth) + (nLastWidth+nX+nCheck)] = ((bRedWantedColor  <<m_stBitsMaskInfo.bRShift) |
																										  (bGreenWantedColor<<m_stBitsMaskInfo.bGShift) |
																									 	  (bBlueWantedColor <<m_stBitsMaskInfo.bBShift));			
								}
								x += nCntCopyWord;
							}
							else
							{
								for ( INT nCheck = 0; nCheck < nCntCopyWord; nCheck++ )
								{
									wPixel	  = wChooseColor;
									bBlueWantedColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenWantedColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedWantedColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									wPixel	  = pwSrc[x+nCheck];
									bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
									rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
									bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

									bBlueWantedColor = (BYTE)(((FLOAT)bBlueWantedColor*rBlueRate));
									bGreenWantedColor = (BYTE)(((FLOAT)bGreenWantedColor*rGreenRate));
									bRedWantedColor = (BYTE)(((FLOAT)bRedWantedColor*bRedRate));

									if ( bBlueWantedColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
										bBlueWantedColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
									if ( bGreenWantedColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
										bGreenWantedColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
									if ( bRedWantedColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
										bRedWantedColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);

									pwdDst[((nYCnt+nY) * lWidth) + (nLastWidth+nX+nCheck)] = ((bRedWantedColor  <<m_stBitsMaskInfo.bRShift) |
																										  (bGreenWantedColor<<m_stBitsMaskInfo.bGShift) |
																									 	  (bBlueWantedColor <<m_stBitsMaskInfo.bBShift));			
								}
								x += nCntCopyWord;
							}
						}
					}	
					else
					{
						m_pddsBackBuffer->Unlock(NULL);
						return FALSE;
					}
				}
				// ?????? ???? ???? ?????? ???????? ??????.
				nWidthEnd++;

				nWidthStart	= nWidthEnd;
				nCurrWidth = 0;
			}
		}

		m_pddsBackBuffer->Unlock(NULL);
		return TRUE;
	}
	return FALSE;
}

BOOL CWHDXGraphicWindow::DrawWithImageForCompClipRgnBaseNoColor(INT nX, INT nY, INT nXSize, INT nYSize, WORD* pwSrc, WORD wClipWidth, WORD wClipHeight)
{
	RECT			rc;
	DDSURFACEDESC2	ddsd;

	INT	nWidth		= nXSize;
	INT	nHeight		= nYSize;
	INT	nXOffset	= 0;
	INT	nYOffset	= 0;
	INT	nStartX		= 0;
	INT	nStartY		= 0;
	INT	nEndX		= wClipWidth -1;
	INT	nEndY		= wClipHeight-1;

	if ( m_pddsBackBuffer != NULL )
	{
		if (nX < nStartX )
		{ 
			nXOffset	= nStartX - nX;
			nWidth		= nXSize - nXOffset;
		}
		if ( (nX+nXSize-1) > nEndX )
			nWidth		= nEndX - nX - nXOffset + 1;		
		if ( nY < nStartY )	
		{ 
			nYOffset	= nStartY - nY;
			nHeight		= nYSize - nYOffset;
		}		
		if ( (nY+nYSize-1) > nEndY )
			nHeight	= nEndY - nY - nYOffset + 1;

		if ( (nWidth > 0) && (nHeight > 0) )
		{
			rc.left		= nXOffset;
			rc.right	= nXOffset+nWidth;
			rc.top		= nYOffset;
			rc.bottom	= nYOffset+nHeight;

			LONG	lWidth;
			ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
			ddsd.dwSize = sizeof(DDSURFACEDESC2);
			if ( FAILED(m_pddsBackBuffer->Lock(NULL, &ddsd, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL)) )
			{
				return FALSE;
			}

			WORD* pwdDst = (WORD*)ddsd.lpSurface;
			lWidth		 = ddsd.lPitch >> 1;

			INT nWidthStart	= 0;
			INT nWidthEnd	= 0;
			INT nCurrWidth  = 0;
			INT nCntCopyWord = 0;
			INT nYCnt =0;
			INT nLastWidth = 0;

			// y?? ?????.
  			for ( nYCnt=0 ; nYCnt < rc.top ; nYCnt++ )
			{
				nWidthEnd += pwSrc[nWidthStart];
				nWidthStart++;
				nWidthEnd++;
				nWidthStart	= nWidthEnd;
			}

			// y?? ????? ?????? ???��?? Count???? ?????.
			for ( nYCnt=rc.top ; nYCnt < rc.bottom ; nYCnt++ )
			{
				// ??????? ????? ???��?.(?????? ????)
				nWidthEnd += pwSrc[nWidthStart];
				nWidthStart++;

				// ?????? ?????? ??? ??????.
				for ( INT x = nWidthStart; x < nWidthEnd ; )
				{
					if ( pwSrc[x] == 0xC0 )
					{
						x++;
						nCntCopyWord = pwSrc[x];
						x++;
						nCurrWidth += nCntCopyWord;
					}
					else 
					{
						x++;
						nCntCopyWord = pwSrc[x];
						x++;

						nLastWidth = nCurrWidth;
						nCurrWidth += nCntCopyWord;

						if ( rc.left > nCurrWidth || rc.right < nLastWidth )
						{
							x += nCntCopyWord;
						}
						else
						{
							// rc.left???? ???????? ��???? ?????? ??? ???1?71?1?771?1?71?1?777.
							if ( nLastWidth < rc.left && rc.left <= nCurrWidth )
							{
								if ( ((nYCnt+nY) * lWidth) + (rc.left+nX) < 0 )
									return FALSE;

								x += (rc.left-nLastWidth);
								memcpy(&pwdDst[((nYCnt+nY) * lWidth) + (rc.left+nX)], &pwSrc[x], sizeof(WORD)*(nCurrWidth-rc.left));
								x += (nCurrWidth-rc.left);
							}
							// rc.right???? ???????? ��???? ?????? ??? ???1?71?1?771?1?71?1?777.
							else if ( nLastWidth <= rc.right && rc.right < nCurrWidth )
							{
								memcpy(&pwdDst[((nYCnt+nY) * lWidth) + (nLastWidth+nX)], &pwSrc[x], sizeof(WORD)*(rc.right-nLastWidth));
								x += nCntCopyWord;
							}
							else
							{
								memcpy(&pwdDst[((nYCnt+nY) * lWidth) + (nLastWidth+nX)], &pwSrc[x], sizeof(WORD)*nCntCopyWord);
								x += nCntCopyWord;
							}
						}
					}
				}


				// ?????? ???? ???? ?????? ???????? ??????.
				nWidthEnd++;

				nWidthStart	= nWidthEnd;
				nCurrWidth = 0;
			}
		}

		m_pddsBackBuffer->Unlock(NULL);
		return TRUE;
	}
	return FALSE;
}


BOOL CWHDXGraphicWindow::DrawWithABlendCompDataWithBackBuffer(INT nX, INT nY, 
															  INT nXSize, INT nYSize, WORD* pwSrc,
															  WORD wClipWidth, WORD wClipHeight,
															  WORD wChooseColor1, WORD wChooseColor2, BYTE bOpa)
{
	RECT			rc;
	DDSURFACEDESC2	ddsd;

	INT	nWidth		= nXSize;
	INT	nHeight		= nYSize;
	INT	nXOffset	= 0;
	INT	nYOffset	= 0;
	INT	nStartX		= 0;
	INT	nStartY		= 0;
	INT	nEndX		= wClipWidth -1;
	INT	nEndY		= wClipHeight-1;

	WORD wDyingKind, wChooseColor;

	if ( m_pddsBackBuffer != NULL )
	{
		if (nX < nStartX )
		{ 
			nXOffset	= nStartX - nX;
			nWidth		= nXSize - nXOffset;
		}
		if ( (nX+nXSize-1) > nEndX )
			nWidth		= nEndX - nX - nXOffset + 1;		
		if ( nY < nStartY )	
		{ 
			nYOffset	= nStartY - nY;
			nHeight		= nYSize - nYOffset;
		}		
		if ( (nY+nYSize-1) > nEndY )
			nHeight	= nEndY - nY - nYOffset + 1;

		if ( (nWidth > 0) && (nHeight > 0) )
		{
			rc.left		= nXOffset;
			rc.right	= nXOffset+nWidth;
			rc.top		= nYOffset;
			rc.bottom	= nYOffset+nHeight;

			LONG	lWidth;
			ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
			ddsd.dwSize = sizeof(DDSURFACEDESC2);
			if ( FAILED(m_pddsBackBuffer->Lock(NULL, &ddsd, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL)) )
			{
				return FALSE;
			}

			WORD* pwdDst = (WORD*)ddsd.lpSurface;
			lWidth		 = ddsd.lPitch >> 1;

			INT nWidthStart	= 0;
			INT nWidthEnd	= 0;
			INT nCurrWidth  = 0;
			INT nCntCopyWord = 0;
			INT nYCnt =0;
			INT nLastWidth = 0;

			BYTE bRedDst, bGreenDst, bBlueDst;
			BYTE bRedSrc, bGreenSrc, bBlueSrc;
			BYTE bRedWantedColor, bGreenWantedColor, bBlueWantedColor;
			WORD wPixel;
			FLOAT rBlueRate, rGreenRate, bRedRate;

			// y?? ?????.
  			for ( nYCnt=0 ; nYCnt < rc.top ; nYCnt++ )
			{
				nWidthEnd += pwSrc[nWidthStart];
				nWidthStart++;
				nWidthEnd++;
				nWidthStart	= nWidthEnd;
			}

			// y?? ????? ?????? ???��?? Count???? ?????.
			for ( nYCnt=rc.top ; nYCnt < rc.bottom ; nYCnt++ )
			{
				// ??????? ????? ???��?.(?????? ????)
				nWidthEnd += pwSrc[nWidthStart];
				nWidthStart++;

				// ?????? ?????? ??? ??????.
				for ( INT x = nWidthStart; x < nWidthEnd ; )
				{
					if ( pwSrc[x] == 0xC0 )
					{
						x++;
						nCntCopyWord = pwSrc[x];
						x++;
						nCurrWidth += nCntCopyWord;
					}
					else if ( pwSrc[x] == 0xC1 )
					{
						x++;
						nCntCopyWord = pwSrc[x];
						x++;

						nLastWidth = nCurrWidth;
						nCurrWidth += nCntCopyWord;

						if ( rc.left > nCurrWidth || rc.right < nLastWidth )
						{
							x += nCntCopyWord;
						}
						else
						{
							// rc.left???? ???????? ��???? ?????? ??? ???1?71?1?771?1?71?1?777.
							if ( nLastWidth < rc.left && rc.left <= nCurrWidth )
							{
								x += (rc.left-nLastWidth);
								for ( INT nCheck = 0; nCheck < nCurrWidth-rc.left; nCheck++ )
								{
									wPixel	   = pwSrc[x+nCheck];
									bBlueSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedSrc    = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									wPixel	   = pwdDst[((nYCnt+nY) * lWidth) + (rc.left+nX+nCheck)];
									bBlueDst   = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenDst  = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedDst    = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									bBlueDst   = (BYTE)((bOpa*(bBlueDst -bBlueSrc )+100*bBlueSrc  ) / 100);
									bGreenDst  = (BYTE)((bOpa*(bGreenDst-bGreenSrc)+100*bGreenSrc ) / 100);
									bRedDst    = (BYTE)((bOpa*(bRedDst  -bRedSrc  )+100*bRedSrc   ) / 100);

									if ( ((nYCnt+nY) * lWidth) + (rc.left+nX+nCheck) < 0 )
										return FALSE;

									pwdDst[((nYCnt+nY) * lWidth) + (rc.left+nX+nCheck)] = ((bRedDst  <<m_stBitsMaskInfo.bRShift) |
																									   (bGreenDst<<m_stBitsMaskInfo.bGShift) |
																									   (bBlueDst <<m_stBitsMaskInfo.bBShift));										
								}
								x += (nCurrWidth-rc.left);
							}
							// rc.right???? ???????? ��???? ?????? ??? ???1?71?1?771?1?71?1?777.
							else if ( nLastWidth <= rc.right && rc.right < nCurrWidth )
							{
								for ( INT nCheck = 0; nCheck < rc.right-nLastWidth; nCheck++ )
								{
									wPixel	   = pwSrc[x+nCheck];
									bBlueSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedSrc    = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									wPixel	   = pwdDst[((nYCnt+nY) * lWidth) + (nLastWidth+nX+nCheck)];
									bBlueDst   = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenDst  = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedDst    = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									bBlueDst   = (BYTE)((bOpa*(bBlueDst -bBlueSrc )+100*bBlueSrc  ) / 100);
									bGreenDst  = (BYTE)((bOpa*(bGreenDst-bGreenSrc)+100*bGreenSrc ) / 100);
									bRedDst    = (BYTE)((bOpa*(bRedDst  -bRedSrc  )+100*bRedSrc   ) / 100);

									pwdDst[((nYCnt+nY) * lWidth) + (nLastWidth+nX+nCheck)] = ((bRedDst  <<m_stBitsMaskInfo.bRShift) |
																									      (bGreenDst<<m_stBitsMaskInfo.bGShift) |
																										  (bBlueDst <<m_stBitsMaskInfo.bBShift));			
								}
								x += nCntCopyWord;
							}
							else
							{
								for ( INT nCheck = 0; nCheck < nCntCopyWord; nCheck++ )
								{
									wPixel	   = pwSrc[x+nCheck];
									bBlueSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedSrc    = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									wPixel	   = pwdDst[((nYCnt+nY) * lWidth) + (nLastWidth+nX+nCheck)];
									bBlueDst   = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenDst  = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedDst    = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									bBlueDst   = (BYTE)((bOpa*(bBlueDst -bBlueSrc )+100*bBlueSrc  ) / 100);
									bGreenDst  = (BYTE)((bOpa*(bGreenDst-bGreenSrc)+100*bGreenSrc ) / 100);
									bRedDst    = (BYTE)((bOpa*(bRedDst  -bRedSrc  )+100*bRedSrc   ) / 100);

									pwdDst[((nYCnt+nY) * lWidth) + (nLastWidth+nX+nCheck)] = ((bRedDst  <<m_stBitsMaskInfo.bRShift) |
																									      (bGreenDst<<m_stBitsMaskInfo.bGShift) |
																										  (bBlueDst <<m_stBitsMaskInfo.bBShift));			
								}
								x += nCntCopyWord;
							}
						}
					}
					else if ( pwSrc[x] == 0xC2 || pwSrc[x] == 0xC3 )
					{
						wDyingKind = pwSrc[x];
						switch ( wDyingKind )
						{
						case 0xC2: 
							wChooseColor = wChooseColor1;
							break;
						case 0xC3: 
							wChooseColor = wChooseColor2;
							break;
						}
						x++;
						nCntCopyWord = pwSrc[x];
						x++;

						nLastWidth = nCurrWidth;
						nCurrWidth += nCntCopyWord;

						if ( rc.left > nCurrWidth || rc.right < nLastWidth )
						{
							x += nCntCopyWord;
						}
						else
						{
							// rc.left???? ???????? ��???? ?????? ??? ???1?71?1?771?1?71?1?777.
							if ( nLastWidth < rc.left && rc.left <= nCurrWidth )
							{
								x += (rc.left-nLastWidth);
								for ( INT nCheck = 0; nCheck < nCurrWidth-rc.left; nCheck++ )
								{
									wPixel	  = wChooseColor;
									bBlueWantedColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenWantedColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedWantedColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									wPixel	  = pwSrc[x+nCheck];
									bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
									rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
									bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

									bBlueWantedColor = (BYTE)(((FLOAT)bBlueWantedColor*rBlueRate));
									bGreenWantedColor = (BYTE)(((FLOAT)bGreenWantedColor*rGreenRate));
									bRedWantedColor = (BYTE)(((FLOAT)bRedWantedColor*bRedRate));

									if ( bBlueWantedColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
										bBlueWantedColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
									if ( bGreenWantedColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
										bGreenWantedColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
									if ( bRedWantedColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
										bRedWantedColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);


									wPixel  = pwdDst[((nYCnt+nY) * lWidth) + (rc.left+nX+nCheck)];
									bBlueDst   = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenDst  = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedDst    = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									bBlueDst   = (BYTE)((bOpa*(bBlueDst -bBlueWantedColor )+100*bBlueWantedColor ) / 100);
									bGreenDst  = (BYTE)((bOpa*(bGreenDst-bGreenWantedColor)+100*bGreenWantedColor) / 100);
									bRedDst    = (BYTE)((bOpa*(bRedDst  -bRedWantedColor  )+100*bRedWantedColor  ) / 100);

									if ( ((nYCnt+nY) * lWidth) + (rc.left+nX+nCheck) < 0 )
										return FALSE;

									pwdDst[((nYCnt+nY) * lWidth) + (rc.left+nX+nCheck)] = ((bRedDst  <<m_stBitsMaskInfo.bRShift) |
																									   (bGreenDst<<m_stBitsMaskInfo.bGShift) |
																									   (bBlueDst <<m_stBitsMaskInfo.bBShift));
								}
								x += (nCurrWidth-rc.left);
							}
							// rc.right???? ???????? ��???? ?????? ??? ???1?71?1?771?1?71?1?777.
							else if ( nLastWidth <= rc.right && rc.right < nCurrWidth )
							{
								for ( INT nCheck = 0; nCheck < rc.right-nLastWidth; nCheck++ )
								{
									wPixel	  = wChooseColor;
									bBlueWantedColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenWantedColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedWantedColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									wPixel	  = pwSrc[x+nCheck];
									bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
									rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
									bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

									bBlueWantedColor = (BYTE)(((FLOAT)bBlueWantedColor*rBlueRate));
									bGreenWantedColor = (BYTE)(((FLOAT)bGreenWantedColor*rGreenRate));
									bRedWantedColor = (BYTE)(((FLOAT)bRedWantedColor*bRedRate));

									if ( bBlueWantedColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
										bBlueWantedColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
									if ( bGreenWantedColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
										bGreenWantedColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
									if ( bRedWantedColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
										bRedWantedColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);


									wPixel  = pwdDst[((nYCnt+nY) * lWidth) + (nLastWidth+nX+nCheck)];
									bBlueDst   = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenDst  = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedDst    = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									bBlueDst   = (BYTE)((bOpa*(bBlueDst -bBlueWantedColor )+100*bBlueWantedColor ) / 100);
									bGreenDst  = (BYTE)((bOpa*(bGreenDst-bGreenWantedColor)+100*bGreenWantedColor) / 100);
									bRedDst    = (BYTE)((bOpa*(bRedDst  -bRedWantedColor  )+100*bRedWantedColor  ) / 100);

									pwdDst[((nYCnt+nY) * lWidth) + (nLastWidth+nX+nCheck)] = ((bRedDst  <<m_stBitsMaskInfo.bRShift) |
																									      (bGreenDst<<m_stBitsMaskInfo.bGShift) |
																										  (bBlueDst <<m_stBitsMaskInfo.bBShift));			
								}
								x += nCntCopyWord;
							}
							else
							{
								for ( INT nCheck = 0; nCheck < nCntCopyWord; nCheck++ )
								{
									wPixel	  = wChooseColor;
									bBlueWantedColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenWantedColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedWantedColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									wPixel	  = pwSrc[x+nCheck];
									bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
									rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
									bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

									bBlueWantedColor = (BYTE)(((FLOAT)bBlueWantedColor*rBlueRate));
									bGreenWantedColor = (BYTE)(((FLOAT)bGreenWantedColor*rGreenRate));
									bRedWantedColor = (BYTE)(((FLOAT)bRedWantedColor*bRedRate));

									if ( bBlueWantedColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
										bBlueWantedColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
									if ( bGreenWantedColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
										bGreenWantedColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
									if ( bRedWantedColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
										bRedWantedColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);


									wPixel  = pwdDst[((nYCnt+nY) * lWidth) + (nLastWidth+nX+nCheck)];
									bBlueDst   = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenDst  = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedDst    = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									bBlueDst   = (BYTE)((bOpa*(bBlueDst -bBlueWantedColor )+100*bBlueWantedColor ) / 100);
									bGreenDst  = (BYTE)((bOpa*(bGreenDst-bGreenWantedColor)+100*bGreenWantedColor) / 100);
									bRedDst    = (BYTE)((bOpa*(bRedDst  -bRedWantedColor  )+100*bRedWantedColor  ) / 100);

									pwdDst[((nYCnt+nY) * lWidth) + (nLastWidth+nX+nCheck)] = ((bRedDst  <<m_stBitsMaskInfo.bRShift) |
																									      (bGreenDst<<m_stBitsMaskInfo.bGShift) |
																										  (bBlueDst <<m_stBitsMaskInfo.bBShift));			
								}
								x += nCntCopyWord;
							}
						}
					}	
					else
					{
						m_pddsBackBuffer->Unlock(NULL);
						return FALSE;
					}
				}
				// ?????? ???? ???? ?????? ???????? ??????.
				nWidthEnd++;

				nWidthStart	= nWidthEnd;
				nCurrWidth = 0;
			}
		}

		m_pddsBackBuffer->Unlock(NULL);
		return TRUE;
	}
	return FALSE;
}



BOOL CWHDXGraphicWindow::DrawWithABlendCompDataWithBackBufferEx(INT nX, INT nY, 
															  INT nXSize, INT nYSize, WORD* pwSrc,
															  WORD wClipWidth, WORD wClipHeight,
															  WORD wChooseColor1, WORD wChooseColor2, BYTE bOpa)
{
	RECT			rc;
	int				imgW;
	int				imgH;
	DDSURFACEDESC2	ddsd;

	if ( IS_OUTSIDE( nX, nY, nXSize, nYSize ) )
		return TRUE;

	CREATE_CLIP_REGION( nX, nY, nXSize, nYSize );

	LONG	lWidth;
	ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
	ddsd.dwSize = sizeof(DDSURFACEDESC2);
	if ( FAILED(m_pddsBackBuffer->Lock(NULL, &ddsd, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL)) )
	{
		return FALSE;
	}

	WORD* pwdDst = (WORD*)ddsd.lpSurface;
	lWidth		 = ddsd.lPitch >> 1;

	WORD *dst = (WORD *) ddsd.lpSurface + (nY * lWidth) + nX;
	WORD *src = (WORD *) pwSrc;
	WORD *nxt, *tmp;
	
	int   y, pixelCnt, pixelPos, curPos;
	WORD  cc;				// choosecolor
	BYTE  Rc, Gc, Bc;		// choosecolor's RGB
	BYTE  Rs, Gs, Bs;		// sourcecolor's RGB
	float Rr, Gr, Br;		// sourcecolor's RGB Rate
	WORD  finalColor;		// final RGB

	// Y?? ?????
	for ( y = 0; y < rc.top; y++ )
		src	+= *src + 1;

	for ( y = nY; y < nY + imgH; y++ )
	{
		// [DX9][T77] �޸�: nxt Ӧ=�����ݽ���(ǰ׺��+�п�), ԭ +1 ����1��WORD��BADCODE��֡����
		nxt		= src + *src++;
		curPos	= 0;

		while ( src < nxt )
		{
			if ( *src == 0xC0 )
			{
				++src;
				curPos += *src++;
			}
			else if ( *src == 0xC1 )
			{
				++src;
				pixelCnt = *src++;

				if ( curPos + pixelCnt > rc.left && curPos < rc.right )
				{
					// ???? ?????
					if ( curPos < rc.left )
					{
						src		 += (rc.left - curPos);
						pixelCnt -= (rc.left - curPos);
						curPos	  = rc.left;
					}
								
					// ???? ?????
					if ( curPos + pixelCnt > rc.right )
						putAlpha32( dst + curPos - rc.left, src, rc.right - curPos, bOpa );
					else
						putAlpha32( dst + curPos - rc.left, src, pixelCnt, bOpa );
				}

				src    += pixelCnt;
				curPos += pixelCnt;
			}
			else if ( *src == 0xC2 || *src == 0xC3 )				
			{
				cc = *src == 0xC2 ? wChooseColor1 : wChooseColor2;

				// ???? RGB
				Rc = (BYTE)((cc & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);
				Gc = (BYTE)((cc & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
				Bc = (BYTE)((cc & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);

				++src;
				pixelCnt = *src++;


				if ( curPos + pixelCnt > rc.left && curPos < rc.right )
				{				
					// ???? ?????
					if ( curPos < rc.left )
					{
						src		 += (rc.left - curPos);
						pixelCnt -= (rc.left - curPos);
						curPos	  = rc.left;
					}

					tmp = dst + curPos - rc.left;

					for ( pixelPos = 0; pixelPos < pixelCnt; pixelPos++ )
					{
						// ???? ?????
						if ( curPos + pixelPos > rc.right )
							break;

						cc = src[pixelPos];
						
						// ???? RGB
						Rs = (BYTE)((cc & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);
						Gs = (BYTE)((cc & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
						Bs = (BYTE)((cc & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
						
						// ???? RGB Rate
						Rr = (float)((float) Rs / (float)(m_stBitsMaskInfo.dwRMask >> m_stBitsMaskInfo.bRShift));
						Gr = (float)((float) Gs / (float)(m_stBitsMaskInfo.dwGMask >> m_stBitsMaskInfo.bGShift));
						Br = (float)((float) Bs / (float)(m_stBitsMaskInfo.dwBMask >> m_stBitsMaskInfo.bBShift));
						
						// ???? ???? RGB
						Rs = (BYTE)((float) Rc * Rr);
						Gs = (BYTE)((float) Gc * Gr);
						Bs = (BYTE)((float) Bc * Br);

						finalColor = Rs << m_stBitsMaskInfo.bRShift | 
									 Gs << m_stBitsMaskInfo.bGShift | 
									 Bs << m_stBitsMaskInfo.bBShift;

						putAlpha32Pixel( tmp++, finalColor, bOpa );
					}
				}

				src	   += pixelCnt;
				curPos += pixelCnt;
			}
			else
			{
				m_pddsBackBuffer->Unlock(NULL);
				return FALSE;
			}
		}

		dst += lWidth;
	}
	
	m_pddsBackBuffer->Unlock(NULL);

	return TRUE;
}


BOOL CWHDXGraphicWindow::DrawWithShadowABlend(INT nX, INT nY, INT nXSize, INT nYSize, WORD* pwSrc, WORD wClipWidth, WORD wClipHeight,
											  WORD* pwShadowClrSrc, BOOL bBlend, BYTE bShadowType, BYTE bOpa)

{
	RECT			rc;
	DDSURFACEDESC2	ddsd;

	INT	nXOffset = 0;
	INT	nYOffset = 0;
	INT	nStartX	 = 0;
	INT	nStartY	 = 0;
	INT	nWidth	 = nXSize;
	INT	nHeight	 = nYSize;
	INT	nEndX	 = wClipWidth -1;
	INT	nEndY	 = wClipHeight-1;

	// ???? ??? ?????.
	if ( bShadowType == 48 )
	{
		return FALSE;
	}

	if ( m_pddsBackBuffer != NULL )
	{
		rc.left		= 0;
		rc.top		= 0;
		rc.right	= nWidth;
		rc.bottom	= nHeight;

		LONG	lWidth;
		ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
		ddsd.dwSize = sizeof(DDSURFACEDESC2);
		if ( FAILED(m_pddsBackBuffer->Lock(NULL, &ddsd, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL)) )
		{
			return FALSE;
		}

		WORD* pwdDst = (WORD*)ddsd.lpSurface;
		lWidth		 = ddsd.lPitch >> 1;

		INT nWidthStart	 = 0;
		INT nWidthEnd	 = 0;
		INT nCurrWidth   = 0;
		INT nCntCopyWord = 0;
		INT nYCnt		 = 0;
		INT nLastWidth   = 0;
		INT nDstXPos	 = 0;
		INT nDstYPos	 = 0;

		RECT rcScrnShadow;

		if ( bShadowType == 49 )
		{
			SetRect(&rcScrnShadow, nX, nY, nX+nHeight/2+nWidth, nY+nHeight/2);
		}
		else if ( bShadowType == 50 )
		{
			SetRect(&rcScrnShadow, nX, nY, nX+nWidth, nY+nHeight);
		}

		// y?? ????? ?????? ???��?? Count???? ?????.
		for ( nYCnt=rc.top ; nYCnt < rc.bottom ; nYCnt++ )
		{
			// ??????? ????? ???��?.(?????? ????)
			nWidthEnd += pwSrc[nWidthStart];
			nWidthStart++;

			if ( bShadowType == 49 )
			{
				nDstYPos = nYCnt-nYCnt/2+nY;
			}
			else if ( bShadowType == 50 )
			{
				nDstYPos = nYCnt+nY;
			}

			if ( nDstYPos < 0 || nDstYPos >= wClipHeight || (bShadowType==49 && nYCnt%2) )
			{
				// ?????? ???? ???? ?????? ???????? ??????.
				nWidthEnd++;
				nWidthStart	= nWidthEnd;
				nCurrWidth = 0;
				continue;
			}

			// ?????? ?????? ??? ??????.
			for ( INT x = nWidthStart; x < nWidthEnd ; )
			{
				if ( pwSrc[x] == 0xC0 )
				{
					x++;
					nCntCopyWord = pwSrc[x];
					x++;
					nCurrWidth += nCntCopyWord;
				}
				else if ( pwSrc[x] == 0xC1 || pwSrc[x] == 0xC2 || pwSrc[x] == 0xC3 )
				{
					x++;
					nCntCopyWord = pwSrc[x];
					x++;

					nLastWidth = nCurrWidth;
					nCurrWidth += nCntCopyWord;

					if ( bShadowType == 49 )
					{
						nDstXPos = (nLastWidth+nX+(rc.bottom-nYCnt)/2);
					}
					else if ( bShadowType == 50 )
					{
						nDstXPos = (nLastWidth+nX);
					}

					if ( !bBlend ) 
					{
						// ???? ?????.
						if ( nDstXPos < 0 && nDstXPos+nCntCopyWord >= 0 )
						{
							for ( INT nCheck = 0; nCheck < nCntCopyWord; nCheck++ )
							{
								if ( nDstXPos+nCheck >= 0 && nDstXPos+nCheck < wClipWidth )
								{
									pwdDst[(nDstYPos * lWidth) + nDstXPos+nCheck] = *pwShadowClrSrc;
								}
							}
							x += nCntCopyWord;
						}
						// ?????? ?????.
						else if ( nDstXPos < wClipWidth && nDstXPos+nCntCopyWord >= wClipWidth )
						{
							for ( INT nCheck = 0; nCheck < nCntCopyWord; nCheck++ )
							{
								if ( nDstXPos+nCheck >= 0 && nDstXPos+nCheck < wClipWidth )
								{
									pwdDst[(nDstYPos * lWidth) + nDstXPos+nCheck] = *pwShadowClrSrc;

								}
							}
							x += nCntCopyWord;
						}
						// ??????.
						else if ( nDstXPos >= 0 && nDstXPos+nCntCopyWord < wClipWidth )
						{
							memcpy(&pwdDst[(nDstYPos * lWidth) + nDstXPos], pwShadowClrSrc, sizeof(WORD)*nCntCopyWord);
							x += nCntCopyWord;
						}
						else
						{
							x += nCntCopyWord;
						}
					}
					else
					{
						// ???? ?????.
						if ( nDstXPos < 0 && nDstXPos+nCntCopyWord >= 0 )
						{
							if ( (nDstYPos * lWidth) + nDstXPos < 0 )
							{
								return	FALSE;
							}
							putAlpha32( &pwdDst[(nDstYPos * lWidth) + nDstXPos], pwShadowClrSrc, nCntCopyWord, bOpa );

							x += nCntCopyWord;
						}
						// ?????? ?????.
						else if ( nDstXPos < wClipWidth && nDstXPos+nCntCopyWord >= wClipWidth )
						{
							putAlpha32( &pwdDst[(nDstYPos * lWidth) + nDstXPos], pwShadowClrSrc, nDstXPos+nCntCopyWord-wClipWidth, bOpa );

							x += nCntCopyWord;
						}
						// ??????.
						else if ( nDstXPos >= 0 && nDstXPos+nCntCopyWord < wClipWidth )
						{
							putAlpha32( &pwdDst[(nDstYPos * lWidth) + nDstXPos], pwShadowClrSrc, nCntCopyWord, bOpa );

							x += nCntCopyWord;
						}
						else
						{
							x += nCntCopyWord;
						}
					}
				}
				else
				{
					m_pddsBackBuffer->Unlock(NULL);
					return FALSE;
				}
			}
			// ?????? ???? ???? ?????? ???????? ??????.
			nWidthEnd++;

			nWidthStart	= nWidthEnd;
			nCurrWidth = 0;
		}
			
		m_pddsBackBuffer->Unlock(NULL);
		return TRUE;
	}
	return FALSE;
}


VOID CWHDXGraphicWindow::DrawWithShadowABlend(INT nX, INT nY, INT nXSize, INT nYSize, INT nPX, INT nPY, WORD* pwSrc, WORD wClipWidth, WORD wClipHeight,
											  WORD* pwShadowClrSrc, BOOL bBlend, BYTE bOpa)
{
	DDSURFACEDESC2	ddsd;

	// ???(800*600)?????? ??????????? ??? ???? ????? ???(???? ??????????.)
	INT	  nScrnCenterX   = nX - nPX + 24;
	INT	  nScrnCenterY   = nY - nPY + 16;
	INT	  nRealCenterX   = nX - nScrnCenterX;
	INT	  nRealCenterY   = nY - nScrnCenterY;
	INT	  nShadowCenterX = nX;
	INT	  nShadowCenterY = nY-nRealCenterY/2;

	RECT  rcSrc		  = {0, 0, nXSize, nYSize};
	WORD wSahdowColor = pwShadowClrSrc[0];

	if ( m_pddsBackBuffer != NULL )
	{
		LONG	lWidth;
		ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
		ddsd.dwSize = sizeof(DDSURFACEDESC2);
		if ( FAILED(m_pddsBackBuffer->Lock(NULL, &ddsd, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL)) )
		{
			return;
		}

		WORD* pwdDst = (WORD*)ddsd.lpSurface;
		lWidth		 = ddsd.lPitch >> 1;

		if ( pwdDst )
		{
			INT nWidthStart	 = 0;
			INT nWidthEnd	 = 0;
			INT nCurrWidth   = 0;
			INT nLastWidth	 = 0;
			INT nCntCopyWord = 0;
			INT nYCnt		 = 0;

			INT nDstYPos = nShadowCenterY;

			for ( nYCnt = rcSrc.top ; nYCnt < rcSrc.bottom ; nYCnt++ )
			{
				if ( nDstYPos >= wClipHeight )
				{
					m_pddsBackBuffer->Unlock(NULL);
					return;
				}
				// y?? ????��? ????? ?????.
				if ( nYCnt%2 == 0 ||  nDstYPos < 0 )
				{
					nWidthEnd += pwSrc[nWidthStart];
					nWidthStart++;
					nWidthEnd++;
					nWidthStart	= nWidthEnd;

					nDstYPos++;
				}
				else
				{
					// ??????? ????? ???��?.(?????? ????)
					nWidthEnd += pwSrc[nWidthStart];
					nWidthStart++;

					// ?????? ?????? ??? ??????.
					for ( INT x = nWidthStart; x < nWidthEnd ; )
					{
						if ( pwSrc[x] == 0xC0 )
						{		
							x++;
							nCntCopyWord = pwSrc[x];
							x++;
							nCurrWidth += nCntCopyWord;
						}
						else
						{
							x++;
							nCntCopyWord = pwSrc[x];
							x++;
							nLastWidth = nCurrWidth;
							nCurrWidth += nCntCopyWord;

							if ( bBlend )
							{
								if ((nShadowCenterX+nLastWidth+((-nRealCenterY/2)-nYCnt/2)) < 0 && (nShadowCenterX+nCurrWidth+((-nRealCenterY/2)-nYCnt/2)) >= 0 )
								{
								}
								else if ((nShadowCenterX+nLastWidth+((-nRealCenterY/2)-nYCnt/2)) < wClipWidth && (nShadowCenterX+nCurrWidth+((-nRealCenterY/2)-nYCnt/2)) >= wClipWidth )
								{
								}
								else if ( (nShadowCenterX+nCurrWidth+((-nRealCenterY/2)-nYCnt/2)) < 0 )
								{
								}
								else if ( (nShadowCenterX+nCurrWidth+((-nRealCenterY/2)-nYCnt/2)) >= wClipWidth )
								{
								}
								else
								{
									putAlpha32( &pwdDst[nDstYPos*lWidth + (nShadowCenterX+nLastWidth+((-nRealCenterY/2)-nYCnt/2))], pwShadowClrSrc, nCntCopyWord, bOpa );
								}
							}
							else
							{
								if ((nShadowCenterX+nLastWidth+((-nRealCenterY/2)-nYCnt/2)) < 0 && (nShadowCenterX+nCurrWidth+((-nRealCenterY/2)-nYCnt/2)) >= 0 )
								{
								}
								else if ((nShadowCenterX+nLastWidth+((-nRealCenterY/2)-nYCnt/2)) < wClipWidth && (nShadowCenterX+nCurrWidth+((-nRealCenterY/2)-nYCnt/2)) >= wClipWidth )
								{
								}
								else if ( (nShadowCenterX+nCurrWidth+((-nRealCenterY/2)-nYCnt/2)) < 0 )
								{
								}
								else if ( (nShadowCenterX+nCurrWidth+((-nRealCenterY/2)-nYCnt/2)) >= wClipWidth )
								{
								}
								else
								{
									memcpy(&pwdDst[nDstYPos*lWidth + (nShadowCenterX+nLastWidth+((-nRealCenterY/2)-nYCnt/2))], pwShadowClrSrc, sizeof(WORD)*nCntCopyWord);
								}
							}
							x += nCntCopyWord;
						}
					}
					// ?????? ???? ???? ?????? ???????? ??????.
					nWidthEnd++;

					nWidthStart	= nWidthEnd;
					nCurrWidth = 0;
				}
			}
			m_pddsBackBuffer->Unlock(NULL);
		}
	}
}


BOOL CWHDXGraphicWindow::DrawWithABlendCompDataWithLightedColor(INT nX, INT nY, 
															    INT nXSize, INT nYSize, WORD* pwSrc,
															    WORD wClipWidth, WORD wClipHeight,
															    WORD wChooseColor1, WORD wChooseColor2)
{
	RECT			rc;
	DDSURFACEDESC2	ddsd;

	INT	nWidth		= nXSize;
	INT	nHeight		= nYSize;
	INT	nXOffset	= 0;
	INT	nYOffset	= 0;
	INT	nStartX		= 0;
	INT	nStartY		= 0;
	INT	nEndX		= wClipWidth -1;
	INT	nEndY		= wClipHeight-1;

	FLOAT fBright = 0.5f;

	if ( m_pddsBackBuffer != NULL )
	{
		if (nX < nStartX )
		{ 
			nXOffset	= nStartX - nX;
			nWidth		= nXSize - nXOffset;
		}
		if ( (nX+nXSize-1) > nEndX )
			nWidth		= nEndX - nX - nXOffset + 1;		
		if ( nY < nStartY )	
		{ 
			nYOffset	= nStartY - nY;
			nHeight		= nYSize - nYOffset;
		}		
		if ( (nY+nYSize-1) > nEndY )
			nHeight	= nEndY - nY - nYOffset + 1;

		if ( (nWidth > 0) && (nHeight > 0) )
		{
			rc.left		= nXOffset;
			rc.right	= nXOffset+nWidth;
			rc.top		= nYOffset;
			rc.bottom	= nYOffset+nHeight;

			LONG	lWidth;
			ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
			ddsd.dwSize = sizeof(DDSURFACEDESC2);
			if ( FAILED(m_pddsBackBuffer->Lock(NULL, &ddsd, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL)) )
			{
				return FALSE;
			}

			WORD* pwdDst = (WORD*)ddsd.lpSurface;
			lWidth		 = ddsd.lPitch >> 1;

			INT nWidthStart	= 0;
			INT nWidthEnd	= 0;
			INT nCurrWidth  = 0;
			INT nCntCopyWord = 0;
			INT nYCnt =0;
			INT nLastWidth = 0;

			BYTE  bRedWantedColor, bGreenWantedColor, bBlueWantedColor;
			BYTE bRedSrc, bGreenSrc, bBlueSrc;
			WORD wPixel;
			WORD wDyingKind, wChooseColor;
			FLOAT rBlueRate, rGreenRate, bRedRate;

			// y?? ?????.
  			for ( nYCnt=0 ; nYCnt < rc.top ; nYCnt++ )
			{
				nWidthEnd += pwSrc[nWidthStart];
				nWidthStart++;
				nWidthEnd++;
				nWidthStart	= nWidthEnd;
			}

			// y?? ????? ?????? ???��?? Count???? ?????.
			for ( nYCnt=rc.top ; nYCnt < rc.bottom ; nYCnt++ )
			{
				// ??????? ????? ???��?.(?????? ????)
				nWidthEnd += pwSrc[nWidthStart];
				nWidthStart++;

				// ?????? ?????? ??? ??????.
				for ( INT x = nWidthStart; x < nWidthEnd ; )
				{
					if ( pwSrc[x] == 0xC0 )
					{
						x++;
						nCntCopyWord = pwSrc[x];
						x++;
						nCurrWidth += nCntCopyWord;
					}
					else if ( pwSrc[x] == 0xC1 )
					{
						x++;
						nCntCopyWord = pwSrc[x];
						x++;

						nLastWidth = nCurrWidth;
						nCurrWidth += nCntCopyWord;

						if ( rc.left > nCurrWidth || rc.right < nLastWidth )
						{
							x += nCntCopyWord;
						}
						else
						{
							// rc.left???? ???????? ��???? ?????? ??? ???1?71?1?771?1?71?1?777.
							if ( nLastWidth < rc.left && rc.left <= nCurrWidth )
							{
								x += (rc.left-nLastWidth);
								for ( INT nCheck = 0; nCheck < nCurrWidth-rc.left; nCheck++ )
								{
									wPixel = pwSrc[x+nCheck];
									bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
									rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
									bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

									bBlueSrc = (BYTE)((FLOAT)bBlueSrc + ((FLOAT)bBlueSrc*rBlueRate*fBright));
									bGreenSrc = (BYTE)((FLOAT)bGreenSrc + ((FLOAT)bGreenSrc*rGreenRate*fBright));
									bRedSrc = (BYTE)((FLOAT)bRedSrc + ((FLOAT)bRedSrc*bRedRate*fBright));

									if ( bBlueSrc > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
										bBlueSrc = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
									if ( bGreenSrc > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
										bGreenSrc = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
									if ( bRedSrc > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
										bRedSrc = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);

									if ( ((nYCnt+nY) * lWidth) + (nLastWidth+nX+nCheck) < 0 )
										return FALSE;

									pwdDst[((nYCnt+nY) * lWidth) + (rc.left+nX+nCheck)] = ((bBlueSrc <<m_stBitsMaskInfo.bBShift) |
																						   (bGreenSrc<<m_stBitsMaskInfo.bGShift) |
																						   (bRedSrc  <<m_stBitsMaskInfo.bRShift));			
								}
								x += (nCurrWidth-rc.left);
							}
							// rc.right???? ???????? ��???? ?????? ??? ???1?71?1?771?1?71?1?777.
							else if ( nLastWidth <= rc.right && rc.right < nCurrWidth )
							{
								for ( INT nCheck = 0; nCheck < rc.right-nLastWidth; nCheck++ )
								{
									wPixel = pwSrc[x+nCheck];
									bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
									rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
									bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

									bBlueSrc = (BYTE)((FLOAT)bBlueSrc + ((FLOAT)bBlueSrc*rBlueRate*fBright));
									bGreenSrc = (BYTE)((FLOAT)bGreenSrc + ((FLOAT)bGreenSrc*rGreenRate*fBright));
									bRedSrc = (BYTE)((FLOAT)bRedSrc + ((FLOAT)bRedSrc*bRedRate*fBright));

									if ( bBlueSrc > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
										bBlueSrc = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
									if ( bGreenSrc > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
										bGreenSrc = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
									if ( bRedSrc > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
										bRedSrc = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);

									pwdDst[((nYCnt+nY) * lWidth) + (nLastWidth+nX+nCheck)] = ((bBlueSrc <<m_stBitsMaskInfo.bBShift) |
																						      (bGreenSrc<<m_stBitsMaskInfo.bGShift) |
																							  (bRedSrc  <<m_stBitsMaskInfo.bRShift));			
								}
								x += nCntCopyWord;
							}
							else
							{
								for ( INT nCheck = 0; nCheck < nCntCopyWord; nCheck++ )
								{
									wPixel = pwSrc[x+nCheck];
									bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
									rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
									bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

									bBlueSrc = (BYTE)((FLOAT)bBlueSrc + ((FLOAT)bBlueSrc*rBlueRate*fBright));
									bGreenSrc = (BYTE)((FLOAT)bGreenSrc + ((FLOAT)bGreenSrc*rGreenRate*fBright));
									bRedSrc = (BYTE)((FLOAT)bRedSrc + ((FLOAT)bRedSrc*bRedRate*fBright));

									if ( bBlueSrc > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
										bBlueSrc = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
									if ( bGreenSrc > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
										bGreenSrc = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
									if ( bRedSrc > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
										bRedSrc = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);

									pwdDst[((nYCnt+nY) * lWidth) + (nLastWidth+nX+nCheck)] = ((bBlueSrc <<m_stBitsMaskInfo.bBShift) |
																						      (bGreenSrc<<m_stBitsMaskInfo.bGShift) |
																							  (bRedSrc  <<m_stBitsMaskInfo.bRShift));			
								}
								x += nCntCopyWord;
							}
						}
					}
					else if ( pwSrc[x] == 0xC2 || pwSrc[x] == 0xC3 )
					{
						wDyingKind = pwSrc[x];
						switch ( wDyingKind )
						{
						case 0xC2: 
							wChooseColor = wChooseColor1;
							break;
						case 0xC3: 
							wChooseColor = wChooseColor2;
							break;
						}
						x++;
						nCntCopyWord = pwSrc[x];
						x++;

						nLastWidth = nCurrWidth;
						nCurrWidth += nCntCopyWord;

						if ( rc.left > nCurrWidth || rc.right < nLastWidth )
						{
							x += nCntCopyWord;
						}
						else
						{
							// rc.left???? ???????? ��???? ?????? ??? ???1?71?1?771?1?71?1?777.
							if ( nLastWidth < rc.left && rc.left <= nCurrWidth )
							{
								x += (rc.left-nLastWidth);
								for ( INT nCheck = 0; nCheck < nCurrWidth-rc.left; nCheck++ )
								{
									wPixel	  = wChooseColor;
									bBlueWantedColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenWantedColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedWantedColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									wPixel	  = pwSrc[x+nCheck];
									bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
									rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
									bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

									bBlueWantedColor = (BYTE)(((FLOAT)bBlueWantedColor*rBlueRate));
									bGreenWantedColor = (BYTE)(((FLOAT)bGreenWantedColor*rGreenRate));
									bRedWantedColor = (BYTE)(((FLOAT)bRedWantedColor*bRedRate));

									if ( bBlueWantedColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
										bBlueWantedColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
									if ( bGreenWantedColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
										bGreenWantedColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
									if ( bRedWantedColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
										bRedWantedColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);

									bBlueSrc = bBlueWantedColor;
									bGreenSrc = bGreenWantedColor;
									bRedSrc = bRedWantedColor;

									rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
									rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
									bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

									bBlueSrc = (BYTE)((FLOAT)bBlueSrc + ((FLOAT)bBlueSrc*rBlueRate*fBright));
									bGreenSrc = (BYTE)((FLOAT)bGreenSrc + ((FLOAT)bGreenSrc*rGreenRate*fBright));
									bRedSrc = (BYTE)((FLOAT)bRedSrc + ((FLOAT)bRedSrc*bRedRate*fBright));

									if ( bBlueSrc > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
										bBlueSrc = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
									if ( bGreenSrc > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
										bGreenSrc = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
									if ( bRedSrc > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
										bRedSrc = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);

									if ( ((nYCnt+nY) * lWidth) + (rc.left+nX+nCheck) < 0 )
										return FALSE;

									pwdDst[((nYCnt+nY) * lWidth) + (rc.left+nX+nCheck)] = ((bBlueSrc <<m_stBitsMaskInfo.bBShift) |
																						   (bGreenSrc<<m_stBitsMaskInfo.bGShift) |
																						   (bRedSrc  <<m_stBitsMaskInfo.bRShift));			
								}
								x += (nCurrWidth-rc.left);
							}
							// rc.right???? ???????? ��???? ?????? ??? ???1?71?1?771?1?71?1?777.
							else if ( nLastWidth <= rc.right && rc.right < nCurrWidth )
							{
								for ( INT nCheck = 0; nCheck < rc.right-nLastWidth; nCheck++ )
								{
									wPixel	  = wChooseColor;
									bBlueWantedColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenWantedColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedWantedColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									wPixel	  = pwSrc[x+nCheck];
									bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
									rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
									bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

									bBlueWantedColor = (BYTE)(((FLOAT)bBlueWantedColor*rBlueRate));
									bGreenWantedColor = (BYTE)(((FLOAT)bGreenWantedColor*rGreenRate));
									bRedWantedColor = (BYTE)(((FLOAT)bRedWantedColor*bRedRate));

									if ( bBlueWantedColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
										bBlueWantedColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
									if ( bGreenWantedColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
										bGreenWantedColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
									if ( bRedWantedColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
										bRedWantedColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);

									bBlueSrc = bBlueWantedColor;
									bGreenSrc = bGreenWantedColor;
									bRedSrc = bRedWantedColor;

									rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
									rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
									bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

									bBlueSrc = (BYTE)((FLOAT)bBlueSrc + ((FLOAT)bBlueSrc*rBlueRate*fBright));
									bGreenSrc = (BYTE)((FLOAT)bGreenSrc + ((FLOAT)bGreenSrc*rGreenRate*fBright ));
									bRedSrc = (BYTE)((FLOAT)bRedSrc + ((FLOAT)bRedSrc*bRedRate*fBright));

									if ( bBlueSrc > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
										bBlueSrc = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
									if ( bGreenSrc > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
										bGreenSrc = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
									if ( bRedSrc > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
										bRedSrc = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);

									pwdDst[((nYCnt+nY) * lWidth) + (nLastWidth+nX+nCheck)] = ((bBlueSrc <<m_stBitsMaskInfo.bBShift) |
																						      (bGreenSrc<<m_stBitsMaskInfo.bGShift) |
																							  (bRedSrc  <<m_stBitsMaskInfo.bRShift));			
								}
								x += nCntCopyWord;
							}
							else
							{
								for ( INT nCheck = 0; nCheck < nCntCopyWord; nCheck++ )
								{
									wPixel	  = wChooseColor;
									bBlueWantedColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenWantedColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedWantedColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									wPixel	  = pwSrc[x+nCheck];
									bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
									rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
									bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

									bBlueWantedColor = (BYTE)(((FLOAT)bBlueWantedColor*rBlueRate));
									bGreenWantedColor = (BYTE)(((FLOAT)bGreenWantedColor*rGreenRate));
									bRedWantedColor = (BYTE)(((FLOAT)bRedWantedColor*bRedRate));

									if ( bBlueWantedColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
										bBlueWantedColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
									if ( bGreenWantedColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
										bGreenWantedColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
									if ( bRedWantedColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
										bRedWantedColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);

									bBlueSrc = bBlueWantedColor;
									bGreenSrc = bGreenWantedColor;
									bRedSrc = bRedWantedColor;

									rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
									rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
									bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

									bBlueSrc = (BYTE)((FLOAT)bBlueSrc + ((FLOAT)bBlueSrc*rBlueRate*fBright));
									bGreenSrc = (BYTE)((FLOAT)bGreenSrc + ((FLOAT)bGreenSrc*rGreenRate*fBright));
									bRedSrc = (BYTE)((FLOAT)bRedSrc + ((FLOAT)bRedSrc*bRedRate*fBright));

									if ( bBlueSrc > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
										bBlueSrc = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
									if ( bGreenSrc > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
										bGreenSrc = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
									if ( bRedSrc > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
										bRedSrc = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);

									pwdDst[((nYCnt+nY) * lWidth) + (nLastWidth+nX+nCheck)] = ((bBlueSrc <<m_stBitsMaskInfo.bBShift) |
																						      (bGreenSrc<<m_stBitsMaskInfo.bGShift) |
																							  (bRedSrc  <<m_stBitsMaskInfo.bRShift));			
								}
								x += nCntCopyWord;
							}
						}
					}	
					else
					{
						m_pddsBackBuffer->Unlock(NULL);
						return FALSE;
					}
				}
				// ?????? ???? ???? ?????? ???????? ??????.
				nWidthEnd++;

				nWidthStart	= nWidthEnd;
				nCurrWidth = 0;
			}
		}

		m_pddsBackBuffer->Unlock(NULL);
		return TRUE;
	}
	return FALSE;
}



BOOL CWHDXGraphicWindow::DrawWithImageForCompClipRgnColor(INT nX, INT nY, INT nXSize, INT nYSize, WORD* pwSrc, WORD wClipWidth, WORD wClipHeight, WORD wColor, BOOL bFocused, BOOL bBlend)
{
	RECT			rc;
	DDSURFACEDESC2	ddsd;

	INT	nWidth		= nXSize;
	INT	nHeight		= nYSize;
	INT	nXOffset	= 0;
	INT	nYOffset	= 0;
	INT	nStartX		= 0;
	INT	nStartY		= 0;
	INT	nEndX		= wClipWidth -1;
	INT	nEndY		= wClipHeight-1;

	if ( m_pddsBackBuffer != NULL )
	{
		if (nX < nStartX )
		{ 
			nXOffset	= nStartX - nX;
			nWidth		= nXSize - nXOffset;
		}
		if ( (nX+nXSize-1) > nEndX )
			nWidth		= nEndX - nX - nXOffset + 1;		
		if ( nY < nStartY )	
		{ 
			nYOffset	= nStartY - nY;
			nHeight		= nYSize - nYOffset;
		}		
		if ( (nY+nYSize-1) > nEndY )
			nHeight	= nEndY - nY - nYOffset + 1;

		if ( (nWidth > 0) && (nHeight > 0) )
		{
			rc.left		= nXOffset;
			rc.right	= nXOffset+nWidth;
			rc.top		= nYOffset;
			rc.bottom	= nYOffset+nHeight;

			LONG	lWidth;
			ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
			ddsd.dwSize = sizeof(DDSURFACEDESC2);
			if ( FAILED(m_pddsBackBuffer->Lock(NULL, &ddsd, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL)) )
			{
				return FALSE;
			}

			WORD* pwdDst = (WORD*)ddsd.lpSurface;
			lWidth		 = ddsd.lPitch >> 1;

			INT nWidthStart	= 0;
			INT nWidthEnd	= 0;
			INT nCurrWidth  = 0;
			INT nCntCopyWord = 0;
			INT nYCnt =0;
			INT nLastWidth = 0;

			BYTE  bRedStateColor,  bGreenStateColor,  bBlueStateColor;
			BYTE  bRedSrc, bGreenSrc, bBlueSrc;
			BYTE  bBlueDst, bGreenDst, bRedDst;
			WORD  wPixel;
			FLOAT rBlueRate, rGreenRate, rRedRate;
			FLOAT rbLightRate = 0.0f;

			// y?? ?????.
  			for ( nYCnt=0 ; nYCnt < rc.top ; nYCnt++ )
			{
				nWidthEnd += pwSrc[nWidthStart];
				nWidthStart++;
				nWidthEnd++;
				nWidthStart	= nWidthEnd;
			}

			// y?? ????? ?????? ???��?? Count???? ?????.
			for ( nYCnt=rc.top ; nYCnt < rc.bottom ; nYCnt++ )
			{
				// ??????? ????? ???��?.(?????? ????)
				nWidthEnd += pwSrc[nWidthStart];
				nWidthStart++;

				// ?????? ?????? ??? ??????.
				for ( INT x = nWidthStart; x < nWidthEnd ; )
				{
					if ( pwSrc[x] == 0xC0 )
					{
						x++;
						nCntCopyWord = pwSrc[x];
						x++;
						nCurrWidth += nCntCopyWord;
					}
					else if ( pwSrc[x] == 0xC1 || pwSrc[x] == 0xC2 || pwSrc[x] == 0xC3 )
					{
						x++;
						nCntCopyWord = pwSrc[x];
						x++;

						nLastWidth = nCurrWidth;
						nCurrWidth += nCntCopyWord;

						if ( rc.left > nCurrWidth || rc.right < nLastWidth )
						{
							x += nCntCopyWord;
						}
						else
						{
							// rc.left???? ???????? ��???? ?????? ??? ???1?71?1?771?1?71?1?777.
							if ( nLastWidth < rc.left && rc.left <= nCurrWidth )
							{
								x += (rc.left-nLastWidth);
								for ( INT nCheck = 0; nCheck < nCurrWidth-rc.left; nCheck++ )
								{
									wPixel = wColor;
									bBlueStateColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenStateColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedStateColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									wPixel = pwSrc[x+nCheck];
									bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									// Focus????.
									if ( bFocused )			rbLightRate = 0.6f;
									else					rbLightRate = 0.0f;

									if ( !wColor )	
									{
										BYTE bTemp = (BYTE)(bBlueSrc+(bGreenSrc>>1)+bRedSrc)/3;
										bTemp = bTemp + (BYTE)(bTemp*rbLightRate);
										if ( bTemp > 31 )	bTemp = 31;
										bBlueStateColor = bRedStateColor = bTemp;
										bGreenStateColor = bTemp<<1;
									}
									else
									{
										rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
										rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
										rRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

										bBlueStateColor  = ( (BYTE)((FLOAT)bBlueStateColor *rBlueRate ) + (BYTE)((FLOAT)bBlueStateColor *rBlueRate *rbLightRate) );
										bGreenStateColor = ( (BYTE)((FLOAT)bGreenStateColor*rGreenRate) + (BYTE)((FLOAT)bGreenStateColor*rGreenRate*rbLightRate) );
										bRedStateColor   = ( (BYTE)((FLOAT)bRedStateColor  *rRedRate  ) + (BYTE)((FLOAT)bRedStateColor  *rRedRate  *rbLightRate) );					

										if ( bBlueStateColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
											bBlueStateColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
										if ( bGreenStateColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
											bGreenStateColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
										if ( bRedStateColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
											bRedStateColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);
									}

									if ( bBlend )
									{
										wPixel	   = pwdDst[((nYCnt+nY) * lWidth) + (nLastWidth+nX+nCheck)];
										bBlueDst   = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
										bGreenDst  = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
										bRedDst    = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

										bBlueDst   = (BYTE)((50*(bBlueDst -bBlueStateColor )+100*bBlueStateColor  ) / 100);
										bGreenDst  = (BYTE)((50*(bGreenDst-bGreenStateColor)+100*bGreenStateColor ) / 100);
										bRedDst    = (BYTE)((50*(bRedDst  -bRedStateColor  )+100*bRedStateColor   ) / 100);
									}
									else
									{
										bBlueDst   = bBlueStateColor;
										bGreenDst  = bGreenStateColor;
										bRedDst    = bRedStateColor;
									}

									if ( ((nYCnt+nY) * lWidth) + (rc.left+nX+nCheck) < 0 )
										return FALSE;

									pwdDst[((nYCnt+nY) * lWidth) + (rc.left+nX+nCheck)] = ((bBlueDst <<m_stBitsMaskInfo.bBShift) |
																									   (bGreenDst<<m_stBitsMaskInfo.bGShift) |
																									   (bRedDst  <<m_stBitsMaskInfo.bRShift));			
								}
								x += (nCurrWidth-rc.left);
							}
							// rc.right???? ???????? ��???? ?????? ??? ???1?71?1?771?1?71?1?777.
							else if ( nLastWidth <= rc.right && rc.right < nCurrWidth )
							{
								for ( INT nCheck = 0; nCheck < rc.right-nLastWidth; nCheck++ )
								{
									wPixel = wColor;
									bBlueStateColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenStateColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedStateColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									wPixel = pwSrc[x+nCheck];
									bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									// Focus????.
									if ( bFocused )			rbLightRate = 0.6f;
									else					rbLightRate = 0.0f;

									if ( !wColor )	
									{
										BYTE bTemp = (BYTE)(bBlueSrc+(bGreenSrc>>1)+bRedSrc)/3;
										bTemp = bTemp + (BYTE)(bTemp*rbLightRate);
										if ( bTemp > 31 )	bTemp = 31;
										bBlueStateColor = bRedStateColor = bTemp;
										bGreenStateColor = bTemp<<1;
									}
									else
									{
										rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
										rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
										rRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

										bBlueStateColor  = ( (BYTE)((FLOAT)bBlueStateColor *rBlueRate ) + (BYTE)((FLOAT)bBlueStateColor *rBlueRate *rbLightRate) );
										bGreenStateColor = ( (BYTE)((FLOAT)bGreenStateColor*rGreenRate) + (BYTE)((FLOAT)bGreenStateColor*rGreenRate*rbLightRate) );
										bRedStateColor   = ( (BYTE)((FLOAT)bRedStateColor  *rRedRate  ) + (BYTE)((FLOAT)bRedStateColor  *rRedRate  *rbLightRate) );					

										if ( bBlueStateColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
											bBlueStateColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
										if ( bGreenStateColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
											bGreenStateColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
										if ( bRedStateColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
											bRedStateColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);
									}

									if ( bBlend )
									{
										wPixel	   = pwdDst[((nYCnt+nY) * lWidth) + (nLastWidth+nX+nCheck)];
										bBlueDst   = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
										bGreenDst  = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
										bRedDst    = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

										bBlueDst   = (BYTE)((50*(bBlueDst -bBlueStateColor )+100*bBlueStateColor  ) / 100);
										bGreenDst  = (BYTE)((50*(bGreenDst-bGreenStateColor)+100*bGreenStateColor ) / 100);
										bRedDst    = (BYTE)((50*(bRedDst  -bRedStateColor  )+100*bRedStateColor   ) / 100);
									}
									else
									{
										bBlueDst   = bBlueStateColor;
										bGreenDst  = bGreenStateColor;
										bRedDst    = bRedStateColor;
									}

									pwdDst[((nYCnt+nY) * lWidth) + (nLastWidth+nX+nCheck)] = ((bBlueDst <<m_stBitsMaskInfo.bRShift) |
																									      (bGreenDst<<m_stBitsMaskInfo.bGShift) |
																										  (bRedDst  <<m_stBitsMaskInfo.bBShift));			
								}
								x += nCntCopyWord;
							}
							else
							{
								for ( INT nCheck = 0; nCheck < nCntCopyWord; nCheck++ )
								{
									wPixel = wColor;
									bBlueStateColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenStateColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedStateColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									wPixel = pwSrc[x+nCheck];
									bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									// Focus????.
									if ( bFocused )			rbLightRate = 0.6f;
									else					rbLightRate = 0.0f;

									if ( !wColor )	
									{
										BYTE bTemp = (BYTE)(bBlueSrc+(bGreenSrc>>1)+bRedSrc)/3;
										bTemp = bTemp + (BYTE)(bTemp*rbLightRate);
										if ( bTemp > 31 )	bTemp = 31;
										bBlueStateColor = bRedStateColor = bTemp;
										bGreenStateColor = bTemp<<1;
									}
									else
									{
										rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
										rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
										rRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

										bBlueStateColor  = ( (BYTE)((FLOAT)bBlueStateColor *rBlueRate ) + (BYTE)((FLOAT)bBlueStateColor *rBlueRate *rbLightRate) );
										bGreenStateColor = ( (BYTE)((FLOAT)bGreenStateColor*rGreenRate) + (BYTE)((FLOAT)bGreenStateColor*rGreenRate*rbLightRate) );
										bRedStateColor   = ( (BYTE)((FLOAT)bRedStateColor  *rRedRate  ) + (BYTE)((FLOAT)bRedStateColor  *rRedRate  *rbLightRate) );					

										if ( bBlueStateColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
											bBlueStateColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
										if ( bGreenStateColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
											bGreenStateColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
										if ( bRedStateColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
											bRedStateColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);
									}

									if ( bBlend )
									{
										wPixel	   = pwdDst[((nYCnt+nY) * lWidth) + (nLastWidth+nX+nCheck)];
										bBlueDst   = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
										bGreenDst  = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
										bRedDst    = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

										bBlueDst   = (BYTE)((50*(bBlueDst -bBlueStateColor )+100*bBlueStateColor  ) / 100);
										bGreenDst  = (BYTE)((50*(bGreenDst-bGreenStateColor)+100*bGreenStateColor ) / 100);
										bRedDst    = (BYTE)((50*(bRedDst  -bRedStateColor  )+100*bRedStateColor   ) / 100);
									}
									else
									{
										bBlueDst   = bBlueStateColor;
										bGreenDst  = bGreenStateColor;
										bRedDst    = bRedStateColor;
									}

									pwdDst[((nYCnt+nY) * lWidth) + (nLastWidth+nX+nCheck)] = ((bBlueDst <<m_stBitsMaskInfo.bBShift) |
																									      (bGreenDst<<m_stBitsMaskInfo.bGShift) |
																										  (bRedDst  <<m_stBitsMaskInfo.bRShift));			
								}
								x += nCntCopyWord;
							}
						}
					}
					else
					{
						m_pddsBackBuffer->Unlock(NULL);
						return FALSE;
					}
				}
				// ?????? ???? ???? ?????? ???????? ??????.
				nWidthEnd++;

				nWidthStart	= nWidthEnd;
				nCurrWidth = 0;
			}
		}

		m_pddsBackBuffer->Unlock(NULL);
		return TRUE;
	}
	return FALSE;
}



BOOL CWHDXGraphicWindow::DrawWithImageForCompClipRgnColor(INT nX, INT nY, INT nXSize, INT nYSize, WORD* pwSrc, WORD wClipWidth, WORD wClipHeight,WORD wColor , WORD wChooseColor1, WORD wChooseColor2)
{
	RECT			rc;
	DDSURFACEDESC2	ddsd;

	INT	nWidth		= nXSize;
	INT	nHeight		= nYSize;
	INT	nXOffset	= 0;
	INT	nYOffset	= 0;
	INT	nStartX		= 0;
	INT	nStartY		= 0;
	INT	nEndX		= wClipWidth -1;
	INT	nEndY		= wClipHeight-1;

	INT FiveMiddle	= 0xf;
	INT SixMiddle	= 0x1f;

	BYTE bsBlueColor,bsGreenColor,bsRedColor;
	BYTE bdBlueColor,bdGreenColor,bdRedColor;

	bdBlueColor  = (BYTE)((wColor & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
	bdGreenColor = (BYTE)((wColor & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
	bdRedColor   = (BYTE)((wColor & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

	if ( m_pddsBackBuffer != NULL )
	{
		if (nX < nStartX )
		{ 
			nXOffset	= nStartX - nX;
			nWidth		= nXSize - nXOffset;
		}
		if ( (nX+nXSize-1) > nEndX )
			nWidth		= nEndX - nX - nXOffset + 1;		
		if ( nY < nStartY )	
		{ 
			nYOffset	= nStartY - nY;
			nHeight		= nYSize - nYOffset;
		}		
		if ( (nY+nYSize-1) > nEndY )
			nHeight	= nEndY - nY - nYOffset + 1;

		if ( (nWidth > 0) && (nHeight > 0) )
		{
			rc.left		= nXOffset;
			rc.right	= nXOffset+nWidth;
			rc.top		= nYOffset;
			rc.bottom	= nYOffset+nHeight;

			LONG	lWidth;
			ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
			ddsd.dwSize = sizeof(DDSURFACEDESC2);
			if ( FAILED(m_pddsBackBuffer->Lock(NULL, &ddsd, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL)) )
			{
				return FALSE;
			}

			WORD* pwdDst = (WORD*)ddsd.lpSurface;
			lWidth		 = ddsd.lPitch >> 1;

			INT nWidthStart	= 0;
			INT nWidthEnd	= 0;
			INT nCurrWidth  = 0;
			INT nCntCopyWord = 0;
			INT nYCnt =0;
			INT nLastWidth = 0;

			FLOAT rBlueRate, rGreenRate, bRedRate;
			BYTE  bRedSrc, bGreenSrc, bBlueSrc;
			BYTE  bRedWantedColor, bGreenWantedColor, bBlueWantedColor;
			WORD  wPixel;

			// y?? ?????.
			for ( nYCnt=0 ; nYCnt < rc.top ; nYCnt++ )
			{
				nWidthEnd += pwSrc[nWidthStart];
				nWidthStart++;
				nWidthEnd++;
				nWidthStart	= nWidthEnd;
			}

			// y?? ????? ?????? ???��?? Count???? ?????.
			for ( nYCnt=rc.top ; nYCnt < rc.bottom ; nYCnt++ )
			{
				// ??????? ????? ???��?.(?????? ????)
				nWidthEnd += pwSrc[nWidthStart];
				nWidthStart++;

				// ?????? ?????? ??? ??????.
				for ( INT x = nWidthStart; x < nWidthEnd ; )
				{
					if ( pwSrc[x] == 0xC0 )
					{
						x++;
						nCntCopyWord = pwSrc[x];
						x++;
						nCurrWidth += nCntCopyWord;
					}
					else if ( pwSrc[x] == 0xC1 )
					{
						x++;
						nCntCopyWord = pwSrc[x];	// ????
						x++;

						nLastWidth = nCurrWidth;
						nCurrWidth += nCntCopyWord;

						if ( rc.left > nCurrWidth || rc.right < nLastWidth )
						{
							x += nCntCopyWord;		// ???? ???? ???? ???1?71?1?771?1?71?1?777
						}
						else
						{
							// rc.left???? ???????? ��???? ?????? ??? ???1?71?1?771?1?71?1?777.
							if ( nLastWidth < rc.left && rc.left <= nCurrWidth )
							{
								x += (rc.left-nLastWidth);

								for(WORD i = 0 ; i < (sizeof(WORD)*(nCurrWidth-rc.left)/2);i++)
								{
									bsBlueColor  = (BYTE)((pwSrc[x+i] & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bsGreenColor = (BYTE)((pwSrc[x+i] & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bsRedColor   = (BYTE)((pwSrc[x+i] & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									BYTE	temp;

									temp = (bsRedColor+(bsGreenColor>>1)+bsBlueColor)/3;

									bsBlueColor = temp>0 ? (temp&bdBlueColor) : 0;
									bsGreenColor = temp>0 ? (temp&(bdGreenColor<<1)) : 0;
									bsRedColor = temp>0 ? (temp&bdRedColor) :  0;

									if ( bsBlueColor >= 32 )  bsBlueColor = 31;
									if ( bsGreenColor >= 64 )  bsGreenColor = 63;
									if ( bsRedColor >= 32 )  bsRedColor = 31;

									if ( ((nYCnt+nY) * lWidth) + (rc.left+nX) + i < 0 )
										return FALSE;

									pwdDst[((nYCnt+nY) * lWidth) + (rc.left+nX) + i] = (bsBlueColor<<m_stBitsMaskInfo.bBShift) |  (bsGreenColor<<m_stBitsMaskInfo.bGShift) | (bsRedColor<<m_stBitsMaskInfo.bRShift);
								}

								x += (nCurrWidth-rc.left);
							}
							// rc.right???? ???????? ��???? ?????? ??? ???1?71?1?771?1?71?1?777.
							else if ( nLastWidth <= rc.right && rc.right < nCurrWidth )
							{
								for(WORD i = 0 ; i < (sizeof(WORD)*(rc.right-nLastWidth));i++)
								{
									bsBlueColor  = (BYTE)((pwSrc[x+i] & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bsGreenColor = (BYTE)((pwSrc[x+i] & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bsRedColor   = (BYTE)((pwSrc[x+i] & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);
									
									BYTE	temp;

									temp = (bsRedColor+(bsGreenColor>>1)+bsBlueColor)/3;

									bsBlueColor = temp>0 ? (temp&bdBlueColor) : 0;
									bsGreenColor = temp>0 ? (temp&(bdGreenColor<<1)) : 0;
									bsRedColor = temp>0 ? (temp&bdRedColor) :  0;

									if ( bsBlueColor >= 32 )  bsBlueColor = 31;
									if ( bsGreenColor >= 64 )  bsGreenColor = 63;
									if ( bsRedColor >= 32 )  bsRedColor = 31;
									
									pwdDst[((nYCnt+nY) * lWidth) + (nLastWidth+nX) + i] = (bsBlueColor<<m_stBitsMaskInfo.bBShift) |  (bsGreenColor<<m_stBitsMaskInfo.bGShift) | (bsRedColor<<m_stBitsMaskInfo.bRShift);
								}
//								memcpy(&pwdDst[((nYCnt+nY) * lWidth) + (nLastWidth+nX)], &pwSrc[x], sizeof(WORD)*(rc.right-nLastWidth));
								x += nCntCopyWord;
							}
							else
							{
								for(WORD i = 0 ; i < (sizeof(WORD)*nCntCopyWord/2);i++)
								{
									bsBlueColor  = (BYTE)((pwSrc[x+i] & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bsGreenColor = (BYTE)((pwSrc[x+i] & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bsRedColor   = (BYTE)((pwSrc[x+i] & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									BYTE	temp;

									temp = (bsRedColor+(bsGreenColor>>1)+bsBlueColor)/3;

									bsBlueColor = temp>0 ? (temp&bdBlueColor) : 0;
									bsGreenColor = temp>0 ? (temp&(bdGreenColor<<1)) : 0;
									bsRedColor = temp>0 ? (temp&bdRedColor) :  0;

									if ( bsBlueColor >= 32 )  bsBlueColor = 31;
									if ( bsGreenColor >= 64 )  bsGreenColor = 63;
									if ( bsRedColor >= 32 )  bsRedColor = 31;

									pwdDst[((nYCnt+nY) * lWidth) + (nLastWidth+nX) + i] = (bsBlueColor<<m_stBitsMaskInfo.bBShift) |  (bsGreenColor<<m_stBitsMaskInfo.bGShift) | (bsRedColor<<m_stBitsMaskInfo.bRShift);
								}
								x += nCntCopyWord;
							}
						}
					}
					else if ( pwSrc[x] == 0xC2 || pwSrc[x] == 0xC3 )
					{
						WORD wDyingKind, wChooseColor;
						wDyingKind = pwSrc[x];
						switch ( wDyingKind )
						{
						case 0xC2: 
							wChooseColor = wChooseColor1;
							break;
						case 0xC3: 
							wChooseColor = wChooseColor2;
							break;
						}
						x++;
						nCntCopyWord = pwSrc[x];
						x++;

						nLastWidth = nCurrWidth;
						nCurrWidth += nCntCopyWord;

						if ( rc.left > nCurrWidth || rc.right < nLastWidth )
						{
							x += nCntCopyWord;
						}
						else
						{
							// rc.left???? ???????? ��???? ?????? ??? ???1?71?1?771?1?71?1?777.
							if ( nLastWidth < rc.left && rc.left <= nCurrWidth )
							{
								x += (rc.left-nLastWidth);
								for ( INT nCheck = 0; nCheck < nCurrWidth-rc.left; nCheck++ )
								{
									wPixel	  = wChooseColor;
									bBlueWantedColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenWantedColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedWantedColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									wPixel	  = pwSrc[x+nCheck];
									bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
									rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
									bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

									bBlueWantedColor = (BYTE)(((FLOAT)bBlueWantedColor*rBlueRate));
									bGreenWantedColor = (BYTE)(((FLOAT)bGreenWantedColor*rGreenRate));
									bRedWantedColor = (BYTE)(((FLOAT)bRedWantedColor*bRedRate));

									if ( bBlueWantedColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
										bBlueWantedColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
									if ( bGreenWantedColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
										bGreenWantedColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
									if ( bRedWantedColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
										bRedWantedColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);

									BYTE	temp;

									temp = (bsRedColor+(bGreenWantedColor>>1)+bBlueWantedColor)/3;

									bBlueWantedColor = temp>0 ? (temp&bdBlueColor) : 0;
									bGreenWantedColor = temp>0 ? (temp&(bdGreenColor<<1)) : 0;
									bRedWantedColor = temp>0 ? (temp&bdRedColor) :  0;

									if ( bBlueWantedColor >= 32 )  bBlueWantedColor = 31;
									if ( bGreenWantedColor >= 64 )  bGreenWantedColor = 63;
									if ( bRedWantedColor >= 32 )  bRedWantedColor = 31;

									if ( ((nYCnt+nY) * lWidth) + (nLastWidth+nX+nCheck) < 0 )
										return FALSE;

									pwdDst[((nYCnt+nY) * lWidth) + (nLastWidth+nX+nCheck)] = ((bRedWantedColor  <<m_stBitsMaskInfo.bRShift) |
																										  (bGreenWantedColor<<m_stBitsMaskInfo.bGShift) |
																										  (bBlueWantedColor <<m_stBitsMaskInfo.bBShift));			
								}
								x += (nCurrWidth-rc.left);
							}
							// rc.right???? ???????? ��???? ?????? ??? ???1?71?1?771?1?71?1?777.
							else if ( nLastWidth <= rc.right && rc.right < nCurrWidth )
							{
								for ( INT nCheck = 0; nCheck < rc.right-nLastWidth; nCheck++ )
								{
									wPixel	  = wChooseColor;
									bBlueWantedColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenWantedColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedWantedColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									wPixel	  = pwSrc[x+nCheck];
									bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
									rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
									bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

									bBlueWantedColor = (BYTE)(((FLOAT)bBlueWantedColor*rBlueRate));
									bGreenWantedColor = (BYTE)(((FLOAT)bGreenWantedColor*rGreenRate));
									bRedWantedColor = (BYTE)(((FLOAT)bRedWantedColor*bRedRate));

									if ( bBlueWantedColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
										bBlueWantedColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
									if ( bGreenWantedColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
										bGreenWantedColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
									if ( bRedWantedColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
										bRedWantedColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);

									BYTE	temp;

									temp = (bsRedColor+(bGreenWantedColor>>1)+bBlueWantedColor)/3;

									bBlueWantedColor = temp>0 ? (temp&bdBlueColor) : 0;
									bGreenWantedColor = temp>0 ? (temp&(bdGreenColor<<1)) : 0;
									bRedWantedColor = temp>0 ? (temp&bdRedColor) :  0;

									if ( bBlueWantedColor >= 32 )  bBlueWantedColor = 31;
									if ( bGreenWantedColor >= 64 )  bGreenWantedColor = 63;
									if ( bRedWantedColor >= 32 )  bRedWantedColor = 31;

									pwdDst[((nYCnt+nY) * lWidth) + (nLastWidth+nX+nCheck)] = ((bRedWantedColor  <<m_stBitsMaskInfo.bRShift) |
																										  (bGreenWantedColor<<m_stBitsMaskInfo.bGShift) |
																										  (bBlueWantedColor <<m_stBitsMaskInfo.bBShift));			
								}
								x += nCntCopyWord;
							}
							else
							{
								for ( INT nCheck = 0; nCheck < nCntCopyWord; nCheck++ )
								{
									wPixel	  = wChooseColor;
									bBlueWantedColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenWantedColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedWantedColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									wPixel	  = pwSrc[x+nCheck];
									bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
									rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
									bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

									bBlueWantedColor = (BYTE)(((FLOAT)bBlueWantedColor*rBlueRate));
									bGreenWantedColor = (BYTE)(((FLOAT)bGreenWantedColor*rGreenRate));
									bRedWantedColor = (BYTE)(((FLOAT)bRedWantedColor*bRedRate));

									if ( bBlueWantedColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
										bBlueWantedColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
									if ( bGreenWantedColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
										bGreenWantedColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
									if ( bRedWantedColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
										bRedWantedColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);

									BYTE	temp;

									temp = (bsRedColor+(bGreenWantedColor>>1)+bBlueWantedColor)/3;

									bBlueWantedColor = temp>0 ? (temp&bdBlueColor) : 0;
									bGreenWantedColor = temp>0 ? (temp&(bdGreenColor<<1)) : 0;
									bRedWantedColor = temp>0 ? (temp&bdRedColor) :  0;

									if ( bBlueWantedColor >= 32 )  bBlueWantedColor = 31;
									if ( bGreenWantedColor >= 64 )  bGreenWantedColor = 63;
									if ( bRedWantedColor >= 32 )  bRedWantedColor = 31;

									pwdDst[((nYCnt+nY) * lWidth) + (nLastWidth+nX+nCheck)] = ((bRedWantedColor  <<m_stBitsMaskInfo.bRShift) |
																										  (bGreenWantedColor<<m_stBitsMaskInfo.bGShift) |
																									 	  (bBlueWantedColor <<m_stBitsMaskInfo.bBShift));			
								}
								x += nCntCopyWord;
							}
						}
					}	
					else
					{
						m_pddsBackBuffer->Unlock(NULL);
						return FALSE;
					}
				}
				// ?????? ???? ???? ?????? ???????? ??????.
				nWidthEnd++;

				nWidthStart	= nWidthEnd;
				nCurrWidth = 0;
			}
		}

		m_pddsBackBuffer->Unlock(NULL);
		return TRUE;
	}
	return FALSE;
}



BOOL CWHDXGraphicWindow::DrawWithImageForCompClipRgnGray(INT nX, INT nY, INT nXSize, INT nYSize, WORD* pwSrc , WORD wClipWidth, WORD wClipHeight, WORD wChooseColor1, WORD wChooseColor2)
{
	RECT			rc;
	DDSURFACEDESC2	ddsd;

	INT	nWidth		= nXSize;
	INT	nHeight		= nYSize;
	INT	nXOffset	= 0;
	INT	nYOffset	= 0;
	INT	nStartX		= 0;
	INT	nStartY		= 0;
	INT	nEndX		= wClipWidth -1;
	INT	nEndY		= wClipHeight-1;

	INT FiveMiddle	= 0xf;
	INT SixMiddle	= 0x1f;

	BYTE bsBlueColor,bsGreenColor,bsRedColor;
	BYTE bdBlueColor,bdGreenColor,bdRedColor;

	bdBlueColor  = 0x0f;
	bdGreenColor = 0x1f;
	bdRedColor   = 0x0f;

	if ( m_pddsBackBuffer != NULL )
	{
		if (nX < nStartX )
		{ 
			nXOffset	= nStartX - nX;
			nWidth		= nXSize - nXOffset;
		}
		if ( (nX+nXSize-1) > nEndX )
			nWidth		= nEndX - nX - nXOffset + 1;		
		if ( nY < nStartY )	
		{ 
			nYOffset	= nStartY - nY;
			nHeight		= nYSize - nYOffset;
		}		
		if ( (nY+nYSize-1) > nEndY )
			nHeight	= nEndY - nY - nYOffset + 1;

		if ( (nWidth > 0) && (nHeight > 0) )
		{
			rc.left		= nXOffset;
			rc.right	= nXOffset+nWidth;
			rc.top		= nYOffset;
			rc.bottom	= nYOffset+nHeight;

			LONG	lWidth;
			ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
			ddsd.dwSize = sizeof(DDSURFACEDESC2);
			if ( FAILED(m_pddsBackBuffer->Lock(NULL, &ddsd, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL)) )
			{
				return FALSE;
			}

			WORD* pwdDst = (WORD*)ddsd.lpSurface;
			lWidth		 = ddsd.lPitch >> 1;

			INT nWidthStart	= 0;
			INT nWidthEnd	= 0;
			INT nCurrWidth  = 0;
			INT nCntCopyWord = 0;
			INT nYCnt =0;
			INT nLastWidth = 0;

			FLOAT rBlueRate, rGreenRate, bRedRate;
			BYTE  bRedSrc, bGreenSrc, bBlueSrc;
			BYTE  bRedWantedColor, bGreenWantedColor, bBlueWantedColor;
			WORD  wPixel;

			// y?? ?????.
			for ( nYCnt=0 ; nYCnt < rc.top ; nYCnt++ )
			{
				nWidthEnd += pwSrc[nWidthStart];
				nWidthStart++;
				nWidthEnd++;
				nWidthStart	= nWidthEnd;
			}

			// y?? ????? ?????? ???��?? Count???? ?????.
			for ( nYCnt=rc.top ; nYCnt < rc.bottom ; nYCnt++ )
			{
				// ??????? ????? ???��?.(?????? ????)
				nWidthEnd += pwSrc[nWidthStart];
				nWidthStart++;

				// ?????? ?????? ??? ??????.
				for ( INT x = nWidthStart; x < nWidthEnd ; )
				{
					if ( pwSrc[x] == 0xC0 )
					{
						x++;
						nCntCopyWord = pwSrc[x];
						x++;
						nCurrWidth += nCntCopyWord;
					}
					else if ( pwSrc[x] == 0xC1 )
					{
						x++;
						nCntCopyWord = pwSrc[x];	// ????
						x++;

						nLastWidth = nCurrWidth;
						nCurrWidth += nCntCopyWord;

						if ( rc.left > nCurrWidth || rc.right < nLastWidth )
						{
							x += nCntCopyWord;		// ???? ???? ???? ???1?71?1?771?1?71?1?777
						}
						else
						{
							// rc.left???? ???????? ��???? ?????? ??? ???1?71?1?771?1?71?1?777.
							if ( nLastWidth < rc.left && rc.left <= nCurrWidth )
							{
								x += (rc.left-nLastWidth);

								for(WORD i = 0 ; i < (sizeof(WORD)*(nCurrWidth-rc.left)/2);i++)
								{
									bsBlueColor  = (BYTE)((pwSrc[x+i] & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bsGreenColor = (BYTE)((pwSrc[x+i] & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bsRedColor   = (BYTE)((pwSrc[x+i] & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									// Draw Gray 									
									BYTE	temp;

									temp = (bsRedColor+bsGreenColor/2+bsBlueColor)/3;

									bsBlueColor = temp>0 ? (temp) : 0;
									bsGreenColor = temp>0 ? (temp<<1) : 0;
									bsRedColor = temp>0 ? (temp) :  0;									

									if ( ((nYCnt+nY) * lWidth) + (rc.left+nX) + i < 0 )
										return FALSE;

									pwdDst[((nYCnt+nY) * lWidth) + (rc.left+nX) + i] = (bsBlueColor<<m_stBitsMaskInfo.bBShift) |  (bsGreenColor<<m_stBitsMaskInfo.bGShift) | (bsRedColor<<m_stBitsMaskInfo.bRShift);
								}

								x += (nCurrWidth-rc.left);
							}
							// rc.right???? ???????? ��???? ?????? ??? ???1?71?1?771?1?71?1?777.
							else if ( nLastWidth <= rc.right && rc.right < nCurrWidth )
							{
								for(WORD i = 0 ; i < (sizeof(WORD)*(rc.right-nLastWidth));i++)
								{
									bsBlueColor  = (BYTE)((pwSrc[x+i] & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bsGreenColor = (BYTE)((pwSrc[x+i] & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bsRedColor   = (BYTE)((pwSrc[x+i] & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									// Draw Gray 									

									BYTE	temp;

									temp = (bsRedColor+bsGreenColor/2+bsBlueColor)/3;

									bsBlueColor = temp>0 ? (temp) : 0;
									bsGreenColor = temp>0 ? (temp<<1) : 0;
									bsRedColor = temp>0 ? (temp) :  0;

									pwdDst[((nYCnt+nY) * lWidth) + (nLastWidth+nX) + i] = (bsBlueColor<<m_stBitsMaskInfo.bBShift) |  (bsGreenColor<<m_stBitsMaskInfo.bGShift) | (bsRedColor<<m_stBitsMaskInfo.bRShift);
								}
//								memcpy(&pwdDst[((nYCnt+nY) * lWidth) + (nLastWidth+nX)], &pwSrc[x], sizeof(WORD)*(rc.right-nLastWidth));
								x += nCntCopyWord;
							}
							else
							{
								for(WORD i = 0 ; i < (sizeof(WORD)*nCntCopyWord/2);i++)
								{
									bsBlueColor  = (BYTE)((pwSrc[x+i] & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bsGreenColor = (BYTE)((pwSrc[x+i] & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bsRedColor   = (BYTE)((pwSrc[x+i] & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);
									// Draw Gray 
									BYTE	temp;

									temp = (bsRedColor+bsGreenColor/2+bsBlueColor)/3;

									bsBlueColor = temp>0 ? (temp) : 0;
									bsGreenColor = temp>0 ? (temp<<1) : 0;
									bsRedColor = temp>0 ? (temp) :  0;

									pwdDst[((nYCnt+nY) * lWidth) + (nLastWidth+nX) + i] = (bsBlueColor<<m_stBitsMaskInfo.bBShift) |  (bsGreenColor<<m_stBitsMaskInfo.bGShift) | (bsRedColor<<m_stBitsMaskInfo.bRShift);
								}
								x += nCntCopyWord;
							}
						}
					}
					else if ( pwSrc[x] == 0xC2 || pwSrc[x] == 0xC3 )
					{
						WORD wDyingKind, wChooseColor;
						wDyingKind = pwSrc[x];
						switch ( wDyingKind )
						{
						case 0xC2: 
							wChooseColor = wChooseColor1;
							break;
						case 0xC3: 
							wChooseColor = wChooseColor2;
							break;
						}
						x++;
						nCntCopyWord = pwSrc[x];
						x++;

						nLastWidth = nCurrWidth;
						nCurrWidth += nCntCopyWord;

						if ( rc.left > nCurrWidth || rc.right < nLastWidth )
						{
							x += nCntCopyWord;
						}
						else
						{
							// rc.left???? ???????? ��???? ?????? ??? ???1?71?1?771?1?71?1?777.
							if ( nLastWidth < rc.left && rc.left <= nCurrWidth )
							{
								x += (rc.left-nLastWidth);
								for ( INT nCheck = 0; nCheck < nCurrWidth-rc.left; nCheck++ )
								{
									wPixel	  = wChooseColor;
									bBlueWantedColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenWantedColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedWantedColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									wPixel	  = pwSrc[x+nCheck];
									bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
									rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
									bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

									bBlueWantedColor = (BYTE)(((FLOAT)bBlueWantedColor*rBlueRate));
									bGreenWantedColor = (BYTE)(((FLOAT)bGreenWantedColor*rGreenRate));
									bRedWantedColor = (BYTE)(((FLOAT)bRedWantedColor*bRedRate));

									if ( bBlueWantedColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
										bBlueWantedColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
									if ( bGreenWantedColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
										bGreenWantedColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
									if ( bRedWantedColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
										bRedWantedColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);

									// Draw Gray 
									BYTE	temp;
									temp = (bRedWantedColor+bGreenWantedColor/2+bBlueWantedColor)/3;

									bBlueWantedColor = temp>0 ? (temp) : 0;
									bGreenWantedColor = temp>0 ? (temp<<1) : 0;
									bRedWantedColor = temp>0 ? (temp) :  0;

									if ( ((nYCnt+nY) * lWidth) + (nLastWidth+nX+nCheck) < 0 )
										return FALSE;

									pwdDst[((nYCnt+nY) * lWidth) + (nLastWidth+nX+nCheck)] = ((bRedWantedColor  <<m_stBitsMaskInfo.bRShift) |
																										  (bGreenWantedColor<<m_stBitsMaskInfo.bGShift) |
																										  (bBlueWantedColor <<m_stBitsMaskInfo.bBShift));			
								}
								x += (nCurrWidth-rc.left);
							}
							// rc.right???? ???????? ��???? ?????? ??? ???1?71?1?771?1?71?1?777.
							else if ( nLastWidth <= rc.right && rc.right < nCurrWidth )
							{
								for ( INT nCheck = 0; nCheck < rc.right-nLastWidth; nCheck++ )
								{
									wPixel	  = wChooseColor;
									bBlueWantedColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenWantedColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedWantedColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									wPixel	  = pwSrc[x+nCheck];
									bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
									rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
									bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

									bBlueWantedColor = (BYTE)(((FLOAT)bBlueWantedColor*rBlueRate));
									bGreenWantedColor = (BYTE)(((FLOAT)bGreenWantedColor*rGreenRate));
									bRedWantedColor = (BYTE)(((FLOAT)bRedWantedColor*bRedRate));

									if ( bBlueWantedColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
										bBlueWantedColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
									if ( bGreenWantedColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
										bGreenWantedColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
									if ( bRedWantedColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
										bRedWantedColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);

									// Draw Gray 
									BYTE	temp;
									temp = (bRedWantedColor+bGreenWantedColor/2+bBlueWantedColor)/3;

									bBlueWantedColor = temp>0 ? (temp) : 0;
									bGreenWantedColor = temp>0 ? (temp<<1) : 0;
									bRedWantedColor = temp>0 ? (temp) :  0;

									pwdDst[((nYCnt+nY) * lWidth) + (nLastWidth+nX+nCheck)] = ((bRedWantedColor  <<m_stBitsMaskInfo.bRShift) |
																										  (bGreenWantedColor<<m_stBitsMaskInfo.bGShift) |
																										  (bBlueWantedColor <<m_stBitsMaskInfo.bBShift));			
								}
								x += nCntCopyWord;
							}
							else
							{
								for ( INT nCheck = 0; nCheck < nCntCopyWord; nCheck++ )
								{
									wPixel	  = wChooseColor;
									bBlueWantedColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenWantedColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedWantedColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									wPixel	  = pwSrc[x+nCheck];
									bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
									rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
									bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

									bBlueWantedColor = (BYTE)(((FLOAT)bBlueWantedColor*rBlueRate));
									bGreenWantedColor = (BYTE)(((FLOAT)bGreenWantedColor*rGreenRate));
									bRedWantedColor = (BYTE)(((FLOAT)bRedWantedColor*bRedRate));

									if ( bBlueWantedColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
										bBlueWantedColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
									if ( bGreenWantedColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
										bGreenWantedColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
									if ( bRedWantedColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
										bRedWantedColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);
									
									// Draw Gray 
									BYTE	temp;
									temp = (bRedWantedColor+bGreenWantedColor/2+bBlueWantedColor)/3;

									bBlueWantedColor = temp>0 ? (temp) : 0;
									bGreenWantedColor = temp>0 ? (temp<<1) : 0;
									bRedWantedColor = temp>0 ? (temp) :  0;

									pwdDst[((nYCnt+nY) * lWidth) + (nLastWidth+nX+nCheck)] = ((bRedWantedColor  <<m_stBitsMaskInfo.bRShift) |
																										  (bGreenWantedColor<<m_stBitsMaskInfo.bGShift) |
																									 	  (bBlueWantedColor <<m_stBitsMaskInfo.bBShift));			
								}
								x += nCntCopyWord;
							}
						}
					}	
					else
					{
						m_pddsBackBuffer->Unlock(NULL);
						return FALSE;
					}
				}
				// ?????? ???? ???? ?????? ???????? ??????.
				nWidthEnd++;

				nWidthStart	= nWidthEnd;
				nCurrWidth = 0;
			}
		}

		m_pddsBackBuffer->Unlock(NULL);
		return TRUE;
	}
	return FALSE;
}


VOID CWHDXGraphicWindow::DrawWithGrayBackBuffer()
{
	DDSURFACEDESC2	ddsd;
	RECT			rc = {0, 0, 800, 500};

	if ( m_pddsBackBuffer == NULL )			return;

	LONG	lWidth;
	ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
	ddsd.dwSize = sizeof(DDSURFACEDESC2);
	if ( FAILED(m_pddsBackBuffer->Lock(NULL, &ddsd, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL)) )
	{
		return;
	}

	lWidth = ddsd.lPitch >> 1;

	WORD *pwdSrc, *pwdDst;
	BYTE bsBlueColor, bsGreenColor, bsRedColor;
	pwdDst = (WORD*)ddsd.lpSurface;
	pwdSrc = (WORD*)ddsd.lpSurface;

	for ( INT y=rc.top ; y < rc.bottom ; y++ )
	{
		for ( INT x=rc.left ; x < rc.right ; x++ )
		{
			bsBlueColor  = (BYTE)((pwdSrc[y * lWidth + x] & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
			bsGreenColor = (BYTE)((pwdSrc[y * lWidth + x] & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
			bsRedColor   = (BYTE)((pwdSrc[y * lWidth + x] & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

			// Draw Gray 									
			BYTE	temp;

			temp = (bsRedColor+bsGreenColor/2+bsBlueColor)/3;

			bsBlueColor  = temp>0 ? (temp)    : 0;
			bsGreenColor = temp>0 ? (temp<<1) : 0;
			bsRedColor   = temp>0 ? (temp)    : 0;									

			pwdDst[y * lWidth + x] = (bsBlueColor<<m_stBitsMaskInfo.bBShift) |  (bsGreenColor<<m_stBitsMaskInfo.bGShift) | (bsRedColor<<m_stBitsMaskInfo.bRShift);
		}
	}

	m_pddsBackBuffer->Unlock(NULL);
}



/******************************************************************************************************************

	????? : CWHDXGraphicWindow::DrawWithImageForCompClipRgnToMem()

	????? : 
	????? : 

	????   : ?????1?71?1?771?1?71?1?777 ??? ??????? ????? ?????? ?????? ??? ???��? ??????? ???(pwDst)?? ��?????.

	???   : INT nStartX
	         INT nStartY
	         INT nWantedXSize
	         INT nWantedYSize
	         WORD* pwSrc
	         WORD wClipWidth
	         WORD wClipHeight
	         WORD wChooseColor1
	         WORD wChooseColor2
	???1?71?1?771?1?71?1?777   : BOOL

	[????][??????] : ????????

*******************************************************************************************************************/
BOOL CWHDXGraphicWindow::DrawWithImageForCompColorToMem(RECT rcWanted, WORD* pwSrc, WORD wColor, WORD* pwDst)
{
	if ( rcWanted.left < 0 || rcWanted.right < 0 || rcWanted.top < 0 || rcWanted.bottom < 0 ||
		 rcWanted.right - rcWanted.left < 0 || rcWanted.bottom - rcWanted.top < 0 || pwDst == NULL )
		 return FALSE;

	INT nWidthStart		= 0;
	INT nWidthEnd		= 0;
	INT nCurrWidth		= 0;
	INT nCntCopyWord	= 0;
	INT nYCnt			= 0;
	INT nLastWidth		= 0;

	// y?? ?????.
  	for ( nYCnt=0 ; nYCnt < rcWanted.top ; nYCnt++ )
	{
 		nWidthEnd += pwSrc[nWidthStart];
		nWidthStart++;
		nWidthEnd++;
		nWidthStart	= nWidthEnd;
	}

	INT nNewCurrWidth = 0;

	FLOAT rBlueRate, rGreenRate, rRedRate;
	BYTE  bRedSrc, bGreenSrc, bBlueSrc;
	BYTE  bRedStateColor, bGreenStateColor, bBlueStateColor;
	WORD  wPixel;

	// y?? ????? ?????? ???��?? Count???? ?????.
	for ( nYCnt=rcWanted.top ; nYCnt < rcWanted.bottom ; nYCnt++ )
	{
		// ??????? ????? ???��?.(?????? ????)
		nWidthEnd += pwSrc[nWidthStart];
		nWidthStart++;

		// ?????? ?????? ??? ??????.
		for ( INT x = nWidthStart; x < nWidthEnd ; )
		{
			if ( pwSrc[x] == 0xC0 )
			{
				x++;
				nCntCopyWord = pwSrc[x];
				x++;
				nLastWidth = nCurrWidth;
				nCurrWidth += nCntCopyWord;

				// rcWanted.left???? ???????? ��???? ?????? ??? ???1?71?1?771?1?71?1?777.
				if ( nLastWidth <= rcWanted.left && nCurrWidth >= rcWanted.right )
				{
					if ( ((nYCnt-rcWanted.top) * (rcWanted.right-rcWanted.left))  + nNewCurrWidth < 0 )
						return FALSE;

					memset(&pwDst[((nYCnt-rcWanted.top) * (rcWanted.right-rcWanted.left))  + nNewCurrWidth], 0, sizeof(WORD)*(rcWanted.right-rcWanted.left));
					nNewCurrWidth += rcWanted.right-rcWanted.left;
				}
				else if ( nLastWidth < rcWanted.left && nCurrWidth > rcWanted.left )
				{
					memset(&pwDst[((nYCnt-rcWanted.top) * (rcWanted.right-rcWanted.left)) + nNewCurrWidth], 0, sizeof(WORD)*(nCurrWidth-rcWanted.left));
					nNewCurrWidth += nCurrWidth-rcWanted.left;
				}
				else if ( nLastWidth >= rcWanted.left && nCurrWidth <= rcWanted.right )
				{
					memset(&pwDst[((nYCnt-rcWanted.top) * (rcWanted.right-rcWanted.left))  + nNewCurrWidth], 0, sizeof(WORD)*(nCntCopyWord));
					nNewCurrWidth += nCntCopyWord;
				}
				// rcWanted.right???? ???????? ��???? ?????? ??? ???1?71?1?771?1?71?1?777.
				else if ( nLastWidth < rcWanted.right && nCurrWidth > rcWanted.right )
				{
					memset(&pwDst[((nYCnt-rcWanted.top) * (rcWanted.right-rcWanted.left)) + nNewCurrWidth], 0, sizeof(WORD)*(rcWanted.right-nLastWidth));
					nNewCurrWidth += rcWanted.right-nLastWidth;
				}
			}
			else if ( pwSrc[x] == 0xC1 || pwSrc[x] == 0xC2 || pwSrc[x] == 0xC3 )
			{
				x++;
				nCntCopyWord = pwSrc[x];
				x++;

				nLastWidth = nCurrWidth;
				nCurrWidth += nCntCopyWord;

				if ( rcWanted.left > nCurrWidth || rcWanted.right < nLastWidth )
				{
					x += nCntCopyWord;
				}
				else
				{
					// rcWanted.left???? ???????? ��???? ?????? ??? ???1?71?1?771?1?71?1?777.
					if ( nLastWidth <= rcWanted.left && nCurrWidth >= rcWanted.right )
					{
						x += (rcWanted.left-nLastWidth);
						for ( INT nCheck = 0; nCheck < rcWanted.right-rcWanted.left; nCheck++ )
						{
							wPixel = wColor;
							bBlueStateColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
							bGreenStateColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
							bRedStateColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

							wPixel = pwSrc[x+nCheck];
							bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
							bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
							bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

							if ( !wColor )	
							{
								BYTE bTemp = (BYTE)(bBlueSrc+(bGreenSrc>>1)+bRedSrc)/3;
								if ( bTemp > 31 )	bTemp = 31;
								bBlueStateColor = bRedStateColor = bTemp;
								bGreenStateColor = bTemp<<1;
							}
							else
							{
								rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
								rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
								rRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

								bBlueStateColor  = (BYTE)((FLOAT)bBlueStateColor *rBlueRate );
								bGreenStateColor = (BYTE)((FLOAT)bGreenStateColor*rGreenRate);
								bRedStateColor   = (BYTE)((FLOAT)bRedStateColor  *rRedRate  );

								if ( bBlueStateColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
									bBlueStateColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
								if ( bGreenStateColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
									bGreenStateColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
								if ( bRedStateColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
									bRedStateColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);
							}

							if ( !bRedStateColor && !bGreenStateColor && !bBlueStateColor )		bRedStateColor = bGreenStateColor = bBlueStateColor = 1;

							if ( ((nYCnt-rcWanted.top) * (rcWanted.right-rcWanted.left)) + (nNewCurrWidth+nCheck) < 0 )
								return FALSE;

							pwDst[((nYCnt-rcWanted.top) * (rcWanted.right-rcWanted.left)) + (nNewCurrWidth+nCheck)] = ((bRedStateColor  <<m_stBitsMaskInfo.bRShift) |
																													   (bGreenStateColor<<m_stBitsMaskInfo.bGShift) |
																													   (bBlueStateColor <<m_stBitsMaskInfo.bBShift));			
						}
						nNewCurrWidth += rcWanted.right-rcWanted.left;
						x += (nCurrWidth-rcWanted.left);
					}
					else if ( nLastWidth < rcWanted.left && nCurrWidth > rcWanted.left )
					{
						x += (rcWanted.left-nLastWidth);
						for ( INT nCheck = 0; nCheck < nCurrWidth-rcWanted.left; nCheck++ )
						{
							wPixel = wColor;
							bBlueStateColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
							bGreenStateColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
							bRedStateColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

							wPixel = pwSrc[x+nCheck];
							bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
							bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
							bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

							if ( !wColor )	
							{
								BYTE bTemp = (BYTE)(bBlueSrc+(bGreenSrc>>1)+bRedSrc)/3;
								if ( bTemp > 31 )	bTemp = 31;
								bBlueStateColor = bRedStateColor = bTemp;
								bGreenStateColor = bTemp<<1;
							}
							else
							{
								rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
								rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
								rRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

								bBlueStateColor  = (BYTE)((FLOAT)bBlueStateColor *rBlueRate );
								bGreenStateColor = (BYTE)((FLOAT)bGreenStateColor*rGreenRate);
								bRedStateColor   = (BYTE)((FLOAT)bRedStateColor  *rRedRate  );

								if ( bBlueStateColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
									bBlueStateColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
								if ( bGreenStateColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
									bGreenStateColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
								if ( bRedStateColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
									bRedStateColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);
							}

							if ( !bRedStateColor && !bGreenStateColor && !bBlueStateColor )		bRedStateColor = bGreenStateColor = bBlueStateColor = 1;

							pwDst[((nYCnt-rcWanted.top) * (rcWanted.right-rcWanted.left)) + (nNewCurrWidth+nCheck)] = ((bRedStateColor  <<m_stBitsMaskInfo.bRShift) |
																													   (bGreenStateColor<<m_stBitsMaskInfo.bGShift) |
																													   (bBlueStateColor <<m_stBitsMaskInfo.bBShift));										
						}
						nNewCurrWidth += nCurrWidth-rcWanted.left;
						x += (nCurrWidth-rcWanted.left);
					}
					else if ( nLastWidth >= rcWanted.left && nCurrWidth <= rcWanted.right )
					{
						for ( INT nCheck = 0; nCheck < nCntCopyWord; nCheck++ )
						{
							wPixel = wColor;
							bBlueStateColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
							bGreenStateColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
							bRedStateColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

							wPixel = pwSrc[x+nCheck];
							bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
							bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
							bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

							if ( !wColor )	
							{
								BYTE bTemp = (BYTE)(bBlueSrc+(bGreenSrc>>1)+bRedSrc)/3;
								if ( bTemp > 31 )	bTemp = 31;
								bBlueStateColor = bRedStateColor = bTemp;
								bGreenStateColor = bTemp<<1;
							}
							else
							{
								if ( nCheck == 17 )
									int b = 0;

								rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
								rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
								rRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

								bBlueStateColor  = (BYTE)((FLOAT)bBlueStateColor *rBlueRate );
								bGreenStateColor = (BYTE)((FLOAT)bGreenStateColor*rGreenRate);
								bRedStateColor   = (BYTE)((FLOAT)bRedStateColor  *rRedRate  );

								if ( bBlueStateColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
									bBlueStateColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
								if ( bGreenStateColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
									bGreenStateColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
								if ( bRedStateColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
									bRedStateColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);
							}

							if ( !bRedStateColor && !bGreenStateColor && !bBlueStateColor )		bRedStateColor = bGreenStateColor = bBlueStateColor = 1;

							pwDst[((nYCnt-rcWanted.top) * (rcWanted.right-rcWanted.left)) + (nNewCurrWidth+nCheck)] = ((bRedStateColor  <<m_stBitsMaskInfo.bRShift) |
																													   (bGreenStateColor<<m_stBitsMaskInfo.bGShift) |
																													   (bBlueStateColor <<m_stBitsMaskInfo.bBShift));			
						}
						nNewCurrWidth += nCntCopyWord;
						x += nCntCopyWord;
					}
					// rcWanted.right???? ???????? ��???? ?????? ??? ???1?71?1?771?1?71?1?777.
					else if ( nLastWidth < rcWanted.right && nCurrWidth > rcWanted.right )
					{
						for ( INT nCheck = 0; nCheck < rcWanted.right-nLastWidth; nCheck++ )
						{
							wPixel = wColor;
							bBlueStateColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
							bGreenStateColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
							bRedStateColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

							wPixel = pwSrc[x+nCheck];
							bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
							bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
							bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

							if ( !wColor )	
							{
								BYTE bTemp = (BYTE)(bBlueSrc+(bGreenSrc>>1)+bRedSrc)/3;
								if ( bTemp > 31 )	bTemp = 31;
								bBlueStateColor = bRedStateColor = bTemp;
								bGreenStateColor = bTemp<<1;
							}
							else
							{
								rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
								rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
								rRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

								bBlueStateColor  = (BYTE)((FLOAT)bBlueStateColor *rBlueRate );
								bGreenStateColor = (BYTE)((FLOAT)bGreenStateColor*rGreenRate);
								bRedStateColor   = (BYTE)((FLOAT)bRedStateColor  *rRedRate  );

								if ( bBlueStateColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
									bBlueStateColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
								if ( bGreenStateColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
									bGreenStateColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
								if ( bRedStateColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
									bRedStateColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);
							}

							if ( !bRedStateColor && !bGreenStateColor && !bBlueStateColor )		bRedStateColor = bGreenStateColor = bBlueStateColor = 1;

							pwDst[((nYCnt-rcWanted.top) * (rcWanted.right-rcWanted.left)) + (nNewCurrWidth+nCheck)] = ((bRedStateColor  <<m_stBitsMaskInfo.bRShift) |
																													   (bGreenStateColor<<m_stBitsMaskInfo.bGShift) |
																													   (bBlueStateColor <<m_stBitsMaskInfo.bBShift));			
						}
						nNewCurrWidth += rcWanted.right-nLastWidth;
						x += nCntCopyWord;
					}
					else
					{
						x += nCntCopyWord;
					}
				}
			}	
			else
			{
				return FALSE;
			}
		}
		// ?????? ???? ???? ?????? ???????? ??????.
		nWidthEnd++;

		nWidthStart	= nWidthEnd;
		nCurrWidth = 0;
		nNewCurrWidth = 0;
	}
	return TRUE;
}




BOOL CWHDXGraphicWindow::DrawWithImageForCompToMem(RECT rcWanted, WORD* pwSrc, WORD wChooseColor1, WORD wChooseColor2, WORD* pwDst)
{
	if ( rcWanted.left < 0 || rcWanted.right < 0 || rcWanted.top < 0 || rcWanted.bottom < 0 ||
		 rcWanted.right - rcWanted.left < 0 || rcWanted.bottom - rcWanted.top < 0 || pwDst == NULL )
		 return FALSE;

	INT nWidthStart		= 0;
	INT nWidthEnd		= 0;
	INT nCurrWidth		= 0;
	INT nCntCopyWord	= 0;
	INT nYCnt			= 0;
	INT nLastWidth		= 0;

	// y?? ?????.
  	for ( nYCnt=0 ; nYCnt < rcWanted.top ; nYCnt++ )
	{
 		nWidthEnd += pwSrc[nWidthStart];
		nWidthStart++;
		nWidthEnd++;
		nWidthStart	= nWidthEnd;
	}

	INT nNewCurrWidth = 0;

	FLOAT rBlueRate, rGreenRate, bRedRate;
	BYTE  bRedSrc, bGreenSrc, bBlueSrc;
	BYTE  bRedWantedColor, bGreenWantedColor, bBlueWantedColor;
	WORD  wPixel;

	// y?? ????? ?????? ???��?? Count???? ?????.
	for ( nYCnt=rcWanted.top ; nYCnt < rcWanted.bottom ; nYCnt++ )
	{
		// ??????? ????? ???��?.(?????? ????)
		nWidthEnd += pwSrc[nWidthStart];
		nWidthStart++;

		// ?????? ?????? ??? ??????.
		for ( INT x = nWidthStart; x < nWidthEnd ; )
		{
			if ( pwSrc[x] == 0xC0 )
			{
				x++;
				nCntCopyWord = pwSrc[x];
				x++;
				nLastWidth = nCurrWidth;
				nCurrWidth += nCntCopyWord;

				// rcWanted.left???? ???????? ��???? ?????? ??? ???1?71?1?771?1?71?1?777.
				if ( nLastWidth <= rcWanted.left && nCurrWidth >= rcWanted.right )
				{
					if ( ((nYCnt-rcWanted.top) * (rcWanted.right-rcWanted.left))  + nNewCurrWidth < 0 )
						return FALSE;

					memset(&pwDst[((nYCnt-rcWanted.top) * (rcWanted.right-rcWanted.left))  + nNewCurrWidth], 0, sizeof(WORD)*(rcWanted.right-rcWanted.left));
					nNewCurrWidth += rcWanted.right-rcWanted.left;
				}
				else if ( nLastWidth < rcWanted.left && nCurrWidth > rcWanted.left )
				{
					memset(&pwDst[((nYCnt-rcWanted.top) * (rcWanted.right-rcWanted.left)) + nNewCurrWidth], 0, sizeof(WORD)*(nCurrWidth-rcWanted.left));
					nNewCurrWidth += nCurrWidth-rcWanted.left;
				}
				else if ( nLastWidth >= rcWanted.left && nCurrWidth <= rcWanted.right )
				{
					memset(&pwDst[((nYCnt-rcWanted.top) * (rcWanted.right-rcWanted.left))  + nNewCurrWidth], 0, sizeof(WORD)*(nCntCopyWord));
					nNewCurrWidth += nCntCopyWord;
				}
				// rcWanted.right???? ???????? ��???? ?????? ??? ???1?71?1?771?1?71?1?777.
				else if ( nLastWidth < rcWanted.right && nCurrWidth > rcWanted.right )
				{
					memset(&pwDst[((nYCnt-rcWanted.top) * (rcWanted.right-rcWanted.left)) + nNewCurrWidth], 0, sizeof(WORD)*(rcWanted.right-nLastWidth));
					nNewCurrWidth += rcWanted.right-nLastWidth;
				}
			}
			else if ( pwSrc[x] == 0xC1 )
			{
				x++;
				nCntCopyWord = pwSrc[x];
				x++;

				nLastWidth = nCurrWidth;
				nCurrWidth += nCntCopyWord;

				if ( rcWanted.left > nCurrWidth || rcWanted.right < nLastWidth )
				{
					x += nCntCopyWord;
				}
				else
				{
					// rcWanted.left???? ???????? ��???? ?????? ??? ???1?71?1?771?1?71?1?777.
					if ( nLastWidth <= rcWanted.left && nCurrWidth >= rcWanted.right )
					{
						if ( ((nYCnt-rcWanted.top) * (rcWanted.right-rcWanted.left)) + nNewCurrWidth < 0 )
							return FALSE;

						x += (rcWanted.left-nLastWidth);
						memcpy(&pwDst[((nYCnt-rcWanted.top) * (rcWanted.right-rcWanted.left)) + nNewCurrWidth], &pwSrc[x], sizeof(WORD)*(rcWanted.right-rcWanted.left));
						nNewCurrWidth += rcWanted.right-rcWanted.left;
						x += (nCurrWidth-rcWanted.left);
					}
					else if ( nLastWidth < rcWanted.left && nCurrWidth > rcWanted.left )
					{
						x += (rcWanted.left-nLastWidth);
						memcpy(&pwDst[((nYCnt-rcWanted.top) * (rcWanted.right-rcWanted.left)) + nNewCurrWidth], &pwSrc[x], sizeof(WORD)*(nCurrWidth-rcWanted.left));
						nNewCurrWidth += nCurrWidth-rcWanted.left;
						x += (nCurrWidth-rcWanted.left);
					}
					else if ( nLastWidth >= rcWanted.left && nCurrWidth <= rcWanted.right )
					{
						memcpy(&pwDst[((nYCnt-rcWanted.top) * (rcWanted.right-rcWanted.left)) + nNewCurrWidth], &pwSrc[x], sizeof(WORD)*nCntCopyWord);
						nNewCurrWidth += nCntCopyWord;
						x += nCntCopyWord;
					}
					// rcWanted.right???? ???????? ��???? ?????? ??? ???1?71?1?771?1?71?1?777.
					else if ( nLastWidth < rcWanted.right && nCurrWidth > rcWanted.right )
					{
						memcpy(&pwDst[((nYCnt-rcWanted.top) * (rcWanted.right-rcWanted.left)) + nNewCurrWidth], &pwSrc[x], sizeof(WORD)*(rcWanted.right-nLastWidth));
						nNewCurrWidth += rcWanted.right-nLastWidth;
						x += nCntCopyWord;
					}
					else
					{
						x += nCntCopyWord;
					}
				}
			}
			else if ( pwSrc[x] == 0xC2 || pwSrc[x] == 0xC3 )
			{
				WORD wDyingKind, wChooseColor;
				wDyingKind = pwSrc[x];
				switch ( wDyingKind )
				{
				case 0xC2: 
					wChooseColor = wChooseColor1;
					break;
				case 0xC3: 
					wChooseColor = wChooseColor2;
					break;
				}
				x++;
				nCntCopyWord = pwSrc[x];
				x++;

				nLastWidth = nCurrWidth;
				nCurrWidth += nCntCopyWord;

				if ( rcWanted.left > nCurrWidth || rcWanted.right < nLastWidth )
				{
					x += nCntCopyWord;
				}
				else
				{
					// rcWanted.left???? ???????? ��???? ?????? ??? ???1?71?1?771?1?71?1?777.
					if ( nLastWidth <= rcWanted.left && nCurrWidth >= rcWanted.right )
					{
						x += (rcWanted.left-nLastWidth);
						for ( INT nCheck = 0; nCheck < rcWanted.right-rcWanted.left; nCheck++ )
						{
							wPixel	  = wChooseColor;
							bBlueWantedColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
							bGreenWantedColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
							bRedWantedColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

							wPixel	  = pwSrc[x+nCheck];
							bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
							bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
							bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

							rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
							rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
							bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

							bBlueWantedColor = (BYTE)(((FLOAT)bBlueWantedColor*rBlueRate));
							bGreenWantedColor = (BYTE)(((FLOAT)bGreenWantedColor*rGreenRate));
							bRedWantedColor = (BYTE)(((FLOAT)bRedWantedColor*bRedRate));

							if ( bBlueWantedColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
								bBlueWantedColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
							if ( bGreenWantedColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
								bGreenWantedColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
							if ( bRedWantedColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
								bRedWantedColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);

							if ( ((nYCnt-rcWanted.top) * (rcWanted.right-rcWanted.left)) + (nNewCurrWidth+nCheck) < 0 )
								return FALSE;

							pwDst[((nYCnt-rcWanted.top) * (rcWanted.right-rcWanted.left)) + (nNewCurrWidth+nCheck)] = ((bRedWantedColor  <<m_stBitsMaskInfo.bRShift) |
																													   (bGreenWantedColor<<m_stBitsMaskInfo.bGShift) |
																													   (bBlueWantedColor <<m_stBitsMaskInfo.bBShift));			
						}
						nNewCurrWidth += rcWanted.right-rcWanted.left;
						x += (nCurrWidth-rcWanted.left);
					}
					else if ( nLastWidth < rcWanted.left && nCurrWidth > rcWanted.left )
					{
						x += (rcWanted.left-nLastWidth);
						for ( INT nCheck = 0; nCheck < nCurrWidth-rcWanted.left; nCheck++ )
						{
							wPixel	  = wChooseColor;
							bBlueWantedColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
							bGreenWantedColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
							bRedWantedColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

							wPixel	  = pwSrc[x+nCheck];
							bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
							bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
							bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

							rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
							rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
							bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

							bBlueWantedColor = (BYTE)(((FLOAT)bBlueWantedColor*rBlueRate));
							bGreenWantedColor = (BYTE)(((FLOAT)bGreenWantedColor*rGreenRate));
							bRedWantedColor = (BYTE)(((FLOAT)bRedWantedColor*bRedRate));

							if ( bBlueWantedColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
								bBlueWantedColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
							if ( bGreenWantedColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
								bGreenWantedColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
							if ( bRedWantedColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
								bRedWantedColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);

							pwDst[((nYCnt-rcWanted.top) * (rcWanted.right-rcWanted.left)) + (nNewCurrWidth+nCheck)] = ((bRedWantedColor  <<m_stBitsMaskInfo.bRShift) |
																													   (bGreenWantedColor<<m_stBitsMaskInfo.bGShift) |
																													   (bBlueWantedColor <<m_stBitsMaskInfo.bBShift));										
						}
						nNewCurrWidth += nCurrWidth-rcWanted.left;
						x += (nCurrWidth-rcWanted.left);
					}
					else if ( nLastWidth >= rcWanted.left && nCurrWidth <= rcWanted.right )
					{
						for ( INT nCheck = 0; nCheck < nCntCopyWord; nCheck++ )
						{
							wPixel	  = wChooseColor;
							bBlueWantedColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
							bGreenWantedColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
							bRedWantedColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

							wPixel	  = pwSrc[x+nCheck];
							bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
							bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
							bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

							rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
							rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
							bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

							bBlueWantedColor = (BYTE)(((FLOAT)bBlueWantedColor*rBlueRate));
							bGreenWantedColor = (BYTE)(((FLOAT)bGreenWantedColor*rGreenRate));
							bRedWantedColor = (BYTE)(((FLOAT)bRedWantedColor*bRedRate));

							if ( bBlueWantedColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
								bBlueWantedColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
							if ( bGreenWantedColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
								bGreenWantedColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
							if ( bRedWantedColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
								bRedWantedColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);

							pwDst[((nYCnt-rcWanted.top) * (rcWanted.right-rcWanted.left)) + (nNewCurrWidth+nCheck)] = ((bRedWantedColor  <<m_stBitsMaskInfo.bRShift) |
																													   (bGreenWantedColor<<m_stBitsMaskInfo.bGShift) |
																													   (bBlueWantedColor <<m_stBitsMaskInfo.bBShift));			
						}
						nNewCurrWidth += nCntCopyWord;
						x += nCntCopyWord;
					}
					// rcWanted.right???? ???????? ��???? ?????? ??? ???1?71?1?771?1?71?1?777.
					else if ( nLastWidth < rcWanted.right && nCurrWidth > rcWanted.right )
					{
						for ( INT nCheck = 0; nCheck < rcWanted.right-nLastWidth; nCheck++ )
						{
							wPixel	  = wChooseColor;
							bBlueWantedColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
							bGreenWantedColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
							bRedWantedColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

							wPixel	  = pwSrc[x+nCheck];
							bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
							bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
							bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

							rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
							rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
							bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

							bBlueWantedColor = (BYTE)(((FLOAT)bBlueWantedColor*rBlueRate));
							bGreenWantedColor = (BYTE)(((FLOAT)bGreenWantedColor*rGreenRate));
							bRedWantedColor = (BYTE)(((FLOAT)bRedWantedColor*bRedRate));

							if ( bBlueWantedColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
								bBlueWantedColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
							if ( bGreenWantedColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
								bGreenWantedColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
							if ( bRedWantedColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
								bRedWantedColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);


							pwDst[((nYCnt-rcWanted.top) * (rcWanted.right-rcWanted.left)) + (nNewCurrWidth+nCheck)] = ((bRedWantedColor  <<m_stBitsMaskInfo.bRShift) |
																													   (bGreenWantedColor<<m_stBitsMaskInfo.bGShift) |
																													   (bBlueWantedColor <<m_stBitsMaskInfo.bBShift));			
						}
						nNewCurrWidth += rcWanted.right-nLastWidth;
						x += nCntCopyWord;
					}
					else
					{
						x += nCntCopyWord;
					}
				}
			}	
			else
			{
				return FALSE;
			}
		}
		// ?????? ???? ???? ?????? ???????? ??????.
		nWidthEnd++;

		nWidthStart	= nWidthEnd;
		nCurrWidth = 0;
		nNewCurrWidth = 0;
	}
	return TRUE;
}











BOOL CWHDXGraphicWindow::DrawWithImageForComp(RECT* prcSrc, WORD* pwSrc, CDX9SoftSurface* pddsDst)
{
	DDSURFACEDESC2		ddsd;

	LONG	lWidth;
	ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
	ddsd.dwSize = sizeof(DDSURFACEDESC2);
	if ( FAILED(pddsDst->Lock(NULL, &ddsd, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL)) )
	{
		return FALSE;
	}

	WORD* pwdDst = (WORD*)ddsd.lpSurface;
	lWidth		 = ddsd.lPitch >> 1;

	INT nYCnt		= 0;
	INT nWidthStart	= 0;
	INT nWidthEnd	= 0;
	INT nCurrWidth  = 0;
	INT nLastWidth	= 0;
	INT nCntCopyWord= 0;

	// y?? ?????.
	for ( nYCnt=0 ; nYCnt < prcSrc->top ; nYCnt++ )
	{
		nWidthEnd += pwSrc[nWidthStart];
		nWidthStart++;
		nWidthEnd++;
		nWidthStart	= nWidthEnd;
	}

	// y?? ????? ?????? ???��?? Count???? ?????.
	for ( nYCnt=prcSrc->top; nYCnt < prcSrc->bottom ; nYCnt++ )
	{
		// ??????? ????? ???��?.(?????? ????)
		nWidthEnd += pwSrc[nWidthStart];
		nWidthStart++;

		// ?????? ?????? ??? ??????.
		for ( INT x = nWidthStart; x < nWidthEnd ; )
		{
			if ( pwSrc[x] == 0xC0 )
			{
				x++;
				nCntCopyWord = pwSrc[x];
				x++;
				nLastWidth = nCurrWidth;
				nCurrWidth += nCntCopyWord;
			}
			else if ( pwSrc[x] == 0xC1 )
			{
				x++;
				nCntCopyWord = pwSrc[x];
				x++;

				nLastWidth = nCurrWidth;
				nCurrWidth += nCntCopyWord;

				// prcSrc->left???? ???????? ��???? ?????? ??? ???1?71?1?771?1?71?1?777.
				if ( nLastWidth <= prcSrc->left && prcSrc->left <= nCurrWidth )
				{
					if ( ((nYCnt-prcSrc->top) * lWidth) < 0 )
						return FALSE;

					if ( prcSrc->right <= nCurrWidth )
					{
						x += (prcSrc->left-nLastWidth);
						memcpy(&pwdDst[((nYCnt-prcSrc->top) * lWidth)], &pwSrc[x], sizeof(WORD)*(prcSrc->right-prcSrc->left));
						x += (nCurrWidth-prcSrc->left);
					}
					else
					{
						x += (prcSrc->left-nLastWidth);
						memcpy(&pwdDst[((nYCnt-prcSrc->top) * lWidth)], &pwSrc[x], sizeof(WORD)*(nCurrWidth-prcSrc->left));
						x += (nCurrWidth-prcSrc->left);
					}
				}
				// prcSrc->right???? ???????? ��???? ?????? ??? ???1?71?1?771?1?71?1?777.
				else if ( nLastWidth < prcSrc->right && prcSrc->right < nCurrWidth )
				{
					memcpy(&pwdDst[((nYCnt-prcSrc->top) * lWidth) + (nLastWidth-prcSrc->left)], &pwSrc[x], sizeof(WORD)*(prcSrc->right-nLastWidth));
					x += nCntCopyWord;
				}
				// prcSrc->left?????????? ??��??1?71?1?771?1?71?1?777.
				else if ( nLastWidth < prcSrc->left && nCurrWidth <= prcSrc->left )
				{
					x += nCntCopyWord;
				}
				// prcSrc->right?????????? ??��??1?71?1?771?1?71?1?777.
				else if ( nLastWidth >= prcSrc->right && nCurrWidth > prcSrc->right )
				{
					x += nCntCopyWord;
				}
				// prcSrc->left???? prcSrc->right?? ????? ??��??1?71?1?771?1?71?1?777.
				else
				{
					memcpy(&pwdDst[((nYCnt-prcSrc->top) * lWidth) + (nLastWidth-prcSrc->left)], &pwSrc[x], sizeof(WORD)*nCntCopyWord);
					x += nCntCopyWord;
				}
			}
			else
			{
				pddsDst->Unlock(NULL);
				return FALSE;
			}
		}
		// ?????? ???? ???? ?????? ???????? ??????.
		nWidthEnd++;

		nWidthStart	= nWidthEnd;
		nCurrWidth = 0;
	}

	pddsDst->Unlock(NULL);
	return TRUE;
}















/******************************************************************************************************************

	????? : CWHDXGraphicWindow::DrawWithABlendF2orIntersectCompData()

	????? : 
	????? : 

	????   : 
	???   : INT nX					: ????????1?71?1?771?1?71?1?777 ??????????.
	         INT nY					
	         INT nSrcXSize			: ????????? ??????.
	         INT nSrcYSize
	         WORD* pwSrc			: ????????? ?????.
	         INT nDstXSize			: ??????? ??????.
	         INT nDstYSize
	         WORD* pwDst			: ??????? ?????.
	         WORD wSrcChooseColor1	: ????????? ???? ?????1 ?��?.
	         WORD wSrcChooseColor2
	         WORD wDstChooseColor1	: ??????? ???? ?????1 ?��?.
	         WORD wDstChooseColor2
	???1?71?1?771?1?71?1?777   : BOOL 

	[????][??????] : ????????

*******************************************************************************************************************/
BOOL CWHDXGraphicWindow::DrawWithABlendForIntersectCompData(INT nSrcX, INT nSrcY, 
															INT nSrcXSize, INT nSrcYSize, WORD* pwSrc,
															INT nDstX, INT nDstY,
															INT nDstXSize, INT nDstYSize, WORD* pwDst,
															WORD wClipWidth, WORD wClipHeight,
															BYTE bOpa, BOOL bFocused,
															WORD wSrcChooseColor1, WORD wSrcChooseColor2, 
															WORD wDstChooseColor1, WORD wDstChooseColor2,
															WORD wSrcColor, WORD wDstColor)
{
	DDSURFACEDESC2	ddsd;
	RECT rcSrc, rcDst;
	RECT rcSrcIntersect = { 0, 0, 0, 0 };
	RECT rcDstIntersect = { 0, 0, 0, 0 };
	WORD* pwdDst = NULL;

	INT	nSrcWidth	= nSrcXSize;
	INT	nSrcHeight	= nSrcYSize;
	INT	nXSrcOffset	= 0;
	INT	nYSrcOffset	= 0;

	INT	nDstWidth	= nDstXSize;
	INT	nDstHeight	= nDstYSize;
	INT	nXDstOffset	= 0;
	INT	nYDstOffset	= 0;

	INT	nEndX		= wClipWidth -1;
	INT	nEndY		= wClipHeight-1;
	INT	nStartX		= 0;
	INT	nStartY		= 0;

	WORD* pwDstSave = NULL;
	WORD* pwSrcSave = NULL;

	if ( m_pddsBackBuffer != NULL )
	{
		// Source?? ?????????? ????.
		if ( nSrcX < nStartX )
		{ 
			nXSrcOffset	= nStartX - nSrcX;
			nSrcWidth	= nSrcXSize - nXSrcOffset;
		}
		if ( (nSrcX+nSrcXSize-1) > nEndX )
			nSrcWidth	= nEndX - nSrcX - nXSrcOffset + 1;		
		if ( nSrcY < nStartY )	
		{ 
			nYSrcOffset	= nStartY - nSrcY;
			nSrcHeight	= nSrcYSize - nYSrcOffset;
		}		
		if ( (nSrcY+nSrcYSize-1) > nEndY )
			nSrcHeight	= nEndY - nSrcY - nYSrcOffset + 1;

		if ( (nSrcWidth > 0) && (nSrcHeight > 0) )
		{
			// Source?????? ????.
			rcSrc.left		= nXSrcOffset;
			rcSrc.right		= nXSrcOffset+nSrcWidth;
			rcSrc.top		= nYSrcOffset;
			rcSrc.bottom	= nYSrcOffset+nSrcHeight;

			// Destination?? ?????????? ????.
			if ( nDstX < nStartX )
			{ 
				nXDstOffset	= nStartX - nDstX;
				nDstWidth	= nDstXSize - nXDstOffset;
			}
			if ( (nDstX+nDstXSize-1) > nEndX )
				nDstWidth	= nEndX - nDstX - nXDstOffset + 1;		
			if ( nDstY < nStartY )	
			{ 
				nYDstOffset	= nStartY - nDstY;
				nDstHeight	= nDstYSize - nYDstOffset;
			}		
			if ( (nDstY+nDstYSize-1) > nEndY )
				nDstHeight	= nEndY - nDstY - nYDstOffset + 1;

			if ( (nDstWidth > 0) && (nDstHeight > 0) )
			{
				// Destination?????? ????.
				rcDst.left		= nXDstOffset;
				rcDst.right		= nXDstOffset+nDstWidth;
				rcDst.top		= nYDstOffset;
				rcDst.bottom	= nYDstOffset+nDstHeight;

				// ????????? ????? ?????? ?????.
				RECT rcTemp;

				// ????????? ???????1?71?1?771?1?71?1?777 ??????????? ?????.
				rcTemp.left = rcDst.left + (nDstX-nSrcX);
				rcTemp.top = rcDst.top + (nDstY-nSrcY);
				rcTemp.right = rcDst.right + (nDstX-nSrcX);
				rcTemp.bottom = rcDst.bottom + (nDstY-nSrcY);

				// ??? ????????? ???????? ?? ?? ?��???? ?????��???? ?????.
				IntersectRect(&rcSrcIntersect, &rcSrc, &rcTemp);

				// ??????????? ???????1?71?1?771?1?71?1?777 ????????? ?????.
				rcTemp.left = rcSrc.left + (nSrcX-nDstX);
				rcTemp.top = rcSrc.top + (nSrcY-nDstY);
				rcTemp.right = rcSrc.right + (nSrcX-nDstX);
				rcTemp.bottom = rcSrc.bottom + (nSrcY-nDstY);

				// ????? ????????? ???????? ?? ?? ?��???? ?????��???? ?????.
				IntersectRect(&rcDstIntersect, &rcTemp, &rcDst);

				LONG	lWidth;
				ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
				ddsd.dwSize = sizeof(DDSURFACEDESC2);
				if ( FAILED(m_pddsBackBuffer->Lock(NULL, &ddsd, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL)) )
				{
					return FALSE;
				}

				WORD* pwdDst = (WORD*)ddsd.lpSurface;
				lWidth		 = ddsd.lPitch >> 1;

				pwSrcSave = new WORD[(rcSrcIntersect.right-rcSrcIntersect.left)*(rcSrcIntersect.bottom-rcSrcIntersect.top)];
				pwDstSave = new WORD[(rcDstIntersect.right-rcDstIntersect.left)*(rcDstIntersect.bottom-rcDstIntersect.top)];

				if ( wSrcColor == 0XFFFF && wDstColor == 0XFFFF )
				{
					DrawWithImageForCompToMem(rcSrcIntersect, (WORD*)pwSrc, wSrcChooseColor1, wSrcChooseColor2, pwSrcSave);
					DrawWithImageForCompToMem(rcDstIntersect, (WORD*)pwDst, wDstChooseColor1, wDstChooseColor2, pwDstSave);
				}
				else if ( wSrcColor != 0XFFFF && wDstColor == 0XFFFF )
				{
					DrawWithImageForCompColorToMem(rcSrcIntersect, (WORD*)pwSrc, wSrcColor, pwSrcSave);
					DrawWithImageForCompToMem(rcDstIntersect, (WORD*)pwDst, wDstChooseColor1, wDstChooseColor2, pwDstSave);
				}
				else if ( wSrcColor == 0XFFFF && wDstColor != 0XFFFF )
				{
					DrawWithImageForCompToMem(rcSrcIntersect, (WORD*)pwSrc, wSrcChooseColor1, wSrcChooseColor2, pwSrcSave);
					DrawWithImageForCompColorToMem(rcDstIntersect, (WORD*)pwDst, wDstColor, pwDstSave);
				}
				else
				{
					DrawWithImageForCompColorToMem(rcSrcIntersect, (WORD*)pwSrc, wSrcColor, pwSrcSave);
					DrawWithImageForCompColorToMem(rcDstIntersect, (WORD*)pwDst, wDstColor, pwDstSave);
				}

				BYTE bRedDst, bGreenDst, bBlueDst;
				BYTE bRedSrc, bGreenSrc, bBlueSrc;
				WORD wSrcPixel, wDstPixel;
				FLOAT rBlueRate, rGreenRate, bRedRate;

				for ( INT nYCnt = 0; nYCnt < rcSrcIntersect.bottom - rcSrcIntersect.top; nYCnt++ )
					for ( INT nXCnt = 0; nXCnt < rcSrcIntersect.right - rcSrcIntersect.left; nXCnt++ )
				{
					wSrcPixel = pwSrcSave[nYCnt*(rcSrcIntersect.right - rcSrcIntersect.left) + nXCnt];
					wDstPixel = pwDstSave[nYCnt*(rcDstIntersect.right - rcDstIntersect.left) + nXCnt];
					if ( wSrcPixel != 0 && wDstPixel != 0 )
					{
						bBlueSrc  = (BYTE)((wSrcPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
						bGreenSrc = (BYTE)((wSrcPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
						bRedSrc   = (BYTE)((wSrcPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

						if ( bFocused )
						{
							rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
							rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
							bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

							bBlueSrc = (BYTE)((FLOAT)bBlueSrc + ((FLOAT)bBlueSrc*rBlueRate));
							bGreenSrc = (BYTE)((FLOAT)bGreenSrc + ((FLOAT)bGreenSrc*rGreenRate));
							bRedSrc = (BYTE)((FLOAT)bRedSrc + ((FLOAT)bRedSrc*bRedRate));

							if ( bBlueSrc > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
								bBlueSrc = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
							if ( bGreenSrc > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
								bGreenSrc = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
							if ( bRedSrc > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
								bRedSrc = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);

							// ??????? ?????? Opacity???? ???????.
							bOpa = 40;
						}

						bBlueDst  = (BYTE)((wDstPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
						bGreenDst = (BYTE)((wDstPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
						bRedDst   = (BYTE)((wDstPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

						bBlueDst  = (BYTE)((bOpa*(bBlueDst -bBlueSrc )+100*bBlueSrc ) / 100);
						bGreenDst = (BYTE)((bOpa*(bGreenDst-bGreenSrc)+100*bGreenSrc) / 100);
						bRedDst   = (BYTE)((bOpa*(bRedDst  -bRedSrc  )+100*bRedSrc  ) / 100);

						pwdDst[((nYCnt+nSrcY+rcSrcIntersect.top) * lWidth) + (rcSrcIntersect.left+nSrcX+nXCnt)] = 
												((bRedDst  <<m_stBitsMaskInfo.bRShift) |
												 (bGreenDst<<m_stBitsMaskInfo.bGShift) |
												 (bBlueDst <<m_stBitsMaskInfo.bBShift));
					}
				}

				SAFE_DELETEARR(pwSrcSave);
				SAFE_DELETEARR(pwDstSave);

				m_pddsBackBuffer->Unlock(NULL);
				return TRUE;
			}
		}
	}
	return FALSE;
}





/*******************************************************************************************************************/

HRESULT CWHDXGraphicWindow::Present()
{
	// [#24-FIX] TestCooperativeLevel before NULL guard to allow recovery
	if ( !m_pd3dDevice )
		return E_POINTER;
	HRESULT hrCoop = m_pd3dDevice->TestCooperativeLevel();
	if ( hrCoop == D3DERR_DEVICELOST )
	{
		Sleep(10);  // �ȴ��豸�ָ�
		return D3DERR_DEVICELOST;
	}
	if ( hrCoop == D3DERR_DEVICENOTRESET )
	{
		// �豸��Ҫ����
		// �ͷ� D3DPOOL_DEFAULT ��Դ
		if ( m_pd3dPresentTex ) { m_pd3dPresentTex->Release(); m_pd3dPresentTex = NULL; }
		if ( m_pSprite ) { m_pSprite->Release(); m_pSprite = NULL; }
		// �����豸
		D3DPRESENT_PARAMETERS d3dpp;
		ZeroMemory(&d3dpp, sizeof(d3dpp));
		d3dpp.Windowed          = TRUE;
		d3dpp.SwapEffect        = D3DSWAPEFFECT_DISCARD;
		d3dpp.hDeviceWindow     = m_hWnd;
		d3dpp.BackBufferFormat  = D3DFMT_UNKNOWN;
		d3dpp.BackBufferWidth   = m_stDisplayInfo.wWidth;
		d3dpp.BackBufferHeight  = m_stDisplayInfo.wHeight;
		hrCoop = m_pd3dDevice->Reset(&d3dpp);
		if ( SUCCEEDED(hrCoop) )
		{
			// �ؽ���Դ
			DWORD w = m_stDisplayInfo.wWidth, h = m_stDisplayInfo.wHeight;
			hrCoop = m_pd3dDevice->CreateTexture(w, h, 1, D3DUSAGE_DYNAMIC, D3DFMT_R5G6B5, D3DPOOL_DEFAULT, &m_pd3dPresentTex, NULL);
			if ( FAILED(hrCoop) )
				hrCoop = m_pd3dDevice->CreateTexture(w, h, 1, D3DUSAGE_DYNAMIC, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &m_pd3dPresentTex, NULL);
			if ( FAILED(hrCoop) )
				hrCoop = m_pd3dDevice->CreateTexture(w, h, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pd3dPresentTex, NULL);
			if ( SUCCEEDED(hrCoop) )
			{
				HRESULT hrSprite = D3DXCreateSprite(m_pd3dDevice, &m_pSprite);
				if ( FAILED(hrSprite) ) m_pSprite = NULL;  // [FIX-F3] 检查返回值
			}
		}
		else
		{
			return hrCoop;
		}
	}
	// 1) ���� backbuffer(565) ת 32bit �ϴ�������
	// [FIX-F3] 检查纹理是否创建成功，防止 NULL 指针崩溃
	if ( !m_pd3dPresentTex ) return D3DERR_DEVICELOST;
	D3DLOCKED_RECT lr;
	// [FIX-F5] D3DUSAGE_DYNAMIC ����Ӧʹ�� D3DLOCK_DISCARD,
	// �����������ܵȴ� GPU ������һ֡����, ���ͬ��ͣ�١�
	if ( FAILED(m_pd3dPresentTex->LockRect(0, &lr, NULL, D3DLOCK_DISCARD)) ) return E_FAIL;
	DWORD w = m_stDisplayInfo.wWidth, h = m_stDisplayInfo.wHeight;
	BYTE* pDst = (BYTE*)lr.pBits;
	WORD* pSrc = m_pSoftBackBuffer;

	// [FIX-R5] 纹理是 565 时直接整行拷贝, 不再逐像素展开成 32 位。
	// 每帧 48 万像素的移位/或运算在这里整体消失。
	D3DSURFACE_DESC sdPresent;
	BOOL bPresent565 = ( SUCCEEDED(m_pd3dPresentTex->GetLevelDesc(0, &sdPresent)) &&
	                     sdPresent.Format == D3DFMT_R5G6B5 );

	if ( bPresent565 )
	{
		DWORD dwRowBytes = w * sizeof(WORD);
		for ( DWORD y=0; y<h; y++ )
			memcpy( pDst + y*lr.Pitch, pSrc + y*w, dwRowBytes );
	}
	else
	{
		for ( DWORD y=0; y<h; y++ )
		{
			BYTE* dRow = pDst + y*lr.Pitch;
			WORD* sRow = pSrc + y*w;
			for ( DWORD x=0; x<w; x++ )
			{
				WORD c = sRow[x];
				BYTE r = (BYTE)((c>>11)&0x1F); r = (BYTE)((r<<3)|(r>>2));
				BYTE g = (BYTE)((c>>5)&0x3F);  g = (BYTE)((g<<2)|(g>>4));
				BYTE b = (BYTE)(c&0x1F);       b = (BYTE)((b<<3)|(b>>2));
				dRow[x*4+0]=b; dRow[x*4+1]=g; dRow[x*4+2]=r; dRow[x*4+3]=0xFF;
			}
		}
	}
	m_pd3dPresentTex->UnlockRect(0);
	// 2) �� D3DXSprite ��������
	HRESULT hrScene = m_pd3dDevice->BeginScene();
	if ( FAILED(hrScene) )
	{
		// �豸��ʧ����һ֡����
		return hrScene;
	}
	if ( m_pSprite )
	{
		m_pSprite->Begin(D3DXSPRITE_ALPHABLEND);
		// [DX9][FIX] Դ����=ʵ�ʻ�����(w*h)��identity ������
		RECT rcSrc;
		SetRect(&rcSrc, 0, 0, (LONG)w, (LONG)h);
		D3DXMATRIX matIdent;
		D3DXMatrixIdentity(&matIdent);
		m_pSprite->SetTransform(&matIdent);
		m_pSprite->Draw(m_pd3dPresentTex, &rcSrc, NULL, NULL, 0xFFFFFFFF);
		m_pSprite->End();
	}
	m_pd3dDevice->EndScene();
			// [weather][fx] overlay effects before swap
		g_EffectLayer.Render();
HRESULT hrPresent = m_pd3dDevice->Present(NULL, NULL, NULL, NULL);
	if ( FAILED(hrPresent) )
	{
		return hrPresent;
	}
	return S_OK;
}



/******************************************************************************************************************

	????? : RestoreSurfaces()

	????? : 
	????? : 

	????   : 
	???1?71?1?771?1?71?1?777   : HRESULT 

	[????][??????] : ????????

*******************************************************************************************************************/
HRESULT CWHDXGraphicWindow::RestoreSurfaces()
{
	return S_OK;
}













/******************************************************************************************************************

  Callback Function

*******************************************************************************************************************/


/******************************************************************************************************************

	????? : WINAPI DXGDriverEnumCallbackEx()

	????? : 
	????? : 

	????   : 
	???   : GUID *pGUID
	         LPSTR szDesc
	         LPSTR szName
	         LPVOID pContext
	         HMONITOR hm
	???1?71?1?771?1?71?1?777   : static BOOL 

	[????][??????] : ????????

*******************************************************************************************************************/
static BOOL WINAPI DXGDriverEnumCallbackEx(GUID *pGUID, LPSTR szDesc, LPSTR szName, LPVOID pContext, HMONITOR hm)
{
	return DDENUMRET_OK;
}


/******************************************************************************************************************

	????? : WINAPI DXGEnumDisplayModeCallback()

	????? : 
	????? : 

	????   : 
	???   : DDSURFACEDESC2* pddsd
	         VOID* pParentInfo
	???1?71?1?771?1?71?1?777   : static HRESULT 

	[????][??????] : ????????

*******************************************************************************************************************/
static HRESULT WINAPI DXGEnumDisplayModeCallback(DDSURFACEDESC2* pddsd, VOID* pParentInfo)
{
	return DDENUMRET_OK;
}


/******************************************************************************************************************

	????? : DXGModesSortCallback()

	????? : 
	????? : 

	????   : 
	???   : const VOID* arg1
	         const VOID* arg2
	???1?71?1?771?1?71?1?777   : static int 

	[????][??????] : ????????

*******************************************************************************************************************/
static int DXGModesSortCallback(const VOID* arg1, const VOID* arg2)
{
    DDSURFACEDESC2* p1 = (DDSURFACEDESC2*)arg1;
    DDSURFACEDESC2* p2 = (DDSURFACEDESC2*)arg2;

    if ( p1->dwWidth < p2->dwWidth )        return -1;
    if ( p1->dwWidth > p2->dwWidth )        return +1;

    if ( p1->dwHeight < p2->dwHeight )      return -1;
    if ( p1->dwHeight > p2->dwHeight )      return +1;

    if ( p1->ddpfPixelFormat.dwRGBBitCount < p2->ddpfPixelFormat.dwRGBBitCount )        return -1;
    if ( p1->ddpfPixelFormat.dwRGBBitCount > p2->ddpfPixelFormat.dwRGBBitCount )        return +1;

    return 0;
}


/******************************************************************************************************************

	????? : WINAPI DXG3DDeviceEnumCallback()

	????? : 
	????? : 

	????   : 
	???   : TCHAR* szDesc
	         TCHAR* szName
	         D3DDEVICEDESC7* pDesc
	         VOID* pParentInfo
	???1?71?1?771?1?71?1?777   : static HRESULT 

	[????][??????] : ????????

*******************************************************************************************************************/
static HRESULT WINAPI DXG3DDeviceEnumCallback(TCHAR* szDesc, TCHAR* szName, LPVOID pDesc, VOID* pParentInfo)
{
	return DDENUMRET_OK;
}


/******************************************************************************************************************

	????? : WINAPI DXGEnumZBufferFormatsCallback()

	????? : 
	????? : 

	????   : 
	???   : DDPIXELFORMAT* pddpf
	         VOID* pContext
	???1?71?1?771?1?71?1?777   : static HRESULT 

	[????][??????] : ????????

*******************************************************************************************************************/
static HRESULT WINAPI DXGEnumZBufferFormatsCallback(DDPIXELFORMAT* pddpf, VOID* pContext)
{
	return DDENUMRET_OK;
}
