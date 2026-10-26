/******************************************************************************************************************
                                                                                                                   
	����:																											
																													
	�ۼ���:																											
	�ۼ���:																											
																													
	[����][������] : ���� ����																						
                                                                                                                   
*******************************************************************************************************************/
#include "StdAfx.h"

#ifndef min
#define min(a,b) (((a)<(b))?(a):(b))
#endif


struct WILTextureContainer
{
    WILTextureContainer* m_pNext;

    TCHAR   m_strWilFileName[80];
	INT		m_nIdx;
	WORD*	m_pwData;				// Surface�� �ε���ų���� ���� ���ǵȴ�. �׿ܿ� �׻� NULL�̾��̾߸� �Ѵ�.
    DWORD   m_dwFlags;
    DWORD   m_dwWidth;
    DWORD   m_dwHeight;
    IDirect3DTexture9* m_pd3dTexture;

public:
    HRESULT Restore(LPDIRECT3DDEVICE9 pd3dDevice);
    HRESULT CopyDataToSurface();
	HRESULT CopyDataToSurfaceToSurface();

    WILTextureContainer(TCHAR* strWilFileName, INT nIndex, DWORD dwFlags);
    ~WILTextureContainer();
};
static WILTextureContainer* g_ptcWILTextureList = NULL;


class CD3DWILTextureManager
{
public:
    CD3DWILTextureManager () {}
    ~CD3DWILTextureManager() { if ( g_ptcWILTextureList ) delete g_ptcWILTextureList; }
};


struct TEXTURESEARCHINFO
{
    DWORD dwDesiredBPP;
    BOOL  bUseAlpha;
    BOOL  bUsePalette;
    BOOL  bFoundGoodFormat;

    DDPIXELFORMAT* pddpf;
};


static HRESULT CALLBACK TextureSearchCallback(DDPIXELFORMAT* pddpf, VOID* param)
{
    if ( NULL==pddpf || NULL==param )
        return DDENUMRET_OK;

    TEXTURESEARCHINFO* ptsi = (TEXTURESEARCHINFO*)param;

    if ( pddpf->dwFlags & (DDPF_LUMINANCE|DDPF_BUMPLUMINANCE|DDPF_BUMPDUDV) )
        return DDENUMRET_OK;

    if ( ptsi->bUsePalette )
    {
        if( !(pddpf->dwFlags & DDPF_PALETTEINDEXED8) )
            return DDENUMRET_OK;
        memcpy(ptsi->pddpf, pddpf, sizeof(DDPIXELFORMAT));
        ptsi->bFoundGoodFormat = TRUE;
        return DDENUMRET_CANCEL;
    }

    if ( pddpf->dwRGBBitCount < 16 )
        return DDENUMRET_OK;

    if ( pddpf->dwFourCC != 0 )
        return DDENUMRET_OK;

    if( pddpf->dwRGBAlphaBitMask == 0x0000f000 )
        return DDENUMRET_OK;

    if ( (ptsi->bUseAlpha==TRUE) && !(pddpf->dwFlags&DDPF_ALPHAPIXELS) )
        return DDENUMRET_OK;
    if ( (ptsi->bUseAlpha==FALSE) && (pddpf->dwFlags&DDPF_ALPHAPIXELS) )
        return DDENUMRET_OK;

    if ( pddpf->dwRGBBitCount == ptsi->dwDesiredBPP )
    {
        memcpy(ptsi->pddpf, pddpf, sizeof(DDPIXELFORMAT));
        ptsi->bFoundGoodFormat = TRUE;
        return DDENUMRET_CANCEL;
    }

    return DDENUMRET_OK;
}


static WILTextureContainer* FindWilTexture(TCHAR* strWilFileName, INT nIdx)
{
    WILTextureContainer* ptcWILTexture = g_ptcWILTextureList;

    while ( ptcWILTexture )
    {
        if ( !lstrcmpi(strWilFileName, ptcWILTexture->m_strWilFileName) &&
			 nIdx == ptcWILTexture->m_nIdx )
		{
            return ptcWILTexture;
		}
        ptcWILTexture = ptcWILTexture->m_pNext;
    }

    return NULL;
}













WILTextureContainer::WILTextureContainer(TCHAR* strWilFileName, INT nIndex, DWORD dwFlags)
{
    lstrcpy(m_strWilFileName, strWilFileName);
	m_nIdx				= nIndex;
    m_dwWidth			= 0;
    m_dwHeight			= 0;
    m_dwFlags			= dwFlags;
    m_pd3dTexture		= NULL;
	m_pwData			= NULL;
    m_pNext				= g_ptcWILTextureList;
    g_ptcWILTextureList	= this;
}


