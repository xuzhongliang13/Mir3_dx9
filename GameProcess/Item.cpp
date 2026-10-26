/******************************************************************************************************************
                                                                                                                   
	����:																											
																													
	�ۼ���:																											
	�ۼ���:																											
																													
	[����][������] : ���� ����																						
                                                                                                                   
*******************************************************************************************************************/


#include "StdAfx.h"



CItem::CItem()
{
	m_shCellNum	= 0;
	m_bAttr		= 0;
	m_nStateCnt = 0;
	ZeroMemory(m_stItemState, sizeof(ITEMSTATE)*_ITEM_MAX_STATE);	
}


CItem::~CItem()
{
}

BOOL CItem::SetItem(CLIENTITEMRCD stItemInfo, SHORT shInventoryNum)
{
	m_stItemInfo = stItemInfo;
	m_shCellNum = shInventoryNum;
/*
a:����
b:��
c:���� ���� �����
g:��Ÿ
*/
/*
#define _U_DRESS					0
#define _U_WEAPON					1
#define _U_RIGHTHAND				2
#define _U_NECKLACE					3
#define _U_HELMET					4
#define _U_ARMRINGL					5
#define _U_ARMRINGR					6            
#define _U_RINGL					7 
#define _U_RINGR					8 
#define _U_CHARM					9
#define _U_UNEQUIP					100
*/

	switch ( m_stItemInfo.stStdItem.bStdMode )
	{
	// ����.
	case 5:		
	case 6:		
	case 54:	// ǥâ.
	case 55:	// ������
	case 56:	// �����߰�.
		m_bAttr = _U_WEAPON;
		break;
	// ��.
	case 10:
	case 11:
		m_bAttr = _U_DRESS;
		break;
	case 15:
		m_bAttr = _U_HELMET;
		break;
	case 19:
	case 20:
	case 21:
	case 57:	// ������߰�.
		m_bAttr = _U_NECKLACE;
		break;
	case 22:
	case 23:
		m_bAttr = _U_RINGL;
		break;
	case 24:
	case 26:
		m_bAttr = _U_ARMRINGL;
		break;
	case 28:	// �������� - �����߰�
	// ��, ȶ��.
	case 30:
		m_bAttr = _U_RIGHTHAND;
		break;
	// ������, ����.
	case 25:
		m_bAttr = _U_CHARM;
		break;
	// �Ź�.
	case 53:
		m_bAttr = _U_BOOTS;
		break;
	case 60:
		m_bAttr = _U_ETC;
		break;
	}	

	return TRUE;
}


BOOL CItem::DrawItem(INT nX, INT nY, BYTE bItemType, BOOL bDrawCount)
{
	CWHWilImageData*	pxItemImg;	
	BOOL bDrawItem = FALSE;
	INT	nAmountX = nX;
	INT nAmountY = nY;

	switch ( bItemType )
	{
	case _ITEM_TYPE_STORE:
		pxItemImg = &g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_STORE];
		bDrawItem = pxItemImg->NewSetIndex(m_stItemInfo.stStdItem.wLooks);
		if ( bDrawItem )
		{
			nX = nX+_INVENTORY_CELL_WIDTH/2-pxItemImg->m_lpstNewCurrWilImageInfo->shWidth/2;
			nY = nY+_INVENTORY_CELL_HEIGHT/2-pxItemImg->m_lpstNewCurrWilImageInfo->shHeight/2;
		}
		break;
	case _ITEM_TYPE_INVENTORY:
		pxItemImg = &g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY];
		bDrawItem = pxItemImg->NewSetIndex(m_stItemInfo.stStdItem.wLooks);
		if ( bDrawItem )
		{
			nX = nX+_INVENTORY_CELL_WIDTH/2-pxItemImg->m_lpstNewCurrWilImageInfo->shWidth/2;
			nY = nY+_INVENTORY_CELL_HEIGHT/2-pxItemImg->m_lpstNewCurrWilImageInfo->shHeight/2;
		}
		break;
	case _ITEM_TYPE_EQUIP:
		pxItemImg = &g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_EQUIP];
		bDrawItem = pxItemImg->NewSetIndex(m_stItemInfo.stStdItem.wLooks);
		if ( bDrawItem )
		{
			nX += g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_EQUIP].m_lpstNewCurrWilImageInfo->shPX;
			nY += g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_EQUIP].m_lpstNewCurrWilImageInfo->shPY;
		}
		break;
	}
		
	if ( bDrawItem )
	{

		WORD wClr = 0XFFFF;
		
		if ( m_stItemInfo.stStdItem.bStdMode == 10 || m_stItemInfo.stStdItem.bStdMode == 11 /*|| m_stItemInfo.stStdItem.bStdMode == 15 */)
		{
			wClr = g_xMainWnd.ConvertColor24To16(RGB(m_stItemInfo.bLevel_R, 
													 LOBYTE(m_stItemInfo.wAmount_GB), 
													 HIBYTE(m_stItemInfo.wAmount_GB)));
		}

		RECT rcTemp;
		SetRect(&rcTemp, nX, nY, nX + pxItemImg->m_lpstNewCurrWilImageInfo->shWidth, nY+pxItemImg->m_lpstNewCurrWilImageInfo->shHeight);

 		g_xMainWnd.DrawWithImageForCompClipRgn(
						nX,
						nY,
						pxItemImg->m_lpstNewCurrWilImageInfo->shWidth,
						pxItemImg->m_lpstNewCurrWilImageInfo->shHeight,
						(WORD*)pxItemImg->m_pbCurrImage, _CLIP_WIDTH, 600, wClr);

		if (bDrawCount == TRUE)
		{
			if (m_stItemInfo.stStdItem.bThrow >= _MULTIPLE_TYPE_ITEM)
			{
				RECT rc;
				CHAR pszTmp[MAX_PATH];

				DWORD	dwCount = m_stItemInfo.wAmount_GB;

				INT		nCnt = 0;
				nCnt = log10(dwCount);

				rc.left = nAmountX + 25 - (nCnt * 5);
				rc.top = nAmountY + 23;


				sprintf(pszTmp, "%d", dwCount);
				g_xMainWnd.PutsHan(NULL, rc.left, rc.top, RGB(255, 255, 0), RGB(0,0,0), pszTmp);
			}	
		}
	}
	return TRUE;
}

BOOL CItem::DrawItemUseColorIdx(INT nX, INT nY, INT nColorIdx, BYTE bItemType, BOOL bDrawCount)
{
	CWHWilImageData*	pxItemImg;	
	BOOL bDrawItem = FALSE;
	INT	nAmountX = nX;
	INT nAmountY = nY;

	switch ( bItemType )
	{
	case _ITEM_TYPE_STORE:
		pxItemImg = &g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_STORE];
		bDrawItem = pxItemImg->NewSetIndex(m_stItemInfo.stStdItem.wLooks);
		if ( bDrawItem )
		{
			nX = nX+_INVENTORY_CELL_WIDTH/2-pxItemImg->m_lpstNewCurrWilImageInfo->shWidth/2;
			nY = nY+_INVENTORY_CELL_HEIGHT/2-pxItemImg->m_lpstNewCurrWilImageInfo->shHeight/2;
		}
		break;
	case _ITEM_TYPE_INVENTORY:
		pxItemImg = &g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY];
		bDrawItem = pxItemImg->NewSetIndex(m_stItemInfo.stStdItem.wLooks);
		if ( bDrawItem )
		{
			nX = nX+_INVENTORY_CELL_WIDTH/2-pxItemImg->m_lpstNewCurrWilImageInfo->shWidth/2;
			nY = nY+_INVENTORY_CELL_HEIGHT/2-pxItemImg->m_lpstNewCurrWilImageInfo->shHeight/2;
		}
		break;
	case _ITEM_TYPE_EQUIP:
		pxItemImg = &g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_EQUIP];
		bDrawItem = pxItemImg->NewSetIndex(m_stItemInfo.stStdItem.wLooks);
		if ( bDrawItem )
		{
			nX += g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_EQUIP].m_lpstNewCurrWilImageInfo->shPX;
			nY += g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_EQUIP].m_lpstNewCurrWilImageInfo->shPY;
		}
		break;
	}
		
	if ( bDrawItem )
	{
		WORD wClr = 0XFFFF;
		
		switch ( nColorIdx )
		{
			case 0:
				wClr = RGB(255, 255, 255);
				break;
			case 1:
				wClr = 49345;
				break;
			case 2:
				wClr = 47039;
				break;
			case 3:
				wClr = 15709;
				break;
			case 4:
				wClr = 20382;
				break;
			case 5:
				wClr = 50900;
				break;
			case 6:
				wClr = 0;
				break;
			case 7:
				wClr = 56027;
				break;
		}

		RECT rcTemp;
		SetRect(&rcTemp, nX, nY, nX + pxItemImg->m_lpstNewCurrWilImageInfo->shWidth, nY+pxItemImg->m_lpstNewCurrWilImageInfo->shHeight);

 		g_xMainWnd.DrawWithImageForCompClipRgn(
						nX,
						nY,
						pxItemImg->m_lpstNewCurrWilImageInfo->shWidth,
						pxItemImg->m_lpstNewCurrWilImageInfo->shHeight,
						(WORD*)pxItemImg->m_pbCurrImage, _CLIP_WIDTH, 600, wClr);

		if (bDrawCount == TRUE)
		{
			if (m_stItemInfo.stStdItem.bThrow >= _MULTIPLE_TYPE_ITEM)
			{
				RECT rc;
				CHAR pszTmp[MAX_PATH];

				DWORD	dwCount = m_stItemInfo.wAmount_GB;

				INT		nCnt = 0;
				nCnt = log10(dwCount);

				rc.left = nAmountX + 25 - (nCnt * 5);
				rc.top = nAmountY + 23;


				sprintf(pszTmp, "%d", dwCount);
				g_xMainWnd.PutsHan(NULL, rc.left, rc.top, RGB(255, 255, 0), RGB(0,0,0), pszTmp);
			}	
		}
	}
	return TRUE;
}

VOID CItem::SetItemStatus(BOOL bUseStore, BOOL bUseEquip)
{
	m_nStateCnt = 0;
	ZeroMemory(m_stItemState, sizeof(ITEMSTATE)*_ITEM_MAX_STATE);	

	if ( m_stItemInfo.stStdItem.szName )
	{
		switch ( m_stItemInfo.stStdItem.bStdMode )
		{
			// ���ڿ�, ���ڿ�.
			case 10:
			case 11:
			{
				SetClothStatus(bUseStore, bUseEquip);
				break;
			}
			// ����.
			case 5:		
			case 6:	
			case 54:
			case 55:	//������
			case 56:	// �����߰�.
			{
				SetWeaponStatus(bUseStore, bUseEquip);
				break;
			}
			case 15:	//����,����
			case 19:	//�����
			case 57:	// ������߰�.
			case 20:
			case 21:  
			case 22:	//����
			case 23:  
			case 24:	//����
			case 26:  
			case 28:	// �������� - �����߰�
			{
				SetAccessoryStatus(bUseStore, bUseEquip);
				break;
			}
			case 53:	//�Ź�
			{
				SetBootsStatus(bUseStore, bUseEquip);
				break;
			}
			default:
			{
				SetEtcStatus(bUseStore, bUseEquip);
				break;
			}
		}

		// ��������, �����ۻ��� �߰� �����ۿ� ������ �Ⱓǥ��
		// ������ ���� �߰�
		SetItemDesc( bUseStore );
	}
}

