#ifndef _WINDHORN_DXGRAPHIC
#define	_WINDHORN_DXGRAPHIC

#include <windows.h>
#include <ddraw.h>
#include <d3d9.h>
#include <d3dx9.h>
#include "../Common/DiagLog.h"	// [DX9][LOGGATE] MIR3_DIAG_LOG


/******************************************************************************************************************

	CWHDXGraphicWindow Class Declaration

*******************************************************************************************************************/

typedef struct tagDXG_DISPLAYINFO 
{
	WORD			wWidth;
	WORD			wHeight;
	WORD			wBPP;
	INT				nRefreshRate;
}DXG_DISPLAYINFO, *LPDXG_DISPLAYINFO;

typedef struct DXG_ENUM_DEVICEINFO
{
    TCHAR			szDeviceDesc[256];
    BOOL			f3DHardware;
	TCHAR			szDriverDesc[256];
    GUID            guidDevice;
    GUID            guidDriver;
    DWORD           dwNumModes;
}DXG_ENUM_DEVICEINFO, *LPDXG_ENUM_DEVICEINFO;

typedef struct tagDXG_MASKINFO
{
	BYTE			bRCnt;
	BYTE			bGCnt;
	BYTE			bBCnt;
	BYTE			bRShift;
	BYTE			bGShift;
	BYTE			bBShift;
	DWORD			dwRMask;
	DWORD			dwGMask;
	DWORD			dwBMask;
}DXG_MASKINFO, *LPDXG_MASKINFO;

