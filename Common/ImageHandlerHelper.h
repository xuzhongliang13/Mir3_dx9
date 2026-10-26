#pragma once


#include <indexmap.h>


// 해쉬 테이블 크기
#define MAX_HASHSIZE	1024
// 전체 오브젝트의 인덱스 리스트 검사 주기
#define TEXTURE_CHECKALLTIME	60000	
// 이 시간이 지나면 인덱스를 삭제한다.
#define TEXTURE_SAVETIME		300000


class CImageHandlerHelper
{
public:
	struct sTextureNode
	{
		char					szKey[16];
		WORD					nFileType, nFileIdx, nImgIdx;
		CWHWilImageData*		pImg;
		IDirect3DTexture9*	pSurface;
		DWORD					nLastTick;
	};
	
	CWHWilImageData	*			m_pListImage;
	DWORD						m_nLastTick;

	CIndexMap< sTextureNode >	m_listTexture;

public:
	CImageHandlerHelper( CWHWilImageData *pListImage );
	virtual ~CImageHandlerHelper();

	void CheckAllCachedTexture();
	void DelAllTexture();

	IDirect3DTexture9 * AddTexture( WORD nFileType, WORD nFileIdx, WORD nImgIdx );
	IDirect3DTexture9 * GetTexture( WORD nFileType, WORD nFileIdx, WORD nImgIdx );
	void DelTexture( WORD nFileType, WORD nFileIdx, WORD nImgIdx );	


	HRESULT RestoreImg(sTextureNode* pstTextrNode);
	HRESULT CopyDToSToS(DWORD dwWidth, DWORD dwHeight, WORD* pwSrc, IDirect3DTexture9* lpDst);
	HRESULT CopyDToS(DWORD dwWidth, DWORD dwHeight, WORD* pwSrc, IDirect3DTexture9* lpDst);


protected:
	static char * __cbTextureKey( sTextureNode *pNode );	
	sTextureNode * FindTexture( WORD nFileType, WORD nFileIdx, WORD nImgIdx );
};