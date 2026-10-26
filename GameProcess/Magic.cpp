#include "StdAfx.h"

#define  UNINIT			-1

CMagic::CMagic()
{
	m_bActiveState	= _MAGIC_FINISH;
	m_bRepeat		= FALSE;
	m_bFixed		= FALSE;
	m_bShowLight	= TRUE;

	m_pxOwner		= NULL;
	m_pxTarget		= NULL;

	m_nScrnX		= m_nScrnY		  = 0;
	m_nTileX		= m_nTileY		  = 0;
	m_nFireScrnX	= m_nFireScrnY	  = 0;
	m_nFireTileX	= m_nFireTileY	  = 0;
	m_nTargetScrnX  = m_nTargetScrnY  = 0;
	m_nTargetTileX  = m_nTargetTileY  = 0;	

	m_wMagicNum		= 0;

	m_nCurrDelay	= 0;	
	m_nFrmDelay		= 0;
	m_dwMagicLife	= 0;

	m_dwFstFrame	= 0;				
	m_dwEndFrame	= 0;
	m_dwCurrFrame	= 0;

	m_bBlendType	= 0;
	m_bOpacity		= 0;
	m_bSwingCnt		= 0;
	m_bDir16		= 0;
	m_wStartDelay   = 0;

	m_nPrevDistanceX = 10000;
	m_nPrevDistanceY = 10000;

	ZeroMemory(m_bLightRadius, 2);
	ZeroMemory(m_bLightColor, 2*3);
	ZeroMemory(m_bMagicColor, 3);

	ZeroMemory(m_szServerNotice, MAX_PATH);

	m_nDuplicateNum	 = MAX_DUPLE_COUNT;

	m_wFileType	 = 0;
	m_wFileIdx	 = 0;
}


CMagic::~CMagic()
{
}