// ===== DX9 迁移: 软件 surface 包装(模拟 IDirectDrawSurface7 �?Lock/Unlock/Blt/Restore) =====
class CDX9SoftSurface {
public:
    WORD*  m_pBits;
    LONG   m_lPitch;
    DWORD  m_dwWidth, m_dwHeight;
    DWORD  m_dwColorKey;
    HDC    m_hDC;   HBITMAP m_hDib;   void*  m_dibBits;
    CDX9SoftSurface():m_pBits(NULL),m_lPitch(0),m_dwWidth(0),m_dwHeight(0),m_dwColorKey(0),m_hDC(NULL),m_hDib(NULL),m_dibBits(NULL){}
    HRESULT Lock(LPRECT, LPDDSURFACEDESC2 ddsd, DWORD, LPVOID){
        if(!m_pBits) return DDERR_SURFACELOST;
        ZeroMemory(ddsd,sizeof(*ddsd)); ddsd->dwSize=sizeof(*ddsd);
        ddsd->lpSurface=(LPVOID)m_pBits; ddsd->lPitch=m_lPitch;
        ddsd->dwWidth=m_dwWidth; ddsd->dwHeight=m_dwHeight;
        return DD_OK;
    }
    HRESULT Unlock(LPRECT=NULL){ return DD_OK; }
    // [FIX-F3] m_pBits �?CreateDXG �?malloc 分配(m_pSoftBackBuffer),
    // 原实现用 delete[] 释放 malloc 内存属未定义行为(切分辨率/退出时随机堆损�?�?
    // 缓冲区所有权�?CWHDXGraphicWindow, 这里不释放像�? 只销毁包装对象�?
    void Release(){ m_pBits=NULL; delete this; }
    HRESULT Restore(){ return DD_OK; }
    HRESULT Blt(LPRECT prcDst, CDX9SoftSurface* pSrc, LPRECT prcSrc, DWORD dwFlags, LPDDBLTFX lpDDBltFx){
        if(dwFlags & DDBLT_COLORFILL){
            // [FIX-F4] 原实现忽�?prcDst, 永远整屏填充; 局部填充调用会误刷全屏�?
            DWORD c = lpDDBltFx ? lpDDBltFx->dwFillColor : 0;
            if(m_pBits){
                LONG fx0=prcDst?prcDst->left:0, fy0=prcDst?prcDst->top:0;
                LONG fx1=prcDst?prcDst->right:(LONG)m_dwWidth, fy1=prcDst?prcDst->bottom:(LONG)m_dwHeight;
                if(fx0<0)fx0=0; if(fy0<0)fy0=0;
                if(fx1>(LONG)m_dwWidth)fx1=m_dwWidth; if(fy1>(LONG)m_dwHeight)fy1=m_dwHeight;
                LONG words=m_lPitch>>1;
                for(LONG y=fy0;y<fy1;y++){
                    WORD* row=m_pBits+(size_t)y*words;
                    for(LONG x=fx0;x<fx1;x++) row[x]=(WORD)c;
                }
            }
        } else if(pSrc && pSrc->m_pBits && m_pBits){
            LONG sx=prcSrc?prcSrc->left:0, sy=prcSrc?prcSrc->top:0;
            LONG dx=prcDst?prcDst->left:0, dy=prcDst?prcDst->top:0;
            LONG w=prcDst?(prcDst->right-prcDst->left):pSrc->m_dwWidth;
            LONG h=prcDst?(prcDst->bottom-prcDst->top):pSrc->m_dwHeight;
            // [DX9][T58] 负坐标裁�? 窗口表面 Blt 到主缓冲�?rcDst 起点可为�?�?NPC �?m_nSX=-63)�?
            //   原实现不裁剪�?dx/dy, 直接�?m_pBits 负偏移写 �?越界写堆 �?点击 NPC 必崩
            //   (CrashLog faultAddr=0x0B443D22 = m_pBits �?126B, msvcrt memcpy WRITE)�?
            if(dx<0){ sx-=dx; w+=dx; dx=0; }
            if(dy<0){ sy-=dy; h+=dy; dy=0; }
            if(sx<0){ dx-=sx; w+=sx; sx=0; }
            if(sy<0){ dy-=sy; h+=sy; sy=0; }
            if(dx+w>(LONG)m_dwWidth)  w=m_dwWidth-dx;
            if(dy+h>(LONG)m_dwHeight) h=m_dwHeight-dy;
            if(sx+w>(LONG)pSrc->m_dwWidth)  w=pSrc->m_dwWidth-sx;
            if(sy+h>(LONG)pSrc->m_dwHeight) h=pSrc->m_dwHeight-sy;
            if(w<=0||h<=0) return DD_OK;
            // [FIX-F4] 原实现完全忽�?DDBLT_KEYSRC: DrawWithSurface 等调用要�?源色键透明",
            // 但源图黑底会被原样拷�? 形成黑框/黑边。这里补回色键跳过�?
            BOOL bKey = (dwFlags & DDBLT_KEYSRC) && (pSrc->m_dwColorKey != 0);
            WORD wKey = (WORD)pSrc->m_dwColorKey;
            for(LONG y=0;y<h;y++){
                WORD* s=pSrc->m_pBits+(size_t)(sy+y)*pSrc->m_dwWidth+sx;
                WORD* d=m_pBits+(size_t)(dy+y)*(m_lPitch>>1)+dx;
                if(bKey){
                    for(LONG x=0;x<w;x++){ WORD c=s[x]; if(c!=wKey) d[x]=c; }
                } else {
                    CopyMemory(d,s,(size_t)w*sizeof(WORD));
                }
            }
        }
        return DD_OK;
    }
    HRESULT GetSurfaceDesc(LPDDSURFACEDESC2 ddsd){
        ZeroMemory(ddsd,sizeof(*ddsd)); ddsd->dwSize=sizeof(*ddsd);
        ddsd->dwWidth=m_dwWidth; ddsd->dwHeight=m_dwHeight; ddsd->lPitch=m_lPitch;
        return DD_OK;
    }
    // GDI 文字绘制支持: 创建 565 DIB 关联软件像素
    HRESULT GetDC(HDC* phDC){
        #if MIR3_DIAG_LOG
        { static BOOL bGDC2=FALSE; if(!bGDC2){ FILE*fp=fopen("LogMap.txt","ab"); if(fp){ fprintf(fp,"DX9-GetDC-ENTRY m_hDC=%p m_hDib=%p m_dibBits=%p m_pBits=%p w=%lu h=%lu pitch=%lu\r\n",m_hDC,m_hDib,m_dibBits,m_pBits,(unsigned long)m_dwWidth,(unsigned long)m_dwHeight,(unsigned long)m_lPitch); fclose(fp);} bGDC2=TRUE; } }
        #endif // MIR3_DIAG_LOG
        if(!m_hDC){
            m_hDC=CreateCompatibleDC(NULL);
            BITMAPINFO bmi; ZeroMemory(&bmi,sizeof(bmi));
            bmi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
            bmi.bmiHeader.biWidth=(LONG)m_dwWidth;
            bmi.bmiHeader.biHeight=-(LONG)m_dwHeight;
            bmi.bmiHeader.biPlanes=1;
            bmi.bmiHeader.biBitCount=16;
            bmi.bmiHeader.biCompression=BI_BITFIELDS;
            DWORD masks[3]={0xF800,0x07E0,0x001F};
            memcpy(bmi.bmiColors,masks,12);
            m_hDib=CreateDIBSection(m_hDC,&bmi,DIB_RGB_COLORS,&m_dibBits,NULL,0);
            SelectObject(m_hDC,m_hDib);
        }
        // [DX9][B] 黑块根因修复: GetDC 先把软缓冲当前内容同步进 DIB,
        // �?GDI(TextOut/FillRect)在背景上叠加, 而非在零初始�?全黑)DIB 上覆�?
        // ReleaseDC 复制回时背景立绘得以保留
        // [DX9][AUDIT] 独立审计确认同一根因: CDX9SoftSurface::GetDC() DIB 初始为黑�?
        // ReleaseDC 整体拷贝�?m_pBits 覆盖背景立绘, 详见 AUDIT_REPORT.md
        if(m_hDib && m_dibBits && m_pBits && m_dwWidth && m_dwHeight){
            #if MIR3_DIAG_LOG
            { static BOOL bGDC=FALSE; if(!bGDC){ FILE*fp=fopen("LogMap.txt","ab"); if(fp){ fprintf(fp,"DX9-GetDC-BODY sync m_pBits=%p m_dibBits=%p w=%lu h=%lu pitch=%lu\r\n",m_pBits,m_dibBits,(unsigned long)m_dwWidth,(unsigned long)m_dwHeight,(unsigned long)m_lPitch); fclose(fp);} bGDC=TRUE; } }
            #endif // MIR3_DIAG_LOG
            LONG pitch=m_lPitch?m_lPitch:(LONG)m_dwWidth*2;
            for(LONG y=0;y<(LONG)m_dwHeight;y++)
                memcpy((BYTE*)m_dibBits+(size_t)y*m_dwWidth*2,(BYTE*)m_pBits+y*pitch,m_dwWidth*2);
        }
        if(phDC) *phDC=m_hDC;
        return DD_OK;
    }
    HRESULT ReleaseDC(HDC hDC){
        if(m_hDib && m_dibBits && m_pBits && m_dwWidth && m_dwHeight){
            LONG pitch=m_lPitch?m_lPitch:(LONG)m_dwWidth*2;
            for(LONG y=0;y<(LONG)m_dwHeight;y++)
                memcpy((BYTE*)m_pBits+y*pitch,(BYTE*)m_dibBits+(size_t)y*m_dwWidth*2,m_dwWidth*2);
        }
        return DD_OK;
    }
};



