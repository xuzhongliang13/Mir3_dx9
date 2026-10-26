#include "StdAfx.h"


CLightFog::CLightFog()
{
	Init(RGB(220, 220, 220));
}


CLightFog::~CLightFog()
{
}


VOID CLightFog::Init(DWORD dwLightColor)
{
	m_dwLightColor = dwLightColor;

	for ( INT nCntY = 0; nCntY < (NUM_CNTY_LIGHTTILE+1); nCntY++ )
	{
		for ( INT nCntX = 0; nCntX < (NUM_CNTX_LIGHTTILE+1); nCntX++ )
		{
			m_avLight[nCntY][nCntX].x = (FLOAT)(TILE_START_XPOS+(nCntX*TILE_WIDTH));
m_avLight[nCntY][nCntX].y = (FLOAT)(-TILE_START_YPOS-(nCntY*TILE_HEIGHT));
m_avLight[nCntY][nCntX].z = 0;
m_avLight[nCntY][nCntX].dwColor = m_dwLightColor;
m_avLight[nCntY][nCntX].dwSpecular = RGB(0, 0, 0);
m_avLight[nCntY][nCntX].tu = 0;
m_avLight[nCntY][nCntX].tv = 0; 
		}
	}

	memcpy(m_avSaveLight, m_avLight, sizeof(D3DLVERTEX)*(NUM_CNTY_LIGHTTILE+1)*(NUM_CNTX_LIGHTTILE+1));

	// Ÿ���������� �ε��� ����.
    WORD* pwIndex = m_pwLightIndices;
	for ( INT nCntY = 0; nCntY < NUM_CNTY_LIGHTTILE; nCntY++ )
	{
		for ( INT nCntX = 0; nCntX < NUM_CNTX_LIGHTTILE; nCntX++ )
		{
			*pwIndex++ = (WORD)((nCntX+1)+((NUM_CNTX_LIGHTTILE+1)*nCntY));  
			*pwIndex++ = (WORD)(nCntX+((NUM_CNTX_LIGHTTILE+1)*(nCntY+1)));  
			*pwIndex++ = (WORD)((nCntX+1)+((NUM_CNTX_LIGHTTILE+1)*(nCntY+1)));
			*pwIndex++ = (WORD)(nCntX+((NUM_CNTX_LIGHTTILE+1)*(nCntY+1)));  
			*pwIndex++ = (WORD)((nCntX+1)+((NUM_CNTX_LIGHTTILE+1)*nCntY));  
			*pwIndex++ = (WORD)((nCntX)+((NUM_CNTX_LIGHTTILE+1)*nCntY));
		}
	}
}


VOID CLightFog::ChangeLightColor(DWORD dwColor)
{
	if ( m_dwLightColor != dwColor )
	{
		for ( INT nCntY = 0; nCntY < NUM_CNTY_LIGHTTILE+1; nCntY++ )
		{
			for ( INT nCntX = 0; nCntX < NUM_CNTX_LIGHTTILE+1; nCntX++ )
			{
				m_avLight[nCntY][nCntX].dwColor = dwColor;
			}
		}
		m_dwLightColor = dwColor;
		memcpy(m_avSaveLight, m_avLight, sizeof(D3DLVERTEX)*(NUM_CNTY_LIGHTTILE+1)*(NUM_CNTX_LIGHTTILE+1));
	}
}


VOID CLightFog::ClearSavedLightColor()
{
	memcpy(m_avLight, m_avSaveLight, sizeof(D3DLVERTEX)*(NUM_CNTY_LIGHTTILE+1)*(NUM_CNTX_LIGHTTILE+1));
}


BOOL CLightFog::GetScreenPosToLightTile(INT nX, INT nY, INT* nTileX, INT* nTileY)
{
	*nTileX = (nX - TILE_START_XPOS) / TILE_WIDTH;
	*nTileY = (nY - TILE_START_YPOS) / TILE_HEIGHT;
	
	return TRUE;
}