BOOL CMagic::CreateMagic(WORD wMagicNum, INT nFireTileX, INT nFireTileY, 
		                 INT nTargetTileX, INT nTargetTileY, CActor* pxOwner, INT nTarget, WORD wStartDelay, INT nAtom,
						 WORD wRed, WORD wGreen, WORD wBlue)
{
	// �ڱ⿡�� ������� ������ �������� �ʴ´�.
	if ( nFireTileX == nTargetTileX && nFireTileY == nTargetTileY && 
		 (wMagicNum == _SKILL_FIRECHARM|| wMagicNum == _SKILL_FIREBALL|| wMagicNum == _SKILL_FIREBALL2|| wMagicNum == _SKILL_MOONOK|| wMagicNum == _SKILL_ICEBOLT || wMagicNum == _SKILL_SUPERICEBOLT || wMagicNum == _SKILL_MAGICARROW ||
		  wMagicNum == _SKILL_HANDWIND || wMagicNum == _SKILL_DOT_1   || wMagicNum == _SKILL_DOT_2    || wMagicNum == _SKILL_DOT_3 ||
		  wMagicNum == _SKILL_SNOWDART || wMagicNum == _SKILL_CONCENFIREBALL || wMagicNum == _SKILL_MYULSAL ||
		  wMagicNum == _SKILL_BALSACHE) )
	{
		return FALSE;
	}

	LPEFFECTSPRINFO pstMagicSpr = g_xSpriteInfo.GetMagicInfo(wMagicNum);

	if ( pstMagicSpr )
	{
		//------------------------------------------------------------------------------------------------------------
		// ��ǥ����.
		//------------------------------------------------------------------------------------------------------------
		m_pxTarget	= NULL;

		if ( nTarget )
		{
			if ( nTarget == g_xGameProc.m_xMyHero.m_nIdentity )
			{
				m_pxTarget = &g_xGameProc.m_xMyHero;
			}
			else
			{
				CActor* pxActor = NULL;
				CListNode <CActor> *pNode;

				for (pNode = g_xGameProc.m_xActorList.GetHead(); pNode; pNode = pNode->GetNext())
				{
					pxActor = pNode->GetData();

					if ( pxActor )
					{
						if ( pxActor->m_nIdentity == nTarget )
						{
							m_pxTarget = pxActor;
							break;
						}
					}
				}
			}
		}

		// �߻�����.
		m_nFireTileX = nFireTileX;
		m_nFireTileY = nFireTileY;
		g_xGameProc.m_xMap.GetScrnPosFromTilePos(m_nFireTileX, m_nFireTileY, m_nFireScrnX, m_nFireScrnY);

		// ��������.
		m_nTargetTileX = nTargetTileX;
		m_nTargetTileY = nTargetTileY;
		g_xGameProc.m_xMap.GetScrnPosFromTilePos(m_nTargetTileX, m_nTargetTileY, m_nTargetScrnX, m_nTargetScrnY);

		// ������ġ.
		m_nTileX = m_nFireTileX;
		m_nTileY = m_nFireTileY;
		m_nScrnX = m_nFireScrnX;
		m_fFloatScrnX = (FLOAT)m_nFireScrnX;
		m_nScrnY = m_nFireScrnY;
		m_fFloatScrnY = (FLOAT)m_nFireScrnY;
		m_wStartDelay = wStartDelay;

		m_bMagicColor[0] = 255;
		m_bMagicColor[1] = 255;
		m_bMagicColor[2] = 255;

		// �Ÿ�����.
	    INT nScrnGapX, nScrnGapY;
	    if ( m_nFireScrnX != m_nTargetScrnX )
			nScrnGapX = abs(m_nTargetScrnX-m_nFireScrnX);
	    else
			nScrnGapX = 1;

	    if ( m_nFireScrnY != m_nTargetScrnY )
			nScrnGapY = abs(m_nTargetScrnY-m_nFireScrnY);
	    else
			nScrnGapY = 1;

		if ( nScrnGapX == 0 )
		{
			nScrnGapX = 1;
		}
		if ( nScrnGapY == 0 )
		{
			nScrnGapY = 1;
		}
	    FLOAT fGapRateX = (FLOAT)(500/(FLOAT)nScrnGapX);
	    FLOAT fGapRateY = (FLOAT)(500/(FLOAT)nScrnGapY);
	    if ( nScrnGapX > nScrnGapY )
		{
			m_nDistanceX = (INT)((m_nTargetScrnX-m_nFireScrnX) * (fGapRateX));
			m_nDistanceY = (INT)((m_nTargetScrnY-m_nFireScrnY) * (fGapRateX));
		}
	    else
		{
			m_nDistanceX = (INT)((m_nTargetScrnX-m_nFireScrnX) * (fGapRateY));
			m_nDistanceY = (INT)((m_nTargetScrnY-m_nFireScrnY) * (fGapRateY));
		}

		// ���⼳��.
		m_bDir16 = (BYTE)g_xGameProc.m_xMap.CalcDirection16(m_nFireTileX, m_nFireTileY, nTargetTileX, nTargetTileY);
		//------------------------------------------------------------------------------------------------------------

		//------------------------------------------------------------------------------------------------------------
		// �⺻�����Է�.
		//------------------------------------------------------------------------------------------------------------
		m_pxOwner		  = pxOwner;
		m_bActiveState	  = _MAGIC_ACTIVE;
		m_bFixed		  = TRUE;
		m_bRepeat		  = FALSE;
		m_dwMagicLife	  = 0;
		m_wMagicNum		  = wMagicNum;
		m_bCurrSwing	  = 0;
		m_nCurrLightDelay = 0;
		//------------------------------------------------------------------------------------------------------------

		//------------------------------------------------------------------------------------------------------------
		// ������ ���� ��������.
		//------------------------------------------------------------------------------------------------------------
		switch ( wMagicNum )
		{
			case _SKILL_FIRECHARM:
			case _SKILL_FIREBALL2:
			case _SKILL_FIREBALL10:
			case _SKILL_FIREBALL:
			case _SKILL_ICEBOLT:
			case _SKILL_MAGICARROW:
			case _SKILL_HANDWIND:
			case _SKILL_CONCENFIREBALL:
			case _SKILL_DOT_1:
			case _SKILL_DOT_2:
			case _SKILL_DOT_3:
			{
				m_bRepeat = TRUE;
				m_bFixed  = FALSE;
				break;
			}
			case _SKILL_SUPERICEBOLT:
			case _SKILL_MOONOK:
			case _SKILL_MOONCHAM:
			case _SKILL_MOONCHAM_HIGH:
			case _SKILL_SNOWDART:
			{
				m_bRepeat = TRUE;
				m_bFixed  = FALSE;
				m_bDir16  = 0;
				break;
			}
			case _SKILL_BIGCLOAK:
			case _SKILL_HANGMAJINBUB:
			case _SKILL_DEJIWONHO:
			case _SKILL_POWERUP:

			case _SKILL_WITH_FIRE:
			case _SKILL_WITH_ICE:
			case _SKILL_WITH_LIGHT:
			case _SKILL_WITH_WIND:
			case _SKILL_WITH_DEF:
			case _SKILL_WITH_MAGIC:
			case _SKILL_WITH_ATT:

			case _SKILL_MAGICUP:
			case _SKILL_MAGICUP_FIRE:
			case _SKILL_MAGICUP_ICE:
			case _SKILL_MAGICUP_LIGHTENING:
			case _SKILL_MAGICUP_WIND:
			case _SKILL_ANTIMAGIC_FIRE:
			case _SKILL_ANTIMAGIC_ICE:
			case _SKILL_ANTIMAGIC_LIGHTENING:
			case _SKILL_ANTIMAGIC_WIND:
			{
				m_pxTarget = NULL;
				m_bRepeat  = TRUE;
				m_bFixed   = FALSE;
				break;
			}
			// ����.
			case _MONMAGIC_ARROW:
			case _MONMAGIC_MAARROW:
			case _MONMAGIC_BINGHONGUNG_A:
			case _MONMAGIC_CHIM:
			case _MONMAGIC_FLEA_SHOT:
			case _MONMAGIC_FIREBALL:
			case _MONMAGIC_SKELBOSSFIRE:
			case _MONMAGIC_LION1:
			case _MONMAGIC_LION2:
			{
				m_bRepeat	 = TRUE;
				m_bFixed     = FALSE;
				m_bShowLight = FALSE;
				break;
			}
			// ����.
			case _MONMAGIC_DUALAXE:
			case _MONMAGIC_SAHYULGUI_A:
			{
				m_bRepeat	 = TRUE;
				m_bFixed	 = FALSE;
				m_bDir16     = (BYTE)g_xGameProc.m_xMap.CalcDirection8(m_nFireTileX, m_nFireTileY, nTargetTileX, nTargetTileY);
				m_bShowLight = FALSE;
				break;
			}
			case _MONMAGIC_SIEGE_SHOOT:
			case _MONMAGIC_SIEGECAR_SHOOT:
			case _MONMAGIC_SIEGE_HITTED:
			case _MONMAGIC_SIEGECAR_HITTED:
			case _SKILL_MYULSAL:
			case _SKILL_BALSACHE:
			{
				m_bRepeat	 = TRUE;
				m_bFixed	 = FALSE;
				m_bDir16     = (BYTE)g_xGameProc.m_xMap.CalcDirection8(m_nFireTileX, m_nFireTileY, nTargetTileX, nTargetTileY);
				break;
			}
			case _MONMAGIC_POSUANT:
			case _MONMAGIC_HUMANTREE:
			{
				m_bRepeat	 = TRUE;
				m_bFixed	 = FALSE;
				m_bDir16     = 0;
				m_bShowLight = FALSE;
				break;
			}
			case _SKILL_FIRE:	
			case _SKILL_ICE:
			case _SKILL_REFINEADD:
			case _SKILL_SHOOTLIGHTEN_ADD:
			{
				m_bDir16   = 0;                                                    
				m_bRepeat  = FALSE;
				m_bFixed   = TRUE;
				m_pxTarget = NULL;
				break;
			}
			case _MONMAGIC_BINGBAK_MAGIC_A:
			{
				m_bDir16   = 0;                                                    
				m_bRepeat  = TRUE;
				m_bFixed   = FALSE;
				break;
			}
			case _SKILL_CROSS_ICE:
			{
				m_bDir16   = 0;                                                    
				m_bRepeat  = FALSE;
				m_bFixed   = FALSE;
				m_pxTarget = NULL;
				break;
			}
			// ����.
			case _MONMAGIC_COWGHOST:
			case _MONMAGIC_SINSU:
			case _MONMAGIC_ZOMBIE:
			case _MONMAGIC_JUMAWANG:
			case _MONMAGIC_COWFLAME:
			case _MONMAGIC_SSEGI:
			case _MONMAGIC_GUDEGI:
			case _MONMAGIC_DUNG:
			case _EVENT_DUST:
			case _MONMAGIC_GUOREDATT:
			case _MONMAGIC_GUOBLUEATT:
			case _MONMAGIC_KINGKING2:
			case _MONMAGIC_BYDONKING:
			case _MONMAGIC_BLOODSAGAL:
			case _MONMAGIC_MURYUK_S:
			case _MONMAGIC_ELEP:
			case _MONMAGIC_MURYUK_A:
			case _MONMAGIC_NUMABUBSA5:
			case _MONMAGIC_NUMABUBSA6:
			case _MONMAGIC_NUMABODYGUARD_A:
			case _MONMAGIC_NUMABODYGUARD_DIE:
			{
				if ( m_pxOwner )
				{
					m_bDir16 = m_pxOwner->m_bCurrDir;  
				}
				m_bRepeat	 = FALSE;
				m_bFixed	 = TRUE;
				m_pxTarget	 = NULL;
				m_bShowLight = FALSE;
				break;
			}
			case _SKILL_ICEWALL:
			case _SKILL_EARTHFIRE:
			case _SKILL_KANGMAK:
			case _SKILL_HOLYSHIELD:
			case _SKILL_LIGHTWALL:
			case _MONMAGIC_LIGHTENING:
			case _MONMAGIC_BINGBAK_MAGIC_B:
			case _SKILL_LIGHTENING:
			case _SKILL_FIREBOOM:
			case _SKILL_CLOAK_HIGH:
			case _SKILL_INHALEHP:
			case _SKILL_JISOKGYE:
			case _SKILL_HYPERSWORD:
			case _SKILL_POKBAL:
			case _EFFECT_FIRECRACKER_S:
			case _EFFECT_FIRECRACKER_M:
			case _EFFECT_FIRECRACKER_L:
			case _EFFECT_FIRECRACKER_XL:
			case _SKILL_COPY:
			case _SKILL_FIREBOOMUP:
			case _SKILL_EARTH_SUMMON:
			case _SKILL_HURRICANE:
			case _SKILL_HURRICANESHOT:
			case _SKILL_HURRICANEBOMB:
			case _SKILL_MAGICTRAP:
			case _SKILL_SNOWWIND:	
			case _SKILL_THUNDERSTORM:
			case _SKILL_LIGHTFLOWER:	
			case _SKILL_LIGHTFLOWERUP:	
			case _SKILL_BIGHEALLING:
			case _EVENT_LEFTFIRE1:
			case _EVENT_LEFTFIRE2:
			case _EVENT_LEFTFIRE3:
			case _EVENT_LEFTFIRE4:
			case _EVENT_LEFTFIRE5:
			case _SKILL_REVIVE:
			case _SKILL_REVIVE_HIGH:
			case _MONMAGIC_WHAYOUNG_MAGIC:
			case _SKILL_TAWOO:
			case _SKILL_BANTAN:
			{
				m_bRepeat  = FALSE;
				m_bFixed   = TRUE;
				m_bDir16   = 0;
				m_pxTarget = NULL;
				break;
			}
			case _SKILL_AMYOUNSUL:
			case _SKILL_BIGAM:
			case _SKILL_AMYOUNSUL_HIGH:
			case _SKILL_MONSTERUP:
			case _SKILL_REMOVEPOISON:
			case _SKILL_DEJIWONHO_HIGH:
			case _SKILL_REMOVEPOISON_HIGH:
			case _SKILL_BANTAN_HITTED:
			case _SKILL_CHANGEPOSITION:
			case _SKILL_MOOTEBO:	
			case _SKILL_PAWHANGBO:
			case _SKILL_PAWHANGBO_HIGH:
			case _SKILL_TAMMING:
			case _SKILL_TAMMING_HIGH:
			case _SKILL_KILLUNDEAD:
			case _SKILL_HEALLING:	
			case _SKILL_SKELLETON:
			case _SKILL_DOLL:
			case _SKILL_POWERSKELLETON:
			case _SKILL_SINSU:
			case _SKILL_SHIELD:
			case _SKILL_SHIELD_HIGH:
			case _SKILL_DOSASHIELD:
			case _SKILL_SHOWHP:
			case _SKILL_SMALLSPACEMOVE:
			case _SKILL_SPACEMOVESHOW:
			case _SKILL_SPACEMOVESHOW2:
			case _SKILL_SPACEMOVEHIDE2:
			case _SKILL_PULLSHOW:
			case _MONMAGIC_LIGHTFLOWER:
			case _MONMAGIC_YEJIN_HEAL:
			case _SKILL_REFINE_S:
			case _SKILL_REFINE_F:
			case _MONMAGIC_MAWANG_MA:
			{
				m_bRepeat = FALSE;
				m_bFixed  = TRUE;
				m_bDir16  = 0;
				break;
			}
			case _MONMAGIC_JICHUN_MAGIC_B:
			{
				m_bRepeat = FALSE;
				m_bFixed  = TRUE;
				break;
			}
			case _SKILL_SHOOTLIGHTEN:
			case _SKILL_JIKSUNGYE:
			{
				m_bRepeat = FALSE;
				m_bFixed  = TRUE;
				m_bDir16  = (BYTE)g_xGameProc.m_xMap.CalcDirection8(m_nFireTileX, m_nFireTileY, nTargetTileX, nTargetTileY);
				m_pxTarget = NULL;

				m_nTargetTileX = m_nFireTileX;
				m_nTargetTileY = m_nFireTileY;
				g_xGameProc.m_xMap.GetScrnPosFromTilePos(m_nTargetTileX, m_nTargetTileY, m_nTargetScrnX, m_nTargetScrnY);
				break;
			}
			// ����.
			case _MONMAGIC_KEPAL:
			case _MONMAGIC_EXPLODE:
			case _MONMAGIC_EXPLODE1:
			case _MONMAGIC_BLACK1:
			case _MONMAGIC_BLACK2:
			case _MONMAGIC_GREATANT:
			case _MONMAGIC_MAGENUMA:
			case _MONMAGIC_ANT:
			case _MONMAGIC_WORKANT:
			case _MONMAGIC_BIGGINE_CHAR:
			case _MONMAGIC_RED1:
			case _MONMAGIC_RED2:
			case _MONMAGIC_HANBINGMA_MAGIC:
			case _MONMAGIC_BAODIE:
			case _MONMAGIC_ZOMBIE_DIE:
			case _MONMAGIC_HUSU_DIE:
			case _MONMAGIC_OMASKEL_DIE:
			case _MONMAGIC_SINGI_DIE:
			case _MONMAGIC_BIGGINE_ATT:
			case _MONMAGIC_SANDFISH:
			case _MONMAGIC_BOSSDIE:
			case _MONMAGIC_POKAN:
			case _MONMAGIC_BIDOK:
			case _MONMAGIC_REDMOON:
			case _MONMAGIC_WINDGOD:
			case _MONMAGIC_ANTHEAL:
			case _MONMAGIC_REALUMAMAGICATT:
			case _MONMAGIC_GAMU2_MAGIC:
			case _MONMAGIC_GAMU_MAGIC:
			case _MONMAGIC_SAU:

			case _SKILL_STRUCK_FIRE:
			case _SKILL_STRUCK_ICE:
			case _SKILL_STRUCK_LIGHT:
			case _SKILL_STRUCK_WIND:
			case _SKILL_STRUCK_HOLY:
			case _SKILL_STRUCK_DARK:
			case _SKILL_STRUCK_ILLU:
			case _SKILL_STRUCK_NONE:

			case _SKILL_LEVELUP:
			case _SKILL_MAXDEFENCE:
			case _SKILL_MAXOFENCE:
			case _SKILL_BIGMOUNTAIN:
			case _SKILL_MAXDEFEECEMAGIC:
			case _SKILL_MAXDEFEECEMAGIC_HIGH:
			case _MONMAGIC_MAWANG_D:
			case _MONMAGIC_DEARYONG_A:
			{
				m_bRepeat = FALSE;
				m_bFixed = TRUE;
				m_bDir16 = 0;
				m_bShowLight = FALSE;
				break;
			}
			case _MONMAGIC_THROW_STONE:
			case _MONMAGIC_SPEAR:
			case _MONMAGIC_NUMAGIBYUNG:
			{
				m_bRepeat = TRUE;
				m_bFixed  = FALSE;
				m_bShowLight = FALSE;
				m_bDir16 = (BYTE)g_xGameProc.m_xMap.CalcDirection8(m_nFireTileX, m_nFireTileY, nTargetTileX, nTargetTileY);
				break;
			}
		}
		//------------------------------------------------------------------------------------------------------------

		//------------------------------------------------------------------------------------------------------------
		// �������������.
		//------------------------------------------------------------------------------------------------------------
		m_dwFstFrame		= pstMagicSpr->dwFstFrm+(10)*m_bDir16;
		m_dwEndFrame		= pstMagicSpr->dwEndFrm+(10)*m_bDir16;
		m_dwCurrFrame		= m_dwFstFrame;
		m_nFrmDelay			= pstMagicSpr->wDelay;
		m_bLightRadius[0]	= pstMagicSpr->bLightRadius[0];
		m_bLightRadius[1]	= pstMagicSpr->bLightRadius[1];
		m_bLightColor[0][0]	= pstMagicSpr->bLightColor[0][0];
		m_bLightColor[0][1]	= pstMagicSpr->bLightColor[0][1];
		m_bLightColor[0][2]	= pstMagicSpr->bLightColor[0][2];
		m_bLightColor[1][0]	= pstMagicSpr->bLightColor[1][0];
		m_bLightColor[1][1]	= pstMagicSpr->bLightColor[1][1];
		m_bLightColor[1][2]	= pstMagicSpr->bLightColor[1][2];
		m_bMagicColor[0]	= pstMagicSpr->bMagicColor[0];
		m_bMagicColor[1]	= pstMagicSpr->bMagicColor[1];
		m_bMagicColor[2]	= pstMagicSpr->bMagicColor[2];
		m_bBlendType		= pstMagicSpr->bBlendType;
		m_bOpacity			= pstMagicSpr->bOpa;
		m_bSwingCnt			= pstMagicSpr->bSwingCnt;
/*

		// ���� ������ Į�� ����
		if ( nAtom != 0 && wMagicNum == _SKILL_FIREBOOM )
		{
			m_bMagicColor[0]	= g_bWeaponEnergyColor[nAtom-1][0];
			m_bMagicColor[1]	= g_bWeaponEnergyColor[nAtom-1][1];
			m_bMagicColor[2]	= g_bWeaponEnergyColor[nAtom-1][2];
		}
*/
		if ( wRed !=0 || wGreen != 0 || wBlue != 0 )
		{
			m_bMagicColor[0]	= wRed;
			m_bMagicColor[1]	= wGreen;
			m_bMagicColor[2]	= wBlue;
		}

/*		m_bRepeat			= pstMagicSpr->bRepeat;
		m_bFixed			= pstMagicSpr->bFixed;
		m_bShowLight		= pstMagicSpr->bShowLight;

		if ( pstMagicSpr->bDir == 8 )
		{
			m_bDir16  = (BYTE)g_xGameProc.m_xMap.CalcDirection8(m_nFireTileX, m_nFireTileY, nTargetTileX, nTargetTileY);
		}
		else if ( pstMagicSpr->bDir == 0 )
		{
			m_bDir16 = 0;
		}

		if ( pstMagicSpr->bTargetUse )
		{
			m_pxTarget = NULL;
		}
*/		//------------------------------------------------------------------------------------------------------------

		//------------------------------------------------------------------------------------------------------------
		// �ؽ����������.
		//------------------------------------------------------------------------------------------------------------
		m_wFileIdx	 = pstMagicSpr->wImgIdx;
		m_pxMagicImg = &g_xGameProc.m_xImage.m_xImageList[m_wFileIdx];
		 
		if ( m_wFileIdx == _IMAGE_MAGIC || m_wFileIdx == _IMAGE_MAGICEX || m_wFileIdx == _IMAGE_MAGICEX2 || m_wFileIdx == _IMAGE_MAGICEX3 )
		{
			m_wFileType = _TEXTR_FILE_MAGIC;
		}
		else if ( m_wFileIdx == _IMAGE_MONMAGIC || m_wFileIdx == _IMAGE_MONMAGICEX || m_wFileIdx == _IMAGE_MONMAGICEX2 || m_wFileIdx == _IMAGE_MONMAGICEX3 || m_wFileIdx == _IMAGE_MONMAGICEX5 )
		{
			m_wFileType = _TEXTR_FILE_MONMAGIC;
		}

 		D3DVECTOR vNorm(0, 0, -1);
		m_avMagic[0]  = D3DVERTEX(D3DVECTOR(-0.5f, 0.5f, 0), vNorm, 0, 0);
		m_avMagic[1]  = D3DVERTEX(D3DVECTOR(-0.5f,-0.5f, 0), vNorm, 0, 1);
		m_avMagic[2]  = D3DVERTEX(D3DVECTOR( 0.5f, 0.5f, 0), vNorm, 1, 0);
		m_avMagic[3]  = D3DVERTEX(D3DVECTOR( 0.5f,-0.5f, 0), vNorm, 1, 1);

/*		for ( INT nFrm = m_dwFstFrame; nFrm < m_dwEndFrame; nFrm++ )
		{
			g_xGameProc.m_xImage.GetTextrImg(m_wFileType, m_wFileIdx, nFrm);
		}*/
		//------------------------------------------------------------------------------------------------------------

		//------------------------------------------------------------------------------------------------------------
		// �����������.
		//------------------------------------------------------------------------------------------------------------
		INT		nWaveNum;
		BOOL	bIsLoop = FALSE;
		if ( m_wMagicNum == _SKILL_EARTHFIRE || m_wMagicNum == _SKILL_KANGMAK || m_wMagicNum == _SKILL_JISOKGYE )
		{
			bIsLoop = TRUE;
		}

		if ( m_bFixed )
		{
			nWaveNum = m_wMagicNum*10 + 10000 + 2;
			m_nDuplicateNum = g_SoundManager.SndmngrPlayWavSound(nWaveNum, m_nTargetTileX, m_nTargetTileY, bIsLoop);
		}
		else
		{
			nWaveNum = m_wMagicNum*10 + 10000 + 1;
			m_nDuplicateNum = g_SoundManager.SndmngrPlayWavSound(nWaveNum, m_nTileX, m_nTileY, bIsLoop);
		}
		//------------------------------------------------------------------------------------------------------------

		return TRUE;
	}
	return FALSE;
}