VOID CItem::SetClothStatus(BOOL bUseStore, BOOL bUseEquip)
{
	INT	 nCnt; 
	BOOL bUseAble  = FALSE;

	// 1. �̸�.
	if ( strlen(m_stItemInfo.Prefix) )
	{		
		sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s%s %s", g_xMsg.GetMsg(2515), m_stItemInfo.Prefix, m_stItemInfo.stStdItem.szName);
	}
	else
	{
		sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s%s", g_xMsg.GetMsg(2515), m_stItemInfo.stStdItem.szName);
	}
	m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
	m_stItemState[m_nStateCnt].dwFont = RGB(255, 250,   0);
	m_nStateCnt++;

	// 2. ����.
	INT nDura	 = RoundFunc(m_stItemInfo.nDura, 1000);
	INT nDuraMax = RoundFunc(m_stItemInfo.nDuraMax, 1000);
	if ( bUseStore )
	{
		nDura	 = RoundFunc(m_stItemInfo.nDura, 1000);
		nDuraMax = RoundFunc(m_stItemInfo.stStdItem.wDuraMax, 1000);	
	}
	sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d/%d", g_xMsg.GetMsg(2500), nDura, nDuraMax);
	m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
	if ( nDura <= 1 ) 
	{
		m_stItemState[m_nStateCnt].dwFont = RGB(255,  50,  50);
	}
	else
	{
		m_stItemState[m_nStateCnt].dwFont = RGB(128, 255, 255);
	}
	m_nStateCnt++;

	// 3. ����.
	sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d", g_xMsg.GetMsg(2501), m_stItemInfo.stStdItem.bWeight);
	m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
	m_stItemState[m_nStateCnt].dwFont = RGB(255, 255, 255);
	m_nStateCnt++;

	// �ı�.
	if ( m_stItemInfo.stStdItem.wDC + m_stItemInfo.Desc[2] > 0 )
	{
		m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
		if ( m_stItemInfo.Desc[2] > 0 )
		{
			// �ɼ� Plus.
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d-%d (+%d)", g_xMsg.GetMsg(1061), LOBYTE(m_stItemInfo.stStdItem.wDC), HIBYTE(m_stItemInfo.stStdItem.wDC)+m_stItemInfo.Desc[2], m_stItemInfo.Desc[2]);
			m_stItemState[m_nStateCnt].dwFont = RGB(150, 255, 200);
		}
		else
		{
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d-%d ", g_xMsg.GetMsg(1061), LOBYTE(m_stItemInfo.stStdItem.wDC), HIBYTE(m_stItemInfo.stStdItem.wDC));
			m_stItemState[m_nStateCnt].dwFont = RGB(255, 255, 255);
		}
		m_nStateCnt++;

		if (bUseEquip)
			g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[0] = TRUE;
	}

	// 4. ���.
	m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
	if ( m_stItemInfo.Desc[0] > 0 )
	{
		// �ɼ� Plus.
		sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d-%d (+%d)", g_xMsg.GetMsg(5177), LOBYTE(m_stItemInfo.stStdItem.wAC), HIBYTE(m_stItemInfo.stStdItem.wAC)+m_stItemInfo.Desc[0], m_stItemInfo.Desc[0]);
		m_stItemState[m_nStateCnt].dwFont = RGB(150, 255, 200);
	}
	else
	{
		sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d-%d", g_xMsg.GetMsg(5177), LOBYTE(m_stItemInfo.stStdItem.wAC), HIBYTE(m_stItemInfo.stStdItem.wAC));
		m_stItemState[m_nStateCnt].dwFont = RGB(255, 255, 255);
	}
	m_nStateCnt++;

	if (bUseEquip)
		g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[1] = TRUE;

	// �������
	if ( m_stItemInfo.stStdItem.wMAC + m_stItemInfo.Desc[1] > 0 )
	{
		if ( m_stItemInfo.Desc[1] > 0)
		{
			// �ɼ� Plus.
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d-%d (+%d)", g_xMsg.GetMsg(1406), LOBYTE(m_stItemInfo.stStdItem.wMAC), HIBYTE(m_stItemInfo.stStdItem.wMAC)+m_stItemInfo.Desc[1], m_stItemInfo.Desc[1]);
			m_stItemState[m_nStateCnt].dwFont = RGB(150, 255, 200);
		}
		else
		{
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d-%d", g_xMsg.GetMsg(1406), LOBYTE(m_stItemInfo.stStdItem.wMAC), HIBYTE(m_stItemInfo.stStdItem.wMAC));
			m_stItemState[m_nStateCnt].dwFont = RGB(255, 255, 255);
		}
		m_nStateCnt++;
		if (bUseEquip)
			g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[4] = TRUE;
	}

	// ����
	if ( m_stItemInfo.stStdItem.wMC + m_stItemInfo.Desc[3] > 0 )
	{
		m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
		if ( m_stItemInfo.Desc[3] > 0 )
		{
			// �ɼ� Plus.
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d-%d (+%d)", g_xMsg.GetMsg(5100+m_stItemInfo.stStdItem.MCType), LOBYTE(m_stItemInfo.stStdItem.wMC), HIBYTE(m_stItemInfo.stStdItem.wMC)+m_stItemInfo.Desc[3], m_stItemInfo.Desc[3]);
			m_stItemState[m_nStateCnt].dwFont = RGB(150, 255, 200);
		}
		else
		{
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d-%d ", g_xMsg.GetMsg(5100+m_stItemInfo.stStdItem.MCType), LOBYTE(m_stItemInfo.stStdItem.wMC), HIBYTE(m_stItemInfo.stStdItem.wMC));
			m_stItemState[m_nStateCnt].dwFont = RGB(255, 255, 255);
		}
		m_nStateCnt++;


		if (bUseEquip)
		{
			if (m_stItemInfo.stStdItem.MCType == 0) //���迭
			{
				g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[2] = TRUE;
				g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[3] = TRUE;
			}
			else if (m_stItemInfo.stStdItem.MCType == 1) //�ڿ��迭
			{
				g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[2] = TRUE;
			}
			else // ��ȥ��
			{
				g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[3] = TRUE;
			}
		}
	}

	if ( m_stItemInfo.Desc[8] > 0 )
	{
		m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
		// �ɼ� Plus.
		sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: +%d", g_xMsg.GetMsg(2518), m_stItemInfo.Desc[8] * 10);
		m_stItemState[m_nStateCnt].dwFont = RGB(150, 255, 200);
		m_nStateCnt++;
	}
	
	if ( m_stItemInfo.Desc[9] > 0 )
	{
		m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
		// �ɼ� Plus.
		sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: +%d", g_xMsg.GetMsg(2519), m_stItemInfo.Desc[9] * 10);
		m_stItemState[m_nStateCnt].dwFont = RGB(150, 255, 200);
		m_nStateCnt++;
	}

	// ���� �ı�
	if (( m_stItemInfo.Desc[11] > 0 ) || (m_stItemInfo.stStdItem.bAtomDCType> 0))
	{
		CHAR szTemp[MAX_PATH], szTemp2[MAX_PATH];

		sprintf(szTemp2, "%s:", g_xMsg.GetMsg(5178));
		m_stItemState[m_nStateCnt].dwFont = RGB(128, 255, 0);

		byte btAtomType = m_stItemInfo.stStdItem.bAtomDCType;

		if ( btAtomType != m_stItemInfo.Desc[11] && btAtomType != 0 )
		{
			CHAR szTemp3[MAX_PATH];
			sprintf(szTemp3, " %s+%d", g_xMsg.GetMsg(btAtomType+5160), m_stItemInfo.stStdItem.wAtomDC);
			strcat(szTemp2, szTemp3);
		}

		if (m_stItemInfo.Desc[11] > 0)
			btAtomType = m_stItemInfo.Desc[11];

		if ( m_stItemInfo.Desc[12] > 0 )
		{
			if ( btAtomType == m_stItemInfo.stStdItem.bAtomDCType )
				sprintf(szTemp, " %s+%d", g_xMsg.GetMsg(btAtomType+5160), m_stItemInfo.stStdItem.wAtomDC+m_stItemInfo.Desc[12]);
			else
				sprintf(szTemp, " %s+%d", g_xMsg.GetMsg(btAtomType+5160), m_stItemInfo.Desc[12]);

			strcat(szTemp2, szTemp);
		}

		m_stItemState[m_nStateCnt].dwFont = RGB(128, 255, 0);
		sprintf(m_stItemState[m_nStateCnt].pszDesc, szTemp2);

		if (bUseEquip)
			g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[btAtomType+4] = TRUE;

		m_nStateCnt++;
	}

	// ���ѿ���
	if ( m_stItemInfo.Desc[5] > 0 || m_stItemInfo.stStdItem.sAtomACGood )
	{
		INT nMask = 1;
		CHAR szTemp[MAX_PATH], szTemp2[MAX_PATH];
		BOOL bFind = FALSE;

		sprintf(szTemp2, "%s:", g_xMsg.GetMsg(5179));

		for ( INT nI=0; nI<7; nI++)
		{
			BOOL bCheck = FALSE;
			if ( m_stItemInfo.Desc[5] & nMask )
			{
				bCheck = TRUE;
				bFind = TRUE;
				if ( m_stItemInfo.stStdItem.sAtomACGood == nI+1 )
					sprintf(szTemp, " %sx3", g_xMsg.GetMsg(5161+nI));
				else
					sprintf(szTemp, " %sx2", g_xMsg.GetMsg(5161+nI));

				if (bUseEquip)
					g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[12+nI] = TRUE;

			}
			else if ( m_stItemInfo.stStdItem.sAtomACGood == nI+1 )
			{
				bCheck = TRUE;
				bFind = TRUE;
				sprintf(szTemp, " %sx2", g_xMsg.GetMsg(5161+nI));

				if (bUseEquip)
					g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[12+nI] = TRUE;
			}

			if ( bCheck )
				strcat(szTemp2, szTemp);

			nMask <<= 1;
		}

		if ( bFind )
		{
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s", szTemp2);
			m_stItemState[m_nStateCnt].dwFont = RGB(128, 255, 0);
			m_nStateCnt++;
		}
	}
	
	// ���ѿ���
	if ( m_stItemInfo.Desc[6] > 0 || m_stItemInfo.stStdItem.sAtomACBad )
	{
		INT nMask = 1;
		CHAR szTemp[MAX_PATH], szTemp2[MAX_PATH];
		BOOL bFind = FALSE;

		sprintf(szTemp2, "%s:", g_xMsg.GetMsg(5180));

		for ( INT nI=0; nI<7; nI++)
		{
			BOOL bCheck = FALSE;
			if ( m_stItemInfo.Desc[6] & nMask )
			{
				bCheck = TRUE;
				bFind = TRUE;
				if ( m_stItemInfo.stStdItem.sAtomACBad == nI+1 )
					sprintf(szTemp, " %sx3", g_xMsg.GetMsg(5161+nI));
				else
					sprintf(szTemp, " %sx2", g_xMsg.GetMsg(5161+nI));

				if (bUseEquip)
					g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[19+nI] = TRUE;

			}
			else if ( m_stItemInfo.stStdItem.sAtomACBad == nI+1 )
			{
				bCheck = TRUE;
				bFind = TRUE;
				sprintf(szTemp, " %sx2", g_xMsg.GetMsg(5161+nI));

				if (bUseEquip)
					g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[19+nI] = TRUE;
			}

			if ( bCheck )
				strcat(szTemp2, szTemp);

			nMask <<= 1;
		}

		if ( bFind )
		{
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s", szTemp2);
			m_stItemState[m_nStateCnt].dwFont = RGB(150, 255, 200);
			m_nStateCnt++;
		}
	}

	if ( m_stItemInfo.stStdItem.sAtomACBad > 0 && m_stItemInfo.stStdItem.sAtomACBad < 7)
	{
		INT nAmount = m_stItemInfo.stStdItem.sAtomACBad;
		sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s:%sx2", g_xMsg.GetMsg(5179), g_xMsg.GetMsg(5160+nAmount));
		m_stItemState[m_nStateCnt].dwFont = RGB(150, 255, 200);
		m_nStateCnt++;
		if (bUseEquip)
			g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[nAmount+18] = TRUE;
	}

	if ( m_stItemInfo.Desc[11] > 0 )
	{
		m_stItemState[m_nStateCnt].dwFont = RGB(128, 255, 0);

		sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d-%d (+%d)", g_xMsg.GetMsg(m_stItemInfo.Desc[11]+5160), LOBYTE(m_stItemInfo.stStdItem.wAtomDC), HIBYTE(m_stItemInfo.stStdItem.wAtomDC) + m_stItemInfo.Desc[12], m_stItemInfo.Desc[12]);

		m_nStateCnt++;

		if (bUseEquip)
			g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[m_stItemInfo.Desc[11]+4] = TRUE;
	}

	// 8. �ʿ�ɷ�ġ.
	switch ( m_stItemInfo.stStdItem.bNeed )
	{
		case 0:
		{
			if ( g_xGameProc.m_xMyHero.m_stAbility.bLevel >= m_stItemInfo.stStdItem.bNeedLevel )
			{
			   bUseAble = TRUE;
			}

			if ( m_stItemInfo.stStdItem.bNeedLevel > 100 && g_xGameProc.m_xMyHero.m_stAbility.bLevel >= m_stItemInfo.stStdItem.bNeedLevel - 100 )
			{
			   bUseAble = TRUE;
			}

			if ( m_stItemInfo.stStdItem.bNeedLevel == 0 )
			{
				sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s", g_xMsg.GetMsg(2516));
			}
			else if ( m_stItemInfo.stStdItem.bNeedLevel > 100 )
			{
				sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d (%s)", g_xMsg.GetMsg(5140), m_stItemInfo.stStdItem.bNeedLevel-100, g_xMsg.GetMsg(2517));
			}
			else
			{
				sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d", g_xMsg.GetMsg(5140), m_stItemInfo.stStdItem.bNeedLevel);
			}
			break;
		}
		case 1:
		{
			if ( HIWORD(g_xGameProc.m_xMyHero.m_stAbility.dwDC) >= m_stItemInfo.stStdItem.bNeedLevel )
			{
			   bUseAble = TRUE;
			}
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s%d ", g_xMsg.GetMsg(5141), m_stItemInfo.stStdItem.bNeedLevel);
			break;
		}
		case 2:
		{
			bUseAble = TRUE;
			for ( nCnt = 0; nCnt < 4; nCnt++)
			{
				if ( HIBYTE(g_xGameProc.m_xMyHero.m_stAbility.ATOM_DC[nCnt]) < m_stItemInfo.stStdItem.bNeedLevel )
				{
					bUseAble = FALSE;
					break;
				}
			}
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s%d ", g_xMsg.GetMsg(5142), m_stItemInfo.stStdItem.bNeedLevel);
			break;
		}
		case 3:
		{
			bUseAble = TRUE;
			for ( nCnt = 4; nCnt < 7; nCnt++)
			{
				if ( HIBYTE(g_xGameProc.m_xMyHero.m_stAbility.ATOM_DC[nCnt]) < m_stItemInfo.stStdItem.bNeedLevel )
				{
					bUseAble = FALSE;
					break;
				}
			}
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s%d ", g_xMsg.GetMsg(5143), m_stItemInfo.stStdItem.bNeedLevel);
			break;
		}
		case 4:
		{
			bUseAble = TRUE;
			for ( nCnt = 0; nCnt < 7; nCnt++)
			{
				if ( HIBYTE(g_xGameProc.m_xMyHero.m_stAbility.ATOM_DC[nCnt]) < m_stItemInfo.stStdItem.bNeedLevel )
				{
					bUseAble = FALSE;
					break;
				}
			}
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s%d ", g_xMsg.GetMsg(5144), m_stItemInfo.stStdItem.bNeedLevel);
			break;
		}
		case 5:
		{
			if ( HIBYTE(g_xGameProc.m_xMyHero.m_stAbility.ATOM_DC[0]) >= m_stItemInfo.stStdItem.bNeedLevel )
			{
			   bUseAble = TRUE;
			}
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s%d ", g_xMsg.GetMsg(5145), m_stItemInfo.stStdItem.bNeedLevel);
			break;
		}
		case 6:
		{
			if ( HIBYTE(g_xGameProc.m_xMyHero.m_stAbility.ATOM_DC[1]) >= m_stItemInfo.stStdItem.bNeedLevel )
			{
			   bUseAble = TRUE;
			}
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s%d ", g_xMsg.GetMsg(5146), m_stItemInfo.stStdItem.bNeedLevel);
			break;
		}
		case 7:
		{
			if ( HIBYTE(g_xGameProc.m_xMyHero.m_stAbility.ATOM_DC[2]) >= m_stItemInfo.stStdItem.bNeedLevel )
			{
			   bUseAble = TRUE;
			}
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s%d ", g_xMsg.GetMsg(5147), m_stItemInfo.stStdItem.bNeedLevel);
			break;
		}
		case 8:
		{
			if ( HIBYTE(g_xGameProc.m_xMyHero.m_stAbility.ATOM_DC[3]) >= m_stItemInfo.stStdItem.bNeedLevel )
			{
			   bUseAble = TRUE;
			}
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s%d ", g_xMsg.GetMsg(5148), m_stItemInfo.stStdItem.bNeedLevel);

			break;
		}
		case 9:
		{
			if ( HIBYTE(g_xGameProc.m_xMyHero.m_stAbility.ATOM_DC[4]) >= m_stItemInfo.stStdItem.bNeedLevel )
			{
			   bUseAble = TRUE;
			}
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s%d ", g_xMsg.GetMsg(5149), m_stItemInfo.stStdItem.bNeedLevel);

			break;
		}
		case 10:
		{
			if ( HIBYTE(g_xGameProc.m_xMyHero.m_stAbility.ATOM_DC[5]) >= m_stItemInfo.stStdItem.bNeedLevel )
			{
			   bUseAble = TRUE;
			}
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s%d ", g_xMsg.GetMsg(5150), m_stItemInfo.stStdItem.bNeedLevel);

			break;
		}
		case 11:
		{
			if ( HIBYTE(g_xGameProc.m_xMyHero.m_stAbility.ATOM_DC[6]) >= m_stItemInfo.stStdItem.bNeedLevel )
			{
			   bUseAble = TRUE;
			}
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s%d ", g_xMsg.GetMsg(5151), m_stItemInfo.stStdItem.bNeedLevel);

			break;
		}
	}
	m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);

	if ( ( m_stItemInfo.stStdItem.bNeed == 0 && m_stItemInfo.stStdItem.bNeedLevel == 0 ||
		 m_stItemInfo.stStdItem.bNeed == 0 && m_stItemInfo.stStdItem.bNeedLevel > 100 ) && bUseAble )
	{
		m_stItemState[m_nStateCnt].dwFont = RGB(  0, 200,   0);
	}
	else if ( bUseAble )
	{
		m_stItemState[m_nStateCnt].dwFont = RGB(255, 255, 255);
	}
	else
	{
		m_stItemState[m_nStateCnt].dwFont = RGB(255, 0, 0);
	}
	m_nStateCnt++;

	if ( m_stItemInfo.stStdItem.nNeedFame > 0 )
	{
		sprintf(m_stItemState[m_nStateCnt].pszDesc, g_xMsg.GetMsg(2520), m_stItemInfo.stStdItem.nNeedFame);
		m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);

		if ( g_xGameProc.m_xMyHero.m_nFame >= m_stItemInfo.stStdItem.nNeedFame )
			m_stItemState[m_nStateCnt].dwFont = RGB(128, 255, 255);
		else
			m_stItemState[m_nStateCnt].dwFont = RGB(255, 0, 0);

		m_nStateCnt++;
	}
}