WILTextureContainer::~WILTextureContainer()
{
    SAFE_RELEASE( m_pd3dTexture );

    if ( g_ptcWILTextureList == this )
        g_ptcWILTextureList = m_pNext;
    else
    {
        for ( WILTextureContainer* ptc = g_ptcWILTextureList; ptc; ptc=ptc->m_pNext )
            if ( ptc->m_pNext == this )
                ptc->m_pNext = m_pNext;
    }
}


HRESULT WILTextureContainer::Restore(LPDIRECT3DDEVICE9 pd3dDevice)
{
    SAFE_RELEASE(m_pd3dTexture);
    if( NULL == pd3dDevice )
        return DDERR_INVALIDPARAMS;

    D3DCAPS9 caps;
    pd3dDevice->GetDeviceCaps(&caps);

    UINT w = m_dwWidth, h = m_dwHeight;
    if ( caps.TextureCaps & D3DPTEXTURECAPS_POW2 )
    {
        for ( w=1; m_dwWidth>w;   w<<=1 );
        for ( h=1; m_dwHeight>h;  h<<=1 );
    }
    w = min(w, (caps.MaxTextureWidth  ? caps.MaxTextureWidth  : 256));
    h = min(h, (caps.MaxTextureHeight ? caps.MaxTextureHeight : 256));

    HRESULT hr = pd3dDevice->CreateTexture(w, h, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &m_pd3dTexture, NULL);
    if( FAILED(hr) )
        return hr;

    // 解包 wil 数据到纹理(565 -> A8R8G8B8, 黑色作透明)
    CopyDataToSurface();

    return S_OK;
}


HRESULT WILTextureContainer::CopyDataToSurfaceToSurface()
{
    return CopyDataToSurface();
}


HRESULT WILTextureContainer::CopyDataToSurface()
{
	if(!m_pd3dTexture) return E_FAIL;
	D3DLOCKED_RECT lr;
	if(FAILED(m_pd3dTexture->LockRect(0,&lr,NULL,0))) return E_FAIL;

	RECT rc; SetRect(&rc, 0, 0, m_dwWidth, m_dwHeight);

	// 清为透明
	DWORD* pClear=(DWORD*)lr.pBits;
	for(LONG y=0;y<(LONG)m_dwHeight;y++){
		DWORD* row=(DWORD*)((BYTE*)lr.pBits+(size_t)y*lr.Pitch);
		for(LONG x=0;x<(LONG)m_dwWidth;x++) row[x]=0;
	}

	// 解包: 0xC0=跳过N像素, 0xC1=N个像素数据, 其他=transparent
	INT nWidthStart=0, nWidthEnd=0, nCurrWidth=0, nCntCopyWord=0, nYCnt=0, nLastWidth=0;
	for ( nYCnt=0 ; nYCnt < rc.top ; nYCnt++ ) { nWidthEnd += m_pwData[nWidthStart]; nWidthStart++; nWidthEnd++; nWidthStart=nWidthEnd; }
	for ( nYCnt=rc.top ; nYCnt < rc.bottom ; nYCnt++ )
	{
		nWidthEnd += m_pwData[nWidthStart]; nWidthStart++;
		for ( INT x = nWidthStart; x < nWidthEnd ; )
		{
			if ( m_pwData[x] == 0xC0 ){ x++; nCntCopyWord=m_pwData[x]; x++; nCurrWidth+=nCntCopyWord; }
			else if ( m_pwData[x] == 0xC1 )
			{
				x++; nCntCopyWord=m_pwData[x]; x++;
				nLastWidth=nCurrWidth; nCurrWidth+=nCntCopyWord;
				DWORD* row=(DWORD*)((BYTE*)lr.pBits+(size_t)nYCnt*lr.Pitch);
				for(INT k=0;k<nCntCopyWord;k++){
					INT sx=nLastWidth+k; if(sx>=rc.right) break;
					WORD px=m_pwData[x+k];
					DWORD c=0;
					if(px!=0x0000){
						WORD r=(px>>11)&31,g=(px>>5)&63,b=px&31;
						BYTE r8=(BYTE)((r<<3)|(r>>2)), g8=(BYTE)((g<<2)|(g>>4)), b8=(BYTE)((b<<3)|(b>>2));
						c=0xFF000000|(r8<<16)|(g8<<8)|b8;
					}
					row[sx]=c;
				}
				x+=nCntCopyWord;
			}
			else { m_pd3dTexture->UnlockRect(0); return E_FAIL; }
		}
		nWidthEnd++; nWidthStart=nWidthEnd; nCurrWidth=0;
	}
	m_pd3dTexture->UnlockRect(0);
	return S_OK;
}