BOOL CMagic::DrawLight(CLightFog* xLightFog, INT nLoopTime)
{
	m_nCurrLightDelay += nLoopTime;

	if ( m_wMagicNum == _SKILL_EARTHFIRE	  || m_wMagicNum == _SKILL_HOLYSHIELD	  || m_wMagicNum == _SKILL_FIRE		    || m_wMagicNum == _SKILL_SINSU		   || m_wMagicNum == _SKILL_LIGHTENING     || 
		 m_wMagicNum == _EVENT_LEFTFIRE1	  || m_wMagicNum == _EVENT_LEFTFIRE2	  || m_wMagicNum == _EVENT_LEFTFIRE3	|| m_wMagicNum == _SKILL_SPACEMOVESHOW || m_wMagicNum == _SKILL_SPACEMOVESHOW2 ||
		 m_wMagicNum == _EVENT_LEFTFIRE4	  || m_wMagicNum == _EVENT_LEFTFIRE5	  || m_wMagicNum == _SKILL_SHOOTLIGHTEN || m_wMagicNum == _MONMAGIC_LIGHTENING || m_wMagicNum == _MONMAGIC_LIGHTFLOWER ||
		 m_wMagicNum == _SKILL_SPACEMOVEHIDE2 || m_wMagicNum == _SKILL_ICE			  || m_wMagicNum == _SKILL_LIGHTWALL    || m_wMagicNum == _SKILL_SMALLSPACEMOVE|| m_wMagicNum == _SKILL_SHOOTLIGHTEN_ADD || m_wMagicNum == _SKILL_REFINE_S || m_wMagicNum == _SKILL_REFINE_F || m_wMagicNum == _SKILL_CROSS_ICE ||
		 m_wMagicNum == _SKILL_PULLSHOW		  || m_wMagicNum == _SKILL_KANGMAK		  || m_wMagicNum == _SKILL_JIKSUNGYE	|| m_wMagicNum == _SKILL_JISOKGYE )
	{
		if ( m_bActiveState == _MAGIC_ACTIVE )
		{
			if ( m_nCurrLightDelay > 200 )
			{
				m_nCurrLightDelay = 0;
				m_bCurrSwing++;

				if ( m_bCurrSwing > m_bSwingCnt )
				{
					m_bCurrSwing = 0;
				}
			}
		}
		else if( m_bActiveState == _LIGHT_ACTIVE ) 
		{
			m_bCurrSwing = 0;

			m_bLightColor[0][0] -= 2;
			if ( m_bLightColor[0][0] <= 2 )		m_bLightColor[0][0] = 2;

			m_bLightColor[0][1] -= 2;
			if ( m_bLightColor[0][1] <= 2 )		m_bLightColor[0][1] = 2;

			m_bLightColor[0][2] -= 2;
			if ( m_bLightColor[0][2] <= 2 )		m_bLightColor[0][2] = 2;

			m_bLightColor[1][0] -= 2;
			if ( m_bLightColor[1][0] <= 2 )		m_bLightColor[1][0] = 2;

			m_bLightColor[1][1] -= 2;
			if ( m_bLightColor[1][1] <= 2 )		m_bLightColor[1][1] = 2;

			m_bLightColor[1][2] -= 2;
			if ( m_bLightColor[1][2] <= 2 )		m_bLightColor[1][2] = 2;


			if ( m_bLightColor[0][0] == 2 && 	 
				 m_bLightColor[0][1] == 2 &&
				 m_bLightColor[0][2] == 2 &&
				 m_bLightColor[1][0] == 2 && 
				 m_bLightColor[1][1] == 2 &&
				 m_bLightColor[1][2] == 2)
			{
				 m_bActiveState = _MAGIC_FINISH;
			}
		}
	}
	else
	{
		if ( m_nCurrLightDelay > 200 )
		{
			if ( m_bActiveState == _MAGIC_ACTIVE )
			{
				m_nCurrLightDelay = 0;
				m_bCurrSwing++;

				if ( m_bCurrSwing > m_bSwingCnt )
				{
					m_bCurrSwing = 0;
				}
			}
			else if ( m_bActiveState == _LIGHT_ACTIVE )
			{
				m_bCurrSwing	  = 0;
				m_nCurrLightDelay = 150;
				if ( m_bLightRadius[0] > 1 && (m_bLightRadius[1] > m_bLightRadius[0]) )
				{
					m_bLightRadius[0]--;
					m_bLightRadius[1]--;
				}
				else
				{
					m_bActiveState = _MAGIC_FINISH;
				}
			}
		}
	}

	if ( m_bShowLight )	
	{
		if ( m_bActiveState == _MAGIC_ACTIVE || m_bActiveState == _LIGHT_ACTIVE )
		{
			if ( !g_xGameProc.m_bDrawMagicLight )
				return TRUE;

			if ( !m_bFixed )
			{
				xLightFog->SetLightRadiusWithRing(m_nScrnX+24,
							  					  m_nScrnY+16, 
												  m_bLightRadius[0] + m_bCurrSwing, m_bLightColor[0][0], m_bLightColor[0][1], m_bLightColor[0][2],
												  m_bLightRadius[1] + m_bCurrSwing, m_bLightColor[1][0], m_bLightColor[1][1], m_bLightColor[1][2]);
			}
			else
			{
				INT nPosX, nPosY;
				g_xGameProc.m_xMap.GetScrnPosFromTilePos(m_nTileX, m_nTileY, nPosX, nPosY);

				xLightFog->SetLightRadiusWithRing(nPosX+24,
							  					  nPosY+16, 
												  m_bLightRadius[0] + m_bCurrSwing, m_bLightColor[0][0], m_bLightColor[0][1], m_bLightColor[0][2],
												  m_bLightRadius[1] + m_bCurrSwing, m_bLightColor[1][0], m_bLightColor[1][1], m_bLightColor[1][2]);
			}
		}
	}

	return TRUE;
}


BOOL CMagic::DrawMagic()
{
	if ( m_dwMagicLife < m_wStartDelay )
	{
		m_nCurrDelay = 0;
		return FALSE;
	}

	// �ݰ�ȭ����.
	const POINT ptArr1[] = 
	{
		{40, 34}, {42, 35}, {49, 33}, {59, 31}, {65, 31}, {62, 33}, {54, 36}, {41, 44}, {47, 48}, {47, 45}, 
		{43, 37}, {41, 38}, {40, 36}, {42, 37}, {43, 35}, {46, 34}
	};
	// ȭ����.
	const POINT ptArr2[] = 
	{
		{34, 37}, {47, 38}, {62, 48}, {73, 41}, {76, 34}, {72, 43}, {58, 53}, {50, 48}, {34, 49}, {45, 49}, 
		{55, 49}, {56, 43}, {48, 35}, {46, 40}, {46, 47}, {47, 38}, 
	};
	// ������.
	const POINT ptArr3[] = 
	{
		{ 6,  4}, {26,  0}, {44,  2}, {58,  6}, {56,  6}, {52, 10}, {48, 30}, {36, 48}, {10, 49}, { 6, 47}, 
		{ 4, 30}, { 2, 20}, { 0, 0}, { 2,  4}, { 3,  0}, { 4, -1}, 
	};
	// ������õ��.
	const POINT ptArr4[] = 
	{
		{ 6,  4 }
	};
	// ������.
	const POINT ptArr5[] = 
	{
		{ 40,  40 }
	};
	// �����.
	const POINT ptArr6[] = 
	{
		{14,  4}, {28,  0}, {44,  4}, {58,  6}, {56, 12}, {52, 22}, {48, 32}, {36, 40}, {18, 49},{2, 47}, 
		{ 1, 30}, { 2, 20}, { 0, 14}, { 2,  8}, { 3,  6}, { 1, -1}, 
	};
	// ��ǳ
	const POINT ptArr7[] = 
	{
		{24, 8}, {35, 15}, {65, 0}, {60, 13}, {60, 20}, {55, 40}, {53, 42}, {30, 52}, {27, 65}, {16, 65}, 
		{20, 50}, {26, 28}, { 25, 24}, { 30, 16}, { 15,  6}, { 25, 10}, 
	};
	// �߼��� ��
	const POINT ptArr8[] = 
	{
		{8, 8}, {1, 1}, {1, 1}, {1, 1}, {1, 1}, {2, 3}, {1, 1}, {1, 1}
	};

	if ( m_bActiveState == _MAGIC_ACTIVE )
	{
		IDirect3DTexture9* lpddsMagicTextr = g_xGameProc.m_xImage.GetTextrImg(m_wFileType, m_wFileIdx, m_dwCurrFrame);

		if ( m_pxMagicImg->NewSetIndex(m_dwCurrFrame) )
		{
			if ( !m_bFixed )
			{
				if ( m_wMagicNum == _SKILL_FIREBALL2 || m_wMagicNum == _SKILL_FIREBALL10 || m_wMagicNum == _SKILL_BALSACHE )
				{
					g_xGameProc.m_xFlyingTail.SetFlyTailParticle(D3DVECTOR((FLOAT)(m_nScrnX+m_pxMagicImg->m_lpstNewCurrWilImageInfo->shPX+ptArr1[m_bDir16].x), (FLOAT)(m_nScrnY+m_pxMagicImg->m_lpstNewCurrWilImageInfo->shPY+ptArr1[m_bDir16].y), 0));
					g_xGameProc.m_xSmoke.SetSmokeParticle(D3DVECTOR((FLOAT)(m_nScrnX+m_pxMagicImg->m_lpstNewCurrWilImageInfo->shPX+ptArr1[m_bDir16].x), (FLOAT)(m_nScrnY+m_pxMagicImg->m_lpstNewCurrWilImageInfo->shPY+ptArr1[m_bDir16].y), 0));
				}
				else if ( m_wMagicNum == _SKILL_FIREBALL )
				{
					g_xGameProc.m_xFlyingTail.SetFlyTailParticle(D3DVECTOR((FLOAT)(m_nScrnX+m_pxMagicImg->m_lpstNewCurrWilImageInfo->shPX+ptArr2[m_bDir16].x), (FLOAT)(m_nScrnY+m_pxMagicImg->m_lpstNewCurrWilImageInfo->shPY+ptArr2[m_bDir16].y), 0));
					g_xGameProc.m_xSmoke.SetSmokeParticle(D3DVECTOR((FLOAT)(m_nScrnX+m_pxMagicImg->m_lpstNewCurrWilImageInfo->shPX+ptArr2[m_bDir16].x), (FLOAT)(m_nScrnY+m_pxMagicImg->m_lpstNewCurrWilImageInfo->shPY+ptArr2[m_bDir16].y), 0));
				}
				else if ( m_wMagicNum == _SKILL_HANDWIND )
				{
					g_xGameProc.m_xFlyingTail.SetFlyTailParticleEx4(D3DVECTOR((FLOAT)(m_nScrnX+m_pxMagicImg->m_lpstNewCurrWilImageInfo->shPX+ptArr7[m_bDir16].x), (FLOAT)(m_nScrnY+m_pxMagicImg->m_lpstNewCurrWilImageInfo->shPY+ptArr7[m_bDir16].y), 0));
					g_xGameProc.m_xSmoke.SetSmokeParticleEx8(D3DVECTOR((FLOAT)(m_nScrnX+m_pxMagicImg->m_lpstNewCurrWilImageInfo->shPX+ptArr7[m_bDir16].x), (FLOAT)(m_nScrnY+m_pxMagicImg->m_lpstNewCurrWilImageInfo->shPY+ptArr7[m_bDir16].y), 0));
				}
				else if ( m_wMagicNum == _SKILL_ICEBOLT )
				{
					g_xGameProc.m_xSmoke.SetSmokeParticleEx(D3DVECTOR((FLOAT)(m_nScrnX+m_pxMagicImg->m_lpstNewCurrWilImageInfo->shPX+ptArr3[m_bDir16].x), (FLOAT)(m_nScrnY+m_pxMagicImg->m_lpstNewCurrWilImageInfo->shPY+ptArr3[m_bDir16].y), 0));
				}
				else if ( m_wMagicNum == _SKILL_SUPERICEBOLT )
				{
					g_xGameProc.m_xSmoke.SetSmokeParticleEx2(D3DVECTOR((FLOAT)(m_nScrnX+m_pxMagicImg->m_lpstNewCurrWilImageInfo->shPX+ptArr4[0].x), (FLOAT)(m_nScrnY+m_pxMagicImg->m_lpstNewCurrWilImageInfo->shPY+ptArr4[0].y), 0));
					g_xGameProc.m_xFlyingTail.SetFlyTailParticleEx(D3DVECTOR((FLOAT)(m_nScrnX+m_pxMagicImg->m_lpstNewCurrWilImageInfo->shPX+ptArr4[0].x), (FLOAT)(m_nScrnY+m_pxMagicImg->m_lpstNewCurrWilImageInfo->shPY+ptArr4[0].y), 0));
				}
				else if ( m_wMagicNum == _SKILL_MOONOK )
				{
					g_xGameProc.m_xFlyingTail.SetFlyTailParticleEx2(D3DVECTOR((FLOAT)(m_nScrnX+m_pxMagicImg->m_lpstNewCurrWilImageInfo->shPX+ptArr5[0].x), (FLOAT)(m_nScrnY+m_pxMagicImg->m_lpstNewCurrWilImageInfo->shPY+ptArr5[0].y), 0));
				}
				else if ( m_wMagicNum == _SKILL_MAGICARROW )
				{
					g_xGameProc.m_xSmoke.SetSmokeParticleEx3(D3DVECTOR((FLOAT)(m_nScrnX+m_pxMagicImg->m_lpstNewCurrWilImageInfo->shPX+ptArr6[m_bDir16].x), (FLOAT)(m_nScrnY+m_pxMagicImg->m_lpstNewCurrWilImageInfo->shPY+ptArr6[m_bDir16].y), 0));
				}
				else if ( m_wMagicNum == _MONMAGIC_SIEGE_HITTED )
				{
					g_xGameProc.m_xSmoke.SetSmokeParticleEx12(D3DVECTOR((FLOAT)(m_nScrnX+m_pxMagicImg->m_lpstNewCurrWilImageInfo->shPX+ptArr8[m_bDir16].x), (FLOAT)(m_nScrnY+m_pxMagicImg->m_lpstNewCurrWilImageInfo->shPY+ptArr8[m_bDir16].y), 0));
				}
			}
			if ( m_wMagicNum == _SKILL_HURRICANE )
			{
				if (m_dwCurrFrame < m_dwEndFrame - 5)
				{
					g_xGameProc.m_xSmoke.SetSmokeParticleEx6(D3DVECTOR((FLOAT)(m_nScrnX + 24)+GetRandomNum(-10, 10), (FLOAT)(m_nScrnY + 16)+GetRandomNum(0, 20), 0));
					g_xGameProc.m_xBoom.SetBoomParticle2(D3DVECTOR((FLOAT)(m_nScrnX + 24), (FLOAT)(m_nScrnY + 16), 0));
				}
			}
			else if ( m_wMagicNum == _SKILL_HURRICANEBOMB )
			{
//				if (m_dwCurrFrame < m_dwEndFrame - 5)
//				{
					g_xGameProc.m_xSmoke.SetSmokeParticleEx11(D3DVECTOR((FLOAT)(m_nScrnX + 24), (FLOAT)(m_nScrnY + 16), 0));
					g_xGameProc.m_xBoom.SetBoomParticle4(D3DVECTOR((FLOAT)(m_nScrnX + 24), (FLOAT)(m_nScrnY + 16), 0));
//				}
			}
			else if (m_wMagicNum == _SKILL_HURRICANESHOT)
			{
				g_xGameProc.m_xSmoke.SetSmokeParticleEx10(D3DVECTOR((FLOAT)(m_nScrnX + 24)+GetRandomNum(-10, 10), (FLOAT)(m_nScrnY + 16)+GetRandomNum(0, 20), 0));
				g_xGameProc.m_xBoom.SetBoomParticle5(D3DVECTOR((FLOAT)(m_nScrnX + 24), (FLOAT)(m_nScrnY + 16), 0));
			}

			// [T109] 软件混合到软缓冲，支持 blend=2 alpha 混合
			INT nw = m_pxMagicImg->m_lpstNewCurrWilImageInfo->shWidth;
			INT nh = m_pxMagicImg->m_lpstNewCurrWilImageInfo->shHeight;
			INT nEx = m_nScrnX + m_pxMagicImg->m_lpstNewCurrWilImageInfo->shPX;
			INT nEy = m_nScrnY + m_pxMagicImg->m_lpstNewCurrWilImageInfo->shPY;

			CDX9SoftSurface* pBB = g_xMainWnd.GetBackBuffer();
			if ( pBB && pBB->m_pBits )
			{
				WORD* pDstBase = pBB->m_pBits;
				LONG lPitch = pBB->m_lPitch >> 1;
				LONG bw = pBB->m_dwWidth, bh = pBB->m_dwHeight;

				WORD* pwSrc = (WORD*)m_pxMagicImg->m_pbCurrImage;
				INT pos = 0;
				for ( INT row = 0; row < nh; row++ )
				{
					INT rowcnt = (INT)pwSrc[pos++];
					INT rowend = pos + rowcnt;
					INT col = 0;
					while ( pos < rowend )
					{
						WORD code = pwSrc[pos++];
						if ( code == 0xC0 )
						{
							INT cnt = (INT)pwSrc[pos++]; col += cnt;
						}
						else if ( code == 0xC1 || code == 0xC2 || code == 0xC3 )
						{
							INT cnt = (INT)pwSrc[pos++];
							for ( INT k = 0; k < cnt; k++ )
							{
								WORD px = pwSrc[pos + k];
								INT dx = nEx + col, dy = nEy + row;
								if ( dx >= 0 && dx < bw && dy >= 0 && dy < bh )
								{
									WORD* pDst = pDstBase + (LONG)dy * lPitch + dx;
									INT sr = (px >> 11) & 0x1F, sg = (px >> 5) & 0x3F, sb = px & 0x1F;
									INT dr = ((*pDst) >> 11) & 0x1F, dg = ((*pDst) >> 5) & 0x3F, db = (*pDst) & 0x1F;
									
									// 加色混合（恢复 T99 原逻辑，避免过曝）
									dr += sr; if ( dr > 0x1F ) dr = 0x1F;
									dg += sg; if ( dg > 0x3F ) dg = 0x3F;
									db += sb; if ( db > 0x1F ) db = 0x1F;
									*pDst = (WORD)((dr << 11) | (dg << 5) | db);
								}
								col++;
							}
							pos += cnt;
						}
					}
				}
				return TRUE;
			}
		}
	}

	return FALSE;
}