BOOL CLightFog::SetLightRadiusWithRing(INT XLightPos, INT YLightPos, 
									   INT nSmallRadius, 
									   INT nSmallRed, INT nSmallGreen, INT nSmallBlue, 
									   INT nLargeRadius,
									   INT nLargeRed, INT nLargeGreen, INT nLargeBlue)
{
	INT nX, nY;
	GetScreenPosToLightTile(XLightPos, YLightPos, &nX, &nY);

	INT nStartTileX			= nX - nLargeRadius;
	INT nStartTileY			= nY - nLargeRadius;
	INT nEndTileX			= nX + nLargeRadius + 1;
	INT nEndTileY			= nY + nLargeRadius + 1;

	// y�� ������ �ؼ� �� ��ο� �κ��� ���Ѵ�.(x���� y�� �� �����Ƿ�)
	nLargeRadius = (nLargeRadius*TILE_HEIGHT);
	nSmallRadius = (nSmallRadius*TILE_HEIGHT);

	DOUBLE rLengthCnt		= 0;
	INT nSelectedRedValue	= 0; 
	INT nSelectedGreenValue	= 0;
	INT nSelectedBlueValue	= 0;

	if ( nStartTileX < 0 )					nStartTileX = 0;
	if ( nStartTileY < 0 )					nStartTileY = 0;
	if ( nEndTileX > NUM_CNTX_LIGHTTILE+1 )	nEndTileX = NUM_CNTX_LIGHTTILE+1;
	if ( nEndTileY > NUM_CNTY_LIGHTTILE+1 )	nEndTileY = NUM_CNTY_LIGHTTILE+1;	

	for ( INT nCntY = nStartTileY; nCntY < nEndTileY; nCntY++ )
	{
		for ( INT nCntX = nStartTileX; nCntX < nEndTileX; nCntX++ )
		{			
			rLengthCnt	= sqrt((XLightPos-(nCntX*TILE_WIDTH+TILE_START_XPOS))*(XLightPos-(nCntX*TILE_WIDTH+TILE_START_XPOS)) + 
							   (YLightPos-(nCntY*TILE_HEIGHT+TILE_START_YPOS))*(YLightPos-(nCntY*TILE_HEIGHT+TILE_START_YPOS)));
			if ( nCntX < NUM_CNTX_LIGHTTILE && nCntY < NUM_CNTY_LIGHTTILE )
			{
				// ����Ʈ���� �������� �ʴ� ����. ����Ʈ���� �������� �ʴ� ���.
				if ( (nCntX > nX && nCntY < nY) || (nCntX < nX && nCntY > nY) )
				{
					WORD* pwIndex = ((WORD*)m_pwLightIndices+(nCntX+nCntY*(NUM_CNTX_LIGHTTILE))*6);
					*pwIndex++ = (WORD)((nCntX)+((NUM_CNTX_LIGHTTILE+1)*nCntY));
					*pwIndex++ = (WORD)((nCntX+1)+((NUM_CNTX_LIGHTTILE+1)*nCntY));  
					*pwIndex++ = (WORD)((nCntX+1)+((NUM_CNTX_LIGHTTILE+1)*(nCntY+1)));
					*pwIndex++ = (WORD)((nCntX)+((NUM_CNTX_LIGHTTILE+1)*nCntY));
					*pwIndex++ = (WORD)(nCntX+((NUM_CNTX_LIGHTTILE+1)*(nCntY+1)));  
					*pwIndex++ = (WORD)((nCntX+1)+((NUM_CNTX_LIGHTTILE+1)*(nCntY+1)));
				}
				else
				{
					WORD* pwIndex = ((WORD*)m_pwLightIndices+(nCntX+nCntY*(NUM_CNTX_LIGHTTILE))*6);
					*pwIndex++ = (WORD)((nCntX+1)+((NUM_CNTX_LIGHTTILE+1)*nCntY));  
					*pwIndex++ = (WORD)(nCntX+((NUM_CNTX_LIGHTTILE+1)*(nCntY+1)));  
					*pwIndex++ = (WORD)((nCntX+1)+((NUM_CNTX_LIGHTTILE+1)*(nCntY+1)));
					*pwIndex++ = (WORD)(nCntX+((NUM_CNTX_LIGHTTILE+1)*(nCntY+1)));  
					*pwIndex++ = (WORD)((nCntX+1)+((NUM_CNTX_LIGHTTILE+1)*nCntY));  
					*pwIndex++ = (WORD)((nCntX)+((NUM_CNTX_LIGHTTILE+1)*nCntY));
				}
			}

			if ( nLargeRadius <= 0 )
			{
				continue;
			}

			if ( rLengthCnt	< nSmallRadius )
			{
				nSelectedRedValue	= nSmallRed - INT((nSmallRed*rLengthCnt)/(nLargeRadius));
				nSelectedGreenValue	= nSmallGreen - INT((nSmallGreen*rLengthCnt)/(nLargeRadius));
				nSelectedBlueValue	= nSmallBlue - INT((nSmallBlue*rLengthCnt)/(nLargeRadius));
			}
			else if ( rLengthCnt >= nSmallRadius && rLengthCnt <= nLargeRadius )
			{
				nSelectedRedValue	= nLargeRed - INT((nLargeRed*rLengthCnt)/(nLargeRadius));
				nSelectedGreenValue	= nLargeGreen - INT((nLargeGreen*rLengthCnt)/(nLargeRadius));
				nSelectedBlueValue	= nLargeBlue - INT((nLargeBlue*rLengthCnt)/(nLargeRadius));
			}
			else
			{
				nSelectedRedValue = nSelectedGreenValue = nSelectedBlueValue = 0;
			}

			if ( nSelectedRedValue	 < 0 )		nSelectedRedValue	= 0;
			if ( nSelectedGreenValue < 0 )		nSelectedGreenValue = 0;
			if ( nSelectedBlueValue	 < 0 )		nSelectedBlueValue	= 0;

			if ( m_avLight[nCntY][nCntX].dwColor == 0 )
				m_avLight[nCntY][nCntX].dwColor = RGB(nSelectedRedValue, nSelectedGreenValue, nSelectedBlueValue); 
			else
			{
				nSelectedRedValue += (INT)RGB_GETBLUE(m_avLight[nCntY][nCntX].dwColor);
				nSelectedGreenValue += (INT)RGB_GETGREEN(m_avLight[nCntY][nCntX].dwColor);
				nSelectedBlueValue += (INT)RGB_GETRED(m_avLight[nCntY][nCntX].dwColor);

				if ( nSelectedRedValue	 > 255 )		nSelectedRedValue	= 255;
				if ( nSelectedGreenValue > 255 )		nSelectedGreenValue = 255;
				if ( nSelectedBlueValue	 > 255 )		nSelectedBlueValue	= 255;

				m_avLight[nCntY][nCntX].dwColor =  RGB(nSelectedRedValue, nSelectedGreenValue, nSelectedBlueValue);
			}
		}
	}

	return TRUE;
}