VOID CItem::SetBootsStatus(BOOL bUseStore, BOOL bUseEquip)
{
	INT	 nCnt; 
	BOOL bUseAble  = FALSE;

	// 1. �̸�.
	if ( strlen(m_stItemInfo.Prefix) )
	{		
		sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s%s %s", g_xMsg.GetMsg(2515), m_stItemInfo.Prefix, m_stItemInfo.stStdItem.szName);
	}
	else
	{
		sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s%s", g_xMsg.GetMsg(2515), m_stItemInfo.stStdItem.szName);
	}
	m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
	m_stItemState[m_nStateCnt].dwFont = RGB(255, 250,   0);
	m_nStateCnt++;

	// 2. ����.
	INT nDura	 = RoundFunc(m_stItemInfo.nDura, 1000);
	INT nDuraMax = RoundFunc(m_stItemInfo.nDuraMax, 1000);
	if ( bUseStore )
	{
		nDura	 = RoundFunc(m_stItemInfo.nDura, 1000);
		nDuraMax = RoundFunc(m_stItemInfo.stStdItem.wDuraMax, 1000);	
	}
	sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d/%d", g_xMsg.GetMsg(2500), nDura, nDuraMax);
	m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
	if ( nDura <= 1 ) 
	{
		m_stItemState[m_nStateCnt].dwFont = RGB(255,  50,  50);
	}
	else
	{
		m_stItemState[m_nStateCnt].dwFont = RGB(128, 255, 255);
	}
	m_nStateCnt++;

	// 3. ����.
	sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d", g_xMsg.GetMsg(2501), m_stItemInfo.stStdItem.bWeight);
	m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
	m_stItemState[m_nStateCnt].dwFont = RGB(255, 255, 255);
	m_nStateCnt++;

	// 4. ������.
	if ( m_stItemInfo.stStdItem.wDC + m_stItemInfo.Desc[0] > 0 )
	{
		m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
		if ( m_stItemInfo.Desc[0] > 0 )
		{
			// �ɼ� Plus.
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d (+%d)", g_xMsg.GetMsg(2502), m_stItemInfo.stStdItem.wDC+m_stItemInfo.Desc[0], m_stItemInfo.Desc[0]);
			m_stItemState[m_nStateCnt].dwFont = RGB(150, 255, 200);
		}
		else
		{
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d", g_xMsg.GetMsg(2502), m_stItemInfo.stStdItem.wDC);
			m_stItemState[m_nStateCnt].dwFont = RGB(255, 255, 255);
		}
		m_nStateCnt++;
	}

	// 5. ���빫��.
	if ( LOBYTE(m_stItemInfo.stStdItem.wMC) + m_stItemInfo.Desc[1] > 0 )
	{
		m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
		if ( m_stItemInfo.Desc[1] > 0 )
		{
			// �ɼ� Plus.
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d (+%d)", g_xMsg.GetMsg(1063), LOBYTE(m_stItemInfo.stStdItem.wMC) + m_stItemInfo.Desc[1], m_stItemInfo.Desc[1]);
			m_stItemState[m_nStateCnt].dwFont = RGB(150, 255, 200);
		}
		else
		{
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d", g_xMsg.GetMsg(1063), LOBYTE(m_stItemInfo.stStdItem.wMC));
			m_stItemState[m_nStateCnt].dwFont = RGB(255, 255, 255);
		}
		m_nStateCnt++;
	}

	// 5. ��չ���.
	if ( HIBYTE(m_stItemInfo.stStdItem.wMC) + m_stItemInfo.Desc[2] > 0 )
	{
		m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
		
		if ( m_stItemInfo.Desc[2] > 0 )
		{
			// �ɼ� Plus.
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d (+%d)", g_xMsg.GetMsg(1064), HIBYTE(m_stItemInfo.stStdItem.wMC)+m_stItemInfo.Desc[2], m_stItemInfo.Desc[2]);
			m_stItemState[m_nStateCnt].dwFont = RGB(150, 255, 200);
		}
		else
		{
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d", g_xMsg.GetMsg(1064), HIBYTE(m_stItemInfo.stStdItem.wMC));
			m_stItemState[m_nStateCnt].dwFont = RGB(255, 255, 255);
		}
		m_nStateCnt++;
	}

	// ���ѿ���
	if ( m_stItemInfo.Desc[5] > 0 || m_stItemInfo.stStdItem.sAtomACGood )
	{
		INT nMask = 1;
		CHAR szTemp[MAX_PATH], szTemp2[MAX_PATH];
		BOOL bFind = FALSE;

		sprintf(szTemp2, "%s:", g_xMsg.GetMsg(5179));

		for ( INT nI=0; nI<7; nI++)
		{
			BOOL bCheck = FALSE;
			if ( m_stItemInfo.Desc[5] & nMask )
			{
				bCheck = TRUE;
				bFind = TRUE;
				if ( m_stItemInfo.stStdItem.sAtomACGood == nI+1 )
					sprintf(szTemp, " %sx2", g_xMsg.GetMsg(5161+nI));
				else
					sprintf(szTemp, " %sx1", g_xMsg.GetMsg(5161+nI));

				if (bUseEquip)
					g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[12+nI] = TRUE;

			}
			else if ( m_stItemInfo.stStdItem.sAtomACGood == nI+1 )
			{
				bCheck = TRUE;
				bFind = TRUE;
				sprintf(szTemp, " %sx1", g_xMsg.GetMsg(5161+nI));

				if (bUseEquip)
					g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[12+nI] = TRUE;
			}

			if ( bCheck )
				strcat(szTemp2, szTemp);

			nMask <<= 1;
		}

		if ( bFind )
		{
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s", szTemp2);
			m_stItemState[m_nStateCnt].dwFont = RGB(128, 255, 0);
			m_nStateCnt++;
		}
	}

	if ( m_stItemInfo.Desc[8] > 0 )
	{
		m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
		// �ɼ� Plus.
		sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: +%d", g_xMsg.GetMsg(2518), m_stItemInfo.Desc[8] * 10);
		m_stItemState[m_nStateCnt].dwFont = RGB(150, 255, 200);
		m_nStateCnt++;
	}
	
	if ( m_stItemInfo.Desc[9] > 0 )
	{
		m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
		// �ɼ� Plus.
		sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: +%d", g_xMsg.GetMsg(2519), m_stItemInfo.Desc[9] * 10);
		m_stItemState[m_nStateCnt].dwFont = RGB(150, 255, 200);
		m_nStateCnt++;
	}

	// ���ѿ���
	if ( m_stItemInfo.Desc[6] > 0 || m_stItemInfo.stStdItem.sAtomACBad )
	{
		INT nMask = 1;
		CHAR szTemp[MAX_PATH], szTemp2[MAX_PATH];
		BOOL bFind = FALSE;

		sprintf(szTemp2, "%s:", g_xMsg.GetMsg(5180));

		for ( INT nI=0; nI<7; nI++)
		{
			BOOL bCheck = FALSE;
			if ( m_stItemInfo.Desc[6] & nMask )
			{
				bCheck = TRUE;
				bFind = TRUE;
				if ( m_stItemInfo.stStdItem.sAtomACBad == nI+1 )
					sprintf(szTemp, " %sx2", g_xMsg.GetMsg(5161+nI));
				else
					sprintf(szTemp, " %sx1", g_xMsg.GetMsg(5161+nI));

				if (bUseEquip)
					g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[19+nI] = TRUE;

			}
			else if ( m_stItemInfo.stStdItem.sAtomACBad == nI+1 )
			{
				bCheck = TRUE;
				bFind = TRUE;
				sprintf(szTemp, " %sx1", g_xMsg.GetMsg(5161+nI));

				if (bUseEquip)
					g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[19+nI] = TRUE;
			}

			if ( bCheck )
				strcat(szTemp2, szTemp);

			nMask <<= 1;
		}

		if ( bFind )
		{
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s", szTemp2);
			m_stItemState[m_nStateCnt].dwFont = RGB(150, 255, 200);
			m_nStateCnt++;
		}
	}


	// 6. �ʿ�ɷ�ġ.
	switch ( m_stItemInfo.stStdItem.bNeed )
	{
		case 0:
		{
			if ( g_xGameProc.m_xMyHero.m_stAbility.bLevel >= m_stItemInfo.stStdItem.bNeedLevel )
			{
			   bUseAble = TRUE;
			}

			if ( m_stItemInfo.stStdItem.bNeedLevel > 100 && g_xGameProc.m_xMyHero.m_stAbility.bLevel >= m_stItemInfo.stStdItem.bNeedLevel - 100 )
			{
			   bUseAble = TRUE;
			}

			if ( m_stItemInfo.stStdItem.bNeedLevel == 0 )
			{
				sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s", g_xMsg.GetMsg(2516));
			}
			else if ( m_stItemInfo.stStdItem.bNeedLevel > 100 )
			{
				sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d (%s)", g_xMsg.GetMsg(5140), m_stItemInfo.stStdItem.bNeedLevel-100, g_xMsg.GetMsg(2517));
			}
			else
			{
				sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d", g_xMsg.GetMsg(5140), m_stItemInfo.stStdItem.bNeedLevel);
			}

			break;
		}
		case 1:
		{
			if ( HIWORD(g_xGameProc.m_xMyHero.m_stAbility.dwDC) >= m_stItemInfo.stStdItem.bNeedLevel )
			{
			   bUseAble = TRUE;
			}
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s%d ", g_xMsg.GetMsg(5141), m_stItemInfo.stStdItem.bNeedLevel);
			break;
		}
		case 2:
		{
			bUseAble = TRUE;
			for ( nCnt = 0; nCnt < 4; nCnt++)
			{
				if ( HIBYTE(g_xGameProc.m_xMyHero.m_stAbility.ATOM_DC[nCnt]) < m_stItemInfo.stStdItem.bNeedLevel )
				{
					bUseAble = FALSE;
					break;
				}
			}
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s%d ", g_xMsg.GetMsg(5142), m_stItemInfo.stStdItem.bNeedLevel);
			break;
		}
		case 3:
		{
			bUseAble = TRUE;
			for ( nCnt = 4; nCnt < 7; nCnt++)
			{
				if ( HIBYTE(g_xGameProc.m_xMyHero.m_stAbility.ATOM_DC[nCnt]) < m_stItemInfo.stStdItem.bNeedLevel )
				{
					bUseAble = FALSE;
					break;
				}
			}
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s%d ", g_xMsg.GetMsg(5143), m_stItemInfo.stStdItem.bNeedLevel);
			break;
		}
		case 4:
		{
			bUseAble = TRUE;
			for ( nCnt = 0; nCnt < 7; nCnt++)
			{
				if ( HIBYTE(g_xGameProc.m_xMyHero.m_stAbility.ATOM_DC[nCnt]) < m_stItemInfo.stStdItem.bNeedLevel )
				{
					bUseAble = FALSE;
					break;
				}
			}
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s%d ", g_xMsg.GetMsg(5144), m_stItemInfo.stStdItem.bNeedLevel);
			break;
		}
		case 5:
		{
			if ( HIBYTE(g_xGameProc.m_xMyHero.m_stAbility.ATOM_DC[0]) >= m_stItemInfo.stStdItem.bNeedLevel )
			{
			   bUseAble = TRUE;
			}
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s%d ", g_xMsg.GetMsg(5145), m_stItemInfo.stStdItem.bNeedLevel);
			break;
		}
		case 6:
		{
			if ( HIBYTE(g_xGameProc.m_xMyHero.m_stAbility.ATOM_DC[1]) >= m_stItemInfo.stStdItem.bNeedLevel )
			{
			   bUseAble = TRUE;
			}
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s%d ", g_xMsg.GetMsg(5146), m_stItemInfo.stStdItem.bNeedLevel);
			break;
		}
		case 7:
		{
			if ( HIBYTE(g_xGameProc.m_xMyHero.m_stAbility.ATOM_DC[2]) >= m_stItemInfo.stStdItem.bNeedLevel )
			{
			   bUseAble = TRUE;
			}
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s%d ", g_xMsg.GetMsg(5147), m_stItemInfo.stStdItem.bNeedLevel);
			break;
		}
		case 8:
		{
			if ( HIBYTE(g_xGameProc.m_xMyHero.m_stAbility.ATOM_DC[3]) >= m_stItemInfo.stStdItem.bNeedLevel )
			{
			   bUseAble = TRUE;
			}
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s%d ", g_xMsg.GetMsg(5148), m_stItemInfo.stStdItem.bNeedLevel);

			break;
		}
		case 9:
		{
			if ( HIBYTE(g_xGameProc.m_xMyHero.m_stAbility.ATOM_DC[4]) >= m_stItemInfo.stStdItem.bNeedLevel )
			{
			   bUseAble = TRUE;
			}
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s%d ", g_xMsg.GetMsg(5149), m_stItemInfo.stStdItem.bNeedLevel);

			break;
		}
		case 10:
		{
			if ( HIBYTE(g_xGameProc.m_xMyHero.m_stAbility.ATOM_DC[5]) >= m_stItemInfo.stStdItem.bNeedLevel )
			{
			   bUseAble = TRUE;
			}
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s%d ", g_xMsg.GetMsg(5150), m_stItemInfo.stStdItem.bNeedLevel);

			break;
		}
		case 11:
		{
			if ( HIBYTE(g_xGameProc.m_xMyHero.m_stAbility.ATOM_DC[6]) >= m_stItemInfo.stStdItem.bNeedLevel )
			{
			   bUseAble = TRUE;
			}
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s%d ", g_xMsg.GetMsg(5151), m_stItemInfo.stStdItem.bNeedLevel);

			break;
		}
	}
	m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);

	if ( ( m_stItemInfo.stStdItem.bNeed == 0 && m_stItemInfo.stStdItem.bNeedLevel == 0 ||
		 m_stItemInfo.stStdItem.bNeed == 0 && m_stItemInfo.stStdItem.bNeedLevel > 100 ) && bUseAble )
	{
		m_stItemState[m_nStateCnt].dwFont = RGB(  0, 200,   0);
	}
	else if ( bUseAble )
	{
		m_stItemState[m_nStateCnt].dwFont = RGB(255, 255, 255);
	}
	else
	{
		m_stItemState[m_nStateCnt].dwFont = RGB(255, 0, 0);
	}
	m_nStateCnt++;

	if ( m_stItemInfo.stStdItem.nNeedFame > 0 )
	{
		sprintf(m_stItemState[m_nStateCnt].pszDesc, g_xMsg.GetMsg(2520), m_stItemInfo.stStdItem.nNeedFame);
		m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);

		if ( g_xGameProc.m_xMyHero.m_nFame >= m_stItemInfo.stStdItem.nNeedFame )
			m_stItemState[m_nStateCnt].dwFont = RGB(128, 255, 255);
		else
			m_stItemState[m_nStateCnt].dwFont = RGB(255, 0, 0);

		m_nStateCnt++;
	}
}