VOID CMagic::DestoryMagic()
{
}


BOOL CMagic::UpdateMagic(INT nLoopTime)
{
	m_nCurrDelay  += nLoopTime;
	m_dwMagicLife += nLoopTime;

	//------------------------------------------------------------------------------------------------------------
	// ���� �켱 ��������.
	//------------------------------------------------------------------------------------------------------------
	if ( m_dwMagicLife < m_wStartDelay )
	{
		m_nCurrDelay = 0;
		g_xGameProc.m_xMap.GetScrnPosFromTilePos(m_nFireTileX, m_nFireTileY, m_nFireScrnX, m_nFireScrnY);
		g_xGameProc.m_xMap.GetScrnPosFromTilePos(m_nTargetTileX, m_nTargetTileY, m_nTargetScrnX, m_nTargetScrnY);
		m_nScrnX	  = m_nFireScrnX;
		m_nScrnY	  = m_nFireScrnY;
		m_fFloatScrnX = (FLOAT)m_nFireScrnX;
		m_fFloatScrnY = (FLOAT)m_nFireScrnY;
		return FALSE;
	}

	if ( m_dwMagicLife > 60000 )
	{
		m_bActiveState	= _MAGIC_FINISH;
		return FALSE;
	}
	//------------------------------------------------------------------------------------------------------------


	//------------------------------------------------------------------------------------------------------------
	// ������ ó��.
	//------------------------------------------------------------------------------------------------------------
	if ( m_nCurrDelay > m_nFrmDelay )
	{
		m_dwCurrFrame++;
		m_nCurrDelay = 0;

		if ( ( m_wMagicNum == _SKILL_REFINE_S || m_wMagicNum == _SKILL_REFINE_F ) && m_dwCurrFrame == m_dwEndFrame - 1 
			 && m_szServerNotice != NULL )
		{
			DWORD dwFont = GetChatColor(_CHAT_COLOR4);
			DWORD dwBack = GetChatColor(_CHAT_COLOR3);
			g_xGameProc.m_xInterface.MsgAdd(dwFont, dwBack, m_szServerNotice);

			ZeroMemory(m_szServerNotice, MAX_PATH);
		}

		if ( m_wMagicNum == _SKILL_SINSU && m_dwCurrFrame >= 2362 && m_dwCurrFrame <=2373 )
		{
			m_nFrmDelay = 30;
		}
		else if ( m_wMagicNum == _SKILL_SINSU )
		{
			m_nFrmDelay = 110;
		}

		if ( m_nDuplicateNum < MAX_DUPLE_COUNT )
		{
			if (!m_bFixed)	
			{
				INT nWaveNum;

				nWaveNum = m_wMagicNum*10 + 10000 + 1;
				g_SoundManager.SndmngrChgVolnPan(nWaveNum,  m_nDuplicateNum, m_nTileX, m_nTileY);
			}
		}

		if ( m_dwCurrFrame >= m_dwEndFrame )
		{
			if ( m_bRepeat )
			{
				m_dwCurrFrame = m_dwFstFrame;
			}
			else
			{
				m_dwCurrFrame  = m_dwEndFrame-1;
				m_bActiveState = _LIGHT_ACTIVE;
			}
		}
	}
	//------------------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------------------
	// ��ǥ ó��.
	//------------------------------------------------------------------------------------------------------------
	BOOL	bCrash = FALSE;
	INT		nScrnGapX, nScrnGapY;

	if ( !m_bFixed )
	{
		if ( m_pxTarget )
		{
			// Ÿ���� ��ǥ�� �缳���Ѵ�.
			m_nTargetTileX = m_pxTarget->m_nPosX;
			m_nTargetTileY = m_pxTarget->m_nPosY;
			m_nTargetScrnX = m_pxTarget->m_nScrnPosX;
			m_nTargetScrnY = m_pxTarget->m_nScrnPosY;		
			
			m_nTileX = m_nFireTileX;
			m_nTileY = m_nFireTileY;

			// �Ÿ��缳��.
		   if ( m_nScrnX != m_nTargetScrnX )
			   nScrnGapX = abs(m_nTargetScrnX-m_nScrnX);
		   else
			   nScrnGapX = 1;

		   if ( m_nScrnY != m_nTargetScrnY )
			   nScrnGapY = abs(m_nTargetScrnY-m_nScrnY);
		   else
			   nScrnGapY = 1;

		   if ( nScrnGapX == 0 )
		   {
			   nScrnGapX = 1;
		   }

		   if ( nScrnGapY == 0 )
		   {
			   nScrnGapY = 1;
		   }

		   if ( nScrnGapX > nScrnGapY )
		   {
			   m_nDistanceX = (m_nTargetScrnX-m_nScrnX) * (500/nScrnGapX);
			   m_nDistanceY = (m_nTargetScrnY-m_nScrnY) * (500/nScrnGapX);
		   }
		   else
		   {
			   m_nDistanceX = (m_nTargetScrnX-m_nScrnX) * (500/nScrnGapY);
			   m_nDistanceY = (m_nTargetScrnY-m_nScrnY) * (500/nScrnGapY);
		   }
		   m_fFloatScrnX += (FLOAT)(((FLOAT)m_nDistanceX/900) * nLoopTime);
		   m_fFloatScrnY += (FLOAT)(((FLOAT)m_nDistanceY/900) * nLoopTime);

		   m_nScrnX = (INT)m_fFloatScrnX;
		   m_nScrnY = (INT)m_fFloatScrnY;

		   INT nabsX, nabsY;
		   nabsX = abs(m_nTargetScrnX-m_nScrnX);
		   nabsY = abs(m_nTargetScrnY-m_nScrnY);
 		   if ( (nabsX <= 15 && nabsY <= 15) || (nabsX >= m_nPrevDistanceX && nabsY >= m_nPrevDistanceY) )
		   {
			   bCrash = TRUE;
		   }
		   else
		   {
				m_nPrevDistanceX = nabsX;
				m_nPrevDistanceY = nabsY;
		   }
/*			//------------------------------------------------------------------------------------------------------------
			// �������������.
			// Ÿ���� ����������� ��ǥ����.
			//------------------------------------------------------------------------------------------------------------
			LPEFFECTSPRINFO pstMagicSpr = g_xSpriteInfo.GetMagicInfo(m_wMagicNum);

			if ( pstMagicSpr )
			{
				m_bDir16 = (BYTE)g_xGameProc.m_xMap.CalcDirection16(m_nTileX, m_nTileY, m_nTargetTileX, m_nTargetTileY);

				m_dwFstFrame		= pstMagicSpr->dwFstFrm+(10)*m_bDir16;
				m_dwEndFrame		= pstMagicSpr->dwEndFrm+(10)*m_bDir16;
				m_dwCurrFrame		= m_dwFstFrame;
				m_nFrmDelay			= pstMagicSpr->wDelay;
				m_bLightRadius[0]	= pstMagicSpr->bLightRadius[0];
				m_bLightRadius[1]	= pstMagicSpr->bLightRadius[1];
				m_bLightColor[0][0]	= pstMagicSpr->bLightColor[0][0];
				m_bLightColor[0][1]	= pstMagicSpr->bLightColor[0][1];
				m_bLightColor[0][2]	= pstMagicSpr->bLightColor[0][2];
				m_bLightColor[1][0]	= pstMagicSpr->bLightColor[1][0];
				m_bLightColor[1][1]	= pstMagicSpr->bLightColor[1][1];
				m_bLightColor[1][2]	= pstMagicSpr->bLightColor[1][2];
				m_bMagicColor[0]	= pstMagicSpr->bMagicColor[0];
				m_bMagicColor[1]	= pstMagicSpr->bMagicColor[1];
				m_bMagicColor[2]	= pstMagicSpr->bMagicColor[2];
				m_bBlendType		= pstMagicSpr->bBlendType;
				m_bOpacity			= pstMagicSpr->bOpa;
				m_bSwingCnt			= pstMagicSpr->bSwingCnt;
			}*/
		}
		else
		{
			FLOAT fDisX = (FLOAT)((FLOAT)m_nDistanceX/900);
			FLOAT fDisY = (FLOAT)((FLOAT)m_nDistanceY/900);

			m_fFloatScrnX += ((fDisX)*(FLOAT)nLoopTime);
			m_fFloatScrnY += ((fDisY)*(FLOAT)nLoopTime);

			m_nScrnX = (SHORT)m_fFloatScrnX;
			m_nScrnY = (SHORT)m_fFloatScrnY;

			g_xGameProc.m_xMap.GetTilePosFromScrnPos(m_nScrnX, m_nScrnY, m_nTileX, m_nTileY);


			if ( m_wMagicNum == _SKILL_FIRECHARM || m_wMagicNum == _SKILL_FIREBALL2    || m_wMagicNum == _SKILL_FIREBALL   || m_wMagicNum == _SKILL_MOONOK   ||
				 m_wMagicNum == _SKILL_ICEBOLT   || m_wMagicNum == _SKILL_SUPERICEBOLT || m_wMagicNum == _SKILL_MAGICARROW || m_wMagicNum == _SKILL_HANDWIND || 
				 m_wMagicNum == _SKILL_DOT_1	 || m_wMagicNum == _SKILL_DOT_2		   || m_wMagicNum == _SKILL_DOT_3 ||
				 m_wMagicNum == _SKILL_SNOWDART || m_wMagicNum == _SKILL_FIREBALL10 || m_wMagicNum == _SKILL_MYULSAL || m_wMagicNum == _MONMAGIC_BINGBAK_MAGIC_A ||
				 m_wMagicNum == _SKILL_BALSACHE )
			{
				if ( m_dwMagicLife >= 3000 )
				{
					bCrash = TRUE;
					m_bActiveState = _MAGIC_FINISH;
				}
			}
		    else
			{
			    INT nabsX, nabsY;
			    nabsX = abs(m_nTargetScrnX-m_nScrnX);
			    nabsY = abs(m_nTargetScrnY-m_nScrnY);

 			    if ( (nabsX <= 15 && nabsY <= 15) || (nabsX >= m_nPrevDistanceX && nabsY >= m_nPrevDistanceY) )
				{
					bCrash = TRUE;
				}
			    else
				{
					m_nPrevDistanceX = nabsX;
					m_nPrevDistanceY = nabsY;
				}
			}
		}
	}
	// �������� �ʴ� ȿ�����.
	else
	{
		if ( m_pxTarget )
		{
			m_nTileX = m_nTargetTileX = m_pxTarget->m_nPosX;
			m_nTileY = m_nTargetTileY = m_pxTarget->m_nPosY;
			m_nScrnX = m_nTargetScrnX = m_pxTarget->m_nScrnPosX;
			m_nScrnY = m_nTargetScrnY = m_pxTarget->m_nScrnPosY;		
		}
		else
		{
			m_nTileX = m_nTargetTileX;
			m_nTileY = m_nTargetTileY;
			g_xGameProc.m_xMap.GetScrnPosFromTilePos(m_nTargetTileX, m_nTargetTileY, m_nTargetScrnX, m_nTargetScrnY);
			m_nScrnX = m_nTargetScrnX;
			m_nScrnY = m_nTargetScrnY;
		}
	}
	//------------------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------------------
	// ���º�ȭ ó��.
	//------------------------------------------------------------------------------------------------------------
	if ( bCrash )
	{
		if ( m_pxTarget )
		{
			m_nTileX = m_nTargetTileX = m_pxTarget->m_nPosX;
			m_nTileY = m_nTargetTileY = m_pxTarget->m_nPosY;
			m_nScrnX = m_nTargetScrnX = m_pxTarget->m_nScrnPosX;
			m_nScrnY = m_nTargetScrnY = m_pxTarget->m_nScrnPosY;		
		}
		else
		{
			m_nTileX = m_nTargetTileX;
			m_nTileY = m_nTargetTileY;
			g_xGameProc.m_xMap.GetScrnPosFromTilePos(m_nTargetTileX, m_nTargetTileY, m_nTargetScrnX, m_nTargetScrnY);
			m_nScrnX = m_nTargetScrnX;
			m_nScrnY = m_nTargetScrnY;
		}

		m_bFixed  = TRUE;
		m_bRepeat = FALSE;

		// ����������������ȯ�Ѵ�.
		LPEFFECTSPRINFO pstMagicSpr = g_xSpriteInfo.GetExplosionInfo(m_wMagicNum);

		if ( !pstMagicSpr )
		{
			m_bActiveState	= _MAGIC_FINISH;
			return FALSE;
		}

		//------------------------------------------------------------------------------------------------------------
		// �������������.
		//------------------------------------------------------------------------------------------------------------
		if (m_wMagicNum == _MONMAGIC_SIEGECAR_HITTED)
		{
			m_dwFstFrame		= pstMagicSpr->dwFstFrm+(10)*m_bDir16;
			m_dwEndFrame		= pstMagicSpr->dwEndFrm+(10)*m_bDir16;
		}
		else
		{
			m_dwFstFrame		= pstMagicSpr->dwFstFrm;
			m_dwEndFrame		= pstMagicSpr->dwEndFrm;
		}

		m_dwCurrFrame		= m_dwFstFrame;
		m_nFrmDelay			= pstMagicSpr->wDelay;
		m_bLightRadius[0]	= pstMagicSpr->bLightRadius[0];
		m_bLightRadius[1]	= pstMagicSpr->bLightRadius[1];
		m_bLightColor[0][0]	= pstMagicSpr->bLightColor[0][0];
		m_bLightColor[0][1]	= pstMagicSpr->bLightColor[0][1];
		m_bLightColor[0][2]	= pstMagicSpr->bLightColor[0][2];
		m_bLightColor[1][0]	= pstMagicSpr->bLightColor[1][0];
		m_bLightColor[1][1]	= pstMagicSpr->bLightColor[1][1];
		m_bLightColor[1][2]	= pstMagicSpr->bLightColor[1][2];
		m_bMagicColor[0]	= pstMagicSpr->bMagicColor[0];
		m_bMagicColor[1]	= pstMagicSpr->bMagicColor[1];
		m_bMagicColor[2]	= pstMagicSpr->bMagicColor[2];
		m_bBlendType		= pstMagicSpr->bBlendType;
		m_bOpacity			= pstMagicSpr->bOpa;
		m_bSwingCnt			= pstMagicSpr->bSwingCnt;
		//------------------------------------------------------------------------------------------------------------

		//------------------------------------------------------------------------------------------------------------
		// �ؽ����������.
		//------------------------------------------------------------------------------------------------------------
		m_wFileIdx = pstMagicSpr->wImgIdx;
		m_pxMagicImg = &g_xGameProc.m_xImage.m_xImageList[m_wFileIdx];
		 
		if ( m_wFileIdx == _IMAGE_MAGIC || m_wFileIdx == _IMAGE_MAGICEX || m_wFileIdx == _IMAGE_MAGICEX2 || m_wFileIdx == _IMAGE_MAGICEX3 )
		{
			m_wFileType = _TEXTR_FILE_MAGIC;
		}
		else if ( m_wFileIdx == _IMAGE_MONMAGIC || m_wFileIdx == _IMAGE_MONMAGICEX || m_wFileIdx == _IMAGE_MONMAGICEX2 || m_wFileIdx == _IMAGE_MONMAGICEX3 || m_wFileIdx == _IMAGE_MONMAGICEX5 )
		{
			m_wFileType = _TEXTR_FILE_MONMAGIC;
		}

		D3DVECTOR vNorm(0, 0, -1);
		m_avMagic[0]  = D3DVERTEX(D3DVECTOR(-0.5f, 0.5f, 0), vNorm, 0, 0);
		m_avMagic[1]  = D3DVERTEX(D3DVECTOR(-0.5f,-0.5f, 0), vNorm, 0, 1);
		m_avMagic[2]  = D3DVERTEX(D3DVECTOR( 0.5f, 0.5f, 0), vNorm, 1, 0);
		m_avMagic[3]  = D3DVERTEX(D3DVECTOR( 0.5f,-0.5f, 0), vNorm, 1, 1);

/*		for ( INT nFrm = m_dwFstFrame; nFrm < m_dwEndFrame; nFrm++ )
		{
			g_xGameProc.m_xImage.GetTextrImg(m_wFileType, m_wFileIdx, nFrm);
		}*/
		//------------------------------------------------------------------------------------------------------------

		//------------------------------------------------------------------------------------------------------------
		// �����������.
		//------------------------------------------------------------------------------------------------------------
		INT nWaveNum;
		nWaveNum = m_wMagicNum*10 + 10000 + 2;
		if (m_pxTarget != NULL)
		{
			g_SoundManager.SndmngrPlayWavSound(nWaveNum, m_nTileX, m_nTileY);
		}

		if ( m_wMagicNum == _MONMAGIC_SIEGE_HITTED )				// �߼��� �� �´°� ���� ó��
			g_SoundManager.SndmngrPlayWavSound(nWaveNum, m_nTileX, m_nTileY);

		//------------------------------------------------------------------------------------------------------------

		// ������ ���� ���ƾ��Ѵ�.
		return FALSE;
	}
	//------------------------------------------------------------------------------------------------------------

	DrawLight(&g_xGameProc.m_xLightFog, nLoopTime);

	return TRUE;
}