BOOL CLightFog::SetLightRadiusWithCircle(INT XLightPos, INT YLightPos, INT nRadius, 
										 INT nRedValue, INT nGreenValue, INT nBlueValue, 
										 FLOAT rDrakRate)
{
	INT nX, nY;
	GetScreenPosToLightTile(XLightPos, YLightPos, &nX, &nY);

	INT nStartTileX			= nX - nRadius;
	INT nStartTileY			= nY - nRadius;
	INT nEndTileX			= nX + nRadius + 1;
	INT nEndTileY			= nY + nRadius + 1;

	// y�� ������ �ؼ� �� ��ο� �κ��� ���Ѵ�.(x���� y�� �� �����Ƿ�)
	nRadius = (nRadius*TILE_HEIGHT);

	DOUBLE rLengthCnt		= 0;
	INT nSelectedRedValue	= 0; 
	INT nSelectedGreenValue	= 0;
	INT nSelectedBlueValue	= 0;

	if ( nStartTileX < 0 )					nStartTileX = 0;
	if ( nStartTileY < 0 )					nStartTileY = 0;
	if ( nEndTileX > NUM_CNTX_LIGHTTILE+1 )	nEndTileX = NUM_CNTX_LIGHTTILE+1;
	if ( nEndTileY > NUM_CNTY_LIGHTTILE+1 )	nEndTileY = NUM_CNTY_LIGHTTILE+1;	

	for ( INT nCntY = nStartTileY; nCntY < nEndTileY; nCntY++ )
	{
		for ( INT nCntX = nStartTileX; nCntX < nEndTileX; nCntX++ )
		{			
			rLengthCnt	= sqrt((XLightPos-(nCntX*TILE_WIDTH+TILE_START_XPOS))*(XLightPos-(nCntX*TILE_WIDTH+TILE_START_XPOS)) + 
							   (YLightPos-(nCntY*TILE_HEIGHT+TILE_START_YPOS))*(YLightPos-(nCntY*TILE_HEIGHT+TILE_START_YPOS)));
			if ( nCntX < NUM_CNTX_LIGHTTILE && nCntY < NUM_CNTY_LIGHTTILE )
			{
				// ����Ʈ���� �������� �ʴ� ����. ����Ʈ���� �������� �ʴ� ���.
				if ( (nCntX > nX && nCntY < nY) || (nCntX < nX && nCntY > nY) )
				{
					WORD* pwIndex = ((WORD*)m_pwLightIndices+(nCntX+nCntY*(NUM_CNTX_LIGHTTILE))*6);
					*pwIndex++ = (WORD)((nCntX)+((NUM_CNTX_LIGHTTILE+1)*nCntY));
					*pwIndex++ = (WORD)((nCntX+1)+((NUM_CNTX_LIGHTTILE+1)*nCntY));  
					*pwIndex++ = (WORD)((nCntX+1)+((NUM_CNTX_LIGHTTILE+1)*(nCntY+1)));
					*pwIndex++ = (WORD)((nCntX)+((NUM_CNTX_LIGHTTILE+1)*nCntY));
					*pwIndex++ = (WORD)(nCntX+((NUM_CNTX_LIGHTTILE+1)*(nCntY+1)));  
					*pwIndex++ = (WORD)((nCntX+1)+((NUM_CNTX_LIGHTTILE+1)*(nCntY+1)));
				}
				else
				{
					WORD* pwIndex = ((WORD*)m_pwLightIndices+(nCntX+nCntY*(NUM_CNTX_LIGHTTILE))*6);
					*pwIndex++ = (WORD)((nCntX+1)+((NUM_CNTX_LIGHTTILE+1)*nCntY));  
					*pwIndex++ = (WORD)(nCntX+((NUM_CNTX_LIGHTTILE+1)*(nCntY+1)));  
					*pwIndex++ = (WORD)((nCntX+1)+((NUM_CNTX_LIGHTTILE+1)*(nCntY+1)));
					*pwIndex++ = (WORD)(nCntX+((NUM_CNTX_LIGHTTILE+1)*(nCntY+1)));  
					*pwIndex++ = (WORD)((nCntX+1)+((NUM_CNTX_LIGHTTILE+1)*nCntY));  
					*pwIndex++ = (WORD)((nCntX)+((NUM_CNTX_LIGHTTILE+1)*nCntY));
				}
			}

/*			RECT rc;
			SetRect(&rc,
				   (nCntX*TILE_WIDTH+TILE_START_XPOS),
				   (nCntY*TILE_HEIGHT+TILE_START_YPOS),
				   (nCntX*TILE_WIDTH+TILE_START_XPOS+TILE_WIDTH),
				   (nCntY*TILE_HEIGHT+TILE_START_YPOS+TILE_HEIGHT));
			g_xMainWnd.DrawWithGDI(&rc, NULL, RGB(0, 255, 255), 1);
*/
			if ( nRadius <= 0 )
			{
				continue;
			}

			if ( rLengthCnt	<= nRadius )
			{
				nSelectedRedValue	= nRedValue - INT((nRedValue*rLengthCnt*rDrakRate)/(nRadius));
				nSelectedGreenValue	= nGreenValue - INT((nGreenValue*rLengthCnt*rDrakRate)/(nRadius));
				nSelectedBlueValue	= nBlueValue - INT((nBlueValue*rLengthCnt*rDrakRate)/(nRadius));
/*				SetRect(&rc,
					   (nCntX*TILE_WIDTH+TILE_START_XPOS),
					   (nCntY*TILE_HEIGHT+TILE_START_YPOS),
					   (nCntX*TILE_WIDTH+TILE_START_XPOS+2),
					   (nCntY*TILE_HEIGHT+TILE_START_YPOS+2));
				g_xMainWnd.DrawWithGDI(&rc, NULL, RGB(255, 0, 0), 1);
*/			}
			else
			{
				nSelectedRedValue = nSelectedGreenValue = nSelectedBlueValue = 0;
			}

			if ( nSelectedRedValue	 < 0 )		nSelectedRedValue	= 0;
			if ( nSelectedGreenValue < 0 )		nSelectedGreenValue = 0;
			if ( nSelectedBlueValue	 < 0 )		nSelectedBlueValue	= 0;

			if ( m_avLight[nCntY][nCntX].dwColor == 0 )
				m_avLight[nCntY][nCntX].dwColor = RGB(nSelectedRedValue, nSelectedGreenValue, nSelectedBlueValue); 
			else
			{
				nSelectedRedValue += (INT)RGB_GETBLUE(m_avLight[nCntY][nCntX].dwColor);
				nSelectedGreenValue += (INT)RGB_GETGREEN(m_avLight[nCntY][nCntX].dwColor);
				nSelectedBlueValue += (INT)RGB_GETRED(m_avLight[nCntY][nCntX].dwColor);

				if ( nSelectedRedValue	 > 255 )		nSelectedRedValue	= 255;
				if ( nSelectedGreenValue > 255 )		nSelectedGreenValue = 255;
				if ( nSelectedBlueValue	 > 255 )		nSelectedBlueValue	= 255;

				m_avLight[nCntY][nCntX].dwColor =  RGB(nSelectedRedValue, nSelectedGreenValue, nSelectedBlueValue);
			}
		}
	}

	return TRUE;
}