class CWHDXGraphicWindow : public CWHWindow
{
//1: Constuctor/Destructor
public:
	CWHDXGraphicWindow(WORD wWidth = _SCREEN_WIDTH_LARGE, WORD wHeight = _SCREEN_HEIGHT_LARGE, WORD wBpp = _SCREEN_BPP);
	~CWHDXGraphicWindow();

//2: Variables
protected:
		IDirect3D9*				m_pD3D;			// DX9
	IDirect3DDevice9*		m_pd3dDevice;	// DX9 设备
	IDirect3DSurface9*		m_pd3dBackBuffer;	// DX9 backbuffer surface
	IDirect3DTexture9*		m_pd3dPresentTex;	// 软件backbuffer上传目标纹理
	ID3DXSprite*			m_pSprite;		// D3DX9 精灵渲染�?
	ID3DXFont*				m_pFont;		// D3DX9 字体
	CDX9SoftSurface*		m_pddsBackBuffer;	// 软件 backbuffer 包装(兼容 Lock/Blt 调用)
	WORD*					m_pSoftBackBuffer;	// backbuffer 像素(16bit 565)
	DWORD					m_dwSoftBufSize;
	BOOL					m_bSoftBufLocked;

///////<---------------------------------------------------------------------------------------------------------------------------
	
///////<---------------------------------------------------------------------------------------------------------------------------

