/******************************************************************************************************************
                                                                                                                   
	����:																											
																													
	�ۼ���:																											
	�ۼ���:																											
																													
	[����][������] : ���� ����																						
                                                                                                                   
*******************************************************************************************************************/

#ifndef D3DWILTEXTR_H
#define D3DWILTEXTR_H
#include <ddraw.h>
#include <d3d9.h>

#define D3DTEXTR_TRANSPARENTWHITE 0x00000001
#define D3DTEXTR_TRANSPARENTBLACK 0x00000002

IDirect3DTexture9* D3DWILTextr_GetSurface(TCHAR* strWilFileName, INT nIndex);
HRESULT D3DWILTextr_Invalidate(TCHAR* strWilFileName, INT nIndex);
HRESULT D3DWILTextr_Restore(TCHAR* strWilFileName, INT nIndex, LPDIRECT3DDEVICE9 pd3dDevice);
IDirect3DTexture9* D3DWILTextr_RestoreEx(TCHAR* strWilFileName, INT nIndex, LPDIRECT3DDEVICE9 pd3dDevice);
HRESULT D3DWILTextr_InvalidateAllTextures();
HRESULT D3DWILTextr_DestroyAllTextures();
HRESULT D3DWILTextr_RestoreAllTextures(LPDIRECT3DDEVICE9 pd3dDevice);
BOOL	D3DWILTextr_CreateEmptyTexture(TCHAR* strWilFileName, INT nIndex, 
									   DWORD dwWidth, DWORD dwHeight, WORD* pwData, DWORD dwFlags);
HRESULT D3DWILTextr_DestroyTexture(TCHAR* strWilFileName, INT nIndex);
VOID    D3DWILTextr_SetTexturePath(TCHAR* strTexturePath);
INT		GetRandomNum(INT nFrom, INT nTo);


// �ؽ���� ���� ��� �������� �����Ѵ�.
/*
    WILTextureContainer* m_pNext;

    TCHAR   m_strWilFileName[80];
	INT		m_nIdx;
    DWORD   m_dwFlags;
    DWORD   m_dwWidth;
    DWORD   m_dwHeight;
    LPDIRECTDRAWSURFACE7 m_pddsSurface;
*/
// [����]  ���� ������ �̿��Ͽ� ����Ÿ�� �����Ѵ�.(�����ϰ� ���� �����̿�)
//    ---> CMappedWilFileŬ�������� m_strWilFileName, m_nIdx, m_dwWidth, 
//		   m_dwHeight, �̹�������Ÿ�ּ� �� �Ѱ� �޾Ƽ� �ؽ���� ���ǽ��� �����.


#endif // D3DWILTEXTR_H
