#include "stdafx.h"
#include "WHEngine.h"
#include "WHDXGraphic.h"

BOOL CWHSurface::ReleaseSurface()
{
	if ( m_pDDS )
	{
		if ( m_pDDS->m_pBits ) delete[] m_pDDS->m_pBits;
		delete m_pDDS;
		m_pDDS = NULL;
		return TRUE;
	}
	return FALSE;
}

BOOL CWHSurface::CreateOffScreenSurface(DWORD dwXSize, DWORD dwYSize)
{
	ReleaseSurface();
	m_pDDS = new CDX9SoftSurface();
	if ( !m_pDDS ) return FALSE;
	m_pDDS->m_dwWidth  = dwXSize;
	m_pDDS->m_dwHeight = dwYSize;
	m_pDDS->m_lPitch   = dwXSize * 2;
	m_pDDS->m_pBits    = new WORD[dwXSize * dwYSize];
	if ( !m_pDDS->m_pBits ) return FALSE;
	ZeroMemory(m_pDDS->m_pBits, dwXSize*dwYSize*sizeof(WORD));
	return TRUE;
}

BOOL CWHSurface::CopyImageToSurface(DWORD dwSrcWidth, DWORD dwSrcHeight, WORD* pwSrc)
{
	if ( !m_pDDS || !m_pDDS->m_pBits ) return FALSE;
	WORD* pwdDst = m_pDDS->m_pBits;
	CopyMemory(pwdDst, pwSrc, dwSrcWidth * dwSrcHeight * sizeof(WORD));
	return TRUE;
}

BOOL CWHSurface::CopyImageToSurfaceDx(DWORD dwSrcWidth, DWORD dwSrcHeight, DWORD dwDestWidth, WORD* pwSrc)
{
	if ( !m_pDDS || !m_pDDS->m_pBits ) return FALSE;
	WORD* pwdDst = m_pDDS->m_pBits;
	int i;
	for ( i=0; i<(int)dwSrcHeight; i++ ) {
		CopyMemory(pwdDst, pwSrc, dwSrcWidth * sizeof(WORD));
		pwdDst = pwdDst + dwDestWidth;
		pwSrc  = pwSrc + dwSrcWidth;
	}
	return TRUE;
}

BOOL CWHSurface::DDLoadBitmap(LPCSTR szBitmap, int dx, int dy)
{
    HBITMAP hbm;
    BITMAP  bm;
	ReleaseSurface();
    hbm = (HBITMAP) LoadImage(GetModuleHandle(NULL), szBitmap, IMAGE_BITMAP, dx, dy, LR_CREATEDIBSECTION);
    if ( hbm == NULL )
        hbm = (HBITMAP) LoadImage(NULL, szBitmap, IMAGE_BITMAP, dx, dy, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
    if ( hbm == NULL ) return FALSE;
    GetObject(hbm, sizeof(bm), &bm);
    if ( CreateOffScreenSurface(bm.bmWidth, bm.bmHeight) )
	{
        DDCopyBitmap(hbm, 0, 0, 0, 0);
        DeleteObject(hbm);
    }
    return TRUE;
}

HRESULT CWHSurface::DDCopyBitmap(HBITMAP hbm, int x, int y, int dx, int dy)
{
    HDC  hdcImage;
    HDC  hdc;
    BITMAP bm;
    DDSURFACEDESC2 ddsd;
    HRESULT hr;
    if ( hbm == NULL || m_pDDS == NULL ) return E_FAIL;
    hdcImage = CreateCompatibleDC(NULL);
    SelectObject(hdcImage, hbm);
    GetObject(hbm, sizeof(bm), &bm);
    dx = dx == 0 ? bm.bmWidth  : dx;
    dy = dy == 0 ? bm.bmHeight : dy;
    ZeroMemory(&ddsd, sizeof(ddsd)); ddsd.dwSize = sizeof(ddsd);
    m_pDDS->GetSurfaceDesc(&ddsd);
    if ( (hr = m_pDDS->GetDC(&hdc)) == DD_OK )
	{
        StretchBlt(hdc, 0, 0, ddsd.dwWidth, ddsd.dwHeight, hdcImage, x, y, dx, dy, SRCCOPY);
        m_pDDS->ReleaseDC(hdc);
    }
    DeleteDC(hdcImage);
    return hr;
}

HRESULT CWHSurface::Draw(CDX9SoftSurface* pddsDst, RECT* prcDst)
{
	if ( NULL == pddsDst || NULL == m_pDDS || NULL == m_pDDS->m_pBits )
		return E_POINTER;
	LONG dstX = prcDst ? prcDst->left : 0;
	LONG dstY = prcDst ? prcDst->top  : 0;
	DWORD dstW = prcDst ? (prcDst->right-prcDst->left) : m_pDDS->m_dwWidth;
	DWORD dstH = prcDst ? (prcDst->bottom-prcDst->top) : m_pDDS->m_dwHeight;
	DWORD copyW = dstW < m_pDDS->m_dwWidth ? dstW : m_pDDS->m_dwWidth;
	DWORD copyH = dstH < m_pDDS->m_dwHeight ? dstH : m_pDDS->m_dwHeight;
	for ( DWORD y=0; y<copyH; y++ ) {
		WORD* s = m_pDDS->m_pBits + (size_t)y*m_pDDS->m_dwWidth;
		WORD* d = pddsDst->m_pBits + (size_t)(dstY+y)*(pddsDst->m_lPitch>>1) + dstX;
		CopyMemory(d, s, copyW*sizeof(WORD));
	}
	return DD_OK;
}