VOID CItem::SetWeaponStatus(BOOL bUseStore, BOOL bUseEquip)
{
	INT	 nCnt; 
	BOOL bUseAble  = FALSE;

	// 1. �̸�.
	if ( strlen(m_stItemInfo.Prefix) )
	{
		if ( m_stItemInfo.Desc[10] != 0 )
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s (*)%s %s", g_xMsg.GetMsg(2515), m_stItemInfo.Prefix, m_stItemInfo.stStdItem.szName);
		else
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s %s %s", g_xMsg.GetMsg(2515), m_stItemInfo.Prefix, m_stItemInfo.stStdItem.szName);
	}
	else
	{
		if ( m_stItemInfo.Desc[10] != 0 )
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s (*)%s", g_xMsg.GetMsg(2515), m_stItemInfo.stStdItem.szName);
		else
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s %s", g_xMsg.GetMsg(2515), m_stItemInfo.stStdItem.szName);
	}

	m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
	m_stItemState[m_nStateCnt].dwFont = RGB(255, 250,   0);
	m_nStateCnt++;

	// ��������� ǥ��
	if ( (m_stItemInfo.stStdItem.bItemDesc & 128) )
	{
		sprintf(m_stItemState[m_nStateCnt].pszDesc, g_xMsg.GetMsg(2521));
			
		m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
		m_stItemState[m_nStateCnt].dwFont = RGB( 58, 191, 241);
		m_nStateCnt++;
	}

	// 2. ����.
	INT nDura	 = RoundFunc(m_stItemInfo.nDura, 1000);
	INT nDuraMax = RoundFunc(m_stItemInfo.nDuraMax, 1000);
	if ( bUseStore )
	{
		nDura	 = RoundFunc(m_stItemInfo.nDura, 1000);
		nDuraMax = RoundFunc(m_stItemInfo.stStdItem.wDuraMax, 1000);	
	}
	if ( m_stItemInfo.stStdItem.bStdMode != 54  && m_stItemInfo.stStdItem.bStdMode != 55 /*������*/ )
	{
		sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d/%d", g_xMsg.GetMsg(2500), nDura, nDuraMax);
	}
	else // ǥâ.
	{
		nDura	 = RoundFunc(m_stItemInfo.nDura, 100);
		sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d", g_xMsg.GetMsg(2503), nDura);
	}
	m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
	if ( nDura <= 1 ) 
	{
		m_stItemState[m_nStateCnt].dwFont = RGB(255,  50,  50);
	}
	else
	{
		m_stItemState[m_nStateCnt].dwFont = RGB(128, 255, 255);
	}
	m_nStateCnt++;

	// 3. ����.
	sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d", g_xMsg.GetMsg(2501), m_stItemInfo.stStdItem.bWeight);
	m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
	m_stItemState[m_nStateCnt].dwFont = RGB(255, 255, 255);
	m_nStateCnt++;

	// 4. �ı�.
	if ( m_stItemInfo.stStdItem.wDC + m_stItemInfo.Desc[0] > 0 )
	{
		m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
		if ( m_stItemInfo.Desc[0] > 0 )
		{
			// �ɼ� Plus.
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d-%d (+%d)", g_xMsg.GetMsg(1061), LOBYTE(m_stItemInfo.stStdItem.wDC), HIBYTE(m_stItemInfo.stStdItem.wDC)+m_stItemInfo.Desc[0], m_stItemInfo.Desc[0]);
			m_stItemState[m_nStateCnt].dwFont = RGB(150, 255, 200);
		}
		else
		{
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d-%d ", g_xMsg.GetMsg(1061), LOBYTE(m_stItemInfo.stStdItem.wDC), HIBYTE(m_stItemInfo.stStdItem.wDC));
			m_stItemState[m_nStateCnt].dwFont = RGB(255, 255, 255);
		}

		if (bUseEquip)
			g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[0] = TRUE;

		m_nStateCnt++;
	}

	// Ȯ�� ����
	if ( m_stItemInfo.Desc[2] > 0 )
	{
		BYTE bDesc = atoi(&m_stItemInfo.Desc[2]);

		BYTE bKind, bLevel;

		bLevel = m_stItemInfo.Desc[2] & 0xF0;
		bLevel = bLevel >> 4;

		bKind = m_stItemInfo.Desc[2] & 0x0F;

		m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
		if ( m_stItemInfo.Desc[2] > 0 )
		{
			// �ɼ� Plus.
			sprintf(m_stItemState[m_nStateCnt].pszDesc, g_xMsg.GetMsg(2521+bKind), bLevel);
			m_stItemState[m_nStateCnt].dwFont = RGB(255, 150, 94);
		}
		m_nStateCnt++;
	}

	// ����
	if ( m_stItemInfo.stStdItem.wMC + m_stItemInfo.Desc[1] > 0 )
	{
		m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
		if ( m_stItemInfo.Desc[1] > 0 )
		{
			// �ɼ� Plus.
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d-%d (+%d)", g_xMsg.GetMsg(5100+m_stItemInfo.stStdItem.MCType), LOBYTE(m_stItemInfo.stStdItem.wMC), HIBYTE(m_stItemInfo.stStdItem.wMC)+m_stItemInfo.Desc[1], m_stItemInfo.Desc[1]);
			m_stItemState[m_nStateCnt].dwFont = RGB(150, 255, 200);
		}
		else
		{
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d-%d ", g_xMsg.GetMsg(5100+m_stItemInfo.stStdItem.MCType), LOBYTE(m_stItemInfo.stStdItem.wMC), HIBYTE(m_stItemInfo.stStdItem.wMC));
			m_stItemState[m_nStateCnt].dwFont = RGB(255, 255, 255);
		}
		m_nStateCnt++;

		if (bUseEquip)
		{
			if (m_stItemInfo.stStdItem.MCType == 0) //���迭
			{
				g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[2] = TRUE;
				g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[3] = TRUE;
			}
			else if (m_stItemInfo.stStdItem.MCType == 1) //�ڿ��迭
			{
				g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[2] = TRUE;
			}
			else // ��ȥ��
			{
				g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[3] = TRUE;
			}
		}
	}

	// ���� �ı�
	if (( m_stItemInfo.Desc[11] > 0 ) || (m_stItemInfo.stStdItem.bAtomDCType> 0))
	{
		byte btAtomType = m_stItemInfo.stStdItem.bAtomDCType;

		if (m_stItemInfo.Desc[11] > 0)
			btAtomType = m_stItemInfo.Desc[11];

		if ( m_stItemInfo.Desc[12] > 0 )
		{
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %s+%d", g_xMsg.GetMsg(5178), g_xMsg.GetMsg(btAtomType+5160), m_stItemInfo.stStdItem.wAtomDC+m_stItemInfo.Desc[12]);
		}
		else
		{
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %s+%d", g_xMsg.GetMsg(5178), g_xMsg.GetMsg(btAtomType+5160), m_stItemInfo.stStdItem.wAtomDC);
		}

		m_stItemState[m_nStateCnt].dwFont = RGB(128, 255, 0);

		if (bUseEquip)
			g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[btAtomType+4] = TRUE;

		m_nStateCnt++;
	}

	// 6. �ż� �� ����.
	if ( m_stItemInfo.stStdItem.sSpecialPower + m_stItemInfo.Desc[7] > 0 )
	{
		m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
		if ( m_stItemInfo.stStdItem.bShape == 19 || (m_stItemInfo.stStdItem.bShape == 18 && m_stItemInfo.stStdItem.bStdMode == 56))
		{
			if ( m_stItemInfo.Desc[7] > 0 )
			{
				// �ɼ� Plus.
				m_stItemState[m_nStateCnt].dwFont = RGB(150, 255, 200);
//				sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d (+%d)", g_xMsg.GetMsg(5054), m_stItemInfo.stStdItem.sSpecialPower+m_stItemInfo.Desc[7], m_stItemInfo.Desc[7]);
				sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d (+%d)", g_xMsg.GetMsg(2504), m_stItemInfo.stStdItem.sSpecialPower+m_stItemInfo.Desc[7], m_stItemInfo.Desc[7]);
			}
			else
			{
				m_stItemState[m_nStateCnt].dwFont = RGB(255, 255, 255);
//				sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d", g_xMsg.GetMsg(5054), m_stItemInfo.stStdItem.sSpecialPower);
				sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d", g_xMsg.GetMsg(2504), m_stItemInfo.stStdItem.sSpecialPower);
			}
		}
		else
		{
			if ( m_stItemInfo.Desc[7] > 0 )
			{
				// �ɼ� Plus.
				m_stItemState[m_nStateCnt].dwFont = RGB(150, 255, 200);
				sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d (+%d)", g_xMsg.GetMsg(2504), m_stItemInfo.stStdItem.sSpecialPower+m_stItemInfo.Desc[7], m_stItemInfo.Desc[7]);
			}
			else
			{
				m_stItemState[m_nStateCnt].dwFont = RGB(255, 255, 255);
				sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d", g_xMsg.GetMsg(2504), m_stItemInfo.stStdItem.sSpecialPower);
			}
		}
		m_nStateCnt++;
	}

	// 7. ��Ȯ.
	if ( HIBYTE(m_stItemInfo.stStdItem.wAC) + m_stItemInfo.Desc[5] > 0 )
	{
		m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
		if ( m_stItemInfo.Desc[5] > 0 )
		{
			// �ɼ� Plus.
			m_stItemState[m_nStateCnt].dwFont = RGB(150, 255, 200);
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d (+%d)", g_xMsg.GetMsg(1053), HIBYTE(m_stItemInfo.stStdItem.wAC)+m_stItemInfo.Desc[5], m_stItemInfo.Desc[5]);
		}
		else
		{
			INT nAccuracy = HIBYTE(m_stItemInfo.stStdItem.wAC);

			if ( nAccuracy > 100 )
			{
				nAccuracy -= 100;
				nAccuracy = nAccuracy * -1;
			}

			m_stItemState[m_nStateCnt].dwFont = RGB(255, 255, 255);
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d", g_xMsg.GetMsg(1053), nAccuracy);
		}
		m_nStateCnt++;
	}

	// 8. ���ݼӵ�.
	if ( HIBYTE(m_stItemInfo.stStdItem.wMAC) + m_stItemInfo.Desc[6] > 0 )
	{
		m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);

		if ( HIBYTE(m_stItemInfo.stStdItem.wMAC) + m_stItemInfo.Desc[6] > 10 )
		{
			if ( m_stItemInfo.Desc[6] > 0 )
			{
				// �ɼ� Plus.
				m_stItemState[m_nStateCnt].dwFont = RGB(150, 255, 200);
				sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: +%d (+%d)", g_xMsg.GetMsg(2505), HIBYTE(m_stItemInfo.stStdItem.wMAC)-10+m_stItemInfo.Desc[6], m_stItemInfo.Desc[6]-10);
			}
			else
			{
				m_stItemState[m_nStateCnt].dwFont = RGB(255, 255, 255);
				sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: +%d", g_xMsg.GetMsg(2505), HIBYTE(m_stItemInfo.stStdItem.wMAC)-10);
			}
		}
		else
		{
			if ( m_stItemInfo.Desc[6] > 0 )
			{
				// �ɼ� Plus.
				m_stItemState[m_nStateCnt].dwFont = RGB(150, 255, 200);
				sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: -%d (-%d)", g_xMsg.GetMsg(2505), HIBYTE(m_stItemInfo.stStdItem.wMAC)+m_stItemInfo.Desc[6], m_stItemInfo.Desc[6]);
			}
			else
			{
				m_stItemState[m_nStateCnt].dwFont = RGB(255, 255, 255);
				sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: -%d", g_xMsg.GetMsg(2505), HIBYTE(m_stItemInfo.stStdItem.wMAC) );
			}
		}

		m_nStateCnt++;
	}

	// 9. ���.
	if ( LOBYTE(m_stItemInfo.stStdItem.wAC) + m_stItemInfo.Desc[3] > 0 )
	{
		m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
		if ( m_stItemInfo.Desc[3] > 0 )
		{
			// �ɼ� Plus.
			m_stItemState[m_nStateCnt].dwFont = RGB(150, 255, 200);
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: +%d (+%d)", g_xMsg.GetMsg(2506), LOBYTE(m_stItemInfo.stStdItem.wAC)+m_stItemInfo.Desc[3], m_stItemInfo.Desc[3]);
		}
		else
		{
			m_stItemState[m_nStateCnt].dwFont = RGB(255, 255, 255);
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: +%d", g_xMsg.GetMsg(2506), LOBYTE(m_stItemInfo.stStdItem.wAC));
		}

		m_nStateCnt++;
	}

	// 10. ����.
	if ( LOBYTE(m_stItemInfo.stStdItem.wMAC) + m_stItemInfo.Desc[4] > 0 )
	{
		m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
		if ( m_stItemInfo.Desc[4] > 0 )
		{
			// �ɼ� Plus.
			m_stItemState[m_nStateCnt].dwFont = RGB(150, 255, 200);
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: +%d (+%d)", g_xMsg.GetMsg(2507), LOBYTE(m_stItemInfo.stStdItem.wMAC)+m_stItemInfo.Desc[4], m_stItemInfo.Desc[4]);
		}
		else
		{
			m_stItemState[m_nStateCnt].dwFont = RGB(255, 255, 255);
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: +%d", g_xMsg.GetMsg(2507), LOBYTE(m_stItemInfo.stStdItem.wMAC));
		}
		m_nStateCnt++;
	}

	// ��ų ����
	if ( m_stItemInfo.stStdItem.bStdMode != 54 && m_stItemInfo.stStdItem.bStdMode != 55/*������*/ )	// ǥâ�˻�.
	{
		m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
		m_stItemState[m_nStateCnt].dwFont = RGB(255, 255, 255);

		INT	 nLevel = 0;
		for ( INT nCnt=0; nCnt<35; nCnt++ )
		{
			if ( g_nWeaponLevelTable[nCnt] <= m_stItemInfo.bLevel_R && g_nWeaponLevelTable[nCnt+1] > m_stItemInfo.bLevel_R )
			{
				break;
			}
		}

		nLevel = nCnt+1;

		if ( nLevel >= 17 )
		{
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s : MAX", g_xMsg.GetMsg(2508));
			m_nStateCnt++;
		}
		else
		{
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s : %d", g_xMsg.GetMsg(2508), nLevel);

			m_nStateCnt++;

			// ����ġ
			UINT  wSwordTrainPoint = m_stItemInfo.wAmount_GB;
			wSwordTrainPoint = wSwordTrainPoint + (60000 * m_stItemInfo.bLevel_R);
			
	//		INT	  nExpIdx = (m_stItemInfo.ColorR + 1) / 4;
			INT	  nExpIdx = nLevel - 1;

			if (nExpIdx > 0)
			{
				for ( INT nIdx=0; nIdx < nLevel-1 && nIdx < 35; nIdx++ )
					wSwordTrainPoint -= g_nWeaponExpTable[nIdx];
			}

			float fPoint = float(wSwordTrainPoint) * 100 / float(g_nWeaponExpTable[nExpIdx]);
			
			m_stItemState[m_nStateCnt].dwFont = RGB(255, 255, 255);
			m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s : %.2f %%", g_xMsg.GetMsg(2509), fPoint);
			m_nStateCnt++;
		}
	}
	
	// 11. �ʿ�ɷ�ġ.
	switch ( m_stItemInfo.stStdItem.bNeed )
	{
		case 0:
		{
			if ( g_xGameProc.m_xMyHero.m_stAbility.bLevel >= m_stItemInfo.stStdItem.bNeedLevel )
			{
			   bUseAble = TRUE;
			}

			if ( m_stItemInfo.stStdItem.bNeedLevel > 100 && g_xGameProc.m_xMyHero.m_stAbility.bLevel >= m_stItemInfo.stStdItem.bNeedLevel - 100 )
			{
			   bUseAble = TRUE;
			}

			if ( m_stItemInfo.stStdItem.bNeedLevel == 0 )
			{
				sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s", g_xMsg.GetMsg(2516));
			}
			else if ( m_stItemInfo.stStdItem.bNeedLevel > 100 )
			{
				sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d (%s)", g_xMsg.GetMsg(5140), m_stItemInfo.stStdItem.bNeedLevel-100, g_xMsg.GetMsg(2517));
			}
			else
			{
				sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d", g_xMsg.GetMsg(5140), m_stItemInfo.stStdItem.bNeedLevel);
			}

			break;
		}
		case 1:
		{
			if ( HIWORD(g_xGameProc.m_xMyHero.m_stAbility.dwDC) >= m_stItemInfo.stStdItem.bNeedLevel )
			{
			   bUseAble = TRUE;
			}
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s%d ", g_xMsg.GetMsg(5141), m_stItemInfo.stStdItem.bNeedLevel);
			break;
		}
		case 2:
		{
			bUseAble = TRUE;
			for ( nCnt = 0; nCnt < 4; nCnt++)
			{
				if ( HIBYTE(g_xGameProc.m_xMyHero.m_stAbility.ATOM_DC[nCnt]) < m_stItemInfo.stStdItem.bNeedLevel )
				{
					bUseAble = FALSE;
					break;
				}
			}
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s%d ", g_xMsg.GetMsg(5142), m_stItemInfo.stStdItem.bNeedLevel);
			break;
		}
		case 3:
		{
			bUseAble = TRUE;
			for ( nCnt = 4; nCnt < 7; nCnt++)
			{
				if ( HIBYTE(g_xGameProc.m_xMyHero.m_stAbility.ATOM_DC[nCnt]) < m_stItemInfo.stStdItem.bNeedLevel )
				{
					bUseAble = FALSE;
					break;
				}
			}
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s%d ", g_xMsg.GetMsg(5143), m_stItemInfo.stStdItem.bNeedLevel);
			break;
		}
		case 4:
		{
			bUseAble = TRUE;
			for ( nCnt = 0; nCnt < 7; nCnt++)
			{
				if ( HIBYTE(g_xGameProc.m_xMyHero.m_stAbility.ATOM_DC[nCnt]) < m_stItemInfo.stStdItem.bNeedLevel )
				{
					bUseAble = FALSE;
					break;
				}
			}
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s%d ", g_xMsg.GetMsg(5144), m_stItemInfo.stStdItem.bNeedLevel);
			break;
		}
		case 5:
		{
			if ( HIBYTE(g_xGameProc.m_xMyHero.m_stAbility.ATOM_DC[0]) >= m_stItemInfo.stStdItem.bNeedLevel )
			{
			   bUseAble = TRUE;
			}
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s%d ", g_xMsg.GetMsg(5145), m_stItemInfo.stStdItem.bNeedLevel);
			break;
		}
		case 6:
		{
			if ( HIBYTE(g_xGameProc.m_xMyHero.m_stAbility.ATOM_DC[1]) >= m_stItemInfo.stStdItem.bNeedLevel )
			{
			   bUseAble = TRUE;
			}
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s%d ", g_xMsg.GetMsg(5146), m_stItemInfo.stStdItem.bNeedLevel);
			break;
		}
		case 7:
		{
			if ( HIBYTE(g_xGameProc.m_xMyHero.m_stAbility.ATOM_DC[2]) >= m_stItemInfo.stStdItem.bNeedLevel )
			{
			   bUseAble = TRUE;
			}
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s%d ", g_xMsg.GetMsg(5147), m_stItemInfo.stStdItem.bNeedLevel);
			break;
		}
		case 8:
		{
			if ( HIBYTE(g_xGameProc.m_xMyHero.m_stAbility.ATOM_DC[3]) >= m_stItemInfo.stStdItem.bNeedLevel )
			{
			   bUseAble = TRUE;
			}
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s%d ", g_xMsg.GetMsg(5148), m_stItemInfo.stStdItem.bNeedLevel);

			break;
		}
		case 9:
		{
			if ( HIBYTE(g_xGameProc.m_xMyHero.m_stAbility.ATOM_DC[4]) >= m_stItemInfo.stStdItem.bNeedLevel )
			{
			   bUseAble = TRUE;
			}
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s%d ", g_xMsg.GetMsg(5149), m_stItemInfo.stStdItem.bNeedLevel);

			break;
		}
		case 10:
		{
			if ( HIBYTE(g_xGameProc.m_xMyHero.m_stAbility.ATOM_DC[5]) >= m_stItemInfo.stStdItem.bNeedLevel )
			{
			   bUseAble = TRUE;
			}
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s%d ", g_xMsg.GetMsg(5150), m_stItemInfo.stStdItem.bNeedLevel);

			break;
		}
		case 11:
		{
			if ( HIBYTE(g_xGameProc.m_xMyHero.m_stAbility.ATOM_DC[6]) >= m_stItemInfo.stStdItem.bNeedLevel )
			{
			   bUseAble = TRUE;
			}
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s%d ", g_xMsg.GetMsg(5151), m_stItemInfo.stStdItem.bNeedLevel);

			break;
		}
	}
	m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);

	if ( ( m_stItemInfo.stStdItem.bNeed == 0 && m_stItemInfo.stStdItem.bNeedLevel == 0 ||
		 m_stItemInfo.stStdItem.bNeed == 0 && m_stItemInfo.stStdItem.bNeedLevel > 100 ) && bUseAble )
	{
		m_stItemState[m_nStateCnt].dwFont = RGB(  0, 200,   0);
	}
	else if ( bUseAble )
	{
		m_stItemState[m_nStateCnt].dwFont = RGB(255, 255, 255);
	}
	else
	{
		m_stItemState[m_nStateCnt].dwFont = RGB(255, 0, 0);
	}
	m_nStateCnt++;

	if ( m_stItemInfo.stStdItem.nNeedFame > 0 )
	{
		sprintf(m_stItemState[m_nStateCnt].pszDesc, g_xMsg.GetMsg(2520), m_stItemInfo.stStdItem.nNeedFame);
		m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);

		if ( g_xGameProc.m_xMyHero.m_nFame >= m_stItemInfo.stStdItem.nNeedFame )
			m_stItemState[m_nStateCnt].dwFont = RGB(128, 255, 255);
		else
			m_stItemState[m_nStateCnt].dwFont = RGB(255, 0, 0);

		m_nStateCnt++;
	}
}