BOOL CConcenFireball::CreateMagic(WORD wMagicNum, INT nFireTileX, INT nFireTileY, INT nTargetTileX, INT nTargetTileY, CActor* pxOwner, INT nTarget, WORD wStartDelay, BOOL bReverse)
{
	if ( CMagic::CreateMagic(wMagicNum, nFireTileX, nFireTileY, nTargetTileX, nTargetTileY, pxOwner, nTarget, wStartDelay) )
	{
		m_bReverse = bReverse;
		m_fRate	   = 0.0f;
		return TRUE;
	}
	return FALSE;
}



BOOL CConcenFireball::UpdateMagic(INT nLoopTime)
{
	if ( CMagic::UpdateMagic(nLoopTime) )
	{
		if ( !m_bFixed )
		{
			FLOAT fRate[16][2] = 
			{
				{1.0f, 0.0f}, {0.75f, 0.25f}, {0.5f, 0.5f}, {0.25f, 0.75f}, 
				{0.0f, 1.0f}, {0.25f, 0.75f}, {1.25f, 0.0f}, {1.25f, 0.25f}, 
				{1.0f, 0.0f}, {0.75f, 0.25f}, {0.5f, 0.5f}, {0.25f, 0.75f}, 
				{0.0f, 1.0f}, {0.25f, 0.75f}, {0.0f, 1.25f}, {1.25f, 0.25f}, 
			};

			m_fRate += 0.4f;
			FLOAT fSine = 20*sin(m_fRate);

			if ( m_bReverse )
			{
				fSine = -fSine;
			}

			m_nScrnX = (INT)m_fFloatScrnX+fSine*fRate[m_bDir16][0];
			m_nScrnY = (INT)m_fFloatScrnY+fSine*fRate[m_bDir16][1];

			g_xGameProc.m_xMap.GetTilePosFromScrnPos(m_nScrnX, m_nScrnY, m_nTileX, m_nTileY);
		}
		return TRUE;
	}
	return FALSE;
}


BOOL CConcenFireball::DrawMagic()
{
	if ( m_dwMagicLife < m_wStartDelay )
	{
		m_nCurrDelay = 0;
		return FALSE;
	}

	// ȭ����.
	const POINT ptArr[] = 
	{
		{34, 37}, {47, 38}, {62, 48}, {73, 41}, {76, 34}, {72, 43}, {58, 53}, {50, 48}, {34, 49}, {45, 49}, 
		{55, 49}, {56, 43}, {48, 35}, {46, 40}, {46, 47}, {47, 38}, 
	};

	if ( m_bActiveState == _MAGIC_ACTIVE )
	{
		if ( !m_bFixed )
		{
			g_xGameProc.m_xFlyingTail.SetFlyTailParticleEx3(D3DVECTOR((FLOAT)(m_nScrnX+m_pxMagicImg->m_lpstNewCurrWilImageInfo->shPX+ptArr[m_bDir16].x), (FLOAT)(m_nScrnY+m_pxMagicImg->m_lpstNewCurrWilImageInfo->shPY+ptArr[m_bDir16].y), 0));
			g_xGameProc.m_xSmoke.SetSmokeParticleEx5(D3DVECTOR((FLOAT)(m_nScrnX+m_pxMagicImg->m_lpstNewCurrWilImageInfo->shPX+ptArr[m_bDir16].x), (FLOAT)(m_nScrnY+m_pxMagicImg->m_lpstNewCurrWilImageInfo->shPY+ptArr[m_bDir16].y), 0));
			return TRUE;
		}
	}
	return FALSE;
}




BOOL CRefineAdd::CreateMagic(WORD wMagicNum, INT nFireTileX, INT nFireTileY, INT nTargetTileX, INT nTargetTileY, CActor* pxOwner, INT nTarget, WORD wStartDelay)
{
	if ( CMagic::CreateMagic(wMagicNum, nFireTileX, nFireTileY, nTargetTileX, nTargetTileY, pxOwner, nTarget, wStartDelay) )
	{
		// ����. -4, -24
		// ����. -10, -24
		m_nScrnX -= 10;
		m_nScrnY -= 24;
		g_xGameProc.m_xMap.GetTilePosFromScrnPos(m_nScrnX, m_nScrnY, m_nTileX, m_nTileY);

		return TRUE;
	}
	return FALSE;
}