	BYTE					m_bScreenModeFlag;
    BYTE					m_bDeviceModeFlag;

public:
	HFONT					m_hDefGameFont;
///////<---------------------------------------------------------------------------------------------------------------------------
	CHAR					m_pszDefFont[MAX_PATH];
///////<---------------------------------------------------------------------------------------------------------------------------

	DWORD					m_dwTextureTotal;	// ��ü ����Ҽ�?�ִ� �޸�.
	DWORD					m_dwTextureFree;    // ���� ���� �޸�.
	DWORD					m_dwVideoTotal; 	// vram ��ü �޸�.
	DWORD					m_dwVideoFree;      // ���� ���� vram �޸� .

	BOOL					m_bIsWindowActive;
	BOOL					m_bIsWindowReady;

	BOOL					m_bIsActiveChanged;

    RECT					m_rcWindow;
	DXG_DISPLAYINFO			m_stDisplayInfo;
	DXG_MASKINFO			m_stBitsMaskInfo;

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// ���μ����� ���������� ��������.
	static CWHDXGraphicWindow*	m_pxDXGWnd;
	CWHDefProcess*				m_pxDefProcess;

	VOID					(*RenderProcess)(int);
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//3: Methods
protected:
	BYTE	FindDriverAndDevice();
	VOID	FreeDXGEnumModeResources();

	HRESULT CreateDXG();
	HRESULT Clear(DWORD dwColor);
	HRESULT	CreatePrimarySurface();
	HRESULT CreateZBuffer(const GUID* pDeviceGUID);
	HRESULT Init3DDeviceObjects();
	HRESULT DestroyDXGObjects();

public:

        __inline IDirect3D9*			GetDirect3D()		{ return m_pD3D;			}
	__inline IDirect3DDevice9*		Get3DDevice()		{ return m_pd3dDevice;		}
    __inline CDX9SoftSurface*		GetBackBuffer()     { return m_pddsBackBuffer;	}
    __inline BYTE					GetDeviceMode()     { return m_bDeviceModeFlag; }
	__inline BYTE					GetScreenMode()		{ return m_bScreenModeFlag; }

    DXG_MASKINFO			GetRGBMaskInfoIN16Bits(LPVOID pSurface);
	virtual VOID			UpdateBoundsWnd();

	HRESULT					ResetDXG(WORD wWidth, WORD wHeight, WORD wBPP, BYTE bScreenModeFlag, BYTE bDeviceModeFlag);
	BOOL					Create(HINSTANCE hInst, LPTSTR lpCaption = NULL, CHAR *pszMenuName = NULL, CHAR* pszIconName = NULL, BYTE bScreenModeFlag = _DXG_SCREENMODE_WINDOW, BYTE bDeviceModeFlag = _DXG_DEVICEMODE_PRIMARY, BOOL bLarge = TRUE);

///////<---------------------------------------------------------------------------------------------------------------------------
	HFONT					CreateGameFont(LPCSTR szFontName, INT nHeight, INT nWidth = 0, INT nWeight = FW_NORMAL, BOOL bItalic = FALSE, BOOL bULine = FALSE, BOOL bStrikeOut = FALSE, DWORD dwCharSet = 134);  // [#38-FIX] GB2312_CHARSET for Chinese
	VOID					SetDefFont(CHAR* pszFont = NULL, INT nFontSize = 9);


	VOID					PutsHan(CDX9SoftSurface* pSurface, INT nX, INT nY, COLORREF foreColor, COLORREF backColor, CHAR* szText, HFONT hFont = NULL);
	VOID					PutsHanA(CDX9SoftSurface* pSurface, INT nX, INT nY, COLORREF foreColor, COLORREF backColor, CHAR* szText, HFONT hFont = NULL);
	VOID					PutsHan(CDX9SoftSurface* pSurface, RECT rc, COLORREF foreColor, COLORREF backColor, CHAR* szText, HFONT hFont = NULL);
	VOID					PutsHanLeft(CDX9SoftSurface* pSurface, RECT rc, COLORREF foreColor, COLORREF backColor, CHAR* szText, HFONT hFont = NULL);
	VOID					PutsHanEx(CDX9SoftSurface* pSurface, RECT rc, COLORREF foreColor, COLORREF backColor, CHAR* szText, HFONT hFont = NULL);

