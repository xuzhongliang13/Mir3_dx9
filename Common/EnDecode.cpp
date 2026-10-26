// EnDecode.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"

static unsigned char Decode6BitMask[5] = { 0xfc, 0xf8, 0xf0, 0xe0, 0xc0 };

int WINAPI fnEncode6BitBuf(unsigned char *pszSrc, char *pszDest, int nSrcLen, int nDestLen)
{
	int				nDestPos	= 0;
	int				nRestCount	= 0;
	unsigned char	chMade = 0, chRest = 0;

	for (int i = 0; i < nSrcLen; i++)
	{
		if (nDestPos >= nDestLen) break;
		
		chMade = ((chRest | (pszSrc[i] >> (2 + nRestCount))) & 0x3f);
		chRest = (((pszSrc[i] << (8 - (2 + nRestCount))) >> 2) & 0x3f);

		nRestCount += 2;

		if (nRestCount < 6)
			pszDest[nDestPos++] = chMade + 0x3c;
		else
		{
			if (nDestPos < nDestLen - 1)
			{
				pszDest[nDestPos++]	= chMade + 0x3c;
				pszDest[nDestPos++]	= chRest + 0x3c;
			}
			else
				pszDest[nDestPos++] = chMade + 0x3c;

			nRestCount	= 0;
			chRest		= 0;
		}
	}

	if (nRestCount > 0)
		pszDest[nDestPos++] = chRest + 0x3c;

	pszDest[nDestPos] = '\0';

	return nDestPos;
}

//�̿ 20051031
int WINAPI fnEncode6BitBufGameMessage(unsigned char *pszSrc, char *pszDest, int nSrcLen, int nDestLen)
{
	int				nDestPos	= 0;
	int				nRestCount	= 0;
	unsigned char	chMade = 0, chRest = 0;

	for (int i = 0; i < nSrcLen; i++)
	{
		if (nDestPos >= nDestLen) break;
		
		chMade = ((chRest | ( (pszSrc[i]^24) >> (2 + nRestCount))) & 0x3f);
		chRest = ((((pszSrc[i]^24) << (8 - (2 + nRestCount))) >> 2) & 0x3f);

		nRestCount += 2;

		if (nRestCount < 6)
			pszDest[nDestPos++] = chMade + 0x4a;
		else
		{
			if (nDestPos < nDestLen - 1)
			{
				pszDest[nDestPos++]	= chMade + 0x4a;
				pszDest[nDestPos++]	= chRest + 0x4a;
			}
			else
				pszDest[nDestPos++] = chMade + 0x4a;

			nRestCount	= 0;
			chRest		= 0;
		}
	}

	if (nRestCount > 0)
		pszDest[nDestPos++] = chRest + 0x4a;

	pszDest[nDestPos] = '\0';

	return nDestPos;
}

//�̿ 20051031
int WINAPI fnEncode6BitBufGameString(unsigned char *pszSrc, char *pszDest, int nSrcLen, int nDestLen)
{
	int				nDestPos	= 0;
	int				nRestCount	= 0;
	unsigned char	chMade = 0, chRest = 0;

	for (int i = 0; i < nSrcLen; i++)
	{
		if (nDestPos >= nDestLen) break;
		
		chMade = ((chRest | ( (pszSrc[i]^37) >> (2 + nRestCount))) & 0x3f);
		chRest = ((((pszSrc[i]^37) << (8 - (2 + nRestCount))) >> 2) & 0x3f);

		nRestCount += 2;

		if (nRestCount < 6)
			pszDest[nDestPos++] = chMade + 0x3d;
		else
		{
			if (nDestPos < nDestLen - 1)
			{
				pszDest[nDestPos++]	= chMade + 0x3d;
				pszDest[nDestPos++]	= chRest + 0x3d;
			}
			else
				pszDest[nDestPos++] = chMade + 0x3d;

			nRestCount	= 0;
			chRest		= 0;
		}
	}

	if (nRestCount > 0)
		pszDest[nDestPos++] = chRest + 0x3d;

	pszDest[nDestPos] = '\0';

	return nDestPos;
}


//20050131  �̿
int  WINAPI fnDecode6BitBufGameMessage(char *pszSrc, char *pszDest, int nDestLen)
	{
	int				nLen = strlen((const char *)pszSrc);
	int				nDestPos = 0, nBitPos = 2;
	int				nMadeBit = 0;
	unsigned char	ch, chCode, tmp;

	for (int i = 0; i < nLen; i++)
	{
		//if ((pszSrc[i] - 0x4d) >= 0)
		//{
			ch = pszSrc[i] - 0x4d; //�߰��ڵ�
			ch  = ch & 0x3f;
		//}	
		//else
		//{
		//	nDestPos = 0;
		//	break;
		//}
		if (nDestPos >= nDestLen) break;
		if ((nMadeBit + 6) >= 8)
		{
			chCode = (tmp | ((ch & 0x3f) >> (6 - nBitPos)));
			chCode = chCode ^ 134;      //�߰��ڵ�..
			pszDest[nDestPos++] = chCode;
			nMadeBit = 0;
			if (nBitPos < 6) 
				nBitPos += 2;
			else
			{
				nBitPos = 2;
				continue;
			}
		}
		
		tmp = ((ch << nBitPos) & Decode6BitMask[nBitPos - 2]);

		nMadeBit += (8 - nBitPos);
	}


	return nDestPos;
}