BOOL CRefineAdd::UpdateMagic(INT nLoopTime)
{
	POINT ptRise[10] = {{-1, -1}, {0, -3}, {-1, -1}, {0, -3}, {-1, -1}, {0, -3}, {-1, -1}, {0, -3}, {-1, -1}, {0, -3}};

	m_nCurrDelay += nLoopTime;

	if ( m_nCurrDelay > m_nFrmDelay )
	{
		m_dwCurrFrame++;
		m_nCurrDelay = 0;

//		if ( m_dwCurrFrame == 9 )
//		{
//			g_xGameProc.m_xScatter.SetParticles(D3DVECTOR(m_nScrnX+20, m_nScrnY+40, 0));
//		}

		if ( m_dwCurrFrame >= m_dwEndFrame )
		{
			m_dwCurrFrame  = m_dwEndFrame-1;
			m_bActiveState = _LIGHT_ACTIVE;
		}
	}

	m_nScrnX += ptRise[m_dwCurrFrame].x;
	m_nScrnY += ptRise[m_dwCurrFrame].y;
//	g_xGameProc.m_xMap.GetTilePosFromScrnPos(m_nScrnX, m_nScrnY, m_nTileX, m_nTileY);

	DrawLight(&g_xGameProc.m_xLightFog, nLoopTime);
	return TRUE;
}


BOOL CRefineAdd::DrawMagic()
{
	if ( m_dwMagicLife < m_wStartDelay )
	{
		m_nCurrDelay = 0;
		return FALSE;
	}

	if ( m_bActiveState == _MAGIC_ACTIVE )
	{
		g_xGameProc.m_xFlyingTail.SetFlyTailParticleEx5(D3DVECTOR((FLOAT)m_nScrnX, (FLOAT)m_nScrnY, 0));
		g_xGameProc.m_xSmoke.SetSmokeParticleEx9(D3DVECTOR((FLOAT)m_nScrnX, (FLOAT)m_nScrnY, 0));
		return TRUE;
	}

	return FALSE;
}





BOOL CElecMagic::CreateMagic(WORD wMagicNum, INT nFireTileX, INT nFireTileY, INT nTargetTileX, INT nTargetTileY, CActor* pxOwner, INT nTarget)
{
	if ( CMagic::CreateMagic(wMagicNum, nFireTileX, nFireTileY, nTargetTileX, nTargetTileY, pxOwner, nTarget) )
	{
		m_xElec.InitElec();
		m_xElec.SetupElec();

		INT		nScrnGapX, nScrnGapY;
		INT		nDisX, nDisY;

	    if ( m_nFireScrnX != m_nTargetScrnX )		nScrnGapX = abs(m_nTargetScrnX-m_nFireScrnX);
	    else										nScrnGapX = 1;

	    if ( m_nFireScrnY != m_nTargetScrnY )		nScrnGapY = abs(m_nTargetScrnY-m_nFireScrnY);
	    else										nScrnGapY = 1;

		if ( nScrnGapX == 0 )
		{
			nScrnGapX = 1;
		}

		if ( nScrnGapY == 0 )
		{
			nScrnGapY = 1;
		}

	    FLOAT fGapRateX = (FLOAT)(25/(FLOAT)nScrnGapX);
	    FLOAT fGapRateY = (FLOAT)(25/(FLOAT)nScrnGapY);

	    if ( nScrnGapX > nScrnGapY )
		{
			nDisX = (INT)((m_nTargetScrnX-m_nFireScrnX) * (fGapRateX));
			nDisY = (INT)((m_nTargetScrnY-m_nFireScrnY) * (fGapRateX));
		}
	    else
		{
			nDisX = (INT)((m_nTargetScrnX-m_nFireScrnX) * (fGapRateY));
			nDisY = (INT)((m_nTargetScrnY-m_nFireScrnY) * (fGapRateY));
		}

		m_nTargetScrnX += nDisX;
		m_nTargetScrnY += nDisY;

		m_bActiveState = _LIGHT_ACTIVE;

		return TRUE;
	}

	return FALSE;	
}

BOOL CElecMagic::UpdateMagic(INT nLoopTime)
{
	m_dwMagicLife += nLoopTime;

	if ( m_dwMagicLife > 1500 )
	{
		m_bActiveState = _MAGIC_FINISH;
	}

	g_xGameProc.m_xMap.GetScrnPosFromTilePos(m_nFireTileX, m_nFireTileY, m_nFireScrnX, m_nFireScrnY);
	g_xGameProc.m_xMap.GetScrnPosFromTilePos(m_nTargetTileX, m_nTargetTileY, m_nTargetScrnX, m_nTargetScrnY);

	m_nScrnX = m_nFireScrnX;
	m_nScrnY = m_nFireScrnY;

	DrawLight(&g_xGameProc.m_xLightFog, nLoopTime);

	return TRUE;
}

BOOL CElecMagic::DrawMagic()
{
	POINT ptHand[8] = { {m_nFireScrnX + 20, m_nFireScrnY - 30}, 
						{m_nFireScrnX + 22, m_nFireScrnY - 28}, 
						{m_nFireScrnX + 24, m_nFireScrnY - 28}, 
						{m_nFireScrnX + 24, m_nFireScrnY - 32}, 

						{m_nFireScrnX + 33, m_nFireScrnY - 34}, 
						{m_nFireScrnX + 24, m_nFireScrnY - 24}, 
						{m_nFireScrnX + 20, m_nFireScrnY - 24}, 
						{m_nFireScrnX + 16, m_nFireScrnY - 26}, };

	m_xElec.UpdateElec((SHORT)ptHand[m_bDir16].x, (SHORT)ptHand[m_bDir16].y, (SHORT)(m_nTargetScrnX + 24), (SHORT)(m_nTargetScrnY - 32));

	if ( m_dwMagicLife < 350 )
	{
		m_xElec.RenderElec(g_xMainWnd.Get3DDevice());
	}
	return TRUE;
}








BOOL CMagicStream::CreateMagic(WORD wMagicNum, INT nFireTileX, INT nFireTileY, 
		                      INT nTargetTileX, INT nTargetTileY, CActor* pxOwner, INT nTarget, WORD wStartMagicTime, POINT ptTileGap)
{
	if ( CMagic::CreateMagic(wMagicNum, nFireTileX, nFireTileY, nTargetTileX, nTargetTileY, pxOwner, nTarget) )
	{
		m_wMagicStart = wStartMagicTime;
		m_ptTileGap	  = ptTileGap;

		m_bDir16 = (BYTE)g_xGameProc.m_xMap.CalcDirection8(nFireTileX, nFireTileY, nTargetTileX, nTargetTileY);
		m_nScrnX = m_nTargetScrnX;
		m_nScrnY = m_nTargetScrnY;

		return TRUE;
	}
	return FALSE;
}


BOOL CMagicStream::UpdateMagic(INT nLoopTime)
{
	m_nCurrDelay += nLoopTime;
	m_dwMagicLife += nLoopTime;

	if ( m_dwMagicLife > m_wMagicStart )
	{
		if ( m_nCurrDelay > m_nFrmDelay )
		{
			m_dwCurrFrame++;
			m_nCurrDelay = 0;

			if ( m_nDuplicateNum < MAX_DUPLE_COUNT)
			{
				if ( !m_bFixed )
				{
					INT nWaveNum;

					nWaveNum = m_wMagicNum*10 + 10000 + 1;

					g_SoundManager.SndmngrChgVolnPan(nWaveNum,  m_nDuplicateNum, m_nTileX, m_nTileY);
				}
			}

			if ( m_wMagicNum == _SKILL_FIRE )
			{
				BYTE bCanMove  = g_xGameProc.m_xMap.GetTileAttribute(m_nTileX, m_nTileY);

				if ( bCanMove == _CAN_WALK )
				{
					if ( m_dwCurrFrame >= 1925 )				m_nFrmDelay = 80;
					if ( m_dwCurrFrame <  1915 )				m_dwCurrFrame++;

					// �ٴڿ� ���°͵�...
					if ( m_dwCurrFrame == m_dwEndFrame-48 )
					{
						WORD wEffectNum = (WORD)GetRandomNum(_EVENT_LEFTFIRE1, _EVENT_LEFTFIRE5);
						CRepeatMagic* pxLeftFire;
						pxLeftFire = new CRepeatMagic;
						if ( pxLeftFire->CreateMagic(0, wEffectNum, 0, 0, m_nTileX, m_nTileY, 2000, NULL, NULL) )
						{
							g_xGameProc.m_xGroundMagicList.Insert (pxLeftFire);
						}
						else
						{
							delete pxLeftFire;
						}

						// �ٴڿ� �� ��.
						LPEVENTIMG pstEvent = new EVENTIMG;
						pstEvent->nType	    = 0;
						pstEvent->nEventID	= 0;
						pstEvent->dwTick	= timeGetTime();
						pstEvent->nTileX	= m_nTileX;
						pstEvent->nTileY	= m_nTileY;
						pstEvent->dwLifeTime = 2000;
						pstEvent->wFrameNum = 220;
						pstEvent->wOutDelay = 0;
						pstEvent->bUseAlpha = FALSE;
						g_xGameProc.m_stEventImgList.Insert(pstEvent);
					}
				}

				if ( m_dwCurrFrame >= m_dwEndFrame-30 )
				{
					m_dwCurrFrame  = m_dwEndFrame-29;
					m_bActiveState = _LIGHT_ACTIVE;
				}
			}
			else if ( m_wMagicNum == _SKILL_ICE || m_wMagicNum == _SKILL_CROSS_ICE)
			{
				BYTE bCanMove  = g_xGameProc.m_xMap.GetTileAttribute(m_nTileX, m_nTileY);

				if ( bCanMove == _CAN_WALK )
				{
					if ( m_dwCurrFrame == m_dwEndFrame-26 )
					{
						// �ٴڿ� �� ��������.
						LPEVENTIMG pstEvent = new EVENTIMG;
						pstEvent->nType	    = 0;
						pstEvent->nEventID	= 0;
						pstEvent->dwTick	= timeGetTime();
						pstEvent->nTileX	= m_nTileX;
						pstEvent->nTileY	= m_nTileY;
						pstEvent->dwLifeTime = 2000;
						pstEvent->wFrameNum = 260+GetRandomNum(0, 1);
						pstEvent->wOutDelay = 0;
						pstEvent->bUseAlpha = FALSE;
						g_xGameProc.m_stEventImgList.Insert(pstEvent);
					}
					if ( m_dwCurrFrame < m_dwEndFrame-26 ) 
					{
						g_xGameProc.m_xSmoke.SetSmokeParticleEx4(D3DVECTOR((FLOAT)(m_nScrnX+24), (FLOAT)(m_nScrnY), 0));
						g_xGameProc.m_xBoom.SetBoomParticle(D3DVECTOR((FLOAT)(m_nScrnX+24), (FLOAT)(m_nScrnY+16), 0));
					}
				}

				if ( m_dwCurrFrame >= m_dwEndFrame-20 )
				{
					m_dwCurrFrame  = m_dwEndFrame-19;
					m_bActiveState = _MAGIC_FINISH;;
				}
			}
			else
			{
				if ( m_dwCurrFrame >= m_dwEndFrame )
				{
					m_dwCurrFrame  = m_dwEndFrame-1;
					m_bActiveState = _LIGHT_ACTIVE;
				}
			}
		}
	}

	DrawLight(&g_xGameProc.m_xLightFog, nLoopTime);

	return TRUE;
}


BOOL CMagicStream::DrawMagic()
{
	if ( m_dwMagicLife > m_wMagicStart )
	{
		m_nTileX = m_nTargetTileX;
		m_nTileY = m_nTargetTileY;
		g_xGameProc.m_xMap.GetScrnPosFromTilePos(m_nTargetTileX, m_nTargetTileY, m_nTargetScrnX, m_nTargetScrnY);
		m_nScrnX = m_nTargetScrnX;
		m_nScrnY = m_nTargetScrnY;

		if ( m_wMagicNum == _SKILL_FIRE )
		{
			BYTE bCanMove  = g_xGameProc.m_xMap.GetTileAttribute(m_nTileX, m_nTileY);
			if ( bCanMove == _CAN_WALK )
			{
				CMagic::DrawMagic();

				if ( m_bDir16 != 0 && m_bDir16 != 4 )
				{
					m_dwCurrFrame += 30;
//					m_dwFstFrame += 30;
					m_nScrnX = m_nScrnX+(m_ptTileGap.x*_CELL_WIDTH/2);
					m_nScrnY = m_nScrnY+(m_ptTileGap.y*_CELL_HEIGHT/2);
					CMagic::DrawMagic();
					m_dwCurrFrame -= 30;
//					m_dwFstFrame -= 30;
					return TRUE;
				}
			}
		}
		else if ( m_wMagicNum == _SKILL_ICE )
		{
			BYTE bCanMove  = g_xGameProc.m_xMap.GetTileAttribute(m_nTileX, m_nTileY);
			if ( bCanMove == _CAN_WALK )
			{
				CMagic::DrawMagic();

				if ( m_bDir16 != 0 && m_bDir16 != 4 && m_dwCurrFrame <= m_dwEndFrame-10 )
				{
					m_dwCurrFrame += 20;
					m_nScrnX = m_nScrnX+(m_ptTileGap.x*_CELL_WIDTH/2);
					m_nScrnY = m_nScrnY+(m_ptTileGap.y*_CELL_HEIGHT/2);
//					CMagic::DrawMagic();
					m_dwCurrFrame -= 20;
					return TRUE;
				}
			}
		}
		else if ( m_wMagicNum == _SKILL_SHOOTLIGHTEN_ADD )
		{
			if ( m_dwCurrFrame == m_dwFstFrame+1 )
			{
				g_xGameProc.m_xSmoke.SetSmokeParticleEx7(D3DVECTOR((FLOAT)(m_nScrnX + 24), (FLOAT)(m_nScrnY - 16), 0));
				g_xGameProc.m_xBoom.SetBoomParticle3(D3DVECTOR((FLOAT)(m_nScrnX + 24), (FLOAT)(m_nScrnY - 16), 0));
			}
		}
		else
		{
			CMagic::DrawMagic();
			if ( m_bDir16 != 0 && m_bDir16 != 2 && m_bDir16 != 4 && m_bDir16 != 6 )
			{
				m_nScrnX = m_nScrnX+(m_ptTileGap.x*_CELL_WIDTH/2);
				m_nScrnY = m_nScrnY+(m_ptTileGap.y*_CELL_HEIGHT/2);
				CMagic::DrawMagic();
			}
		}
	}
	return FALSE;
}












