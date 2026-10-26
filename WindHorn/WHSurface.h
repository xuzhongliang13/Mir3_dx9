#ifndef _WINDHORN_SURFACE
#define	_WINDHORN_SURFACE

class CDX9SoftSurface;

class CWHSurface
{
protected:
	CDX9SoftSurface*	m_pDDS;

public:
	__inline CWHSurface()	{ m_pDDS = NULL; }
	__inline ~CWHSurface()	{ ReleaseSurface(); }

public:
	BOOL		CreateOffScreenSurface(DWORD dwXSize, DWORD dwYSize);
	BOOL		CopyImageToSurface(DWORD dwSrcWidth, DWORD dwSrcHeight, WORD* pwSrc);
	BOOL		CopyImageToSurfaceDx(DWORD dwSrcWidth, DWORD dwSrcHeight, DWORD dwDestWidth, WORD* pwSrc);
	BOOL		ReleaseSurface();

	BOOL		DDLoadBitmap(LPCSTR szBitmap, int dx = 0, int dy = 0);
	HRESULT     DDCopyBitmap(HBITMAP hbm, int x, int y, int dx, int dy);

	HRESULT		Draw(CDX9SoftSurface* pddsDst, RECT* prcDst);

	__inline CDX9SoftSurface* GetSur()	{ return m_pDDS; }
	__inline BOOL CopyImageToSurface(CWHImage* xImage) { return CopyImageToSurface(xImage->m_wWidth, xImage->m_wHeight, xImage->m_pwImage); }
};



#endif //_WINDHORN_SURFACE
