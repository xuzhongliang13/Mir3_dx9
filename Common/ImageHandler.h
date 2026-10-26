/******************************************************************************************************************
                                                                                                                   
	����:																											
																													
	�ۼ���:																											
	�ۼ���:																											
																													
	[����][������] : ���� ����																						
                                                                                                                   
*******************************************************************************************************************/



#ifndef _CIMAGEHANDLER_H
#define _CIMAGEHANDLER_H


#include "ImageHandlerHelper.h"


class CImageHandler
{
public:
	CImageHandler();
	~CImageHandler();

public:
	CWHWilImageData			m_xImageList[_MAX_IMAGE];
	CHAR					m_pszFileList[_MAX_IMAGE][MAX_PATH];
	CImageHandlerHelper		m_helper;	

	D3DVERTEX				m_avBillBoard[4];
public:
	VOID InitAllImage();
	VOID LoadAllImage(BOOL bIsMemMapped = TRUE, BOOL bComp = TRUE);
	VOID DeleteAllImage();

	IDirect3DTexture9* AddTextr(WORD wFileType, WORD wFileIdx, INT nImgIdx);
	IDirect3DTexture9* GetTextrImg(WORD wFileType, WORD wFileIdx, INT nImgIdx);
	VOID DelAllTextrFileList();
	
	HRESULT DrawBillBoard(LPDIRECT3DDEVICE9 lpDevice, D3DVECTOR* pvTrans, D3DVECTOR* pvScale, /*D3DVECTOR vRot, */D3DMATERIAL9* pmtrl, /*FLOAT fRotRad, */IDirect3DTexture9* lpddsTextr, BYTE bBlendType = _BLEND_NORMAL);

	// [FIX-R2] Software rasterisers for rotated / non axis aligned billboards.
	// Every frame ends with an opaque full screen blit of the 565 soft buffer, so
	// anything drawn with D3D is discarded - these write the soft buffer directly.
	HRESULT	DrawTexTriangleSoft(IDirect3DTexture9* lpddsTextr,
							   FLOAT x0, FLOAT y0, FLOAT u0, FLOAT v0,
							   FLOAT x1, FLOAT y1, FLOAT u1, FLOAT v1,
							   FLOAT x2, FLOAT y2, FLOAT u2, FLOAT v2,
							   BYTE bBlendType, BYTE matR, BYTE matG, BYTE matB);
	HRESULT	DrawTexQuadSoft(IDirect3DTexture9* lpddsTextr, FLOAT fCenterX, FLOAT fCenterY,
						   FLOAT fWidth, FLOAT fHeight, FLOAT fRotZ,
						   BYTE bBlendType, BYTE matR, BYTE matG, BYTE matB);
};



#endif // _CIMAGEHANDLER_H