BOOL CLightFog::SetLightRadiusWithElipse(INT XLightPos, INT YLightPos, INT nXRadius, INT nYRadius, 
										 INT nRedValue, INT nGreenValue, INT nBlueValue, 
										 FLOAT rDrakRate)
{
	INT nX, nY;
	GetScreenPosToLightTile(XLightPos, YLightPos, &nX, &nY);

	INT nStartTileX			= nX - nXRadius;
	INT nStartTileY			= nY - nYRadius;
	INT nEndTileX			= nX + nXRadius + 1;
	INT nEndTileY			= nY + nYRadius + 1;
	
	FLOAT rXRadius = (FLOAT)(nXRadius*TILE_WIDTH);
	FLOAT rYRadius = (FLOAT)(nYRadius*TILE_HEIGHT);
	INT	  nRadius  = 0;

	if ( rXRadius > rYRadius )
	{
		nRadius = (INT)rXRadius;
	}
	else
	{
		nRadius = (INT)rYRadius;
	}

	DOUBLE rLengthCnt		= 0;
	INT nSelectedRedValue	= 0; 
	INT nSelectedGreenValue	= 0; 
	INT nSelectedBlueValue	= 0; 

	if ( nStartTileX < 0 )					nStartTileX = 0;
	if ( nStartTileY < 0 )					nStartTileY = 0;
	if ( nEndTileX > NUM_CNTX_LIGHTTILE+1 )	nEndTileX = NUM_CNTX_LIGHTTILE+1;
	if ( nEndTileY > NUM_CNTY_LIGHTTILE+1 )	nEndTileY = NUM_CNTY_LIGHTTILE+1;


	for ( INT nCntY = nStartTileY; nCntY < nEndTileY; nCntY++ )
	{
		for ( INT nCntX = nStartTileX; nCntX < nEndTileX; nCntX++ )
		{
			if ( nCntX < NUM_CNTX_LIGHTTILE && nCntY < NUM_CNTY_LIGHTTILE )
			{
				// ����Ʈ���� �������� �ʴ� ����. ����Ʈ���� �������� �ʴ� ���.
				if ( (nCntX > nX && nCntY < nY) || (nCntX < nX && nCntY > nY) )
				{
					WORD* pwIndex = ((WORD*)m_pwLightIndices+(nCntX+nCntY*(NUM_CNTX_LIGHTTILE))*6);
					*pwIndex++ = (WORD)((nCntX)+((NUM_CNTX_LIGHTTILE+1)*nCntY));
					*pwIndex++ = (WORD)((nCntX+1)+((NUM_CNTX_LIGHTTILE+1)*nCntY));  
					*pwIndex++ = (WORD)((nCntX+1)+((NUM_CNTX_LIGHTTILE+1)*(nCntY+1)));
					*pwIndex++ = (WORD)((nCntX)+((NUM_CNTX_LIGHTTILE+1)*nCntY));
					*pwIndex++ = (WORD)(nCntX+((NUM_CNTX_LIGHTTILE+1)*(nCntY+1)));  
					*pwIndex++ = (WORD)((nCntX+1)+((NUM_CNTX_LIGHTTILE+1)*(nCntY+1)));
				}
				else
				{
					WORD* pwIndex = ((WORD*)m_pwLightIndices+(nCntX+nCntY*(NUM_CNTX_LIGHTTILE))*6);
					*pwIndex++ = (WORD)((nCntX+1)+((NUM_CNTX_LIGHTTILE+1)*nCntY));  
					*pwIndex++ = (WORD)(nCntX+((NUM_CNTX_LIGHTTILE+1)*(nCntY+1)));  
					*pwIndex++ = (WORD)((nCntX+1)+((NUM_CNTX_LIGHTTILE+1)*(nCntY+1)));
					*pwIndex++ = (WORD)(nCntX+((NUM_CNTX_LIGHTTILE+1)*(nCntY+1)));  
					*pwIndex++ = (WORD)((nCntX+1)+((NUM_CNTX_LIGHTTILE+1)*nCntY));  
					*pwIndex++ = (WORD)((nCntX)+((NUM_CNTX_LIGHTTILE+1)*nCntY));
				}
			}

/*			RECT rc;
			SetRect(&rc,
				   (nCntX*TILE_WIDTH+TILE_START_XPOS),
				   (nCntY*TILE_HEIGHT+TILE_START_YPOS),
				   (nCntX*TILE_WIDTH+TILE_START_XPOS+TILE_WIDTH),
				   (nCntY*TILE_HEIGHT+TILE_START_YPOS+TILE_HEIGHT));
			g_xMainWnd.DrawWithGDI(&rc, NULL, RGB(0, 255, 255), 1);
*/
	
			INT nCurrCntXRate, nCurrCntYRate;
			if ( rXRadius > rYRadius )		
			{
				nCurrCntXRate = nCntX;

				if ( rYRadius-1.0f > 0 )
					nCurrCntYRate = nCntY + (INT)((nCntY-nY)*(FLOAT)(rXRadius/rYRadius-1.0f));
				else
					nCurrCntYRate = nCntY;
			}
			else if ( rXRadius < rYRadius )										
			{
				if ( rXRadius-1.0f > 0 )
					nCurrCntXRate = nCntX + (INT)((nCntX-nX)*(FLOAT)(rYRadius/rXRadius-1.0f));
				else
					nCurrCntXRate = nCntX;

				nCurrCntYRate = nCntY;
			}
			else
			{
				nCurrCntXRate = nCntX;
				nCurrCntYRate = nCntY;
			}

			rLengthCnt	= sqrt((XLightPos-(nCurrCntXRate*TILE_WIDTH+TILE_START_XPOS))*(XLightPos-(nCurrCntXRate*TILE_WIDTH+TILE_START_XPOS)) + 
							   (YLightPos-(nCurrCntYRate*TILE_HEIGHT+TILE_START_YPOS))*(YLightPos-(nCurrCntYRate*TILE_HEIGHT+TILE_START_YPOS)));

/*			SetRect(&rc,
				   (XLightPos),
				   (YLightPos),
				   (XLightPos+5),
				   (YLightPos+5));
			g_xMainWnd.DrawWithGDI(&rc, NULL, RGB(255, 0, 0), 1);
*/
			if ( nRadius <= 0 )
			{
				continue;
			}

			if ( rLengthCnt	<= nRadius )
			{
				nSelectedRedValue	= nRedValue - INT((nRedValue*rLengthCnt*rDrakRate)/(nRadius));
				nSelectedGreenValue	= nGreenValue - INT((nGreenValue*rLengthCnt*rDrakRate)/(nRadius));
				nSelectedBlueValue	= nBlueValue - INT((nBlueValue*rLengthCnt*rDrakRate)/(nRadius));
	/*			SetRect(&rc,
					   (nCntX*TILE_WIDTH+TILE_START_XPOS),
					   (nCntY*TILE_HEIGHT+TILE_START_YPOS),
					   (nCntX*TILE_WIDTH+TILE_START_XPOS+2),
					   (nCntY*TILE_HEIGHT+TILE_START_YPOS+2));
				g_xMainWnd.DrawWithGDI(&rc, NULL, RGB(255, 0, 0), 1);
	*/			}
			else
			{
				nSelectedRedValue = nSelectedGreenValue = nSelectedBlueValue = 0;
			}

			if ( nSelectedRedValue	 < 0 )		nSelectedRedValue	= 0;
			if ( nSelectedGreenValue < 0 )		nSelectedGreenValue = 0;
			if ( nSelectedBlueValue	 < 0 )		nSelectedBlueValue	= 0;

			if ( m_avLight[nCntY][nCntX].dwColor == 0 )
				m_avLight[nCntY][nCntX].dwColor = RGB(nSelectedRedValue, nSelectedGreenValue, nSelectedBlueValue); 
			else
			{
				if ( nSelectedRedValue < (INT)RGB_GETBLUE(m_avLight[nCntY][nCntX].dwColor) )
					nSelectedRedValue = (INT)RGB_GETBLUE(m_avLight[nCntY][nCntX].dwColor);
				if ( nSelectedGreenValue < (INT)RGB_GETGREEN(m_avLight[nCntY][nCntX].dwColor) )
					nSelectedGreenValue = (INT)RGB_GETGREEN(m_avLight[nCntY][nCntX].dwColor);
				if ( nSelectedBlueValue < (INT)RGB_GETRED(m_avLight[nCntY][nCntX].dwColor) )
					nSelectedBlueValue = (INT)RGB_GETRED(m_avLight[nCntY][nCntX].dwColor);

				m_avLight[nCntY][nCntX].dwColor =  RGB(nSelectedRedValue, nSelectedGreenValue, nSelectedBlueValue);
			}
		}
	}

	return TRUE;
}