	SIZE					GetStrLength(CDX9SoftSurface* pSurface, HFONT hFont, CHAR* szFormat);
///////<---------------------------------------------------------------------------------------------------------------------------

	BOOL 					StringDivide(INT nDivideWidth, INT& nDividedLine, CHAR* szSrc, CHAR* szResult, HFONT hFont = NULL);
//	BOOL					StringDivideLen(INT nDivideLen, INT& nDividedLine, CHAR* szSrc, CDLList<CHAR*>* m_pxpStr);

	VOID 					StringPlus(CHAR* szResult, CHAR* szSrc, ...);
	BOOL					BelongStr(CHAR* pszSrc, CHAR* pszCmp);
	CHAR*					IntToStr(INT nNum);

	VOID					UsedAndFreeMemoryCheck();


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	WORD					ConvertColor24To16(COLORREF dwColor);
	// ������ �Լ�����.
	// 1. GDI�� �׸��� �Լ�.(��, �簢������ �鼭�ǽ��� �׸���.)
	HRESULT					DrawWithGDI(RECT* prc, CDX9SoftSurface* pSurface, DWORD dwColor, BYTE bKind);
	// 2. WORD�� ����Ÿ �׸��� �Լ�.(�ȼ������� �÷�Ű�� �˻��ϸ鼭 �鼭�ǽ��� �׸���.)
	BOOL					DrawWithImage(INT nX, INT nY, INT nXSize, INT nYSize, WORD* pwSrc);
	// 3. WORD�� ����Ÿ �׸��� �Լ�.(���δ����� �÷�Ű�� �������� �ʰ� �鼭�ǽ��� Ŭ���� ���ο������� �׸���.)
	BOOL					DrawWithImagePerLineClipRgn(INT nX, INT nY, INT nXSize, INT nYSize, WORD* pwSrc, WORD wClipWidth, WORD wClipHeight);
	// 4. Surface�� ����Ÿ �׸��� �Լ�.(�ҽ��� �÷�Ű�� �����ϸ鼭 �鼭�ǽ��� �׸���.)
	BOOL					DrawWithSurface(INT nX, INT nY, INT nXSize, INT nYSize, CDX9SoftSurface* pddsSrc);