VOID CItem::SetAccessoryStatus(BOOL bUseStore, BOOL bUseEquip)
{
	INT	 nCnt; 
	BOOL bUseAble = FALSE;

	// 1. �̸�.
	if ( strlen(m_stItemInfo.Prefix) )
	{		
		sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s%s %s", g_xMsg.GetMsg(2515), m_stItemInfo.Prefix, m_stItemInfo.stStdItem.szName);
	}
	else
	{
		sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s%s", g_xMsg.GetMsg(2515), m_stItemInfo.stStdItem.szName);
	}
	m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
	m_stItemState[m_nStateCnt].dwFont = RGB(255, 250,   0);
	m_nStateCnt++;

	// 2. ����.
	INT nDura	 = RoundFunc(m_stItemInfo.nDura, 1000);
	INT nDuraMax = RoundFunc(m_stItemInfo.nDuraMax, 1000);
	if ( bUseStore )
	{
		nDura	 = RoundFunc(m_stItemInfo.nDura, 1000);
		nDuraMax = RoundFunc(m_stItemInfo.stStdItem.wDuraMax, 1000);	
	}
	sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d/%d", g_xMsg.GetMsg(2500), nDura, nDuraMax);
	m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
	if ( nDura <= 1 ) 
	{
		m_stItemState[m_nStateCnt].dwFont = RGB(255,  50,  50);
	}
	else
	{
		m_stItemState[m_nStateCnt].dwFont = RGB(128, 255, 255);
	}
	m_nStateCnt++;

	// 3. ����.
	sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d", g_xMsg.GetMsg(2501), m_stItemInfo.stStdItem.bWeight);
	m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
	m_stItemState[m_nStateCnt].dwFont = RGB(255, 255, 255);
	m_nStateCnt++;

	// �ı�.
	if ( m_stItemInfo.stStdItem.wDC + m_stItemInfo.Desc[2] > 0 )
	{
		m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
		if ( m_stItemInfo.Desc[2] > 0 )
		{
			// �ɼ� Plus.
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d-%d (+%d)", g_xMsg.GetMsg(1061), LOBYTE(m_stItemInfo.stStdItem.wDC), HIBYTE(m_stItemInfo.stStdItem.wDC)+m_stItemInfo.Desc[2], m_stItemInfo.Desc[2]);
			m_stItemState[m_nStateCnt].dwFont = RGB(150, 255, 200);
		}
		else
		{
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d-%d ", g_xMsg.GetMsg(1061), LOBYTE(m_stItemInfo.stStdItem.wDC), HIBYTE(m_stItemInfo.stStdItem.wDC));
			m_stItemState[m_nStateCnt].dwFont = RGB(255, 255, 255);
		}
		m_nStateCnt++;

		if (bUseEquip)
			g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[0] = TRUE;
	}

	// 4. Ưȭ ����ġ.
	switch ( m_stItemInfo.stStdItem.bStdMode )
	{
		case 19:			// �����
		{
			if ( HIBYTE(m_stItemInfo.stStdItem.wAC) + m_stItemInfo.Desc[0] > 0 )
			{
				m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
				if ( m_stItemInfo.Desc[0] > 0 )
				{
					sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: +%d%% (+%d%%)", g_xMsg.GetMsg(1055), (HIBYTE(m_stItemInfo.stStdItem.wAC)+m_stItemInfo.Desc[0])*10, m_stItemInfo.Desc[0]*10);
					m_stItemState[m_nStateCnt].dwFont = RGB(150, 255, 200);
				}
				else
				{
					sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: +%d%%", g_xMsg.GetMsg(1055), HIBYTE(m_stItemInfo.stStdItem.wAC)*10);
					m_stItemState[m_nStateCnt].dwFont = RGB(255, 255, 255);
				}
				m_nStateCnt++;
			}

			if ( LOBYTE(m_stItemInfo.stStdItem.wMAC) > 0 )
			{
				sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: +%d", g_xMsg.GetMsg(2507), LOBYTE(m_stItemInfo.stStdItem.wMAC));
				m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
				m_stItemState[m_nStateCnt].dwFont = RGB(255, 255, 255);
				m_nStateCnt++;
			}

			if ( HIBYTE(m_stItemInfo.stStdItem.wMAC) + m_stItemInfo.Desc[1] > 0 )
			{
				m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
				if ( m_stItemInfo.Desc[1] > 0 )
				{
					sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: +%d (+%d)", g_xMsg.GetMsg(2506), HIBYTE(m_stItemInfo.stStdItem.wMAC)+m_stItemInfo.Desc[1], m_stItemInfo.Desc[1]);
					m_stItemState[m_nStateCnt].dwFont = RGB(150, 255, 200);
				}
				else
				{
					sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: +%d", g_xMsg.GetMsg(2506), HIBYTE(m_stItemInfo.stStdItem.wMAC));		
					m_stItemState[m_nStateCnt].dwFont = RGB(255, 255, 255);
				}
				m_nStateCnt++;
			}

			// ���� ����
			if ( ( m_stItemInfo.Desc[5] > 0 && m_stItemInfo.Desc[6] > 0 ) || 
				 ( m_stItemInfo.Desc[11] > 0 ) || (m_stItemInfo.stStdItem.bAtomDCType> 0) )
			{
				CHAR szTemp[MAX_PATH], szTemp2[MAX_PATH];
				BOOL bFind = FALSE;

				sprintf(szTemp2, "%s:", g_xMsg.GetMsg(5178));

				if (( m_stItemInfo.Desc[11] > 0 ) || (m_stItemInfo.stStdItem.bAtomDCType> 0))
				{
					m_stItemState[m_nStateCnt].dwFont = RGB(128, 255, 0);

					byte btAtomType = m_stItemInfo.stStdItem.bAtomDCType;

					if ( btAtomType != m_stItemInfo.Desc[11] && btAtomType != 0 )
					{
						CHAR szTemp3[MAX_PATH];
						sprintf(szTemp3, " %s+%d", g_xMsg.GetMsg(btAtomType+5160), m_stItemInfo.stStdItem.wAtomDC);
						strcat(szTemp2, szTemp3);
					}

					if (m_stItemInfo.Desc[11] > 0)
						btAtomType = m_stItemInfo.Desc[11];

					if ( m_stItemInfo.Desc[12] > 0 )
					{
						bFind = TRUE;
						if ( btAtomType == m_stItemInfo.stStdItem.bAtomDCType )
							sprintf(szTemp, " %s+%d", g_xMsg.GetMsg(btAtomType+5160), m_stItemInfo.stStdItem.wAtomDC+m_stItemInfo.Desc[12]);
						else
							sprintf(szTemp, " %s+%d", g_xMsg.GetMsg(btAtomType+5160), m_stItemInfo.Desc[12]);
					}

					if (bUseEquip)
						g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[btAtomType+4] = TRUE;
				}

				if ( bFind )
					strcat(szTemp2, szTemp);

				bFind = FALSE;

				m_stItemState[m_nStateCnt].dwFont = RGB(128, 255, 0);

				BYTE btAtomType = m_stItemInfo.Desc[5];

				if ( m_stItemInfo.Desc[6] > 0 )
				{
					bFind = TRUE;
					sprintf(szTemp, " %s+%d", g_xMsg.GetMsg(btAtomType+5160), m_stItemInfo.Desc[6]);
				
					if (bUseEquip)
						g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[btAtomType+4] = TRUE;
				}

				if ( bFind )
					strcat(szTemp2, szTemp);

				m_stItemState[m_nStateCnt].dwFont = RGB(128, 255, 0);
				sprintf(m_stItemState[m_nStateCnt].pszDesc, szTemp2);

				m_nStateCnt++;
			}

			// ���ѿ���
			if ( m_stItemInfo.stStdItem.sAtomACGood > 0 && m_stItemInfo.stStdItem.sAtomACGood < 7)
			{
				INT nIdx = m_stItemInfo.stStdItem.sAtomACGood;
				sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %sx1", g_xMsg.GetMsg(5179), g_xMsg.GetMsg(5160+nIdx));
				m_stItemState[m_nStateCnt].dwFont = RGB(128, 255, 0);
				m_nStateCnt++;

				if (bUseEquip)
					g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[nIdx+11] = TRUE;
			}

			// ���ѿ���
			if ( m_stItemInfo.stStdItem.sAtomACBad > 0 && m_stItemInfo.stStdItem.sAtomACBad < 7)
			{
				INT nIdx = m_stItemInfo.stStdItem.sAtomACBad;
				sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %sx1", g_xMsg.GetMsg(5180), g_xMsg.GetMsg(5160+nIdx));
				m_stItemState[m_nStateCnt].dwFont = RGB(150, 255, 200);
				m_nStateCnt++;

				if (bUseEquip)
					g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[nIdx+18] = TRUE;
			}

			break;
		}
		case 20:				// �����
		{
			if ( HIBYTE(m_stItemInfo.stStdItem.wAC) + m_stItemInfo.Desc[0] > 0 )
			{
				m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
				if ( m_stItemInfo.Desc[0] > 0 )
				{
					sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: +%d (+%d)", g_xMsg.GetMsg(1053), HIBYTE(m_stItemInfo.stStdItem.wAC)+m_stItemInfo.Desc[0], m_stItemInfo.Desc[0]);
					m_stItemState[m_nStateCnt].dwFont = RGB(150, 255, 200);
				}
				else
				{
					sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: +%d", g_xMsg.GetMsg(1053), HIBYTE(m_stItemInfo.stStdItem.wAC));
					m_stItemState[m_nStateCnt].dwFont = RGB(255, 255, 255);
				}
				m_nStateCnt++;
			}
	
			if ( HIBYTE(m_stItemInfo.stStdItem.wMAC) + m_stItemInfo.Desc[1] > 0 )
			{
				m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
				if ( m_stItemInfo.Desc[1] > 0 )
				{
					sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: +%d (+%d)", g_xMsg.GetMsg(1054), HIBYTE(m_stItemInfo.stStdItem.wMAC)+m_stItemInfo.Desc[1], m_stItemInfo.Desc[1]);
					m_stItemState[m_nStateCnt].dwFont = RGB(150, 255, 200);
				}
				else
				{
					sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: +%d", g_xMsg.GetMsg(1054), HIBYTE(m_stItemInfo.stStdItem.wMAC));
					m_stItemState[m_nStateCnt].dwFont = RGB(255, 255, 255);
				}
				m_nStateCnt++;
			}

			// ���� ����
			if ( ( m_stItemInfo.Desc[5] > 0 && m_stItemInfo.Desc[6] > 0 ) || 
				 ( m_stItemInfo.Desc[11] > 0 ) || (m_stItemInfo.stStdItem.bAtomDCType> 0) )
			{
				CHAR szTemp[MAX_PATH], szTemp2[MAX_PATH];
				BOOL bFind = FALSE;

				sprintf(szTemp2, "%s:", g_xMsg.GetMsg(5178));

				if (( m_stItemInfo.Desc[11] > 0 ) || (m_stItemInfo.stStdItem.bAtomDCType> 0))
				{
					m_stItemState[m_nStateCnt].dwFont = RGB(128, 255, 0);

					byte btAtomType = m_stItemInfo.stStdItem.bAtomDCType;

					if ( btAtomType != m_stItemInfo.Desc[11] && btAtomType != 0 )
					{
						CHAR szTemp3[MAX_PATH];
						sprintf(szTemp3, " %s+%d", g_xMsg.GetMsg(btAtomType+5160), m_stItemInfo.stStdItem.wAtomDC);
						strcat(szTemp2, szTemp3);
					}

					if (m_stItemInfo.Desc[11] > 0)
						btAtomType = m_stItemInfo.Desc[11];

					if ( m_stItemInfo.Desc[12] > 0 )
					{
						bFind = TRUE;
						if ( btAtomType == m_stItemInfo.stStdItem.bAtomDCType )
							sprintf(szTemp, " %s+%d", g_xMsg.GetMsg(btAtomType+5160), m_stItemInfo.stStdItem.wAtomDC+m_stItemInfo.Desc[12]);
						else
							sprintf(szTemp, " %s+%d", g_xMsg.GetMsg(btAtomType+5160), m_stItemInfo.Desc[12]);
					}

					if (bUseEquip)
						g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[btAtomType+4] = TRUE;
				}

				if ( bFind )
					strcat(szTemp2, szTemp);

				bFind = FALSE;

				m_stItemState[m_nStateCnt].dwFont = RGB(128, 255, 0);

				BYTE btAtomType = m_stItemInfo.Desc[5];

				if ( m_stItemInfo.Desc[6] > 0 )
				{
					bFind = TRUE;
					sprintf(szTemp, " %s+%d", g_xMsg.GetMsg(btAtomType+5160), m_stItemInfo.Desc[6]);
				
					if (bUseEquip)
						g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[btAtomType+4] = TRUE;
				}

				if ( bFind )
					strcat(szTemp2, szTemp);

				m_stItemState[m_nStateCnt].dwFont = RGB(128, 255, 0);
				sprintf(m_stItemState[m_nStateCnt].pszDesc, szTemp2);

				m_nStateCnt++;
			}

			// ���ѿ���
			if ( m_stItemInfo.stStdItem.sAtomACGood > 0 && m_stItemInfo.stStdItem.sAtomACGood < 7)
			{
				INT nIdx = m_stItemInfo.stStdItem.sAtomACGood;
				sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %sx1", g_xMsg.GetMsg(5179), g_xMsg.GetMsg(5160+nIdx));
				m_stItemState[m_nStateCnt].dwFont = RGB(128, 255, 0);
				m_nStateCnt++;

				if (bUseEquip)
					g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[nIdx+11] = TRUE;
			}

			// ���ѿ���
			if ( m_stItemInfo.stStdItem.sAtomACBad > 0 && m_stItemInfo.stStdItem.sAtomACBad < 7)
			{
				INT nIdx = m_stItemInfo.stStdItem.sAtomACBad;
				sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %sx1", g_xMsg.GetMsg(5180), g_xMsg.GetMsg(5160+nIdx));
				m_stItemState[m_nStateCnt].dwFont = RGB(150, 255, 200);
				m_nStateCnt++;

				if (bUseEquip)
					g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[nIdx+18] = TRUE;
			}

			break;
		}
		case 24: 		// ����
		{
			if ( HIBYTE(m_stItemInfo.stStdItem.wAC) + m_stItemInfo.Desc[0] > 0 )
			{
				m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
				if ( m_stItemInfo.Desc[0] > 0 )
				{
					sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: +%d (+%d)", g_xMsg.GetMsg(1053), HIBYTE(m_stItemInfo.stStdItem.wAC)+m_stItemInfo.Desc[0], m_stItemInfo.Desc[0]);
					m_stItemState[m_nStateCnt].dwFont = RGB(150, 255, 200);
				}
				else
				{
					sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: +%d", g_xMsg.GetMsg(1053), HIBYTE(m_stItemInfo.stStdItem.wAC));
					m_stItemState[m_nStateCnt].dwFont = RGB(255, 255, 255);
				}
				m_nStateCnt++;
			}
	
			if ( HIBYTE(m_stItemInfo.stStdItem.wMAC) + m_stItemInfo.Desc[1] > 0 )
			{
				m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
				if ( m_stItemInfo.Desc[1] > 0 )
				{
					sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: +%d (+%d)", g_xMsg.GetMsg(1054), HIBYTE(m_stItemInfo.stStdItem.wMAC)+m_stItemInfo.Desc[1], m_stItemInfo.Desc[1]);
					m_stItemState[m_nStateCnt].dwFont = RGB(150, 255, 200);
				}
				else
				{
					sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: +%d", g_xMsg.GetMsg(1054), HIBYTE(m_stItemInfo.stStdItem.wMAC));
					m_stItemState[m_nStateCnt].dwFont = RGB(255, 255, 255);
				}
				m_nStateCnt++;
			}

			// ���� �ı�
			if (( m_stItemInfo.Desc[11] > 0 ) || (m_stItemInfo.stStdItem.bAtomDCType> 0))
			{
				CHAR szTemp[MAX_PATH], szTemp2[MAX_PATH];

				sprintf(szTemp2, "%s:", g_xMsg.GetMsg(5178));
				m_stItemState[m_nStateCnt].dwFont = RGB(128, 255, 0);

				byte btAtomType = m_stItemInfo.stStdItem.bAtomDCType;

				if ( btAtomType != m_stItemInfo.Desc[11] && btAtomType != 0 )
				{
					CHAR szTemp3[MAX_PATH];
					sprintf(szTemp3, " %s+%d", g_xMsg.GetMsg(btAtomType+5160), m_stItemInfo.stStdItem.wAtomDC);
					strcat(szTemp2, szTemp3);
				}

				if (m_stItemInfo.Desc[11] > 0)
					btAtomType = m_stItemInfo.Desc[11];

				if ( m_stItemInfo.Desc[12] > 0 )
				{
					if ( btAtomType == m_stItemInfo.stStdItem.bAtomDCType )
						sprintf(szTemp, " %s+%d", g_xMsg.GetMsg(btAtomType+5160), m_stItemInfo.stStdItem.wAtomDC+m_stItemInfo.Desc[12]);
					else
						sprintf(szTemp, " %s+%d", g_xMsg.GetMsg(btAtomType+5160), m_stItemInfo.Desc[12]);

					strcat(szTemp2, szTemp);
				}
				
				m_stItemState[m_nStateCnt].dwFont = RGB(128, 255, 0);
				sprintf(m_stItemState[m_nStateCnt].pszDesc, szTemp2);

				if (bUseEquip)
					g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[btAtomType+4] = TRUE;

				m_nStateCnt++;
			}


			// ���ѿ���
			if ( m_stItemInfo.Desc[5] > 0 || m_stItemInfo.stStdItem.sAtomACGood )
			{
				INT nMask = 1;
				CHAR szTemp[MAX_PATH], szTemp2[MAX_PATH];
				BOOL bFind = FALSE;

				sprintf(szTemp2, "%s:", g_xMsg.GetMsg(5179));

				for ( INT nI=0; nI<7; nI++)
				{
					BOOL bCheck = FALSE;
					if ( m_stItemInfo.Desc[5] & nMask )
					{
						bCheck = TRUE;
						bFind = TRUE;
						if ( m_stItemInfo.stStdItem.sAtomACGood == nI+1 )
							sprintf(szTemp, " %sx2", g_xMsg.GetMsg(5161+nI));
						else
							sprintf(szTemp, " %sx1", g_xMsg.GetMsg(5161+nI));

						if (bUseEquip)
							g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[12+nI] = TRUE;

					}
					else if ( m_stItemInfo.stStdItem.sAtomACGood == nI+1 )
					{
						bCheck = TRUE;
						bFind = TRUE;
						sprintf(szTemp, " %sx1", g_xMsg.GetMsg(5161+nI));

						if (bUseEquip)
							g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[12+nI] = TRUE;
					}

					if ( bCheck )
						strcat(szTemp2, szTemp);

					nMask <<= 1;
				}

				if ( bFind )
				{
					sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s", szTemp2);
					m_stItemState[m_nStateCnt].dwFont = RGB(128, 255, 0);
					m_nStateCnt++;
				}
			}
			
			// ���ѿ���
			if ( m_stItemInfo.Desc[6] > 0 || m_stItemInfo.stStdItem.sAtomACBad )
			{
				INT nMask = 1;
				CHAR szTemp[MAX_PATH], szTemp2[MAX_PATH];
				BOOL bFind = FALSE;

				sprintf(szTemp2, "%s:", g_xMsg.GetMsg(5180));

				for ( INT nI=0; nI<7; nI++)
				{
					BOOL bCheck = FALSE;
					if ( m_stItemInfo.Desc[6] & nMask )
					{
						bCheck = TRUE;
						bFind = TRUE;
						if ( m_stItemInfo.stStdItem.sAtomACBad == nI+1 )
							sprintf(szTemp, " %sx2", g_xMsg.GetMsg(5161+nI));
						else
							sprintf(szTemp, " %sx1", g_xMsg.GetMsg(5161+nI));

						if (bUseEquip)
							g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[19+nI] = TRUE;

					}
					else if ( m_stItemInfo.stStdItem.sAtomACBad == nI+1 )
					{
						bCheck = TRUE;
						bFind = TRUE;
						sprintf(szTemp, " %sx1", g_xMsg.GetMsg(5161+nI));

						if (bUseEquip)
							g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[19+nI] = TRUE;
					}

					if ( bCheck )
						strcat(szTemp2, szTemp);

					nMask <<= 1;
				}

				if ( bFind )
				{
					sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s", szTemp2);
					m_stItemState[m_nStateCnt].dwFont = RGB(150, 255, 200);
					m_nStateCnt++;
				}
			}
			break;
		}
		case 21:				// �����
		{
			if ( HIBYTE(m_stItemInfo.stStdItem.wAC) + m_stItemInfo.Desc[0] > 0 )
			{
				m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
				if ( m_stItemInfo.Desc[0] > 0 )
				{
					sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: +%d%% (+%d%%)", g_xMsg.GetMsg(1058), (HIBYTE(m_stItemInfo.stStdItem.wAC)+m_stItemInfo.Desc[0])*10, m_stItemInfo.Desc[0]*10);
					m_stItemState[m_nStateCnt].dwFont = RGB(150, 255, 200);
				}
				else
				{
					sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: +%d%%", g_xMsg.GetMsg(1058), HIBYTE(m_stItemInfo.stStdItem.wAC)*10);
					m_stItemState[m_nStateCnt].dwFont = RGB(255, 255, 255);
				}
				m_nStateCnt++;
			}
			if ( HIBYTE(m_stItemInfo.stStdItem.wMAC) + m_stItemInfo.Desc[1] > 0 )
			{
				m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
				if ( m_stItemInfo.Desc[1] > 0 )
				{
					sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: +%d%% (+%d%%)", g_xMsg.GetMsg(1059), (HIBYTE(m_stItemInfo.stStdItem.wMAC)+m_stItemInfo.Desc[1])*10, m_stItemInfo.Desc[1]*10);
					m_stItemState[m_nStateCnt].dwFont = RGB(150, 255, 200);
				}
				else
				{
					sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: +%d%%", g_xMsg.GetMsg(1059), HIBYTE(m_stItemInfo.stStdItem.wMAC)*10);
					m_stItemState[m_nStateCnt].dwFont = RGB(255, 255, 255);
				}
				m_nStateCnt++;
			}

			// ���� ����
			if ( ( m_stItemInfo.Desc[5] > 0 && m_stItemInfo.Desc[6] > 0 ) || 
				 ( m_stItemInfo.Desc[11] > 0 ) || (m_stItemInfo.stStdItem.bAtomDCType> 0) )
			{
				CHAR szTemp[MAX_PATH], szTemp2[MAX_PATH];
				BOOL bFind = FALSE;

				sprintf(szTemp2, "%s:", g_xMsg.GetMsg(5178));

				if (( m_stItemInfo.Desc[11] > 0 ) || (m_stItemInfo.stStdItem.bAtomDCType> 0))
				{
					m_stItemState[m_nStateCnt].dwFont = RGB(128, 255, 0);

					byte btAtomType = m_stItemInfo.stStdItem.bAtomDCType;

					if ( btAtomType != m_stItemInfo.Desc[11] && btAtomType != 0 )
					{
						CHAR szTemp3[MAX_PATH];
						sprintf(szTemp3, " %s+%d", g_xMsg.GetMsg(btAtomType+5160), m_stItemInfo.stStdItem.wAtomDC);
						strcat(szTemp2, szTemp3);
					}

					if (m_stItemInfo.Desc[11] > 0)
						btAtomType = m_stItemInfo.Desc[11];

					if ( m_stItemInfo.Desc[12] > 0 )
					{
						bFind = TRUE;
						if ( btAtomType == m_stItemInfo.stStdItem.bAtomDCType )
							sprintf(szTemp, " %s+%d", g_xMsg.GetMsg(btAtomType+5160), m_stItemInfo.stStdItem.wAtomDC+m_stItemInfo.Desc[12]);
						else
							sprintf(szTemp, " %s+%d", g_xMsg.GetMsg(btAtomType+5160), m_stItemInfo.Desc[12]);
					}

					if (bUseEquip)
						g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[btAtomType+4] = TRUE;
				}

				if ( bFind )
					strcat(szTemp2, szTemp);

				bFind = FALSE;

				m_stItemState[m_nStateCnt].dwFont = RGB(128, 255, 0);

				BYTE btAtomType = m_stItemInfo.Desc[5];

				if ( m_stItemInfo.Desc[6] > 0 )
				{
					bFind = TRUE;
					sprintf(szTemp, " %s+%d", g_xMsg.GetMsg(btAtomType+5160), m_stItemInfo.Desc[6]);
				
					if (bUseEquip)
						g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[btAtomType+4] = TRUE;
				}

				if ( bFind )
					strcat(szTemp2, szTemp);

				m_stItemState[m_nStateCnt].dwFont = RGB(128, 255, 0);
				sprintf(m_stItemState[m_nStateCnt].pszDesc, szTemp2);

				m_nStateCnt++;
			}

			// ���ѿ���
			if ( m_stItemInfo.stStdItem.sAtomACGood > 0 && m_stItemInfo.stStdItem.sAtomACGood < 7)
			{
				INT nIdx = m_stItemInfo.stStdItem.sAtomACGood;
				sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %sx1", g_xMsg.GetMsg(5179), g_xMsg.GetMsg(5160+nIdx));
				m_stItemState[m_nStateCnt].dwFont = RGB(128, 255, 0);
				m_nStateCnt++;

				if (bUseEquip)
					g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[nIdx+11] = TRUE;
			}

			// ���ѿ���
			if ( m_stItemInfo.stStdItem.sAtomACBad > 0 && m_stItemInfo.stStdItem.sAtomACBad < 7)
			{
				INT nIdx = m_stItemInfo.stStdItem.sAtomACBad;
				sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %sx1", g_xMsg.GetMsg(5180), g_xMsg.GetMsg(5160+nIdx));
				m_stItemState[m_nStateCnt].dwFont = RGB(150, 255, 200);
				m_nStateCnt++;

				if (bUseEquip)
					g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[nIdx+18] = TRUE;
			}

			if ( LOBYTE(m_stItemInfo.stStdItem.wAC) > 0 )
			{
				sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: +%d ", g_xMsg.GetMsg(2505), LOBYTE(m_stItemInfo.stStdItem.wAC));
				m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
				m_stItemState[m_nStateCnt].dwFont = RGB(255, 255, 255);
				m_nStateCnt++;
			}
			if ( LOBYTE(m_stItemInfo.stStdItem.wMAC) > 0 )
			{
				sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: -%d ", g_xMsg.GetMsg(2505), LOBYTE(m_stItemInfo.stStdItem.wMAC));
				m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
				m_stItemState[m_nStateCnt].dwFont = RGB(255, 255, 255);
				m_nStateCnt++;
			}
			break;
		}
		case 23:					// ����(�ߵ�ȸ��?)
		{
			if ( HIBYTE(m_stItemInfo.stStdItem.wAC) + m_stItemInfo.Desc[0] > 0 )
			{
				m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
				if ( m_stItemInfo.Desc[0] > 0 )
				{
					sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: +%d%% (+%d%%)", g_xMsg.GetMsg(1056), (HIBYTE(m_stItemInfo.stStdItem.wAC)+m_stItemInfo.Desc[0])*10, m_stItemInfo.Desc[0]*10) ;
					m_stItemState[m_nStateCnt].dwFont = RGB(150, 255, 200);
				}
				else
				{
					sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: +%d%% ", g_xMsg.GetMsg(1056), HIBYTE(m_stItemInfo.stStdItem.wAC)*10);
					m_stItemState[m_nStateCnt].dwFont = RGB(255, 255, 255);
				}
				m_nStateCnt++;
			}
			if ( HIBYTE(m_stItemInfo.stStdItem.wMAC) + m_stItemInfo.Desc[1] > 0 )
			{
				m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
				if ( m_stItemInfo.Desc[1] > 0 )
				{
					sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: +%d%% (+%d%%)", g_xMsg.GetMsg(1057), (HIBYTE(m_stItemInfo.stStdItem.wMAC)+m_stItemInfo.Desc[1])*10, m_stItemInfo.Desc[1]*10);
					m_stItemState[m_nStateCnt].dwFont = RGB(150, 255, 200);
				}
				else
				{
					sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: +%d%% ", g_xMsg.GetMsg(1057), HIBYTE(m_stItemInfo.stStdItem.wMAC)*10);
					m_stItemState[m_nStateCnt].dwFont = RGB(255, 255, 255);
				}
				m_nStateCnt++;
			}

			// ���� ����
			if ( ( m_stItemInfo.Desc[5] > 0 && m_stItemInfo.Desc[6] > 0 ) || 
				 ( m_stItemInfo.Desc[11] > 0 ) || (m_stItemInfo.stStdItem.bAtomDCType> 0) )
			{
				CHAR szTemp[MAX_PATH], szTemp2[MAX_PATH];
				BOOL bFind = FALSE;

				sprintf(szTemp2, "%s:", g_xMsg.GetMsg(5178));

				if (( m_stItemInfo.Desc[11] > 0 ) || (m_stItemInfo.stStdItem.bAtomDCType> 0))
				{
					m_stItemState[m_nStateCnt].dwFont = RGB(128, 255, 0);

					byte btAtomType = m_stItemInfo.stStdItem.bAtomDCType;

					if ( btAtomType != m_stItemInfo.Desc[11] && btAtomType != 0 )
					{
						CHAR szTemp3[MAX_PATH];
						sprintf(szTemp3, " %s+%d", g_xMsg.GetMsg(btAtomType+5160), m_stItemInfo.stStdItem.wAtomDC);
						strcat(szTemp2, szTemp3);
					}

					if (m_stItemInfo.Desc[11] > 0)
						btAtomType = m_stItemInfo.Desc[11];

					if ( m_stItemInfo.Desc[12] > 0 )
					{
						bFind = TRUE;
						if ( btAtomType == m_stItemInfo.stStdItem.bAtomDCType )
							sprintf(szTemp, " %s+%d", g_xMsg.GetMsg(btAtomType+5160), m_stItemInfo.stStdItem.wAtomDC+m_stItemInfo.Desc[12]);
						else
							sprintf(szTemp, " %s+%d", g_xMsg.GetMsg(btAtomType+5160), m_stItemInfo.Desc[12]);
					}

					if (bUseEquip)
						g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[btAtomType+4] = TRUE;
				}

				if ( bFind )
					strcat(szTemp2, szTemp);

				bFind = FALSE;

				m_stItemState[m_nStateCnt].dwFont = RGB(128, 255, 0);

				BYTE btAtomType = m_stItemInfo.Desc[5];

				if ( m_stItemInfo.Desc[6] > 0 )
				{
					bFind = TRUE;
					sprintf(szTemp, " %s+%d", g_xMsg.GetMsg(btAtomType+5160), m_stItemInfo.Desc[6]);
				
					if (bUseEquip)
						g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[btAtomType+4] = TRUE;
				}

				if ( bFind )
					strcat(szTemp2, szTemp);

				m_stItemState[m_nStateCnt].dwFont = RGB(128, 255, 0);
				sprintf(m_stItemState[m_nStateCnt].pszDesc, szTemp2);

				m_nStateCnt++;
			}

			// ���ѿ���
			if ( m_stItemInfo.stStdItem.sAtomACGood > 0 && m_stItemInfo.stStdItem.sAtomACGood < 7)
			{
				INT nIdx = m_stItemInfo.stStdItem.sAtomACGood;
				sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %sx1", g_xMsg.GetMsg(5179), g_xMsg.GetMsg(5160+nIdx));
				m_stItemState[m_nStateCnt].dwFont = RGB(128, 255, 0);
				m_nStateCnt++;

				if (bUseEquip)
					g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[nIdx+11] = TRUE;
			}

			// ���ѿ���
			if ( m_stItemInfo.stStdItem.sAtomACBad > 0 && m_stItemInfo.stStdItem.sAtomACBad < 7)
			{
				INT nIdx = m_stItemInfo.stStdItem.sAtomACBad;
				sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %sx1", g_xMsg.GetMsg(5180), g_xMsg.GetMsg(5160+nIdx));
				m_stItemState[m_nStateCnt].dwFont = RGB(150, 255, 200);
				m_nStateCnt++;

				if (bUseEquip)
					g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[nIdx+18] = TRUE;
			}

			if ( LOBYTE(m_stItemInfo.stStdItem.wAC) > 0 )
			{
				sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: +%d ", g_xMsg.GetMsg(2505), LOBYTE(m_stItemInfo.stStdItem.wAC));
				m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
				m_stItemState[m_nStateCnt].dwFont = RGB(255, 255, 255);
				m_nStateCnt++;
			}
			if ( LOBYTE(m_stItemInfo.stStdItem.wMAC) > 0 )
			{				
				sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: -%d ", g_xMsg.GetMsg(2505), LOBYTE(m_stItemInfo.stStdItem.wMAC));
				m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
				m_stItemState[m_nStateCnt].dwFont = RGB(255, 255, 255);
				m_nStateCnt++;
			}
			break;
		}
		case 15: case 26:
		{
			// ���.
			m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);

			if ( m_stItemInfo.stStdItem.wAC + m_stItemInfo.Desc[0] > 0 )
			{
				if (m_stItemInfo.Desc[0] > 0)
				{
					// �ɼ� Plus.
					sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d-%d (+%d)", g_xMsg.GetMsg(5177), LOBYTE(m_stItemInfo.stStdItem.wAC), HIBYTE(m_stItemInfo.stStdItem.wAC)+m_stItemInfo.Desc[0], m_stItemInfo.Desc[0]);
					m_stItemState[m_nStateCnt].dwFont = RGB(150, 255, 200);
				}
				else
				{
					sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d-%d", g_xMsg.GetMsg(5177), LOBYTE(m_stItemInfo.stStdItem.wAC), HIBYTE(m_stItemInfo.stStdItem.wAC));
					m_stItemState[m_nStateCnt].dwFont = RGB(255, 255, 255);
				}
				m_nStateCnt++;

				if (bUseEquip)
					g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[1] = TRUE;

			}
			
			// ����.
			if ( m_stItemInfo.stStdItem.wMAC + m_stItemInfo.Desc[1] > 0 )
			{
				if ( m_stItemInfo.Desc[1] > 0)
				{
					// �ɼ� Plus.
					sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d-%d (+%d)", g_xMsg.GetMsg(1406), LOBYTE(m_stItemInfo.stStdItem.wMAC), HIBYTE(m_stItemInfo.stStdItem.wMAC)+m_stItemInfo.Desc[1], m_stItemInfo.Desc[1]);
					m_stItemState[m_nStateCnt].dwFont = RGB(150, 255, 200);
				}
				else
				{
					sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d-%d", g_xMsg.GetMsg(1406), LOBYTE(m_stItemInfo.stStdItem.wMAC), HIBYTE(m_stItemInfo.stStdItem.wMAC));
					m_stItemState[m_nStateCnt].dwFont = RGB(255, 255, 255);
				}
				m_nStateCnt++;

				if (bUseEquip)
					g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[4] = TRUE;
			}
			
			// ���� �ı�
			if (( m_stItemInfo.Desc[11] > 0 ) || (m_stItemInfo.stStdItem.bAtomDCType> 0))
			{
				CHAR szTemp[MAX_PATH], szTemp2[MAX_PATH];

				sprintf(szTemp2, "%s:", g_xMsg.GetMsg(5178));
				m_stItemState[m_nStateCnt].dwFont = RGB(128, 255, 0);

				byte btAtomType = m_stItemInfo.stStdItem.bAtomDCType;

				if ( btAtomType != m_stItemInfo.Desc[11] && btAtomType != 0 )
				{
					CHAR szTemp3[MAX_PATH];
					sprintf(szTemp3, " %s+%d", g_xMsg.GetMsg(btAtomType+5160), m_stItemInfo.stStdItem.wAtomDC);
					strcat(szTemp2, szTemp3);
				}

				if (m_stItemInfo.Desc[11] > 0)
					btAtomType = m_stItemInfo.Desc[11];

				if ( m_stItemInfo.Desc[12] > 0 )
				{
					if ( btAtomType == m_stItemInfo.stStdItem.bAtomDCType )
						sprintf(szTemp, " %s+%d", g_xMsg.GetMsg(btAtomType+5160), m_stItemInfo.stStdItem.wAtomDC+m_stItemInfo.Desc[12]);
					else
						sprintf(szTemp, " %s+%d", g_xMsg.GetMsg(btAtomType+5160), m_stItemInfo.Desc[12]);
					strcat(szTemp2, szTemp);
				}
				
				m_stItemState[m_nStateCnt].dwFont = RGB(128, 255, 0);
				sprintf(m_stItemState[m_nStateCnt].pszDesc, szTemp2);

				if (bUseEquip)
					g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[btAtomType+4] = TRUE;

				m_nStateCnt++;
			}


			// ���ѿ���
			if ( m_stItemInfo.Desc[5] > 0 || m_stItemInfo.stStdItem.sAtomACGood )
			{
				INT nMask = 1;
				CHAR szTemp[MAX_PATH], szTemp2[MAX_PATH];
				BOOL bFind = FALSE;

				sprintf(szTemp2, "%s:", g_xMsg.GetMsg(5179));

				for ( INT nI=0; nI<7; nI++)
				{
					BOOL bCheck = FALSE;
					if ( m_stItemInfo.Desc[5] & nMask )
					{
						bCheck = TRUE;
						bFind = TRUE;
						if ( m_stItemInfo.stStdItem.sAtomACGood == nI+1 )
							sprintf(szTemp, " %sx2", g_xMsg.GetMsg(5161+nI));
						else
							sprintf(szTemp, " %sx1", g_xMsg.GetMsg(5161+nI));

						if (bUseEquip)
							g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[12+nI] = TRUE;

					}
					else if ( m_stItemInfo.stStdItem.sAtomACGood == nI+1 )
					{
						bCheck = TRUE;
						bFind = TRUE;
						sprintf(szTemp, " %sx1", g_xMsg.GetMsg(5161+nI));

						if (bUseEquip)
							g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[12+nI] = TRUE;
					}

					if ( bCheck )
						strcat(szTemp2, szTemp);

					nMask <<= 1;
				}

				if ( bFind )
				{
					sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s", szTemp2);
					m_stItemState[m_nStateCnt].dwFont = RGB(128, 255, 0);
					m_nStateCnt++;
				}
			}
			
			// ���ѿ���
			if ( m_stItemInfo.Desc[6] > 0 || m_stItemInfo.stStdItem.sAtomACBad )
			{
				INT nMask = 1;
				CHAR szTemp[MAX_PATH], szTemp2[MAX_PATH];
				BOOL bFind = FALSE;

				sprintf(szTemp2, "%s:", g_xMsg.GetMsg(5180));

				for ( INT nI=0; nI<7; nI++)
				{
					BOOL bCheck = FALSE;
					if ( m_stItemInfo.Desc[6] & nMask )
					{
						bCheck = TRUE;
						bFind = TRUE;
						if ( m_stItemInfo.stStdItem.sAtomACBad == nI+1 )
							sprintf(szTemp, " %sx2", g_xMsg.GetMsg(5161+nI));
						else
							sprintf(szTemp, " %sx1", g_xMsg.GetMsg(5161+nI));

						if (bUseEquip)
							g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[19+nI] = TRUE;

					}
					else if ( m_stItemInfo.stStdItem.sAtomACBad == nI+1 )
					{
						bCheck = TRUE;
						bFind = TRUE;
						sprintf(szTemp, " %sx1", g_xMsg.GetMsg(5161+nI));

						if (bUseEquip)
							g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[19+nI] = TRUE;
					}

					if ( bCheck )
						strcat(szTemp2, szTemp);

					nMask <<= 1;
				}

				if ( bFind )
				{
					sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s", szTemp2);
					m_stItemState[m_nStateCnt].dwFont = RGB(150, 255, 200);
					m_nStateCnt++;
				}
			}
			break;
		}
		case 22:
		case 28:	// �������� - �����߰�
		{
			// ���.
			m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);

			if ( m_stItemInfo.stStdItem.wAC + m_stItemInfo.Desc[0] > 0 )
			{
				if (m_stItemInfo.Desc[0] > 0)
				{
					// �ɼ� Plus.
					sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d-%d (+%d)", g_xMsg.GetMsg(5177), LOBYTE(m_stItemInfo.stStdItem.wAC), HIBYTE(m_stItemInfo.stStdItem.wAC)+m_stItemInfo.Desc[0], m_stItemInfo.Desc[0]);
					m_stItemState[m_nStateCnt].dwFont = RGB(150, 255, 200);
				}
				else
				{
					sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d-%d", g_xMsg.GetMsg(5177), LOBYTE(m_stItemInfo.stStdItem.wAC), HIBYTE(m_stItemInfo.stStdItem.wAC));
					m_stItemState[m_nStateCnt].dwFont = RGB(255, 255, 255);
				}
				m_nStateCnt++;

				if (bUseEquip)
					g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[1] = TRUE;

			}
			
			// ����.
			if ( m_stItemInfo.stStdItem.wMAC + m_stItemInfo.Desc[1] > 0 )
			{
				if ( m_stItemInfo.Desc[1] > 0)
				{
					// �ɼ� Plus.
					sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d-%d (+%d)", g_xMsg.GetMsg(1406), LOBYTE(m_stItemInfo.stStdItem.wMAC), HIBYTE(m_stItemInfo.stStdItem.wMAC)+m_stItemInfo.Desc[1], m_stItemInfo.Desc[1]);
					m_stItemState[m_nStateCnt].dwFont = RGB(150, 255, 200);
				}
				else
				{
					sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d-%d", g_xMsg.GetMsg(1406), LOBYTE(m_stItemInfo.stStdItem.wMAC), HIBYTE(m_stItemInfo.stStdItem.wMAC));
					m_stItemState[m_nStateCnt].dwFont = RGB(255, 255, 255);
				}
				m_nStateCnt++;

				if (bUseEquip)
					g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[4] = TRUE;
			}

			// ���� ����
			if ( ( m_stItemInfo.Desc[5] > 0 && m_stItemInfo.Desc[6] > 0 ) || 
				 ( m_stItemInfo.Desc[11] > 0 ) || (m_stItemInfo.stStdItem.bAtomDCType> 0) )
			{
				CHAR szTemp[MAX_PATH], szTemp2[MAX_PATH];
				BOOL bFind = FALSE;

				sprintf(szTemp2, "%s:", g_xMsg.GetMsg(5178));

				if (( m_stItemInfo.Desc[11] > 0 ) || (m_stItemInfo.stStdItem.bAtomDCType> 0))
				{
					m_stItemState[m_nStateCnt].dwFont = RGB(128, 255, 0);

					byte btAtomType = m_stItemInfo.stStdItem.bAtomDCType;

					if ( btAtomType != m_stItemInfo.Desc[11] && btAtomType != 0 )
					{
						CHAR szTemp3[MAX_PATH];
						sprintf(szTemp3, " %s+%d", g_xMsg.GetMsg(btAtomType+5160), m_stItemInfo.stStdItem.wAtomDC);
						strcat(szTemp2, szTemp3);
					}

					if (m_stItemInfo.Desc[11] > 0)
						btAtomType = m_stItemInfo.Desc[11];

					if ( m_stItemInfo.Desc[12] > 0 )
					{
						bFind = TRUE;
						if ( btAtomType == m_stItemInfo.stStdItem.bAtomDCType )
							sprintf(szTemp, " %s+%d", g_xMsg.GetMsg(btAtomType+5160), m_stItemInfo.stStdItem.wAtomDC+m_stItemInfo.Desc[12]);
						else
							sprintf(szTemp, " %s+%d", g_xMsg.GetMsg(btAtomType+5160), m_stItemInfo.Desc[12]);
					}

					if (bUseEquip)
						g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[btAtomType+4] = TRUE;
				}

				if ( bFind )
					strcat(szTemp2, szTemp);

				bFind = FALSE;

				m_stItemState[m_nStateCnt].dwFont = RGB(128, 255, 0);

				BYTE btAtomType = m_stItemInfo.Desc[5];

				if ( m_stItemInfo.Desc[6] > 0 )
				{
					bFind = TRUE;
					sprintf(szTemp, " %s+%d", g_xMsg.GetMsg(btAtomType+5160), m_stItemInfo.Desc[6]);
				
					if (bUseEquip)
						g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[btAtomType+4] = TRUE;
				}

				if ( bFind )
					strcat(szTemp2, szTemp);

				m_stItemState[m_nStateCnt].dwFont = RGB(128, 255, 0);
				sprintf(m_stItemState[m_nStateCnt].pszDesc, szTemp2);

				m_nStateCnt++;
			}

			// ���ѿ���
			if ( m_stItemInfo.stStdItem.sAtomACGood > 0 && m_stItemInfo.stStdItem.sAtomACGood < 7)
			{
				INT nIdx = m_stItemInfo.stStdItem.sAtomACGood;
				sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %sx1", g_xMsg.GetMsg(5179), g_xMsg.GetMsg(5160+nIdx));
				m_stItemState[m_nStateCnt].dwFont = RGB(128, 255, 0);
				m_nStateCnt++;

				if (bUseEquip)
					g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[nIdx+11] = TRUE;
			}

			// ���ѿ���
			if ( m_stItemInfo.stStdItem.sAtomACBad > 0 && m_stItemInfo.stStdItem.sAtomACBad < 7)
			{
				INT nIdx = m_stItemInfo.stStdItem.sAtomACBad;
				sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %sx1", g_xMsg.GetMsg(5180), g_xMsg.GetMsg(5160+nIdx));
				m_stItemState[m_nStateCnt].dwFont = RGB(150, 255, 200);
				m_nStateCnt++;

				if (bUseEquip)
					g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[nIdx+18] = TRUE;
			}
			break;
		}
	}

	// ����
	if ( m_stItemInfo.stStdItem.wMC + m_stItemInfo.Desc[3] > 0 )
	{
		m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
		if ( m_stItemInfo.Desc[3] > 0 )
		{
			// �ɼ� Plus.
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d-%d (+%d)", g_xMsg.GetMsg(5100+m_stItemInfo.stStdItem.MCType), LOBYTE(m_stItemInfo.stStdItem.wMC), HIBYTE(m_stItemInfo.stStdItem.wMC)+m_stItemInfo.Desc[3], m_stItemInfo.Desc[3]);
			m_stItemState[m_nStateCnt].dwFont = RGB(150, 255, 200);
		}
		else
		{
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d-%d ", g_xMsg.GetMsg(5100+m_stItemInfo.stStdItem.MCType), LOBYTE(m_stItemInfo.stStdItem.wMC), HIBYTE(m_stItemInfo.stStdItem.wMC));
			m_stItemState[m_nStateCnt].dwFont = RGB(255, 255, 255);
		}
		m_nStateCnt++;

		if (bUseEquip)
		{
			if (m_stItemInfo.stStdItem.MCType == 0) //���迭
			{
				g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[2] = TRUE;
				g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[3] = TRUE;
			}
			else if (m_stItemInfo.stStdItem.MCType == 1) //�ڿ��迭
			{
				g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[2] = TRUE;
			}
			else // ��ȥ��
			{
				g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[3] = TRUE;
			}
		}
	}

	// 7. �ʿ�ɷ�ġ.
	switch ( m_stItemInfo.stStdItem.bNeed )
	{
		case 0:
		{
			if ( g_xGameProc.m_xMyHero.m_stAbility.bLevel >= m_stItemInfo.stStdItem.bNeedLevel )
			{
			   bUseAble = TRUE;
			}

			if ( m_stItemInfo.stStdItem.bNeedLevel > 100 && g_xGameProc.m_xMyHero.m_stAbility.bLevel >= m_stItemInfo.stStdItem.bNeedLevel - 100 )
			{
			   bUseAble = TRUE;
			}

			if ( m_stItemInfo.stStdItem.bNeedLevel == 0 )
			{
				sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s", g_xMsg.GetMsg(2516));
			}
			else if ( m_stItemInfo.stStdItem.bNeedLevel > 100 )
			{
				sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d (%s)", g_xMsg.GetMsg(5140), m_stItemInfo.stStdItem.bNeedLevel-100, g_xMsg.GetMsg(2517));
			}
			else
			{
				sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d", g_xMsg.GetMsg(5140), m_stItemInfo.stStdItem.bNeedLevel);
			}

			break;
		}
		case 1:
		{
			if ( HIWORD(g_xGameProc.m_xMyHero.m_stAbility.dwDC) >= m_stItemInfo.stStdItem.bNeedLevel )
			{
			   bUseAble = TRUE;
			}
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s%d ", g_xMsg.GetMsg(5141), m_stItemInfo.stStdItem.bNeedLevel);
			break;
		}
		case 2:
		{
			bUseAble = TRUE;
			if ( HIWORD(g_xGameProc.m_xMyHero.m_stAbility.dwMC) < m_stItemInfo.stStdItem.bNeedLevel )
				bUseAble = FALSE;
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s%d ", g_xMsg.GetMsg(5142), m_stItemInfo.stStdItem.bNeedLevel);
			break;
		}
		case 3:
		{
			bUseAble = TRUE;
			if ( HIWORD(g_xGameProc.m_xMyHero.m_stAbility.dwSC) < m_stItemInfo.stStdItem.bNeedLevel )
				bUseAble = FALSE;
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s%d ", g_xMsg.GetMsg(5143), m_stItemInfo.stStdItem.bNeedLevel);
			break;
		}
		case 4:
		{
			bUseAble = TRUE;
			for ( nCnt = 0; nCnt < 7; nCnt++)
			{
				if ( HIBYTE(g_xGameProc.m_xMyHero.m_stAbility.ATOM_DC[nCnt]) < m_stItemInfo.stStdItem.bNeedLevel )
				{
					bUseAble = FALSE;
					break;
				}
			}
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s%d ", g_xMsg.GetMsg(5144), m_stItemInfo.stStdItem.bNeedLevel);
			break;
		}
		case 5:
		{
			if ( HIBYTE(g_xGameProc.m_xMyHero.m_stAbility.ATOM_DC[0]) >= m_stItemInfo.stStdItem.bNeedLevel )
			{
			   bUseAble = TRUE;
			}
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s%d ", g_xMsg.GetMsg(5145), m_stItemInfo.stStdItem.bNeedLevel);
			break;
		}
		case 6:
		{
			if ( HIBYTE(g_xGameProc.m_xMyHero.m_stAbility.ATOM_DC[1]) >= m_stItemInfo.stStdItem.bNeedLevel )
			{
			   bUseAble = TRUE;
			}
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s%d ", g_xMsg.GetMsg(5146), m_stItemInfo.stStdItem.bNeedLevel);
			break;
		}
		case 7:
		{
			if ( HIBYTE(g_xGameProc.m_xMyHero.m_stAbility.ATOM_DC[2]) >= m_stItemInfo.stStdItem.bNeedLevel )
			{
			   bUseAble = TRUE;
			}
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s%d ", g_xMsg.GetMsg(5147), m_stItemInfo.stStdItem.bNeedLevel);
			break;
		}
		case 8:
		{
			if ( HIBYTE(g_xGameProc.m_xMyHero.m_stAbility.ATOM_DC[3]) >= m_stItemInfo.stStdItem.bNeedLevel )
			{
			   bUseAble = TRUE;
			}
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s%d ", g_xMsg.GetMsg(5148), m_stItemInfo.stStdItem.bNeedLevel);

			break;
		}
		case 9:
		{
			if ( HIBYTE(g_xGameProc.m_xMyHero.m_stAbility.ATOM_DC[4]) >= m_stItemInfo.stStdItem.bNeedLevel )
			{
			   bUseAble = TRUE;
			}
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s%d ", g_xMsg.GetMsg(5149), m_stItemInfo.stStdItem.bNeedLevel);

			break;
		}
		case 10:
		{
			if ( HIBYTE(g_xGameProc.m_xMyHero.m_stAbility.ATOM_DC[5]) >= m_stItemInfo.stStdItem.bNeedLevel )
			{
			   bUseAble = TRUE;
			}
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s%d ", g_xMsg.GetMsg(5150), m_stItemInfo.stStdItem.bNeedLevel);

			break;
		}
		case 11:
		{
			if ( HIBYTE(g_xGameProc.m_xMyHero.m_stAbility.ATOM_DC[6]) >= m_stItemInfo.stStdItem.bNeedLevel )
			{
			   bUseAble = TRUE;
			}
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s%d ", g_xMsg.GetMsg(5151), m_stItemInfo.stStdItem.bNeedLevel);

			break;
		}
	}
	m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);

	if ( ( m_stItemInfo.stStdItem.bNeed == 0 && m_stItemInfo.stStdItem.bNeedLevel == 0 ||
		 m_stItemInfo.stStdItem.bNeed == 0 && m_stItemInfo.stStdItem.bNeedLevel > 100 ) && bUseAble )
	{
		m_stItemState[m_nStateCnt].dwFont = RGB(  0, 200,   0);
	}
	else if ( bUseAble )
	{
		m_stItemState[m_nStateCnt].dwFont = RGB(255, 255, 255);
	}
	else
	{
		m_stItemState[m_nStateCnt].dwFont = RGB(255, 0, 0);
	}
	m_nStateCnt++;

	if ( m_stItemInfo.stStdItem.nNeedFame > 0 )
	{
		sprintf(m_stItemState[m_nStateCnt].pszDesc, g_xMsg.GetMsg(2520), m_stItemInfo.stStdItem.nNeedFame);
		m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);

		if ( g_xGameProc.m_xMyHero.m_nFame >= m_stItemInfo.stStdItem.nNeedFame )
			m_stItemState[m_nStateCnt].dwFont = RGB(128, 255, 255);
		else
			m_stItemState[m_nStateCnt].dwFont = RGB(255, 0, 0);

		m_nStateCnt++;
	}
}