VOID CLightFog::ShowLightFogSoft()
{
	// [FIX-D1] Day/night (fog) effect disabled by request.
	// Set the flag below to TRUE to re-enable it.
	static const BOOL bDayNightEnabled = FALSE;
	if ( !bDayNightEnabled ) return;
	// [FIX-L1] 把 18x19 顶点光照网格乘到 565 软缓冲上(原 D3D 版绘制在 Present 之前,
	//   会被软缓冲整屏覆盖, 导致白天黑夜完全无效)。
	// [FIX-L4] 初版按每个 48x32 tile 取一个平均色 -> 画面出现大量长方形色块(见反馈截图 2)。
	//   现改为**双线性插值**: 先按行做纵向插值得到 18 个顶点色, 再在 tile 内做横向线性过渡,
	//   保证网格之间平滑。
	CDX9SoftSurface* dst = g_xMainWnd.GetBackBuffer();
	if ( !dst || !dst->m_pBits ) return;

	// 全白 = 不打光(白天), 直接跳过
	BOOL bAllWhite = TRUE;
	for ( INT y = 0; y < NUM_CNTY_LIGHTTILE + 1 && bAllWhite; y++ )
	{
		for ( INT x = 0; x < NUM_CNTX_LIGHTTILE + 1; x++ )
		{
			if ( ( m_avLight[y][x].dwColor & 0x00FFFFFF ) != 0x00FFFFFF ) { bAllWhite = FALSE; break; }
		}
	}
	if ( bAllWhite ) return;

	WORD* pBits = dst->m_pBits;
	LONG  pitch = dst->m_lPitch >> 1;
	LONG  W = (LONG)dst->m_dwWidth;
	LONG  H = (LONG)dst->m_dwHeight;

	const INT NX = NUM_CNTX_LIGHTTILE;		// 17 个横向 tile, 18 个顶点
	const INT NY = NUM_CNTY_LIGHTTILE;		// 18 个纵向 tile, 19 个顶点

	for ( LONG y = 0; y < H; y++ )
	{
		// 纵向: iy 与插值系数 ty
		INT   iy;
		FLOAT ty;
		if ( y < -29 )
		{
			iy = 0;   ty = 0.0f;
		}
		else
		{
			iy = ( y + 29 ) / TILE_HEIGHT;
			if ( iy > NY - 1 ) { iy = NY - 1; ty = 1.0f; }
			else
			{
				LONG y0 = iy * TILE_HEIGHT - 29;
				ty = (FLOAT)( y - y0 ) / (FLOAT)TILE_HEIGHT;
			}
		}

		// 该行 18 个顶点经纵向插值后的颜色(8bit)
		FLOAT vR[NUM_CNTX_LIGHTTILE + 1];
		FLOAT vG[NUM_CNTX_LIGHTTILE + 1];
		FLOAT vB[NUM_CNTX_LIGHTTILE + 1];
		INT   iyTop = iy;
		INT   iyBot = iy + 1;
		if ( iyBot > NUM_CNTY_LIGHTTILE ) iyBot = NUM_CNTY_LIGHTTILE;

		for ( INT k = 0; k <= NX; k++ )
		{
			DWORD c0 = m_avLight[iyTop][k].dwColor;
			DWORD c1 = m_avLight[iyBot][k].dwColor;

			FLOAT r0 = (FLOAT)( ( c0 >> 16 ) & 0xFF ), r1 = (FLOAT)( ( c1 >> 16 ) & 0xFF );
			FLOAT g0 = (FLOAT)( ( c0 >>  8 ) & 0xFF ), g1 = (FLOAT)( ( c1 >>  8 ) & 0xFF );
			FLOAT b0 = (FLOAT)(   c0         & 0xFF ), b1 = (FLOAT)(   c1         & 0xFF );

			vR[k] = r0 + ( r1 - r0 ) * ty;
			vG[k] = g0 + ( g1 - g0 ) * ty;
			vB[k] = b0 + ( b1 - b0 ) * ty;
		}

		WORD* row = pBits + (size_t)y * pitch;

		// 横向: 逐 tile 线性过渡
		for ( INT ix = 0; ix < NX; ix++ )
		{
			LONG x0 = ix * TILE_WIDTH - 8;
			LONG x1 = x0 + TILE_WIDTH;

			LONG xs = ( x0 < 0 ) ? 0 : x0;
			LONG xe = ( x1 > W ) ? W : x1;
			if ( xe <= xs ) continue;

			FLOAT r0 = vR[ix], r1 = vR[ix + 1];
			FLOAT g0 = vG[ix], g1 = vG[ix + 1];
			FLOAT b0 = vB[ix], b1 = vB[ix + 1];
			FLOAT invW = 1.0f / (FLOAT)TILE_WIDTH;

			for ( LONG x = xs; x < xe; x++ )
			{
				FLOAT t = (FLOAT)( x - x0 ) * invW;

				DWORD rr = (DWORD)( r0 + ( r1 - r0 ) * t ); if ( rr > 255 ) rr = 255;
				DWORD gg = (DWORD)( g0 + ( g1 - g0 ) * t ); if ( gg > 255 ) gg = 255;
				DWORD bb = (DWORD)( b0 + ( b1 - b0 ) * t ); if ( bb > 255 ) bb = 255;

				if ( rr >= 255 && gg >= 255 && bb >= 255 ) continue;

				DWORD r5 = ( rr * 31 ) / 255;
				DWORD g6 = ( gg * 63 ) / 255;
				DWORD b5 = ( bb * 31 ) / 255;

				WORD  d  = row[x];
				DWORD dr = ( d >> 11 ) & 0x1F;
				DWORD dg = ( d >>  5 ) & 0x3F;
				DWORD db =  d         & 0x1F;

				row[x] = (WORD)( ( ( dr * r5 / 31 ) << 11 ) | ( ( dg * g6 / 63 ) << 5 ) | ( db * b5 / 31 ) );
			}
		}
	}
}