	// wChooseColor1�� ������(0xC2)�� ���ؼ�, wChooseColor2(0xC3)�� �Ӹ���(Į, ��Ÿ ���ϴ»�)�� �������� ����.
	// 5. ����(0��)�� ������ WORD�� ����Ÿ �׸��� �Լ�.(�鼭�ǽ��� �׸���.)
	BOOL					DrawWithImageForComp(INT nX, INT nY, RECT* prcSrc, WORD* pwSrc, WORD wChooseColor1 = 0XFFFF, WORD wChooseColor2 = 0XFFFF, CDX9SoftSurface* pddsDst = NULL);
	BOOL					DrawWithImageForComp(RECT* prcSrc, WORD* pwSrc, CDX9SoftSurface* pddsDst);
	BOOL					DrawWithImageForComp(INT nX, INT nY, INT nXSize, INT nYSize, WORD* pwSrc, WORD wChooseColor1 = 0XFFFF, WORD wChooseColor2 = 0XFFFF);
	// [DX9][T21] RLE wil 图解码后最近邻缩放 blit（背�?640x480 铺满 800x600�?
	BOOL					DrawWithImageForCompScaled(INT nX, INT nY, INT nSrcXSize, INT nSrcYSize, WORD* pwSrc, INT nDstXSize, INT nDstYSize, WORD wChooseColor1 = 0XFFFF, WORD wChooseColor2 = 0XFFFF);
	// [DX9][B] AVI 当前�?24bit BGR)�?65 缩放绘制到软 backbuffer
	BOOL					DrawAviFrame(class CAvi* pAvi, INT nX, INT nY, INT nDstW, INT nDstH);
	// [DX9][B] 加载 BMP(24bit)�?65缓冲（登录静态背景用�?
	WORD*					LoadBmp565(LPCSTR szPath, INT* pW, INT* pH);
	void					FreeBmp565(WORD* pBuf);
	// [DX9][B] 原始565像素最近邻缩放绘制（非wil RLE�?
	BOOL					Draw565Scaled(INT nX, INT nY, INT nSrcW, INT nSrcH, WORD* pwSrc, INT nDstW, INT nDstH);
	// [DX9][T26] 诊断：dump 软件 backbuffer(565) �?24bit BMP
	void					DebugDumpBackBuffer(const char* szPath);
	// 6. ����(0��)�� ������ WORD�� ����Ÿ �׸��� �Լ�.(�鼭�ǽ��� Ŭ���� ���ο������� �׸���.)
	BOOL					DrawWithImageForCompClipRgnBase(INT nX, INT nY, INT nXSize, INT nYSize, WORD* pwSrc, WORD wClipWidth, WORD wClipHeight, WORD wChooseColor1 = 0XFFFF, WORD wChooseColor2 = 0XFFFF);
	BOOL					DrawWithImageForCompClipRgnBaseNoColor(INT nX, INT nY, INT nXSize, INT nYSize, WORD* pwSrc, WORD wClipWidth, WORD wClipHeight);
	__inline VOID			DrawWithImageForCompClipRgn(INT nX, INT nY, INT nXSize, INT nYSize, WORD* pwSrc, WORD wClipWidth, WORD wClipHeight, WORD wChooseColor1 = 0XFFFF, WORD wChooseColor2 = 0XFFFF, BOOL bLighted = FALSE)
	{
		if ( bLighted )		DrawWithABlendCompDataWithLightedColor(nX, nY, nXSize, nYSize, pwSrc, wClipWidth, wClipHeight, wChooseColor1, wChooseColor2);
		else				DrawWithImageForCompClipRgnBase(nX, nY, nXSize, nYSize, pwSrc, wClipWidth, wClipHeight, wChooseColor1, wChooseColor2);
	}
	// 7. ����(0��)�� ������ WORD�� ����Ÿ �׸��� �Լ�.(�ҽ��޸��� �������� ����Ʈ�����޸𸮷� �׸���.)
	BOOL					DrawWithImageForCompMemToMem(INT nX, INT nY, INT nXSize, INT nYSize, WORD* pwSrc, INT nDstXSize, INT nDstYSize, WORD* pwDst, WORD wChooseColor1 = 0XFFFF, WORD wChooseColor2 = 0XFFFF);
	// 8. ����(0��)�� ������ WORD�� ����Ÿ �׸��� �Լ�.(�����?����Ÿ�� �ҽ��޸��� �������� �Ϻη�Ʈ�������� ����Ʈ�����޸𸮷� �׸���.)
	BOOL					DrawWithImageForCompToMem(RECT rcWanted, WORD* pwSrc, WORD wChooseColor1, WORD wChooseColor2, WORD* pwDst);
	BOOL					DrawWithImageForCompColorToMem(RECT rcWanted, WORD* pwSrc, WORD wColor, WORD* pwDst);
	// 9. ����(0��)�� ������ WORD�� ����Ÿ �׸��� �Լ�.(�����?����Ÿ�� �ҽ������� ����Ʈ������ ������Ʈ�� ���ĺ������ؼ� �鼭�ǽ����׸���.)
	BOOL					DrawWithABlendForIntersectCompData(INT nSrcX, INT nSrcY, 
															   INT nSrcXSize, INT nSrcYSize, WORD* pwSrc,
															   INT nDstX, INT nDstY,
															   INT nDstXSize, INT nDstYSize, WORD* pwDst, 
															   WORD wClipWidth, WORD wClipHeight,
															   BYTE bOpa = 50, BOOL bFocused = FALSE,
															   WORD wSrcChooseColor1 = 0XFFFF, WORD wSrcChooseColor2 = 0XFFFF,
															   WORD wDstChooseColor1 = 0XFFFF, WORD wDstChooseColor2 = 0XFFFF,
															   WORD wSrcColor = 0XFFFF, WORD wDstColor = 0XFFFF);
	// 10. ����(0��)�� ������ WORD�� ����Ÿ �׸��� �Լ�.(�����?����Ÿ�� �ҽ������� �鼭�ǽ��� ���ĺ������Ѵ�.)
	BOOL					DrawWithABlendCompDataWithBackBuffer(INT nX, INT nY, 
																 INT nXSize, INT nYSize, WORD* pwSrc,
																 WORD wClipWidth, WORD wClipHeight,
																 WORD wChooseColor1 = 0XFFFF, WORD wChooseColor2 = 0XFFFF, BYTE bOpa = 50);
	BOOL					DrawWithABlendCompDataWithBackBufferEx(INT nX, INT nY, 
																   INT nXSize, INT nYSize, WORD* pwSrc,
																   WORD wClipWidth, WORD wClipHeight,
																   WORD wChooseColor1 = 0XFFFF, WORD wChooseColor2 = 0XFFFF, BYTE bOpa = 14);
	// 11. ����(0��)�� ������ WORD�� ����Ÿ �׸��� �Լ�.(�����?����Ÿ�� �ҽ������� ���ϴ� �÷��� ������ ���·�(���? �鼭�ǽ��� �׸���.)
	BOOL					DrawWithABlendCompDataWithLightedColor(INT nX, INT nY, 
																   INT nXSize, INT nYSize, WORD* pwSrc,
																   WORD wClipWidth, WORD wClipHeight,
																   WORD wChooseColor1 = 0XFFFF, WORD wChooseColor2 = 0XFFFF);
	// �׸��� �׸��� �Լ�.
	// 12. ����(0��)�� ������ WORD�� ����Ÿ �׸��� �Լ�.(�����?����Ÿ�� �ҽ������� �׸��ڸ� �鼭�ǽ��� ���ĺ������Ѵ�. �׸��ڴ� ���̰� �������� �پ��� ���̴� YSize��ŭ �þ�� ���?�鼭�ǽ��� ������.)
	BOOL					DrawWithShadowABlend(INT nX, INT nY, 
												 INT nXSize, INT nYSize, WORD* pwSrc,
												 WORD wClipWidth, WORD wClipHeight, WORD* pwShadowClrSrc,
												 BOOL bBlend = FALSE, BYTE bShadowType = 48, BYTE bOpa = 14);
	VOID					DrawWithShadowABlend(INT nX, INT nY, INT nXSize, INT nYSize, 
		                                         INT nPX, INT nPY, WORD* pwSrc, WORD wClipWidth, WORD wClipHeight, WORD* pwShadowClrSrc,
												 BOOL bBlend = FALSE, BYTE bOpa = 14);
	// �÷�ó��.
	// 13.
	BOOL					DrawWithImageForCompClipRgnColor(INT nX, INT nY, INT nXSize, INT nYSize, WORD* pwSrc, WORD wClipWidth, WORD wClipHeight, WORD wColor, BOOL bFocused = FALSE, BOOL bBlend = FALSE);
	BOOL					DrawWithImageForCompClipRgnColor(INT nX, INT nY, INT nXSize, INT nYSize, WORD* pwSrc, WORD wClipWidth, WORD wClipHeight, WORD wColor, WORD wChooseColor1 = 0XFFFF, WORD wChooseColor2 = 0XFFFF);
	// 14.
	BOOL					DrawWithImageForCompClipRgnGray(INT nX, INT nY, INT nXSize, INT nYSize, WORD* pwSrc , WORD wClipWidth, WORD wClipHeight, WORD wChooseColor1 = 0XFFFF, WORD wChooseColor2 = 0XFFFF);
	// ���ó��?
	// 15.
	VOID					DrawWithGrayBackBuffer();

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	HRESULT					Present();
	HRESULT					RestoreSurfaces();

//4: Message Map
public:
	virtual LRESULT MainWndProcDXG(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT OnSize(WPARAM wParam, LPARAM lParam);
	virtual LRESULT OnMove(WPARAM wParam, LPARAM lParam);
	virtual LRESULT OnSysKeyDown(WPARAM wParam, LPARAM lParam);
	virtual LRESULT OnGetMinMaxInfo(WPARAM wParam, LPARAM lParam);


protected:

	__inline virtual LRESULT MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{ 
		if ( m_pxDefProcess )	return m_pxDefProcess->DefMainWndProc(hWnd, uMsg, wParam, lParam);
		else					return MainWndProcDXG(hWnd, uMsg, wParam, lParam); 
	}

	virtual LRESULT	OnDestroy();
	virtual LRESULT	OnSetCursor();
};

CWHDXGraphicWindow*		GetDXGWindowPtr();

#endif //_WINDHORN_DXGRAPHIC