//�̿ 20051031
int  WINAPI fnDecode6BitBufOriginal(char *pszSrc, char *pszDest, int nDestLen)
{
	int				nLen = strlen((const char *)pszSrc);
	int				nDestPos = 0, nBitPos = 2;
	int				nMadeBit = 0;
	unsigned char	ch, chCode, tmp;

	for (int i = 0; i < nLen; i++)
	{
		if ((pszSrc[i] - 0x3c) >= 0)
			ch = pszSrc[i] - 0x3c;
		else
		{
			nDestPos = 0;
			break;
		}

		if (nDestPos >= nDestLen) break;

		if ((nMadeBit + 6) >= 8)
		{
			chCode = (tmp | ((ch & 0x3f) >> (6 - nBitPos)));
			pszDest[nDestPos++] = chCode;

			nMadeBit = 0;

			if (nBitPos < 6) 
				nBitPos += 2;
			else
			{
				nBitPos = 2;
				continue;
			}
		}

		tmp = ((ch << nBitPos) & Decode6BitMask[nBitPos - 2]);

		nMadeBit += (8 - nBitPos);
	}

//	pszDest[nDestPos] = '\0';

	return nDestPos;
}

//20051031
int  WINAPI fnDecode6BitBuf(char *pszSrc, char *pszDest, int nDestLen)
{
	int				nLen = strlen((const char *)pszSrc);
	int				nDestPos = 0, nBitPos = 2;
	int				nMadeBit = 0;
	unsigned char	ch, chCode, tmp;

	for (int i = 0; i < nLen; i++)
	{
		if ((pszSrc[i] - 0x3c) >= 0)
			ch = pszSrc[i] - 0x3c;
		else
		{
			nDestPos = 0;
			break;
		}

		if (nDestPos >= nDestLen) break;

		if ((nMadeBit + 6) >= 8)
		{
			chCode = (tmp | ((ch & 0x3f) >> (6 - nBitPos)));
			pszDest[nDestPos++] = chCode;

			nMadeBit = 0;

			if (nBitPos < 6) 
				nBitPos += 2;
			else
			{
				nBitPos = 2;
				continue;
			}
		}

		tmp = ((ch << nBitPos) & Decode6BitMask[nBitPos - 2]);

		nMadeBit += (8 - nBitPos);
	}

//	pszDest[nDestPos] = '\0';

	return nDestPos;

/*	int				nLen = strlen((const char *)pszSrc);
	int				nDestPos = 0, nBitPos = 2;
	int				nMadeBit = 0;
	unsigned char	ch, chCode, tmp;

	for (int i = 0; i < nLen; i++)
	{
		//if ((pszSrc[i] - 0x3c) >= 0)
			ch = pszSrc[i] - 0x3e;
			ch  = ch & 0x3f;
		//else
		//{
		//	nDestPos = 0;
		//	break;
		//}

		if (nDestPos >= nDestLen) break;

		if ((nMadeBit + 6) >= 8)
		{
			chCode = (tmp | ((ch & 0x3f) >> (6 - nBitPos)));
			chCode = chCode ^ 35;
			pszDest[nDestPos++] = chCode;

			nMadeBit = 0;

			if (nBitPos < 6) 
				nBitPos += 2;
			else
			{
				nBitPos = 2;
				continue;
			}
		}
		
		tmp = ((ch << nBitPos) & Decode6BitMask[nBitPos - 2]);
		

		nMadeBit += (8 - nBitPos);
	}

//	pszDest[nDestPos] = '\0';

	return nDestPos;
*/
}


int WINAPI fnEncodeMessage(_LPTDEFAULTMESSAGE lptdm, char *pszBuf, int nLen)
{
	unsigned char	btBuffer[32];

	memcpy(btBuffer, (void *)lptdm, sizeof(_TDEFAULTMESSAGE));

	//return fnEncode6BitBuf(btBuffer, pszBuf, sizeof(_TDEFAULTMESSAGE), nLen);

	

	//2000-2004 : login message..
	if((lptdm->wIdent >= 2000)&&(lptdm->wIdent <= 2004))
	{
		return fnEncode6BitBuf(btBuffer, pszBuf, sizeof(_TDEFAULTMESSAGE), nLen);
	}
	//100-104 : select character message..
	else if((lptdm->wIdent >= 100)&&(lptdm->wIdent <= 104))
	{
		return fnEncode6BitBuf(btBuffer, pszBuf, sizeof(_TDEFAULTMESSAGE), nLen);
	}
	//else : game message
	else
	{
		// [FIX-崩溃][2026-10-04] 原代码 else 分支调用 fnEncode6BitBuf 后无 return，
		// 编译后生成 ud2(0x0F0B) 非法指令(反汇编 0x4123C1)。公告确认 CM_LOGINNOTICEOK(1018)
		// 不在 [2000-2004]/[100-104] 区间，走 else 分支时必然撞 ud2 崩溃(0xC000001D)。
		// 修复：补 return，与上两个分支一致（正常客户端 1018 头=16B 6bit 编码，见 1.apmx86 抓包）。
		return fnEncode6BitBuf(btBuffer, pszBuf, sizeof(_TDEFAULTMESSAGE), nLen);
	}
}

int  WINAPI fnDecodeMessage(_LPTDEFAULTMESSAGE lptdm, char *pszBuf)
	{ return fnDecode6BitBufOriginal(pszBuf, (char *)lptdm, sizeof(_TDEFAULTMESSAGE)); }

//�̿ 20051031
int  WINAPI fnDecodeMessageGameProcess(_LPTDEFAULTMESSAGE lptdm, char *pszBuf)
	{ return fnDecode6BitBufGameMessage(pszBuf, (char *)lptdm, sizeof(_TDEFAULTMESSAGE)); }

void WINAPI fnMakeDefMessage(_LPTDEFAULTMESSAGE lptdm, WORD wIdent, int nRecog, WORD wParam, WORD wTag, WORD wSeries)
	{ lptdm->wIdent	= wIdent; lptdm->nRecog	= nRecog; lptdm->wParam	= wParam; lptdm->wTag = wTag; lptdm->wSeries = wSeries; }