BOOL CRepeatMagic::CreateMagic(INT nEventID, WORD wMagicNum, INT nFireTileX, INT nFireTileY, 
		                      INT nTargetTileX, INT nTargetTileY, DWORD dwMagicLifeTotal, CActor* pxOwner, INT nTarget)
{
	CMagic::CreateMagic(wMagicNum, nFireTileX, nFireTileY, nTargetTileX, nTargetTileY, pxOwner, nTarget);

	m_nEventID = nEventID;

	m_nTileX = m_nTargetTileX;
	m_nTileY = m_nTargetTileY;
	g_xGameProc.m_xMap.GetScrnPosFromTilePos(m_nTargetTileX, m_nTargetTileY, m_nTargetScrnX, m_nTargetScrnY);
	m_nScrnX = m_nTargetScrnX;
	m_nScrnY = m_nTargetScrnY;

	m_dwMagiLifeTotal = dwMagicLifeTotal;

	return TRUE;
}



BOOL CRepeatMagic::UpdateMagic(INT nLoopTime)
{
	m_nCurrDelay += nLoopTime;
	m_dwMagicLife += nLoopTime;

	if ( m_nCurrDelay > m_nFrmDelay )
	{
		m_nCurrDelay = 0;
		m_dwCurrFrame++;


		if ( m_nDuplicateNum < MAX_DUPLE_COUNT )
		{
			INT nWaveNum;
			
			if ( m_wMagicNum == _SKILL_EARTHFIRE || m_wMagicNum == _SKILL_KANGMAK || m_wMagicNum == _SKILL_JISOKGYE )
				nWaveNum = m_wMagicNum*10 + 10000 + 2;
			else
				nWaveNum = m_wMagicNum*10 + 10000 + 1;

			g_SoundManager.SndmngrChgVolnPan(nWaveNum,  m_nDuplicateNum, m_nTileX, m_nTileY);
		}

		if ( m_dwCurrFrame >= m_dwEndFrame )
			m_dwCurrFrame = m_dwFstFrame;

		if ( m_dwMagicLife > m_dwMagiLifeTotal )
		{
			m_bActiveState = _MAGIC_FINISH;
		}
	}

	DrawLight(&g_xGameProc.m_xLightFog, nLoopTime);

	if ( m_bActiveState	== _MAGIC_FINISH )
	{
		INT nWaveNum;

		nWaveNum = m_wMagicNum*10 + 10000 + 2;

		if ( m_nDuplicateNum < 10 && m_nDuplicateNum != UNINIT)
		{
			g_SoundManager.SndmngrStopWavSound(nWaveNum, m_nDuplicateNum);
		}
		else
		{
			g_SoundManager.SndmngrStopWavSound(nWaveNum);
		}
	}

	return TRUE;
}



BOOL CRepeatMagic::DrawMagic()
{
	if ( m_bActiveState == _MAGIC_ACTIVE || m_bActiveState == _LIGHT_ACTIVE )
	{
		m_nTileX = m_nTargetTileX;
		m_nTileY = m_nTargetTileY;
		g_xGameProc.m_xMap.GetScrnPosFromTilePos(m_nTargetTileX, m_nTargetTileY, m_nTargetScrnX, m_nTargetScrnY);
		m_nScrnX = m_nTargetScrnX;
		m_nScrnY = m_nTargetScrnY;

		if ( m_bActiveState == _LIGHT_ACTIVE )
		{
			if ( m_bMagicColor[0] > 5 )		m_bMagicColor[0] -= 5;
			else							m_bMagicColor[0]  = 0;
			if ( m_bMagicColor[1] > 5 )		m_bMagicColor[1] -= 5;
			else							m_bMagicColor[1]  = 0;
			if ( m_bMagicColor[2] > 5 )		m_bMagicColor[2] -= 5;
			else							m_bMagicColor[2]  = 0;
		}

		if ( m_wMagicNum == _SKILL_KANGMAK )
		{
			g_xGameProc.m_xSmoke.SetSmokeParticleEx13(D3DVECTOR((FLOAT)(m_nScrnX + 24)+GetRandomNum(-10, 10), (FLOAT)(m_nScrnY)+GetRandomNum(-10, 10), 0));
			g_xGameProc.m_xBoom.SetBoomParticle6(D3DVECTOR((FLOAT)(m_nScrnX + 24), (FLOAT)(m_nScrnY)+GetRandomNum(-10, 10), 0));
		}

		// [SOFTMIX] 火墙作用特效：软件混合到软缓冲（与 CMagic T99 一致，避免被 Present 软缓冲覆盖）
		if ( m_pxMagicImg && m_pxMagicImg->NewSetIndex(m_dwCurrFrame) )
		{
			INT nw = m_pxMagicImg->m_lpstNewCurrWilImageInfo->shWidth;
			INT nh = m_pxMagicImg->m_lpstNewCurrWilImageInfo->shHeight;
			INT nEx = m_nScrnX + m_pxMagicImg->m_lpstNewCurrWilImageInfo->shPX;
			INT nEy = m_nScrnY + m_pxMagicImg->m_lpstNewCurrWilImageInfo->shPY;

			CDX9SoftSurface* pBB = g_xMainWnd.GetBackBuffer();
			if ( pBB && pBB->m_pBits && m_pxMagicImg->m_pbCurrImage )
			{
				WORD* pDstBase = pBB->m_pBits;
				LONG lPitch = pBB->m_lPitch >> 1;
				LONG bw = pBB->m_dwWidth, bh = pBB->m_dwHeight;

				WORD* pwSrc = (WORD*)m_pxMagicImg->m_pbCurrImage;
				INT pos = 0;
				for ( INT row = 0; row < nh; row++ )
				{
					INT rowcnt = (INT)pwSrc[pos++];
					INT rowend = pos + rowcnt;
					INT col = 0;
					while ( pos < rowend )
					{
						WORD code = pwSrc[pos++];
						if ( code == 0xC0 )
						{
							INT cnt = (INT)pwSrc[pos++]; col += cnt;
						}
						else if ( code == 0xC1 || code == 0xC2 || code == 0xC3 )
						{
							INT cnt = (INT)pwSrc[pos++];
							for ( INT k = 0; k < cnt; k++ )
							{
								WORD px = pwSrc[pos + k];
								INT dx = nEx + col, dy = nEy + row;
								if ( dx >= 0 && dx < bw && dy >= 0 && dy < bh )
								{
									WORD* pDst = pDstBase + (LONG)dy * lPitch + dx;
									INT sr = (px >> 11) & 0x1F, sg = (px >> 5) & 0x3F, sb = px & 0x1F;
									INT dr = ((*pDst) >> 11) & 0x1F, dg = ((*pDst) >> 5) & 0x3F, db = (*pDst) & 0x1F;
									
									// 加色混合（统一，避免过曝）
									dr += sr; if ( dr > 0x1F ) dr = 0x1F;
									dg += sg; if ( dg > 0x3F ) dg = 0x3F;
									db += sb; if ( db > 0x1F ) db = 0x1F;
									*pDst = (WORD)((dr << 11) | (dg << 5) | db);
								}
								col++;
							}
							pos += cnt;
						}
					}
				}
				return TRUE;
			}
		}
	}

	return FALSE;
}
CHurricaneMagic::CreateMagic(WORD wMagicNum, INT nFireTileX, INT nFireTileY, 
		                     INT nTargetTileX, INT nTargetTileY, INT nKind, CActor* pxOwner, INT nTarget)
{
	POINT	ptStartTile;
	mHurricaneMagic_nIncrease = 0;							
	mHurricaneMagic_fRadius = 0;
	
	if ( CMagic::CreateMagic(wMagicNum, nFireTileX, nFireTileY, nTargetTileX, nTargetTileY, pxOwner, nTarget) )
	{
		mHurricaneMagic_nState = 0;
		m_wMagicNum = wMagicNum;

		m_bActiveState = _MAGIC_ACTIVE;

		mHurricaneMagic_nKind = nKind;

		if (mHurricaneMagic_nKind == _SKILL_HURRICANESHOT )
		{
			m_bFixed = FALSE;
			m_bRepeat  = TRUE;
			m_bDir16 = (BYTE)g_xGameProc.m_xMap.CalcDirection16(m_nFireTileX, m_nFireTileY, nTargetTileX, nTargetTileY) / 2;
			m_nTargetTileX = nFireTileX;
			m_nTargetTileY = nFireTileY;

//			g_xGameProc.m_xMap.GetScrnPosFromTilePos(m_nTargetTileX, m_nTargetTileY, m_nTargetScrnX, m_nTargetScrnY);

//			m_nScrnX = m_nTargetScrnX;
//			m_nScrnY = m_nTargetScrnY;

		}
		else
		{
			m_bFixed = FALSE;
			m_nTileX = m_nTargetTileX;
			m_nTileY = m_nTargetTileY;

//			g_xGameProc.m_xMap.GetScrnPosFromTilePos(m_nTargetTileX, m_nTargetTileY, m_nTargetScrnX, m_nTargetScrnY);

//			m_nTargetScrnX += 50;
//			m_nTargetScrnY -= 50;

			m_nTargetScrnX += 100;
			m_nTargetScrnY += 100;
			m_nScrnX = m_nTargetScrnX;
			m_nScrnY = m_nTargetScrnY;
		}


		LPEFFECTSPRINFO pstMagicSpr = g_xSpriteInfo.GetMagicInfo(m_wMagicNum);
		m_dwFstFrame		= pstMagicSpr->dwFstFrm;
		m_dwEndFrame		= pstMagicSpr->dwEndFrm;
		m_dwCurrFrame		= m_dwFstFrame;
		m_nFrmDelay			= pstMagicSpr->wDelay;
		m_bLightRadius[0]	= pstMagicSpr->bLightRadius[0];
		m_bLightRadius[1]	= pstMagicSpr->bLightRadius[1];
		m_bLightColor[0][0]	= pstMagicSpr->bLightColor[0][0];
		m_bLightColor[0][1]	= pstMagicSpr->bLightColor[0][1];
		m_bLightColor[0][2]	= pstMagicSpr->bLightColor[0][2];
		m_bLightColor[1][0]	= pstMagicSpr->bLightColor[1][0];
		m_bLightColor[1][1]	= pstMagicSpr->bLightColor[1][1];
		m_bLightColor[1][2]	= pstMagicSpr->bLightColor[1][2];
		m_bMagicColor[0]	= pstMagicSpr->bMagicColor[0];
		m_bMagicColor[1]	= pstMagicSpr->bMagicColor[1];
		m_bMagicColor[2]	= pstMagicSpr->bMagicColor[2];
		m_bBlendType		= pstMagicSpr->bBlendType;
		m_bOpacity			= pstMagicSpr->bOpa;
		m_bSwingCnt			= pstMagicSpr->bSwingCnt;

		// �ؽ�ó ���� 
		m_wFileIdx	 = pstMagicSpr->wImgIdx;
		m_pxMagicImg = &g_xGameProc.m_xImage.m_xImageList[m_wFileIdx];			 
		m_wFileType = _TEXTR_FILE_MAGIC;
 		D3DVECTOR vNorm(0, 0, -1);
		m_avMagic[0]  = D3DVERTEX(D3DVECTOR(-0.5f, 0.5f, 0), vNorm, 0, 0);
		m_avMagic[1]  = D3DVERTEX(D3DVECTOR(-0.5f,-0.5f, 0), vNorm, 0, 1);
		m_avMagic[2]  = D3DVERTEX(D3DVECTOR( 0.5f, 0.5f, 0), vNorm, 1, 0);
		m_avMagic[3]  = D3DVERTEX(D3DVECTOR( 0.5f,-0.5f, 0), vNorm, 1, 1);

		return TRUE;
	}
	return FALSE;
}

