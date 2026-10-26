#include "stdafx.h"



//---------------------------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------------------------
HRESULT CImageHandlerHelper::RestoreImg(sTextureNode* pstTextrNode)
{
    LPDIRECT3DDEVICE9 pd3dDevice = g_xMainWnd.Get3DDevice();
    if ( pstTextrNode == NULL || pd3dDevice == NULL )
        return E_FAIL;
    SAFE_RELEASE(pstTextrNode->pSurface);
    DWORD dwWidth=0,dwHeight=0; WORD* pwSrc=NULL;
    if ( pstTextrNode->pImg->NewSetIndex(pstTextrNode->nImgIdx) )
    {
        dwWidth  = pstTextrNode->pImg->m_lpstNewCurrWilImageInfo->shWidth;
        dwHeight = pstTextrNode->pImg->m_lpstNewCurrWilImageInfo->shHeight;
        pwSrc    = (WORD*)pstTextrNode->pImg->m_pbCurrImage;
    }
    else return E_FAIL;
    if ( FAILED(pd3dDevice->CreateTexture(dwWidth,dwHeight,1,0,D3DFMT_A8R8G8B8,D3DPOOL_MANAGED,&pstTextrNode->pSurface,NULL)) )
        return E_FAIL;
    if ( CopyDToS(dwWidth,dwHeight,pwSrc,pstTextrNode->pSurface)==E_FAIL )
    { SAFE_RELEASE(pstTextrNode->pSurface); return E_FAIL; }
    return S_OK;
}


HRESULT CImageHandlerHelper::CopyDToSToS(DWORD dwWidth, DWORD dwHeight, WORD* pwSrc, IDirect3DTexture9* lpddsDst)
{
    return CopyDToS(dwWidth, dwHeight, pwSrc, lpddsDst);
}


HRESULT CImageHandlerHelper::CopyDToS(DWORD dwWidth, DWORD dwHeight, WORD* pwSrc, IDirect3DTexture9* lpddsDst)
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
            else { lpddsDst->UnlockRect(0); return E_FAIL; }  // [#30-FIX] unknown RLE code
        }
        nWidthEnd++; nWidthStart=nWidthEnd; nCurrWidth=0;
    }
    lpddsDst->UnlockRect(0);
    return S_OK;
}
//---------------------------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------------------------






CImageHandlerHelper::CImageHandlerHelper( CWHWilImageData *pListImage )
{
	m_pListImage = pListImage;

	m_listTexture.InitHashTable( MAX_HASHSIZE, IHT_ROUNDUP );
	m_listTexture.SetGetKeyFunction( __cbTextureKey );
}


CImageHandlerHelper::~CImageHandlerHelper()
{
	m_listTexture.UninitHashTable();
	m_pListImage = NULL;
}


void CImageHandlerHelper::CheckAllCachedTexture()
{
	if ( GetTickCount() - m_nLastTick > TEXTURE_CHECKALLTIME )
	{
		CListNode< sTextureNode >	*pNode = m_listTexture.GetHead();
		CListNode< sTextureNode >	*pTemp;
		sTextureNode				*pObj = NULL;

		while ( pNode )
		{
			pObj = pNode->GetData();

			if ( GetTickCount() - pObj->nLastTick >= TEXTURE_SAVETIME )
			{
				pTemp = pNode->GetNext();

				m_listTexture.Remove( pObj );

				SAFE_RELEASE(pObj->pSurface);
				SAFE_DELETE(pObj);

				pNode = pTemp;
			}
			else
			{
				pNode = pNode->GetNext();
			}
		}

		m_nLastTick = GetTickCount();
	}
}


IDirect3DTexture9* CImageHandlerHelper::AddTexture( WORD nFileType, WORD nFileIdx, WORD nImgIdx )
{
	if ( !m_pListImage[ nFileIdx ].NewSetIndex( nImgIdx ) )
	{
		return NULL;
	}

	sTextureNode *pObj = new sTextureNode;

	sprintf( pObj->szKey, "%05d%05d%05d", nFileType, nFileIdx, nImgIdx );
	pObj->nFileType	= nFileType;
	pObj->nFileIdx	= nFileIdx;
	pObj->nImgIdx	= nImgIdx;
	pObj->pImg		= &m_pListImage[ nFileIdx ];
	pObj->nLastTick	= GetTickCount();
	pObj->pSurface	= NULL;
	if ( RestoreImg(pObj) == E_FAIL )
	{
		SAFE_DELETE(pObj);
		return NULL;
	}

	m_listTexture.Insert( pObj );
	return pObj->pSurface;
}


IDirect3DTexture9* CImageHandlerHelper::GetTexture( WORD nFileType, WORD nFileIdx, WORD nImgIdx )
{
	sTextureNode *pObj = FindTexture( nFileType, nFileIdx, nImgIdx );

	if ( !pObj )
	{
		return AddTexture( nFileType, nFileIdx, nImgIdx );
	}

	pObj->nLastTick = GetTickCount();

	return pObj->pSurface;
}


void CImageHandlerHelper::DelTexture( WORD nFileType, WORD nFileIdx, WORD nImgIdx )
{
	sTextureNode *pObj = FindTexture( nFileType, nFileIdx, nImgIdx );
	if ( pObj )
	{
		m_listTexture.Remove( pObj );
		SAFE_RELEASE(pObj->pSurface);
		SAFE_DELETE(pObj);
	}
}


void CImageHandlerHelper::DelAllTexture()
{
	CListNode< sTextureNode >	*pNode = m_listTexture.GetHead();
	CListNode< sTextureNode >	*pTemp;
	sTextureNode				*pObj = NULL;

	while ( pNode )
	{
		pObj = pNode->GetData();

		pTemp = pNode->GetNext();

		m_listTexture.Remove( pObj );
		SAFE_RELEASE(pObj->pSurface);
		SAFE_DELETE(pObj);

		pNode = pTemp;
	}
}


char * CImageHandlerHelper::__cbTextureKey( CImageHandlerHelper::sTextureNode *pNode )
{
	return pNode->szKey;
}


CImageHandlerHelper::sTextureNode * CImageHandlerHelper::FindTexture( WORD nFileType, WORD nFileIdx, WORD nImgIdx )
{
	char szKey[16];
	sprintf( szKey, "%05d%05d%05d", nFileType, nFileIdx, nImgIdx );

	return m_listTexture.SearchKey( szKey );
}