VOID CItem::SetEtcStatus(BOOL bUseStore, BOOL bUseEquip)
{
	INT	 nCnt; 
	INT  nDuraMax;
	if ( !bUseStore )
	{
		nDuraMax = m_stItemInfo.nDuraMax;
	}
	else
	{
		nDuraMax = m_stItemInfo.stStdItem.wDuraMax;
	}

	// 1. �̸�.
	if ( strlen(m_stItemInfo.Prefix) )
	{		
		sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s%s %s", g_xMsg.GetMsg(2515), m_stItemInfo.Prefix, m_stItemInfo.stStdItem.szName);
	}
	else
	{
		// �߼����� �̺�Ʈ��.
		CHAR pszEventItm[20];
		for ( INT nCnt = 0; nCnt < 10; nCnt++ )
		{
			sprintf(pszEventItm, "%s%d", g_xMsg.GetMsg(1409), nCnt);
			if ( !strcmp(m_stItemInfo.stStdItem.szName, pszEventItm) )
			{
				sprintf(m_stItemState[m_nStateCnt].pszDesc, g_xMsg.GetMsg(2510));
				break;
			}
		}
		if ( !m_stItemState[m_nStateCnt].pszDesc[0] )
		{
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s%s", g_xMsg.GetMsg(2515), m_stItemInfo.stStdItem.szName);
		}
	}
	m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
	m_stItemState[m_nStateCnt].dwFont = RGB(255, 250,   0);
	m_nStateCnt++;

	// ����.
	if (m_stItemInfo.stStdItem.bThrow == _MULTIPLE_TYPE_ITEM)
	{
		DWORD	dwCount = m_stItemInfo.wAmount_GB;

		sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d", g_xMsg.GetMsg(2501), dwCount / 10);
		m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
		m_stItemState[m_nStateCnt].dwFont = RGB(255, 255, 255);
		m_nStateCnt++;
		sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d", g_xMsg.GetMsg(2503), dwCount);
	}
	else if (m_stItemInfo.stStdItem.bThrow > _MULTIPLE_TYPE_ITEM)			//  ����
	{
		DWORD	dwCount = m_stItemInfo.wAmount_GB;

		sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d", g_xMsg.GetMsg(2501), m_stItemInfo.stStdItem.bWeight * dwCount);
		m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
		m_stItemState[m_nStateCnt].dwFont = RGB(255, 255, 255);
		m_nStateCnt++;
		sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d", g_xMsg.GetMsg(2503), dwCount);
	}
	else
		sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d", g_xMsg.GetMsg(2501), m_stItemInfo.stStdItem.bWeight);

	m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
	m_stItemState[m_nStateCnt].dwFont = RGB(255, 255, 255);
	m_nStateCnt++;


	switch ( m_stItemInfo.stStdItem.bStdMode )
	{
		case 0:			// ����.
		{
			if ( m_stItemInfo.stStdItem.wAC > 0 )
			{
				sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: +%dHP ", g_xMsg.GetMsg(1058), m_stItemInfo.stStdItem.wAC);
				m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
				m_stItemState[m_nStateCnt].dwFont = RGB(255, 255, 255);
				m_nStateCnt++;
			}

			if ( m_stItemInfo.stStdItem.wMAC > 0 )
			{
				sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: +%dMP ", g_xMsg.GetMsg(1059), m_stItemInfo.stStdItem.wMAC);
				m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
				m_stItemState[m_nStateCnt].dwFont = RGB(255, 255, 255);
				m_nStateCnt++;
			}
			break;
		}
		case 4:
		case 51:		// ����å��.
		{
			for ( nCnt = 0; nCnt < 3; nCnt++ )
			{
				if ( nCnt == m_stItemInfo.stStdItem.bShape )
				{
					// å����.
					sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s", g_xMsg.GetMsg(nCnt+5170));

					if ( nCnt != g_xGameProc.m_xMyHero.m_bJob )
					{
						m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
						m_stItemState[m_nStateCnt].dwFont = RGB(255,   0,   0);
					}
					else
					{
						m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
						m_stItemState[m_nStateCnt].dwFont = RGB(255, 255, 255);
					}
					m_nStateCnt++;

					// �ʿ�ɷ�ġ
					if ( m_stItemInfo.stStdItem.wDuraMax == 0 )
						sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %s", g_xMsg.GetMsg(5140), g_xMsg.GetMsg(5182));
					else
						sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d", g_xMsg.GetMsg(5140), m_stItemInfo.stStdItem.wDuraMax);
					
					m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
					if ( g_xGameProc.m_xMyHero.m_stAbility.bLevel >= m_stItemInfo.stStdItem.wDuraMax )
					{
						m_stItemState[m_nStateCnt].dwFont = RGB(255, 255, 255);
					}
					else
					{
						m_stItemState[m_nStateCnt].dwFont = RGB(255, 0, 0);
					}
					m_nStateCnt++;
				}
			}
			break;
		}
		case 25:		// ���� ������.
		{
			if ( m_stItemInfo.stStdItem.bShape != 9 )
			{
				sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d/%d", g_xMsg.GetMsg(2511), RoundFunc(m_stItemInfo.nDura, 10), RoundFunc(nDuraMax, 10));
				m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
				m_stItemState[m_nStateCnt].dwFont = RGB(128, 255, 255);
				m_nStateCnt++;
			}

			if ( m_stItemInfo.stStdItem.bShape == 20 )
			{
				BOOL bUseAble = FALSE;

				if ( g_xGameProc.m_xMyHero.m_stAbility.bLevel >= m_stItemInfo.stStdItem.bNeedLevel )
				{
				   bUseAble = TRUE;
				}

				if ( m_stItemInfo.stStdItem.bNeedLevel > 100 && g_xGameProc.m_xMyHero.m_stAbility.bLevel >= m_stItemInfo.stStdItem.bNeedLevel - 100 )
				{
				   bUseAble = TRUE;
				}

				if ( m_stItemInfo.stStdItem.bNeedLevel == 0 )
				{
					sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s", g_xMsg.GetMsg(2516));
				}
				else if ( m_stItemInfo.stStdItem.bNeedLevel > 100 )
				{
					sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d (%s)", g_xMsg.GetMsg(5140), m_stItemInfo.stStdItem.bNeedLevel-100, g_xMsg.GetMsg(2517));
				}
				else
				{
					sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d", g_xMsg.GetMsg(5140), m_stItemInfo.stStdItem.bNeedLevel);
				}
				m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
				m_stItemState[m_nStateCnt].dwFont = RGB(250, 250, 250);
				m_nStateCnt++;
			}

			break;
		}
		case 30:		// ȶ�Ұ� ��.
		{
			// ����.
			INT nDuraApp	= RoundFunc(m_stItemInfo.nDura, 1000);
			INT nDuraMaxApp = RoundFunc(nDuraMax, 1000);
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d/%d", g_xMsg.GetMsg(2500), RoundFunc(m_stItemInfo.nDura, 1000), RoundFunc(nDuraMax, 1000));
			m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
			if ( nDuraApp <= 1 ) 
			{
				m_stItemState[m_nStateCnt].dwFont = RGB(255,  50,  50);
			}
			else
			{
				m_stItemState[m_nStateCnt].dwFont = RGB(128, 255, 255);
			}
			m_nStateCnt++;
			break;
		}
		case 40:		// ���������(���ⵢ�)
		{
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d/%d", g_xMsg.GetMsg(2512), RoundFunc(m_stItemInfo.nDura, 1000), RoundFunc(nDuraMax, 1000));
			m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
			m_stItemState[m_nStateCnt].dwFont = RGB(128, 255, 255);
			m_nStateCnt++;
			break;
		}
		case 42:		// ���������.
		{
			sprintf(m_stItemState[m_nStateCnt].pszDesc, g_xMsg.GetMsg(2513));
			m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
			m_stItemState[m_nStateCnt].dwFont = RGB(255, 255, 255);
			m_nStateCnt++;
			break;
		}
		case 43:		// ����������.
		{
			sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d", g_xMsg.GetMsg(2514), RoundFunc(m_stItemInfo.nDura, 1000));
			m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
			m_stItemState[m_nStateCnt].dwFont = RGB(128, 255, 255);
			m_nStateCnt++;
			break;
		}
		case 60:
		{
			if ( m_stItemInfo.stStdItem.bShape == 3 )
			{
				// ü��������
				m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
				m_stItemState[m_nStateCnt].dwFont = RGB(150, 255, 200);
				sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: +%d", g_xMsg.GetMsg(2518), m_stItemInfo.stStdItem.wAC*m_stItemInfo.nDura/nDuraMax );
				m_nStateCnt++;

				// ����������
				m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
				m_stItemState[m_nStateCnt].dwFont = RGB(150, 255, 200);
				sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: +%d", g_xMsg.GetMsg(2519), m_stItemInfo.stStdItem.wMAC*m_stItemInfo.nDura/nDuraMax );
				m_nStateCnt++;
			}
			else
			{
				// 4. ���.
				m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
				if ( m_stItemInfo.Desc[0] > 0 )
				{
					// �ɼ� Plus.               
					sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d-%d (+%d)", g_xMsg.GetMsg(5177), LOBYTE(m_stItemInfo.stStdItem.wAC), HIBYTE(m_stItemInfo.stStdItem.wAC)+m_stItemInfo.Desc[0], m_stItemInfo.Desc[0]);
					m_stItemState[m_nStateCnt].dwFont = RGB(150, 255, 200);
				}
				else
				{
					sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d-%d", g_xMsg.GetMsg(5177), LOBYTE(m_stItemInfo.stStdItem.wAC), HIBYTE(m_stItemInfo.stStdItem.wAC));
					m_stItemState[m_nStateCnt].dwFont = RGB(255, 255, 255);
				}
				m_nStateCnt++;

				if (bUseEquip)
					g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[1] = TRUE;
				
				// �������
				if ( m_stItemInfo.stStdItem.wMAC + m_stItemInfo.Desc[1] > 0 )
				{
					if ( m_stItemInfo.Desc[1] > 0)
					{
						// �ɼ� Plus.
						sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d-%d (+%d)", g_xMsg.GetMsg(1406), LOBYTE(m_stItemInfo.stStdItem.wMAC), HIBYTE(m_stItemInfo.stStdItem.wMAC)+m_stItemInfo.Desc[1], m_stItemInfo.Desc[1]);
						m_stItemState[m_nStateCnt].dwFont = RGB(150, 255, 200);
					}
					else
					{
						sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d-%d", g_xMsg.GetMsg(1406), LOBYTE(m_stItemInfo.stStdItem.wMAC), HIBYTE(m_stItemInfo.stStdItem.wMAC));
						m_stItemState[m_nStateCnt].dwFont = RGB(255, 255, 255);
					}
					m_nStateCnt++;
					if (bUseEquip)
						g_xGameProc.m_xInterface.m_xStatusWnd.m_bCheckShow[4] = TRUE;
				}

				if ( LOBYTE(m_stItemInfo.stStdItem.bNeedLevel) > 0 )
				{
					BOOL bUseAble = FALSE;
					// �ʿ䷹��.
					if ( g_xGameProc.m_xMyHero.m_stAbility.bLevel >= LOBYTE(m_stItemInfo.stStdItem.bNeedLevel) )
					{
					   bUseAble = TRUE;
					}

					sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d", g_xMsg.GetMsg(5140), LOBYTE(m_stItemInfo.stStdItem.bNeedLevel));

					if ( bUseAble )
					{
						m_stItemState[m_nStateCnt].dwFont = RGB(255, 255, 255);
					}
					else
					{
						m_stItemState[m_nStateCnt].dwFont = RGB(255, 0, 0);
					}
					m_nStateCnt++;
				}
			}
			break;
		}
		case 3:			// Ư���ѹ��� OR ����������.
		{
			if ( m_stItemInfo.stStdItem.bShape == 13 || m_stItemInfo.stStdItem.bShape == 15 ||
				 m_stItemInfo.stStdItem.bShape == 12 )
			{
				BOOL bUseAble = FALSE;
				// 6. �ʿ䷹��.
				if ( g_xGameProc.m_xMyHero.m_stAbility.bLevel >= LOBYTE(m_stItemInfo.stStdItem.bNeedLevel) )
				{
				   bUseAble = TRUE;
				}

				if ( LOBYTE(m_stItemInfo.stStdItem.bNeedLevel) == 0 )
				{
					sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s", g_xMsg.GetMsg(2516));
				}
				else
				{
					sprintf(m_stItemState[m_nStateCnt].pszDesc, "%s: %d", g_xMsg.GetMsg(5140), LOBYTE(m_stItemInfo.stStdItem.bNeedLevel));
				}

				if ( bUseAble )
				{
					m_stItemState[m_nStateCnt].dwFont = RGB(255, 255, 255);
				}
				else
				{
					m_stItemState[m_nStateCnt].dwFont = RGB(255, 0, 0);
				}
				m_nStateCnt++;
			}
		}
		break;
		case 1:			// ��������.
		case 2:			// �ָԹ�.
		case 31:		// ���������ø���.
		case 44:		// �������� ���� Ư�� ������.
		case 45:		// ���� �������� �ٲ�� ��ɼ� ������.
		case 46:		// ����� ���� �ܼ�������.
		default:
		{
			break;
		}
	}

	if ( m_stItemInfo.stStdItem.nNeedFame > 0 )
	{
		sprintf(m_stItemState[m_nStateCnt].pszDesc, g_xMsg.GetMsg(2520), m_stItemInfo.stStdItem.nNeedFame);
		m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);

		if ( g_xGameProc.m_xMyHero.m_nFame >= m_stItemInfo.stStdItem.nNeedFame )
			m_stItemState[m_nStateCnt].dwFont = RGB(128, 255, 255);
		else
			m_stItemState[m_nStateCnt].dwFont = RGB(255, 0, 0);

		m_nStateCnt++;
	}
}