BOOL
CHurricaneMagic::UpdateMagic(INT nLoopTime)
{
	m_nCurrDelay += nLoopTime;

	switch(mHurricaneMagic_nState)
	{
	case 0:															///    ���� ������ ó��
		{
			g_xGameProc.m_xMap.GetScrnPosFromTilePos(m_nTargetTileX, m_nTargetTileY, m_nTargetScrnX, m_nTargetScrnY);

			if ( mHurricaneMagic_nKind != _SKILL_HURRICANESHOT )
			{
				m_nScrnX = m_nTargetScrnX;
				m_nScrnY = m_nTargetScrnY;
				m_nScrnX += 0;
				m_nScrnY += 10;
			}

			if ( m_nCurrDelay > m_nFrmDelay )
			{
				m_nCurrDelay = 0;
				m_dwCurrFrame++;

				if ( m_dwCurrFrame >= 1050 )				///   ���� �������� �Ϸ�Ǿ����Ƿ� ���� �������� ������ ���´�.
				{
					mHurricaneMagic_nState = 1;
				}
			}
		}
		break;
	case 1:															///     ���� ������ ó��
		{
			if ( m_nCurrDelay > m_nFrmDelay )
			{
				m_nCurrDelay = 0;
				m_dwCurrFrame++;
			}

			m_dwMagicLife += nLoopTime;

			if ( m_dwCurrFrame >= m_dwEndFrame)
			{
				m_dwCurrFrame = m_dwFstFrame;
				break;
			}

			if ( m_dwMagicLife >= 1000)
			{
				mHurricaneMagic_nState = 2;

				LPEFFECTSPRINFO pstMagicSpr = g_xSpriteInfo.GetExplosionInfo(m_wMagicNum);
				
				m_dwFstFrame		= pstMagicSpr->dwFstFrm;
				m_dwEndFrame		= pstMagicSpr->dwEndFrm;
				m_dwCurrFrame		= m_dwFstFrame;
				m_nFrmDelay			= pstMagicSpr->wDelay;
				m_bLightRadius[0]	= pstMagicSpr->bLightRadius[0];
				m_bLightRadius[1]	= pstMagicSpr->bLightRadius[1];
				m_bLightColor[0][0]	= pstMagicSpr->bLightColor[0][0];
				m_bLightColor[0][1]	= pstMagicSpr->bLightColor[0][1];
				m_bLightColor[0][2]	= pstMagicSpr->bLightColor[0][2];
				m_bLightColor[1][0]	= pstMagicSpr->bLightColor[1][0];
				m_bLightColor[1][1]	= pstMagicSpr->bLightColor[1][1];
				m_bLightColor[1][2]	= pstMagicSpr->bLightColor[1][2];
				m_bMagicColor[0]	= pstMagicSpr->bMagicColor[0];
				m_bMagicColor[1]	= pstMagicSpr->bMagicColor[1];
				m_bMagicColor[2]	= pstMagicSpr->bMagicColor[2];
				m_bBlendType		= pstMagicSpr->bBlendType;
				m_bOpacity			= pstMagicSpr->bOpa;
				m_bSwingCnt			= pstMagicSpr->bSwingCnt;

				// �ؽ�ó ���� 
				m_wFileIdx	 = pstMagicSpr->wImgIdx;
				m_pxMagicImg = &g_xGameProc.m_xImage.m_xImageList[m_wFileIdx];			 
				m_wFileType = _TEXTR_FILE_MAGIC;
 				D3DVECTOR vNorm(0, 0, -1);
				m_avMagic[0]  = D3DVERTEX(D3DVECTOR(-0.5f, 0.5f, 0), vNorm, 0, 0);
				m_avMagic[1]  = D3DVERTEX(D3DVECTOR(-0.5f,-0.5f, 0), vNorm, 0, 1);
				m_avMagic[2]  = D3DVERTEX(D3DVECTOR( 0.5f, 0.5f, 0), vNorm, 1, 0);
				m_avMagic[3]  = D3DVERTEX(D3DVECTOR( 0.5f,-0.5f, 0), vNorm, 1, 1);
				// �Ҹ� �������� ��ȯ
				break;
			}

			if (mHurricaneMagic_nKind == _SKILL_HURRICANESHOT )
			{
				mHurricaneMagic_fRadius += 30;

				INT		nOffY, nOffX;

				nOffX = 0.8 * mHurricaneMagic_fRadius;
				nOffY = sin(0.01745f * mHurricaneMagic_fRadius) * 15;

				float fDistance = sqrt(nOffY*nOffY + nOffX*nOffX), fTemp;

				fTemp = (float)nOffY/(float)nOffX;
				float fNewArc = atan(fTemp), fAck;
				
				switch ( m_bDir16 )
				{
					case 0:
						fAck = 90;
						break;
					case 1:
						fAck = 30;
						break;
					case 2:
						fAck = 0;
						break;
					case 3:
						fAck = 330;
						break;
					case 4:
						fAck = 270;
						break;
					case 5:
						fAck = 210;
						break;
					case 6:
						fAck = 180;
						break;
					case 7:
						fAck = 150;
						break;
				}

				fAck = 0.01745f * fAck - fNewArc;
				mHurricaneMagic_nAddX = cos(fAck) * fDistance;
				mHurricaneMagic_nAddY = sin(fAck) * fDistance;

				g_xGameProc.m_xMap.GetScrnPosFromTilePos(m_nTargetTileX, m_nTargetTileY, m_nTargetScrnX, m_nTargetScrnY);

				m_nScrnX = m_nTargetScrnX + mHurricaneMagic_nAddX;
				m_nScrnY = m_nTargetScrnY - mHurricaneMagic_nAddY;

				g_xGameProc.m_xMap.GetTilePosFromScrnPos(m_nScrnX, m_nScrnY, m_nTileX, m_nTileY);
				m_nTargetScrnX = m_nTileX;
				m_nTargetScrnY = m_nTileY;
			}
			else
			{
				double	fAngle, fRadius;
				float	fAngleIncreaseValue = 0.30;

				mHurricaneMagic_fRadius = 15;

//				if (mHurricaneMagic_nId == 0)
//				{
//					fAngle = (double)m_dwMagicLife * fAngleIncreaseValue;							// ������ġ�ؾ� �ϴ� ����
//				}

//				mHurricaneMagic_nOffsetX = (double)cos(0.01745f * fAngle) * mHurricaneMagic_fRadius - mHurricaneMagic_fRadius;
//				mHurricaneMagic_nOffsetY = (double)sin(0.01745f * fAngle) * mHurricaneMagic_fRadius;

				g_xGameProc.m_xMap.GetScrnPosFromTilePos(m_nTargetTileX, m_nTargetTileY, m_nTargetScrnX, m_nTargetScrnY);

				m_nScrnX = m_nTargetScrnX /*+ mHurricaneMagic_nOffsetX + 20*/;
				m_nScrnY = m_nTargetScrnY /*+ mHurricaneMagic_nOffsetY*/ - mHurricaneMagic_nIncrease + 10;

				g_xGameProc.m_xMap.GetTilePosFromScrnPos(m_nScrnX, m_nScrnY, m_nTileX, m_nTileY);
				m_nTargetScrnX = m_nTileX;
				m_nTargetScrnY = m_nTileY;
			}
		}
		break;
	case 2:
		{
			g_xGameProc.m_xMap.GetScrnPosFromTilePos(m_nTargetTileX, m_nTargetTileY, m_nTargetScrnX, m_nTargetScrnY);

			if (mHurricaneMagic_nKind == _SKILL_HURRICANESHOT )
			{
				m_nScrnX = m_nTargetScrnX + mHurricaneMagic_nAddX;
				m_nScrnY = m_nTargetScrnY - mHurricaneMagic_nAddY;
			}
			else
			{
				m_nScrnX = m_nTargetScrnX /*+ mHurricaneMagic_nOffsetX + 20*/;
				m_nScrnY = m_nTargetScrnY /*+ mHurricaneMagic_nOffsetY*/ - mHurricaneMagic_nIncrease + 10;
			}

			if ( m_nCurrDelay > m_nFrmDelay )
			{
				m_nCurrDelay = 0;
				m_dwCurrFrame++;

				if ( m_dwCurrFrame >= m_dwEndFrame )
				{
					m_bActiveState	= _LIGHT_ACTIVE;
				}
			}
		}
	}
	DrawLight(&g_xGameProc.m_xLightFog, nLoopTime);
	return TRUE;
}

BOOL
CHurricaneMagic::DrawMagic()
{
	BYTE bCanMove  = g_xGameProc.m_xMap.GetTileAttribute(m_nTileX, m_nTileY);

	if ( bCanMove == _CAN_WALK )
	{
		CMagic::DrawMagic();
		return TRUE;
	}
	else
		return FALSE;
}


BOOL CMiddleRepeateMagic::CreateMagic(INT nEventID, WORD wMagicNum, INT nFireTileX, INT nFireTileY, 
		                      INT nTargetTileX, INT nTargetTileY, DWORD dwMagicLifeTotal, CActor* pxOwner, INT nTarget)
{
	CMagic::CreateMagic(wMagicNum, nFireTileX, nFireTileY, nTargetTileX, nTargetTileY, pxOwner, nTarget);

	m_nEventID = nEventID;

	m_nTileX = m_nTargetTileX;
	m_nTileY = m_nTargetTileY;
	g_xGameProc.m_xMap.GetScrnPosFromTilePos(m_nTargetTileX, m_nTargetTileY, m_nTargetScrnX, m_nTargetScrnY);
	m_nScrnX = m_nTargetScrnX;
	m_nScrnY = m_nTargetScrnY;

	m_dwMagiLifeTotal = dwMagicLifeTotal;

	return TRUE;
}



BOOL CMiddleRepeateMagic::UpdateMagic(INT nLoopTime)
{
	BYTE bCanMove  = g_xGameProc.m_xMap.GetTileAttribute(m_nTileX, m_nTileY);

	if ( bCanMove == _CAN_WALK )
	{
		if ( m_dwCurrFrame >= 107 )
		{
			// �ٴڿ� �� ��������.
			LPEVENTIMG pstEvent = new EVENTIMG;
			pstEvent->nType	    = 0;
			pstEvent->nEventID	= 0;
			pstEvent->dwTick	= timeGetTime();
			pstEvent->nTileX	= m_nTileX;
			pstEvent->nTileY	= m_nTileY;
			pstEvent->dwLifeTime = 3000;
			pstEvent->wFrameNum = 260+GetRandomNum(0, 0);
			pstEvent->wOutDelay = 0;
			pstEvent->bUseAlpha = TRUE;
			g_xGameProc.m_stEventImgList.Insert(pstEvent);
		}

		if ( m_dwCurrFrame >= 101 && m_dwCurrFrame < 106) 
		{
			g_xGameProc.m_xSmoke.SetSmokeParticleEx4(D3DVECTOR((FLOAT)(m_nScrnX+24), (FLOAT)(m_nScrnY), 0));
			g_xGameProc.m_xBoom.SetBoomParticle(D3DVECTOR((FLOAT)(m_nScrnX+24), (FLOAT)(m_nScrnY+16), 0));
		}

	}


	m_nCurrDelay += nLoopTime;
	m_dwMagicLife += nLoopTime;

	if ( m_nCurrDelay > m_nFrmDelay )
	{
		m_nCurrDelay = 0;
		m_dwCurrFrame++;

		if ( m_dwMagicLife > m_dwMagiLifeTotal )
		{
			if ( m_dwCurrFrame >= 110 )
				m_bActiveState = _MAGIC_FINISH;
		}
		else
		{
			if ( m_dwCurrFrame >= 99 )
				m_dwCurrFrame = 100;
		}

		if ( m_nDuplicateNum < MAX_DUPLE_COUNT )
		{
			INT nWaveNum;
			
			if ( m_wMagicNum == _SKILL_EARTHFIRE || m_wMagicNum == _SKILL_KANGMAK || m_wMagicNum == _SKILL_JISOKGYE )
				nWaveNum = m_wMagicNum*10 + 10000 + 2;
			else
				nWaveNum = m_wMagicNum*10 + 10000 + 1;

			g_SoundManager.SndmngrChgVolnPan(nWaveNum,  m_nDuplicateNum, m_nTileX, m_nTileY);
		}

	}

	DrawLight(&g_xGameProc.m_xLightFog, nLoopTime);

	if ( m_bActiveState	== _MAGIC_FINISH )
	{
		INT nWaveNum;

		nWaveNum = m_wMagicNum*10 + 10000 + 2;

		if ( m_nDuplicateNum < 10 && m_nDuplicateNum != UNINIT)
		{
			g_SoundManager.SndmngrStopWavSound(nWaveNum, m_nDuplicateNum);
		}
		else
		{
			g_SoundManager.SndmngrStopWavSound(nWaveNum);
		}
	}

	return TRUE;
}



BOOL CMiddleRepeateMagic::DrawMagic()
{
	if ( m_wMagicNum == _SKILL_ICEWALL )
	{
		m_nTileX = m_nTargetTileX;
		m_nTileY = m_nTargetTileY;
		g_xGameProc.m_xMap.GetScrnPosFromTilePos(m_nTargetTileX, m_nTargetTileY, m_nTargetScrnX, m_nTargetScrnY);
		m_nScrnX = m_nTargetScrnX;
		m_nScrnY = m_nTargetScrnY;

		CMagic::DrawMagic();

		if ( m_dwCurrFrame <= 100 )
		{
			m_dwCurrFrame += 20;
			m_nScrnX = m_nScrnX+(_CELL_WIDTH/4*1);
			m_nScrnY = m_nScrnY+(_CELL_HEIGHT/4*1);
			CMagic::DrawMagic();
			m_dwCurrFrame -= 20;
			return TRUE;
		}
		return TRUE;
	}

	return FALSE;
}