VOID CLightFog::ShowLightFog(WORD wHalfScrnWidth, WORD wHalfScrnHeight)
{
	if ( g_xMainWnd.Get3DDevice() )
	{
		D3DVECTOR vTrans;
		D3DMATRIX matTrans;
		D3DMATRIX matWorldOriginal;

		if( SUCCEEDED(g_xMainWnd.Get3DDevice()->BeginScene()) )
		{
			g_xMainWnd.Get3DDevice()->GetTransform(D3DTS_WORLD, &matWorldOriginal);

			vTrans.x = (FLOAT)-wHalfScrnWidth; vTrans.y = (FLOAT)wHalfScrnHeight; vTrans.z = 0;
			D3DUtil_SetTranslateMatrix(matTrans, vTrans);
			g_xMainWnd.Get3DDevice()->SetTransform(D3DTS_WORLD, &matTrans);

			g_xMainWnd.Get3DDevice()->SetRenderState(D3DRS_LIGHTING, FALSE);
			g_xMainWnd.Get3DDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			g_xMainWnd.Get3DDevice()->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_ZERO);
			g_xMainWnd.Get3DDevice()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);

			g_xMainWnd.Get3DDevice()->SetTexture(0, NULL);

			g_xMainWnd.Get3DDevice()->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, NUM_LIGHTTILE_VERTICES, NUM_LIGHTTILE_INDICES/3, m_pwLightIndices, D3DFMT_INDEX16, m_avLight, sizeof(D3DLVERTEX));

			g_xMainWnd.Get3DDevice()->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_ONE);
			g_xMainWnd.Get3DDevice()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
			g_xMainWnd.Get3DDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
			g_xMainWnd.Get3DDevice()->SetRenderState(D3DRS_LIGHTING, TRUE);

			g_xMainWnd.Get3DDevice()->SetTransform(D3DTS_WORLD, &matWorldOriginal);

			g_xMainWnd.Get3DDevice()->EndScene();
		}
	}
}