VOID CItem::ShowItemStatus(INT nX, INT nY, BOOL bUseStore, BOOL bClothShow, BOOL bUseEquip)
{
	INT		nCnt;
	SIZE	sizeBoard = {0, 0};
	SIZE	sizeLen	  = {0, 0};

	SetItemStatus(bUseStore, bUseEquip);

	for ( nCnt = 0; nCnt < m_nStateCnt; nCnt++ )
	{
		if ( m_stItemState[nCnt].pszDesc[0] != '\0' )
		{
			sizeLen = g_xMainWnd.GetStrLength(NULL, NULL, m_stItemState[nCnt].pszDesc);

			if ( sizeLen.cx > sizeBoard.cx )
			{
				sizeBoard.cx = sizeLen.cx;
			}
			sizeBoard.cy += 15;//sizeLen.cy;
		}
	}

	RECT rcB = {nX-5, nY-5, nX+sizeBoard.cx+5, nY+sizeBoard.cy+2};

	if ( bClothShow )
	{
		CWHWilImageData* pxItemImg = &g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY];
		if ( pxItemImg->NewSetIndex(m_stItemInfo.stStdItem.wLooks) )
		{
			rcB.right += pxItemImg->m_lpstNewCurrWilImageInfo->shWidth + 10;
			if ( (rcB.bottom - rcB.top) < pxItemImg->m_lpstNewCurrWilImageInfo->shHeight )
			{
				rcB.bottom = rcB.top+pxItemImg->m_lpstNewCurrWilImageInfo->shHeight + 10;
			}

		}
	}

	INT  nW  = rcB.right - rcB.left;
	INT  nH  = rcB.bottom - rcB.top;

	RECT rc = rcB;

	if ( rcB.right > 800 )
	{
		INT nWGap = rcB.right - 800;
		SetRect(&rc, rcB.left - nWGap, rcB.top, rcB.right - nWGap, rcB.bottom);
	}

	// ���� �׸���.
	D3DVECTOR	 vecTrans((FLOAT)(rc.left), (FLOAT)(rc.top), 0);
	D3DVECTOR	 vecScale((FLOAT)(nW), (FLOAT)(nH), 1);
	D3DMATERIAL9 mtrl;

	D3DUtil_InitMaterial(mtrl, (FLOAT)100/255.0f, (FLOAT)100/255.0f, (FLOAT)50/255.0f);
	mtrl.Diffuse.a = 230.0f/255.0f;
	g_xGameProc.m_xImage.DrawBillBoard(g_xMainWnd.Get3DDevice(), &vecTrans, &vecScale, &mtrl, NULL);

	g_xMainWnd.DrawWithGDI(&rc, NULL, RGB(150, 150, 50), 1);

	INT nClothSX = 0; 
	INT nClothSY = 0; 
	INT nClothWidth  = 0; 
	INT nClothHeight = 0;

	if ( bClothShow )
	{
		CWHWilImageData* pxItemImg = &g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY];
		if ( pxItemImg->NewSetIndex(m_stItemInfo.stStdItem.wLooks) )
		{
			nClothWidth  = pxItemImg->m_lpstNewCurrWilImageInfo->shWidth;
			nClothHeight = pxItemImg->m_lpstNewCurrWilImageInfo->shHeight;
			nClothSX = 5;
			nClothSY = ((rc.bottom - rc.top)-nClothHeight)/2;

			WORD wClr = g_xMainWnd.ConvertColor24To16(RGB(m_stItemInfo.bLevel_R, LOBYTE(m_stItemInfo.wAmount_GB), HIBYTE(m_stItemInfo.wAmount_GB)));
			g_xMainWnd.DrawWithImageForCompClipRgn(rc.left+nClothSX, rc.top+nClothSY, pxItemImg->m_lpstNewCurrWilImageInfo->shWidth, pxItemImg->m_lpstNewCurrWilImageInfo->shHeight,
												   (WORD*)pxItemImg->m_pbCurrImage, _CLIP_WIDTH, _CLIP_HEIGHT, wClr, 0XFFFF, TRUE);
		}
	}

	// �����ۻ��� �ؽ�Ʈ.
	for ( nCnt = 0; nCnt < m_nStateCnt; nCnt++ )
	{
		if ( m_stItemState[nCnt].pszDesc[0] != '\0' )
		{
			g_xMainWnd.PutsHan(NULL, rc.left+5+nClothSX+nClothWidth, rc.top+5+nCnt*15, m_stItemState[nCnt].dwFont, m_stItemState[nCnt].dwBack, m_stItemState[nCnt].pszDesc);	
		}
	}
}