BOOL D3DWILTextr_CreateEmptyTexture(TCHAR* strWilFileName, INT nIndex, DWORD dwWidth, DWORD dwHeight, WORD* pwData, DWORD dwFlags)
{
    if ( NULL == strWilFileName )
	{
        return FALSE;
	}

    if ( NULL != FindWilTexture(strWilFileName, nIndex) )
	{
        return FALSE;
	}

    WILTextureContainer* ptcWILTexture = new WILTextureContainer(strWilFileName, nIndex, dwFlags);

    if ( NULL == ptcWILTexture )
        return FALSE;

    ptcWILTexture->m_dwWidth  = dwWidth;
    ptcWILTexture->m_dwHeight = dwHeight;
	ptcWILTexture->m_pwData	  = pwData;

    return TRUE;
}


HRESULT D3DWILTextr_Restore(TCHAR* strWilFileName, INT nIndex, LPDIRECT3DDEVICE9 pd3dDevice)
{
    WILTextureContainer* ptcWILTexture = FindWilTexture(strWilFileName, nIndex);
    if ( NULL == ptcWILTexture )
        return DDERR_NOTFOUND;

    return ptcWILTexture->Restore(pd3dDevice);
}


IDirect3DTexture9* D3DWILTextr_RestoreEx(TCHAR* strWilFileName, INT nIndex, LPDIRECT3DDEVICE9 pd3dDevice)
{
    WILTextureContainer* ptcWILTexture = FindWilTexture(strWilFileName, nIndex);

	if ( ptcWILTexture )
	{
		if ( ptcWILTexture->m_pd3dTexture )
		{
			return ptcWILTexture->m_pd3dTexture;
		}
		else
		{
			if ( (ptcWILTexture->Restore(pd3dDevice) == S_OK) )
			{
				return ptcWILTexture->m_pd3dTexture;
			}
			else
				return NULL;
		}
	}

	return NULL;
		
}


HRESULT D3DWILTextr_RestoreAllTextures(LPDIRECT3DDEVICE9 pd3dDevice)
{
    WILTextureContainer* ptcWILTexture = g_ptcWILTextureList;

    while ( ptcWILTexture && ptcWILTexture->m_pd3dTexture )
    {
        D3DWILTextr_Restore(ptcWILTexture->m_strWilFileName, ptcWILTexture->m_nIdx, pd3dDevice);
        ptcWILTexture = ptcWILTexture->m_pNext;
    }

    return S_OK;
}


HRESULT D3DWILTextr_Invalidate(TCHAR* strWilFileName, INT nIndex)
{
    WILTextureContainer* ptcWILTexture = FindWilTexture(strWilFileName, nIndex);
    if ( NULL == ptcWILTexture )
        return DDERR_NOTFOUND;

    SAFE_RELEASE(ptcWILTexture->m_pd3dTexture);

    return S_OK;
}


HRESULT D3DWILTextr_InvalidateAllTextures()
{
    WILTextureContainer* ptcWILTexture = g_ptcWILTextureList;

    while( ptcWILTexture )
    {
        SAFE_RELEASE(ptcWILTexture->m_pd3dTexture);
        ptcWILTexture = ptcWILTexture->m_pNext;
    }

    return S_OK;
}

HRESULT D3DWILTextr_DestroyAllTextures()
{
    WILTextureContainer* ptcWILTexture = g_ptcWILTextureList;
	WILTextureContainer* ptcWILMextTexture = NULL;

    while( ptcWILTexture )
    {
        ptcWILMextTexture = ptcWILTexture->m_pNext;

		SAFE_DELETE(ptcWILTexture);

		ptcWILTexture = ptcWILMextTexture;
		
    }
    return S_OK;
}

HRESULT D3DWILTextr_DestroyTexture(TCHAR* strWilFileName, INT nIndex)
{
    WILTextureContainer* ptcWILTexture = FindWilTexture(strWilFileName, nIndex);

    SAFE_DELETE(ptcWILTexture);

    return S_OK;
}


IDirect3DTexture9* D3DWILTextr_GetSurface(TCHAR* strWilFileName, INT nIndex)
{
    WILTextureContainer* ptcWILTexture = FindWilTexture(strWilFileName, nIndex);

    return ptcWILTexture ? ptcWILTexture->m_pd3dTexture : NULL;
}