VOID
CItem::SetItemDesc( BOOL bUseStore )
{
	if( bUseStore )
	{
		if( m_stItemInfo.ExpireTime == 0 )
		{
			sprintf( m_stItemState[ m_nStateCnt ].pszDesc, g_xMsg.GetMsg( 7000 ) );
		}
		else
		{
			// �ð����� ���� 2006.10.26

			INT nTime	= m_stItemInfo.ExpireTime;
			INT nDay	= nTime / 24;
			INT nHour	= nTime % 24;

			sprintf( m_stItemState[ m_nStateCnt ].pszDesc, g_xMsg.GetMsg( 7001 ), nDay, nHour );
		}
		
		m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
		m_stItemState[m_nStateCnt].dwFont = RGB(255, 250,   0);
		m_nStateCnt++;

		// ������ ���� �߰�
		// �ٹٲ� �߰� 2006.10.23
		CHAR* pszMsg = NULL;
 		pszMsg = g_xItemMsg.GetMsg( m_nSellIndex );
		if( pszMsg )
		{
			INT nLen = strlen( pszMsg );
			INT nIdx = 0;
			CHAR szSplit[ 50 ];

			for( INT n = 0; n < nLen; ++n )
			{
				if( pszMsg[ n ] == '_' )	// �ٹٲ�
				{
					szSplit[ nIdx ] = '\0';
					
					sprintf( m_stItemState[ m_nStateCnt ].pszDesc, szSplit );
					m_stItemState[ m_nStateCnt ].dwBack = RGB( 0, 0, 0 );
					m_stItemState[ m_nStateCnt ].dwFont = RGB( 255, 255, 255 );
					++m_nStateCnt;

					nIdx = 0;
				}
				else
				{
					szSplit[ nIdx++ ] = pszMsg[ n ];

					if( nIdx >= 49 )
					{
						szSplit[ nIdx ] = '\0';

						// Ȥ�ó� �ؼ� üũ�غ���. �ٹٲ��� �߸� ����� ��츦 ���ؼ�
						if( pszMsg[ n ] < 0 )
						{
							if( n != 0 && pszMsg[ n - 1 ] < 0 )
							{
							}
							else
							{
								--n;
								nIdx = nIdx - 2;
								szSplit[ nIdx ] = '\0';
							}
						}
						
						sprintf( m_stItemState[ m_nStateCnt ].pszDesc, szSplit );
						m_stItemState[ m_nStateCnt ].dwBack = RGB( 0, 0, 0 );
						m_stItemState[ m_nStateCnt ].dwFont = RGB( 255, 255, 255 );
						++m_nStateCnt;
						
						nIdx = 0;
					}
				}
			}

			// ������ �κ��� ó���Ѵ�
			if( nIdx > 0 )
			{
				szSplit[ nIdx ] = '\0';

				sprintf( m_stItemState[ m_nStateCnt ].pszDesc, szSplit );
				m_stItemState[ m_nStateCnt ].dwBack = RGB( 0, 0, 0 );
				m_stItemState[ m_nStateCnt ].dwFont = RGB( 255, 255, 255 );
				++m_nStateCnt;
				
				nIdx = 0;
			}
		}
	}
	else
	{
		// 606281900 - 2006�� 6�� 28�� 19�ñ���
		if( m_stItemInfo.ExpireTime <= 0 )
		{
			sprintf( m_stItemState[ m_nStateCnt ].pszDesc, g_xMsg.GetMsg( 7002 ) );
		}
		// ���� ������ ���� �������� ��� ������ڸ� ǥ�����ش�
		// ����ǰ� ���� �������ڰ� ���´�
		else if( m_stItemInfo.ExpireTime > 0 && m_stItemInfo.ExpireTime < 1000 )
		{
			// �ð����� ���� 2006.10.26

			INT nTime	= m_stItemInfo.ExpireTime;
			INT nDay	= nTime / 24;
			INT nHour	= nTime % 24;

			sprintf( m_stItemState[ m_nStateCnt ].pszDesc, g_xMsg.GetMsg( 7001 ), nDay, nHour );
		}
		else
		{
			INT nTime	= m_stItemInfo.ExpireTime;
			INT nYear	= nTime / 100000000;
			INT nMonth	= ( nTime - 100000000 * nYear ) / 1000000;
			INT nDay	= ( nTime - 100000000 * nYear - 1000000 * nMonth ) / 10000;
			INT nHour	= ( nTime - 100000000 * nYear - 1000000 * nMonth - 10000 * nDay ) / 100;
			INT nMin	= ( nTime - 100000000 * nYear - 1000000 * nMonth - 10000 * nDay - 100 * nHour );
			
			sprintf( m_stItemState[ m_nStateCnt ].pszDesc, g_xMsg.GetMsg( 7003 ), nMonth, nDay, nHour, nMin );
		}
		
		m_stItemState[m_nStateCnt].dwBack = RGB(  0,   0,   0);
		m_stItemState[m_nStateCnt].dwFont = RGB(255, 250,   0);
		m_nStateCnt++;
	}
}
