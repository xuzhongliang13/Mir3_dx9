/******************************************************************************************************************
                                                                                                                   
	����:																											
																													
	�ۼ���:																											
	�ۼ���:																											
																													
	[����][������] : ���� ����																						
                                                                                                                   
*******************************************************************************************************************/
#include "StdAfx.h"



/******************************************************************************************************************

	CParticleSystem Class	

*******************************************************************************************************************/
CParticleSystem::CParticleSystem()
{
	InitSystem();
}


CParticleSystem::~CParticleSystem()
{
	DestroySystem();
}


VOID CParticleSystem::InitSystem()
{
	m_nNum			 = 0;		
	m_pxParticle	 = NULL;
	m_fDeltaTime	 = 0.02f;

	m_fAirFiction	 = -0.05f;
	m_vecBounding[0] = D3DVECTOR(0, 0, 0);
	m_vecBounding[1] = D3DVECTOR(0, 0, 0);
	m_vecEnvironment = D3DVECTOR(0, 300, 0);

	D3DVECTOR vNorm(0, 0, -1);
	m_avShape[0] = D3DVERTEX(D3DVECTOR(-0.5f,-0.5f, 0.0f ), vNorm, 0.0f, 1.0f);
	m_avShape[1] = D3DVERTEX(D3DVECTOR( 0.5f,-0.5f, 0.0f ), vNorm, 0.0f, 0.0f);
	m_avShape[2] = D3DVERTEX(D3DVECTOR(-0.5f, 0.5f, 0.0f ), vNorm, 1.0f, 1.0f);
	m_avShape[3] = D3DVERTEX(D3DVECTOR( 0.5f, 0.5f, 0.0f ), vNorm, 1.0f, 0.0f);
}


VOID CParticleSystem::SetupSystem(WORD wCnt)
{
	InitSystem();
	m_nNum = wCnt;

    m_pxParticle = new CParticle[m_nNum];

	for ( INT nCnt = 0; nCnt < m_nNum; nCnt++ )
	{
		m_pxParticle[nCnt].Init();
	}
}


VOID CParticleSystem::DestroySystem()
{
	SAFE_DELETEARR(m_pxParticle);

	InitSystem();
}


VOID CParticleSystem::UpdateAirFiction(INT nNum)
{
	if ( !m_pxParticle[nNum].m_bIsDead )
	{
		m_pxParticle[nNum].m_vecLocalForce.x = -m_pxParticle[nNum].m_vecVel.x * m_fAirFiction;
		m_pxParticle[nNum].m_vecLocalForce.y = -m_pxParticle[nNum].m_vecVel.y * m_fAirFiction;
		m_pxParticle[nNum].m_vecLocalForce.z = m_pxParticle[nNum].m_vecVel.z * m_fAirFiction;
	}
}


VOID CParticleSystem::UpdateMove(INT nNum)
{
	if ( !m_pxParticle[nNum].m_bIsDead )
	{
		if ( m_pxParticle[nNum].m_fMass == 0.0f )
		{
			m_pxParticle[nNum].m_fMass = 1.0f;
		}

		m_pxParticle[nNum].m_vecAccel.x += (m_vecEnvironment.x + m_pxParticle[nNum].m_vecLocalForce.x) / m_pxParticle[nNum].m_fMass;
		m_pxParticle[nNum].m_vecAccel.y += (m_vecEnvironment.y + m_pxParticle[nNum].m_vecLocalForce.y) / m_pxParticle[nNum].m_fMass;
		m_pxParticle[nNum].m_vecAccel.z += (m_vecEnvironment.z + m_pxParticle[nNum].m_vecLocalForce.z) / m_pxParticle[nNum].m_fMass;

		m_pxParticle[nNum].m_vecVel.x += m_pxParticle[nNum].m_vecAccel.x * m_fDeltaTime;//*0.01f;
		m_pxParticle[nNum].m_vecVel.y += m_pxParticle[nNum].m_vecAccel.y * m_fDeltaTime;//*0.01f;
		m_pxParticle[nNum].m_vecVel.z += m_pxParticle[nNum].m_vecAccel.z * m_fDeltaTime;//*0.01f;

		m_pxParticle[nNum].m_vecOldPos = m_pxParticle[nNum].m_vecPos;

		m_pxParticle[nNum].m_vecPos.x += m_pxParticle[nNum].m_vecVel.x * m_fDeltaTime;//*0.01f;
		m_pxParticle[nNum].m_vecPos.y += m_pxParticle[nNum].m_vecVel.y * m_fDeltaTime;//*0.01f;
		m_pxParticle[nNum].m_vecPos.z += m_pxParticle[nNum].m_vecVel.z * m_fDeltaTime;//*0.01f;
	}
}


VOID CParticleSystem::UpdateCrash(INT nNum)
{
	if ( !m_pxParticle[nNum].m_bIsDead )
	{
		if ( m_pxParticle[nNum].m_vecPos.x <= m_vecBounding[0].x || m_pxParticle[nNum].m_vecPos.x >= m_vecBounding[1].x )
		{	
			m_pxParticle[nNum].m_vecVel.x = -m_pxParticle[nNum].m_vecVel.x * 0.7f;
		}

		if ( m_pxParticle[nNum].m_vecPos.y <= m_vecBounding[0].y || m_pxParticle[nNum].m_vecPos.y >= m_vecBounding[1].y )
		{
			FLOAT fOldX = m_pxParticle[nNum].m_vecPos.x - m_pxParticle[nNum].m_vecVel.x * m_pxParticle[nNum].m_nDelay;
			FLOAT fOldY = m_pxParticle[nNum].m_vecPos.y - m_pxParticle[nNum].m_vecVel.y * m_pxParticle[nNum].m_nDelay;

			FLOAT fBefore = 0.0f;
			FLOAT fAfter  = 0.0f;

			if ( (m_pxParticle[nNum].m_vecPos.y - fOldY) != 0 )
				fBefore = m_pxParticle[nNum].m_nDelay * (m_vecBounding[1].y - fOldY)/(m_pxParticle[nNum].m_vecPos.y - fOldY);				// �浹��.

			if ( (m_pxParticle[nNum].m_vecPos.y - fOldY) != 0 )
				fAfter  = m_pxParticle[nNum].m_nDelay * (m_pxParticle[nNum].m_vecPos.y - m_vecBounding[1].y)/(m_pxParticle[nNum].m_vecPos.y - fOldY);	// �浹��.
			
			m_pxParticle[nNum].m_vecPos.x = fOldX + m_pxParticle[nNum].m_vecVel.x * fBefore;
			m_pxParticle[nNum].m_vecPos.y = fOldY + m_pxParticle[nNum].m_vecVel.y * fBefore;
			
			m_pxParticle[nNum].m_vecVel.y = -m_pxParticle[nNum].m_vecVel.y * 0.6f;
			
			m_pxParticle[nNum].m_vecPos.x += m_pxParticle[nNum].m_vecVel.x * fAfter;
			m_pxParticle[nNum].m_vecPos.y += m_pxParticle[nNum].m_vecVel.y * fAfter;			
		}

		if ( m_pxParticle[nNum].m_vecPos.z <= m_vecBounding[0].z || m_pxParticle[nNum].m_vecPos.z >= m_vecBounding[1].y )
		{
			m_pxParticle[nNum].m_vecVel.z = -m_pxParticle[nNum].m_vecVel.z * 0.6f;
		}
	}
}



/******************************************************************************************************************

	CScatter Class	

*******************************************************************************************************************/
CScatter::CScatter()
{
	InitSystem();
}


CScatter::~CScatter()
{
	DestroySystem();
}



VOID CScatter::InitSystem()
{
	CParticleSystem::InitSystem();
	m_shPartNum = 0;
}


VOID CScatter::DestroySystem()
{
	CParticleSystem::DestroySystem();
	InitSystem();
}


VOID CScatter::SetupSystem(WORD wCnt)
{
	InitSystem();
	CParticleSystem::SetupSystem(wCnt);
}


VOID CScatter::UpdateSystem(INT nLoopTime, D3DVECTOR vecGenPos)
{
	// ���� ��ǥ���� ������ǥ�� �̵�.
	INT nPartCnt = 0;

    for ( INT nCnt = 0; nCnt < m_nNum; nCnt++ )
    {
		if ( !m_pxParticle[nCnt].m_bIsDead )
		{
			if ( !m_pxParticle[nCnt].m_bIsDead )
			{
				m_pxParticle[nCnt].m_nCurrLife += nLoopTime;
			}

			// �Ÿ�����.
			INT nScrnGapX, nScrnGapY;
			if ( m_pxParticle[nCnt].m_vecPos.x != m_pxParticle[nCnt].m_vecDstPos.x )
			{
				nScrnGapX = abs(m_pxParticle[nCnt].m_vecDstPos.x-m_pxParticle[nCnt].m_vecPos.x);
			}
			else
			{
				nScrnGapX = 1;
			}

			if ( m_pxParticle[nCnt].m_vecPos.y != m_pxParticle[nCnt].m_vecDstPos.y )
			{
				nScrnGapY = abs(m_pxParticle[nCnt].m_vecDstPos.y-m_pxParticle[nCnt].m_vecPos.y);
			}
			else
			{
				nScrnGapY = 1;
			}

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

			INT nDisX, nDisY;

			if ( nScrnGapX > nScrnGapY )
			{
				nDisX = (INT)((m_pxParticle[nCnt].m_vecDstPos.x-m_pxParticle[nCnt].m_vecPos.x) * (fGapRateX));
				nDisY = (INT)((m_pxParticle[nCnt].m_vecDstPos.y-m_pxParticle[nCnt].m_vecPos.y) * (fGapRateX));
			}
			else
			{
				nDisX = (INT)((m_pxParticle[nCnt].m_vecDstPos.x-m_pxParticle[nCnt].m_vecPos.x) * (fGapRateY));
				nDisY = (INT)((m_pxParticle[nCnt].m_vecDstPos.y-m_pxParticle[nCnt].m_vecPos.y) * (fGapRateY));
			}

			FLOAT fDisX = (FLOAT)((FLOAT)nDisX/1000);
			FLOAT fDisY = (FLOAT)((FLOAT)nDisY/1000);

			m_pxParticle[nCnt].m_vecOldPos.x = m_pxParticle[nCnt].m_vecPos.x;
			m_pxParticle[nCnt].m_vecOldPos.y = m_pxParticle[nCnt].m_vecPos.y;

			m_pxParticle[nCnt].m_vecPos.x += ((fDisX)*(FLOAT)nLoopTime);
			m_pxParticle[nCnt].m_vecPos.y += ((fDisY)*(FLOAT)nLoopTime);

		   INT nabsX, nabsY;
		   nabsX = abs(m_pxParticle[nCnt].m_vecDstPos.x-m_pxParticle[nCnt].m_vecPos.x);
		   nabsY = abs(m_pxParticle[nCnt].m_vecDstPos.y-m_pxParticle[nCnt].m_vecPos.y);
 		   if ( (nabsX <= 10 && nabsY <= 10) || (nabsX >= m_pxParticle[nCnt].m_vecPrevDis.x && nabsY >= m_pxParticle[nCnt].m_vecPrevDis.y) )
		   {
//			   if ( m_pxParticle[nCnt].m_nCurrLife > m_pxParticle[nCnt].m_nLife /*|| nCnt%5*/ )
			   {
				   m_pxParticle[nCnt].Init();
				   m_shPartNum--;
			   }
//			   else
			   {
//				   m_pxParticle[nCnt].m_fSize = (FLOAT)GetRandomNum(10, 50);
			   }
		   }
		   else
		   {
				m_pxParticle[nCnt].m_vecPrevDis.x = nabsX;
				m_pxParticle[nCnt].m_vecPrevDis.y = nabsY;
		   }


		   nPartCnt++;
		}

		if ( nPartCnt >= m_shPartNum )
		{
			return;
		}
    }
}


HRESULT CScatter::RenderSystem(LPDIRECT3DDEVICE9 lpDevice)
{
	// [FIX-R3] Was a D3D direct draw, which Present()'s opaque full screen blit
	// discards every frame. Blit into the 565 soft buffer instead. Geometry is
	// identical to the old matrix path: Scale(4, fSize) * RotZ(angle) * Translate,
	// with screen = (worldX + 400, 300 - worldY).
	(void)lpDevice;

	INT nPartCnt = 0;

	for ( INT nCnt = 0; nCnt < m_nNum; nCnt++ )
	{
		CParticle* p = &m_pxParticle[nCnt];
		if ( p->m_bIsDead ) continue;

		IDirect3DTexture9* lpddsTextr = g_xGameProc.m_xImage.GetTextrImg(_TEXTR_FILE_PARTICLE, _IMAGE_PROGUSE, 520 + p->m_nCurrFrame);
		if ( lpddsTextr )
		{
			FLOAT fDx = p->m_vecPos.x - p->m_vecOldPos.x;
			FLOAT fDy = p->m_vecPos.y - p->m_vecOldPos.y;
			FLOAT fRot = 0.0f;
			if ( fDx != 0.0f || fDy != 0.0f )
				fRot = (FLOAT)(g_PI / 2 - atan( fDy / fDx ));

			g_xGameProc.m_xImage.DrawTexQuadSoft( lpddsTextr, p->m_vecPos.x, p->m_vecPos.y,
												  4.0f, p->m_fSize, fRot,
												  _BLEND_LIGHT, p->m_bRed, p->m_bGreen, p->m_bBlue );
		}

		nPartCnt++;
		if ( nPartCnt >= m_shPartNum ) break;
	}

	return S_OK;
}


VOID CScatter::SetParticles(D3DVECTOR vecDstPos)
{
	INT nGenCnt = 0;
	for ( INT nCnt = 0; nCnt < m_nNum; nCnt++ )
	{
		if ( m_pxParticle[nCnt].m_bIsDead )
		{
			m_pxParticle[nCnt].m_vecPos		= D3DVECTOR((FLOAT)vecDstPos.x+GetRandomNum(-400, 400), (FLOAT)vecDstPos.y+GetRandomNum(-400, 400), 0.0f);
			m_pxParticle[nCnt].m_vecDstPos	= vecDstPos;
			m_pxParticle[nCnt].m_nLife		= GetRandomNum(2000, 2200);
			m_pxParticle[nCnt].m_fMass		= 1000 + _RAND_FLOAT;
			m_pxParticle[nCnt].m_fSize		= (FLOAT)GetRandomNum(30, 100) + _RAND_FLOAT;
			m_pxParticle[nCnt].m_bIsDead	= FALSE;
			m_pxParticle[nCnt].m_bRed		= m_pxParticle[nCnt].m_bFstRed   = (FLOAT)GetRandomNum(100, 125);
			m_pxParticle[nCnt].m_bGreen		= m_pxParticle[nCnt].m_bFstGreen = m_pxParticle[nCnt].m_bFstRed/2;
			m_pxParticle[nCnt].m_bBlue		= m_pxParticle[nCnt].m_bFstBlue  = (FLOAT)GetRandomNum(0, 15);
/*			m_pxParticle[nCnt].m_bRed		= m_pxParticle[nCnt].m_bFstRed   = (FLOAT)GetRandomNum(50, 255);
			m_pxParticle[nCnt].m_bGreen		= m_pxParticle[nCnt].m_bFstGreen = (FLOAT)GetRandomNum(50, 255);
			m_pxParticle[nCnt].m_bBlue		= m_pxParticle[nCnt].m_bFstBlue  = (FLOAT)GetRandomNum(50, 255);*/
			m_pxParticle[nCnt].m_nDelay		= 300;
			m_pxParticle[nCnt].m_nCurrDelay = 0;
			m_pxParticle[nCnt].m_nCurrFrame = 0;
			m_pxParticle[nCnt].m_bOpa		= 255;

			m_shPartNum++;

			nGenCnt++;

			if ( nGenCnt >= 150 )
				return;
		}
	}
}




/******************************************************************************************************************

	CSnow Class	

*******************************************************************************************************************/
CSnow::CSnow()
{
	InitSystem();
}


CSnow::~CSnow()
{
	DestroySystem();
}



VOID CSnow::InitSystem()
{
	CParticleSystem::InitSystem();
	m_fWidth = m_fGround = 0.0f;
	m_shPartNum = 0;
}


VOID CSnow::DestroySystem()
{
	CParticleSystem::DestroySystem();

	InitSystem();
}


VOID CSnow::SetupSystem(WORD wCnt, FLOAT fWidth, FLOAT fGround)
{
	InitSystem();
	CParticleSystem::SetupSystem(wCnt);

	m_fWidth  = fWidth;
	m_fGround = fGround;

	SetEnvFactor(-0.05f, D3DVECTOR(10, 100, 0));
}


VOID CSnow::UpdateSystem(INT nLoopTime, D3DVECTOR vecGenPos)
{
	INT nGenCnt = 0; 
	INT nPartCnt = 0;
	INT nSpeedRate = (nLoopTime / 17);

	if ( nSpeedRate < 1 ) 		
	{
		nSpeedRate = 1;
	}

	m_fDeltaTime = 0.02f*nSpeedRate;

    for ( INT nCnt = 0; nCnt < m_nNum; nCnt++ )
    {
		// ��ƼŬ ����.
		if ( m_pxParticle[nCnt].m_bIsDead && nGenCnt < 5 )
		{
			SetParticleDefault(nCnt);
			m_shPartNum++;
			nGenCnt++;
		}

		if ( !m_pxParticle[nCnt].m_bIsDead )
		{
			m_pxParticle[nCnt].m_nCurrLife += nLoopTime;

			// ��ƼŬ �Ҹ�����.
			if ( m_pxParticle[nCnt].m_nCurrLife > m_pxParticle[nCnt].m_nLife || m_pxParticle[nCnt].m_vecPos.y >= m_fGround )
			{
				if( (m_pxParticle[nCnt].m_nCurrLife-m_pxParticle[nCnt].m_nLife) > 255 )
				{
					m_pxParticle[nCnt].Init();
					m_shPartNum--;
					nPartCnt--;
				}
				// ���� ������� �Ѵ�.
				else
				{
					BYTE bRate;
					bRate = (BYTE)(255 - (m_pxParticle[nCnt].m_nCurrLife-m_pxParticle[nCnt].m_nLife));

					m_pxParticle[nCnt].m_bOpa = bRate;
					
					if ( bRate < m_pxParticle[nCnt].m_bRed )				m_pxParticle[nCnt].m_bRed = bRate;
					if ( bRate < m_pxParticle[nCnt].m_bGreen )				m_pxParticle[nCnt].m_bGreen = bRate;
					if ( bRate < m_pxParticle[nCnt].m_bBlue )				m_pxParticle[nCnt].m_bBlue = bRate;

					nPartCnt++;
				}
				continue;
			}

			UpdateAirFiction(nCnt);
			UpdateMove(nCnt);

			m_pxParticle[nCnt].m_nCurrDelay += nLoopTime;
			if ( m_pxParticle[nCnt].m_nCurrDelay > m_pxParticle[nCnt].m_nDelay )
			{
				m_pxParticle[nCnt].m_nCurrDelay = 0;
				m_pxParticle[nCnt].m_nCurrFrame++;

//				FLOAT fRand = _RAND_FLOAT;
//				SetEnvFactor(fRand, D3DVECTOR(-fRand, fRand*100, 0));

				if ( m_pxParticle[nCnt].m_nCurrFrame >= _MAX_SNOWFRAME )
				{
					m_pxParticle[nCnt].m_nCurrFrame = 0;
				}
			}

			nPartCnt++;
		}
		if ( nPartCnt >= m_shPartNum )
		{
			return;
		}
    }
}


HRESULT CSnow::RenderSystem(LPDIRECT3DDEVICE9 lpDevice)
{
	if ( lpDevice )
	{
		D3DMATRIX matWorldOriginal;
		D3DMATRIX matTrans;
		D3DMATRIX matScale;
		D3DMATRIX matWorld;
		D3DVECTOR vTrans;
		D3DMATERIAL9 mtrl;
		INT nPartCnt = 0;

		if( SUCCEEDED(lpDevice->BeginScene()) )
		{
			lpDevice->GetTransform(D3DTS_WORLD, &matWorldOriginal);

			for ( INT nCnt = 0; nCnt < m_nNum; nCnt++ )
			{
				if ( !m_pxParticle[nCnt].m_bIsDead )
				{
					D3DUtil_InitMaterial(mtrl, 
										 (FLOAT)m_pxParticle[nCnt].m_bRed/255.0f,
										 (FLOAT)m_pxParticle[nCnt].m_bGreen/255.0f, 
										 (FLOAT)m_pxParticle[nCnt].m_bBlue/255.0f);
					mtrl.Diffuse.a = 1.0f;
					lpDevice->SetMaterial(&mtrl);

  					vTrans.x = (FLOAT) m_pxParticle[nCnt].m_vecPos.x-400;
					vTrans.y = (FLOAT)-m_pxParticle[nCnt].m_vecPos.y+300;
					vTrans.z = (FLOAT) m_pxParticle[nCnt].m_vecPos.z;
					D3DUtil_SetScaleMatrix(matScale, m_pxParticle[nCnt].m_fSize, m_pxParticle[nCnt].m_fSize, m_pxParticle[nCnt].m_fSize);
					D3DUtil_SetTranslateMatrix(matTrans, vTrans);
					D3DMath_MatrixMultiply(matWorld, matScale, matTrans);
					lpDevice->SetTransform(D3DTS_WORLD, &matWorld);

					lpDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
					lpDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
					lpDevice->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE);
					lpDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
					lpDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
					lpDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

					IDirect3DTexture9* lpddsParticleTextr = g_xGameProc.m_xImage.GetTextrImg(_TEXTR_FILE_PARTICLE, _IMAGE_PROGUSE, 500+m_pxParticle[nCnt].m_nCurrFrame);
					lpDevice->SetTexture(0, lpddsParticleTextr);
					lpDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, m_avShape, sizeof(D3DVERTEX));

					lpDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
					lpDevice->SetRenderState(D3DRS_SRCBLEND , D3DBLEND_ONE);
					lpDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
					lpDevice->SetTransform(D3DTS_WORLD, &matWorldOriginal);

					lpDevice->SetTransform(D3DTS_WORLD, &matWorldOriginal);
					nPartCnt++;
				}
				if ( nPartCnt >= m_shPartNum )
				{
					break;
				}
			}
			lpDevice->EndScene();

			return S_OK;
		}
	}
	return E_FAIL;
}


VOID CSnow::SetParticleDefault(INT nNum, D3DVECTOR vecGenPos)
{
    m_pxParticle[nNum].m_vecPos		= D3DVECTOR((FLOAT)GetRandomNum(0, (INT)m_fWidth)/*(-400, 1200)*/, (FLOAT)GetRandomNum(-300,  0), 0.0f);
    m_pxParticle[nNum].m_vecVel		= D3DVECTOR((FLOAT)GetRandomNum(-30,  30), (FLOAT)GetRandomNum(70, 100), 0.0f);
	m_pxParticle[nNum].m_vecAccel	= D3DVECTOR(0.0f, 0.0f, 0.0f);
	m_pxParticle[nNum].m_vecOldPos	= D3DVECTOR(0.0f, 0.0f, 0.0f);
	m_pxParticle[nNum].m_vecLocalForce = D3DVECTOR(0.0f, 0.0f, 0.0f);
	m_pxParticle[nNum].m_nLife		= GetRandomNum(2500,  7000);
	m_pxParticle[nNum].m_fMass		= 1000 + _RAND_FLOAT;
	m_pxParticle[nNum].m_fSize		= (FLOAT)GetRandomNum(2, 6) + _RAND_FLOAT;
//	m_pxParticle[nNum].m_nLife		= (FLOAT)GetRandomNum((BYTE)m_pxParticle[nNum].m_fSize*2000,  6000);
	m_pxParticle[nNum].m_bIsDead	= FALSE;
	m_pxParticle[nNum].m_bRed		= m_pxParticle[nNum].m_bGreen = m_pxParticle[nNum].m_bBlue = (BYTE)GetRandomNum(/*(BYTE)(m_pxParticle[nNum].m_fSize)*30+20*/200, 255);
	//	m_pxParticle[nNum].m_bRed		= (FLOAT)GetRandomNum(0, 255);
//	m_pxParticle[nNum].m_bGreen		= (FLOAT)GetRandomNum(0, 255);
//	m_pxParticle[nNum].m_bBlue		= (FLOAT)GetRandomNum(0, 255);
	m_pxParticle[nNum].m_nDelay		= 300;//(FLOAT)GetRandomNum(50, 150);
	m_pxParticle[nNum].m_nCurrLife	= 0;
	m_pxParticle[nNum].m_nCurrDelay = 0;
	m_pxParticle[nNum].m_nCurrFrame = 0;
	m_pxParticle[nNum].m_bOpa		= 255;
}









/******************************************************************************************************************

	CRain Class	

*******************************************************************************************************************/
CRain::CRain()
{
	InitSystem();
}


CRain::~CRain()
{
	DestroySystem();
}


VOID CRain::InitSystem()
{
	CParticleSystem::InitSystem();
	m_fWidth = m_fGround = 0.0f;
	m_bRainIntensity = 0;
	m_bGenCnt = 10;
}


VOID CRain::DestroySystem()
{
	CParticleSystem::DestroySystem();

	InitSystem();
}


VOID CRain::SetupSystem(WORD wCnt, FLOAT fWidth, FLOAT fGround)
{
	InitSystem();
	CParticleSystem::SetupSystem(wCnt);

	m_fWidth  = fWidth;
	m_fGround = fGround;

	SetEnvFactor(-0.05f, D3DVECTOR(10, 100, 0));
}


VOID CRain::UpdateSystem(INT nLoopTime, D3DVECTOR vecGenPos)
{
	INT nGenCnt = 0; 
	INT nPartCnt = 0;
	INT nSpeedRate = (nLoopTime / 17);
	if ( nSpeedRate < 1 ) 		
	{
		nSpeedRate = 1;
	}

	m_fDeltaTime = 0.02f*nSpeedRate;

    for ( INT nCnt = 0; nCnt < m_nNum; nCnt++ )
    {
		// ��ƼŬ ����.
		if ( m_pxParticle[nCnt].m_bIsDead && nGenCnt < 11/*m_bGenCnt*/ )
		{
			SetParticleDefault(nCnt);
			m_shPartNum++;
			nGenCnt++;
		}

		if ( !m_pxParticle[nCnt].m_bIsDead )
		{
			m_pxParticle[nCnt].m_nCurrLife += nLoopTime;

			// ��ƼŬ �Ҹ�����.
			if ( m_pxParticle[nCnt].m_nCurrLife > m_pxParticle[nCnt].m_nLife || m_pxParticle[nCnt].m_vecPos.y >= m_fGround )
			{
				if( (m_pxParticle[nCnt].m_nCurrLife-m_pxParticle[nCnt].m_nLife) > /*_MAX_RAINDROPFRAME*50*/250 )
				{
					m_pxParticle[nCnt].Init();
					m_shPartNum--;
					nPartCnt--;
				}
				else
				{
					BYTE bRate;
					bRate = (BYTE)(250 - (m_pxParticle[nCnt].m_nCurrLife-m_pxParticle[nCnt].m_nLife) / 4);

					m_pxParticle[nCnt].m_bOpa = bRate;
					m_pxParticle[nCnt].m_nDelay = 50;
					
					if ( bRate < m_pxParticle[nCnt].m_bRed )				m_pxParticle[nCnt].m_bRed = bRate;
					if ( bRate < m_pxParticle[nCnt].m_bGreen )				m_pxParticle[nCnt].m_bGreen = bRate;
					if ( bRate < m_pxParticle[nCnt].m_bBlue )				m_pxParticle[nCnt].m_bBlue = bRate;

					m_pxParticle[nCnt].m_nCurrDelay += nLoopTime;
					if ( m_pxParticle[nCnt].m_nCurrDelay > m_pxParticle[nCnt].m_nDelay )
					{
						m_pxParticle[nCnt].m_nCurrDelay = 0;
						m_pxParticle[nCnt].m_nCurrFrame++;

						if ( m_pxParticle[nCnt].m_nCurrFrame >= _MAX_RAINDROPFRAME )
						{
							m_pxParticle[nCnt].m_nCurrFrame = 0;
						}
					}
					nPartCnt++;
				}
				continue;
			}

			UpdateAirFiction(nCnt);
			UpdateMove(nCnt);

			m_pxParticle[nCnt].m_nCurrDelay += nLoopTime;
			if ( m_pxParticle[nCnt].m_nCurrDelay > m_pxParticle[nCnt].m_nDelay )
			{
				m_pxParticle[nCnt].m_nCurrDelay = 0;
				m_pxParticle[nCnt].m_nCurrFrame++;

				if ( m_pxParticle[nCnt].m_nCurrFrame >= _MAX_RAINFRAME )
				{
					m_pxParticle[nCnt].m_nCurrFrame = 0;
				}
			}

			nPartCnt++;
		}
		if ( nPartCnt >= m_shPartNum )
		{
			return;
		}
    }
}




HRESULT CRain::RenderSystem(LPDIRECT3DDEVICE9 lpDevice)
{
	if ( lpDevice )
	{
		D3DMATRIX matWorldOriginal;
		D3DMATRIX matTrans;
		D3DMATRIX matScale;
		D3DMATRIX matRot;
		D3DMATRIX matWorld;
		D3DMATRIX matTmpWorld;
		D3DVECTOR vTrans;
		D3DMATERIAL9 mtrl;
		INT nPartCnt = 0;

		if( SUCCEEDED(lpDevice->BeginScene()) )
		{
			lpDevice->GetTransform(D3DTS_WORLD, &matWorldOriginal);

			for ( INT nCnt = 0; nCnt < m_nNum; nCnt++ )
			{
				if ( !m_pxParticle[nCnt].m_bIsDead )
				{
					D3DUtil_InitMaterial(mtrl, 
										 (FLOAT)m_pxParticle[nCnt].m_bRed/255.0f,
										 (FLOAT)m_pxParticle[nCnt].m_bGreen/255.0f, 
										 (FLOAT)m_pxParticle[nCnt].m_bBlue/255.0f);
					mtrl.Diffuse.a = 1.0f;
					lpDevice->SetMaterial(&mtrl);
				
  					vTrans.x = (FLOAT) m_pxParticle[nCnt].m_vecPos.x-400;
					vTrans.y = (FLOAT)-m_pxParticle[nCnt].m_vecPos.y+300;
					vTrans.z = (FLOAT) m_pxParticle[nCnt].m_vecPos.z; 
					D3DUtil_SetScaleMatrix(matScale, 1.5f, m_pxParticle[nCnt].m_fSize, 1.0f);
					if ( (m_pxParticle[nCnt].m_vecPos.x - m_pxParticle[nCnt].m_vecOldPos.x) != 0 )
						D3DUtil_SetRotateZMatrix(matRot, (FLOAT)(g_PI/2 - atan((m_pxParticle[nCnt].m_vecPos.y - m_pxParticle[nCnt].m_vecOldPos.y)/(m_pxParticle[nCnt].m_vecPos.x - m_pxParticle[nCnt].m_vecOldPos.x))));
					else
						D3DUtil_SetRotateZMatrix(matRot, 0.0f);
					D3DUtil_SetTranslateMatrix(matTrans, vTrans);
					D3DMath_MatrixMultiply(matTmpWorld, matRot, matTrans);
					D3DMath_MatrixMultiply(matWorld, matScale, matTmpWorld);
					lpDevice->SetTransform(D3DTS_WORLD, &matWorld);

					INT nCurrFrame;
					if( (m_pxParticle[nCnt].m_nCurrLife-m_pxParticle[nCnt].m_nLife) > 0 && (m_pxParticle[nCnt].m_nCurrLife-m_pxParticle[nCnt].m_nLife) < 510 )
					{
						D3DUtil_SetScaleMatrix(matScale, 32.0f, 32.0f, 1.0f);
						D3DUtil_SetTranslateMatrix(matTrans, vTrans);
						D3DMath_MatrixMultiply(matTmpWorld, matRot, matTrans);
						D3DMath_MatrixMultiply(matWorld, matScale, matTmpWorld);
						lpDevice->SetTransform(D3DTS_WORLD, &matWorld);
						nCurrFrame = 510+m_pxParticle[nCnt].m_nCurrFrame;
					}
					else
					{
						nCurrFrame = 500+m_pxParticle[nCnt].m_nCurrFrame;
					}

					SetBlendRenderState(lpDevice, _BLEND_LIGHTINV, &mtrl);

					IDirect3DTexture9* lpddsParticleTextr = g_xGameProc.m_xImage.GetTextrImg(_TEXTR_FILE_PARTICLE, _IMAGE_PROGUSE, nCurrFrame);
					lpDevice->SetTexture(0, lpddsParticleTextr);

					lpDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, m_avShape, sizeof(D3DVERTEX));

					ResetBlendenderState(g_xMainWnd.Get3DDevice());

					lpDevice->SetTransform(D3DTS_WORLD, &matWorldOriginal);
					nPartCnt++;
				}
				if ( nPartCnt >= m_shPartNum )
				{
					break;
				}
			}

			lpDevice->EndScene();

			return S_OK;
		}
	}
	return E_FAIL;
}


VOID CRain::SetParticleDefault(INT nNum, D3DVECTOR vecGenPos)
{
	// m_bRainIntensity ---> ���� ������ ���������� gencnt, mass, size, delay, velocity� ������ ��ģ��.


	ZeroMemory(&m_pxParticle[nNum], sizeof(CParticle));
    m_pxParticle[nNum].m_vecPos		= D3DVECTOR((FLOAT)GetRandomNum(-100, (INT)(m_fWidth+100))/*(-400, 1200)*/, (FLOAT)GetRandomNum(-500, 0), 0.0f);
    m_pxParticle[nNum].m_vecVel		= D3DVECTOR((FLOAT)/*GetRandomNum(0,  10)*/0, 500/*(FLOAT)GetRandomNum(10, 30)*/, 0.0f);
//	m_pxParticle[nNum].m_vecAccel	= D3DVECTOR(0.0f, 0.0f, 0.0f);
//	m_pxParticle[nNum].m_vecOldPos	= D3DVECTOR(0.0f, 0.0f, 0.0f);
//	m_pxParticle[nNum].m_vecLocalForce = D3DVECTOR(0.0f, 0.0f, 0.0f);
	m_pxParticle[nNum].m_nLife		= GetRandomNum(800, 1400);
	m_pxParticle[nNum].m_fMass		= 100.0f;
	m_pxParticle[nNum].m_fSize		= (FLOAT)GetRandomNum(3, 30) + _RAND_FLOAT;
//	m_pxParticle[nNum].m_nLife		= (FLOAT)GetRandomNum((BYTE)m_pxParticle[nNum].m_fSize*2000,  6000);
//	m_pxParticle[nNum].m_bIsDead	= FALSE;
	m_pxParticle[nNum].m_bRed		= m_pxParticle[nNum].m_bGreen = m_pxParticle[nNum].m_bBlue = (BYTE)GetRandomNum(/*(BYTE)(m_pxParticle[nNum].m_fSize)*30+20*/200, 255);
	//	m_pxParticle[nNum].m_bRed		= (FLOAT)GetRandomNum(0, 255);
//	m_pxParticle[nNum].m_bGreen		= (FLOAT)GetRandomNum(0, 255);
//	m_pxParticle[nNum].m_bBlue		= (FLOAT)GetRandomNum(0, 255);
	m_pxParticle[nNum].m_nDelay		= GetRandomNum(50, 150);
//	m_pxParticle[nNum].m_nCurrLife	= 0;
//	m_pxParticle[nNum].m_nCurrDelay = 0;
//	m_pxParticle[nNum].m_nCurrFrame = 0;
//	m_pxParticle[nNum].m_bOpa		= 255;
}






/******************************************************************************************************************

	CFly Class	

*******************************************************************************************************************/
CFly::CFly()
{
	InitSystem();
}


CFly::~CFly()
{
	DestroySystem();
}


VOID CFly::InitSystem()
{
	CParticleSystem::InitSystem();
	m_bGenCnt = 3;
}


VOID CFly::DestroySystem()
{
	CParticleSystem::DestroySystem();

	InitSystem();
}


VOID CFly::SetupSystem(WORD wCnt)
{
	InitSystem();
	CParticleSystem::SetupSystem(wCnt);
	SetEnvFactor(-0.05f, D3DVECTOR(100, 1000, 0));
}


VOID CFly::UpdateSystem(INT nLoopTime, D3DVECTOR vecGenPos)
{
	BYTE bRate;
	INT nGenCnt = 0; 

	INT nSpeedRate = (nLoopTime / 17);
	if ( nSpeedRate < 1 ) 		
		nSpeedRate = 1;

	m_fDeltaTime = 0.02f*nSpeedRate;

    for ( INT nCnt = 0; nCnt < m_nNum; nCnt++ )
    {
		m_pxParticle[nCnt].m_nCurrLife += nLoopTime;

		if ( m_pxParticle[nCnt].m_nCurrLife > m_pxParticle[nCnt].m_nLife )
		{
			m_pxParticle[nCnt].m_bIsDead = TRUE;
		}
		else
		{
			m_pxParticle[nCnt].m_nCurrDelay += nLoopTime;
			if ( m_pxParticle[nCnt].m_nCurrDelay > m_pxParticle[nCnt].m_nDelay )
			{
				m_pxParticle[nCnt].m_nCurrDelay = 0;
				m_pxParticle[nCnt].m_nCurrFrame++;

				m_pxParticle[nCnt].m_fSize += 0.5f;


				if ( m_pxParticle[nCnt].m_nCurrFrame >= _MAX_FLYFRAME )
				{
					m_pxParticle[nCnt].m_nCurrFrame = 0;
				}
			}

/*			bRate = 255 - 255*(FLOAT)((FLOAT)m_pxParticle[nCnt].m_nCurrLife/(FLOAT)m_pxParticle[nCnt].m_nLife);
			m_pxParticle[nCnt].m_bRed = m_pxParticle[nCnt].m_bGreen = m_pxParticle[nCnt].m_bBlue = bRate;
*/			
			if ( m_pxParticle[nCnt].m_nLife == 0 )
			{
				m_pxParticle[nCnt].m_nLife = 1;
			}

			bRate = (BYTE)(m_pxParticle[nCnt].m_bFstRed - m_pxParticle[nCnt].m_bFstRed*(FLOAT)((FLOAT)m_pxParticle[nCnt].m_nCurrLife/(FLOAT)m_pxParticle[nCnt].m_nLife));
			m_pxParticle[nCnt].m_bRed	= bRate;
			bRate = (BYTE)(m_pxParticle[nCnt].m_bFstGreen - m_pxParticle[nCnt].m_bFstGreen*(FLOAT)((FLOAT)m_pxParticle[nCnt].m_nCurrLife/(FLOAT)m_pxParticle[nCnt].m_nLife));
			m_pxParticle[nCnt].m_bGreen = bRate;
			bRate = (BYTE)(m_pxParticle[nCnt].m_bFstBlue - m_pxParticle[nCnt].m_bFstBlue*(FLOAT)((FLOAT)m_pxParticle[nCnt].m_nCurrLife/(FLOAT)m_pxParticle[nCnt].m_nLife));
			m_pxParticle[nCnt].m_bBlue	= bRate;

			UpdateAirFiction(nCnt);
			UpdateMove(nCnt);
		}

		if ( m_pxParticle[nCnt].m_bIsDead )
		{
			if ( nGenCnt < m_bGenCnt )
			{
				SetParticleDefault(nCnt, vecGenPos);
				nGenCnt++;
			}
		}
    }
}


HRESULT CFly::RenderSystem(LPDIRECT3DDEVICE9 lpDevice)
{
	if ( lpDevice )
	{
		D3DMATRIX matWorldOriginal;
		D3DMATRIX matTrans;
		D3DMATRIX matScale;
		D3DMATRIX matWorld;
		D3DVECTOR vTrans;
		D3DMATERIAL9 mtrl;

		if( SUCCEEDED(lpDevice->BeginScene()) )
		{
			lpDevice->GetTransform(D3DTS_WORLD, &matWorldOriginal);

			for ( INT nCnt = 0; nCnt < m_nNum; nCnt++ )
			{
				if ( !m_pxParticle[nCnt].m_bIsDead )
				{
					D3DUtil_InitMaterial(mtrl, 
										 (FLOAT)m_pxParticle[nCnt].m_bRed/255.0f,
										 (FLOAT)m_pxParticle[nCnt].m_bGreen/255.0f, 
										 (FLOAT)m_pxParticle[nCnt].m_bBlue/255.0f);
					mtrl.Diffuse.a = 1.0f/255.0f;
					lpDevice->SetMaterial(&mtrl);

  					vTrans.x = (FLOAT) m_pxParticle[nCnt].m_vecPos.x-400;
					vTrans.y = (FLOAT)-m_pxParticle[nCnt].m_vecPos.y+300;
					vTrans.z = (FLOAT) m_pxParticle[nCnt].m_vecPos.z; 
					D3DUtil_SetScaleMatrix(matScale, m_pxParticle[nCnt].m_fSize, m_pxParticle[nCnt].m_fSize, 1);
					D3DUtil_SetTranslateMatrix(matTrans, vTrans);
					D3DMath_MatrixMultiply(matWorld, matScale, matTrans);
					lpDevice->SetTransform(D3DTS_WORLD, &matWorld);
				
					lpDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE );
					lpDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
					lpDevice->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE);
					lpDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
					lpDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
					lpDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

					IDirect3DTexture9* lpddsParticleTextr = g_xGameProc.m_xImage.GetTextrImg(_TEXTR_FILE_PARTICLE, _IMAGE_PROGUSE, 520+m_pxParticle[nCnt].m_nCurrFrame);
					lpDevice->SetTexture(0, lpddsParticleTextr);
					lpDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, m_avShape, sizeof(D3DVERTEX));

					lpDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
					lpDevice->SetRenderState(D3DRS_SRCBLEND , D3DBLEND_ONE);
					lpDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
					lpDevice->SetTransform(D3DTS_WORLD, &matWorldOriginal);

					lpDevice->SetTransform(D3DTS_WORLD, &matWorldOriginal);
				}
			}
			lpDevice->EndScene();

			return S_OK;
		}
	}
	return E_FAIL;
}


VOID CFly::SetParticleDefault(INT nNum, D3DVECTOR vecGenPos)
{
	// m_bRainIntensity ---> ���� ������ ���������� gencnt, mass, size, delay, velocity� ������ ��ģ��.

	ZeroMemory(&m_pxParticle[nNum], sizeof(CParticle));
    m_pxParticle[nNum].m_vecPos		= vecGenPos;
    m_pxParticle[nNum].m_vecVel		= D3DVECTOR((FLOAT)GetRandomNum(-30, 30), (FLOAT)GetRandomNum(-20, 40), 0.0f);
	m_pxParticle[nNum].m_nLife		= GetRandomNum(500, 1500);
	m_pxParticle[nNum].m_fMass		= 1000.0f;
	m_pxParticle[nNum].m_fSize		= (FLOAT)GetRandomNum(5, 40) + _RAND_FLOAT;
//	m_pxParticle[nNum].m_nLife		= (FLOAT)GetRandomNum((BYTE)m_pxParticle[nNum].m_fSize*2000,  6000);
//	m_pxParticle[nNum].m_bIsDead	= FALSE;
	m_pxParticle[nNum].m_bRed		= m_pxParticle[nNum].m_bFstRed   = (BYTE)GetRandomNum(200, 255);
	m_pxParticle[nNum].m_bGreen		= m_pxParticle[nNum].m_bFstGreen = (BYTE)(m_pxParticle[nNum].m_bFstRed/2);//(FLOAT)GetRandomNum(0, 125);
	m_pxParticle[nNum].m_bBlue		= m_pxParticle[nNum].m_bFstBlue  = (BYTE)GetRandomNum(0, 30);
	m_pxParticle[nNum].m_nDelay		= GetRandomNum(200, 300);

//	m_pxParticle[nNum].m_nCurrLife	= 0;
//	m_pxParticle[nNum].m_nCurrDelay = 0;
//	m_pxParticle[nNum].m_nCurrFrame = 0;
//	m_pxParticle[nNum].m_bOpa		= 255;
}








/******************************************************************************************************************

	CBoom Class	

*******************************************************************************************************************/
CBoom::CBoom()
{
	InitSystem();
}


CBoom::~CBoom()
{
	DestroySystem();
}


VOID CBoom::InitSystem()
{
	CParticleSystem::InitSystem();
	m_shPartNum = 0;
}


VOID CBoom::DestroySystem()
{
	CParticleSystem::DestroySystem();

	InitSystem();
}


VOID CBoom::SetupSystem(WORD wCnt)
{
	InitSystem();
	CParticleSystem::SetupSystem(wCnt);
	SetEnvFactor(-0.05f, D3DVECTOR(0, 200, 0));
}


VOID CBoom::UpdateSystem(INT nLoopTime, D3DVECTOR vecGenPos)
{
	INT nSpeedRate = (nLoopTime / 17);
	INT nPartCnt = 0;

	if ( nSpeedRate < 1 ) 		
		nSpeedRate = 1;

	m_fDeltaTime = 0.02f*nSpeedRate;

    for ( INT nCnt = 0; nCnt < m_nNum; nCnt++ )
    {
		if ( !m_pxParticle[nCnt].m_bIsDead )
		{
			m_pxParticle[nCnt].m_nCurrLife += nLoopTime;

			if ( m_pxParticle[nCnt].m_nCurrLife > m_pxParticle[nCnt].m_nLife )
			{
				m_pxParticle[nCnt].Init();
				m_shPartNum--;
				nPartCnt--;
			}
			else
			{
				m_pxParticle[nCnt].m_nCurrDelay += nLoopTime;

				m_pxParticle[nCnt].m_fMass		+= 3.0f;

				if ( m_pxParticle[nCnt].m_fSize < 0 )
					m_pxParticle[nCnt].m_fSize = 0;

				if ( m_pxParticle[nCnt].m_nCurrDelay > m_pxParticle[nCnt].m_nDelay )
				{
					m_pxParticle[nCnt].m_nCurrDelay = 0;
					m_pxParticle[nCnt].m_nCurrFrame++;

					if ( m_pxParticle[nCnt].m_nCurrFrame >= _MAX_BOOMFRAME )
					{
						m_pxParticle[nCnt].m_nCurrFrame = 0;
					}
				}

				if ( m_pxParticle[nCnt].m_nLife == 0 )
				{
					m_pxParticle[nCnt].m_nLife = 1;
				}

				m_pxParticle[nCnt].m_fSize = m_pxParticle[nCnt].m_fOriSize - m_pxParticle[nCnt].m_fOriSize*(FLOAT)((FLOAT)m_pxParticle[nCnt].m_nCurrLife/(FLOAT)m_pxParticle[nCnt].m_nLife);

				UpdateAirFiction(nCnt);
				UpdateMove(nCnt);
				nPartCnt++;
			}
		}
		if ( nPartCnt >= m_shPartNum )
		{
			return;
		}
    }
}


HRESULT CBoom::RenderSystem(LPDIRECT3DDEVICE9 lpDevice)
{
	// [FIX-R3] soft blit (see CScatter). Original blend was SRCBLEND_ONE /
	// DESTBLEND_INVSRCALPHA with a 1/255 diffuse alpha, i.e. additive.
	(void)lpDevice;

	INT nPartCnt = 0;

	for ( INT nCnt = 0; nCnt < m_nNum; nCnt++ )
	{
		CParticle* p = &m_pxParticle[nCnt];
		if ( p->m_bIsDead ) continue;

		IDirect3DTexture9* lpddsTextr = g_xGameProc.m_xImage.GetTextrImg(_TEXTR_FILE_PARTICLE, _IMAGE_PROGUSE, 520 + p->m_nCurrFrame);
		if ( lpddsTextr )
		{
			g_xGameProc.m_xImage.DrawTexQuadSoft( lpddsTextr, p->m_vecPos.x, p->m_vecPos.y,
												  p->m_fSize, p->m_fSize, 0.0f,
												  _BLEND_LIGHT, p->m_bRed, p->m_bGreen, p->m_bBlue );
		}

		nPartCnt++;
		if ( nPartCnt >= m_shPartNum ) break;
	}

	return S_OK;
}


VOID CBoom::SetBoomParticle(D3DVECTOR vecGenPos)
{
	INT nGenCnt = 0;
	for ( INT nCnt = 0; nCnt < m_nNum; nCnt++ )
	{
		if ( m_pxParticle[nCnt].m_bIsDead )
		{
			SetParticleDefault(nCnt, vecGenPos);
			m_shPartNum++;

			nGenCnt++;

			if ( nGenCnt > 1 )
//			if ( nGenCnt > 10 )
				return;
		}
	}

/*	for ( INT nCnt = 0; nCnt < m_nNum; nCnt++ )
	{
		if ( m_pxParticle[nCnt].m_bIsDead )
		{
//			vecGenPos.x += GetRandomNum(-2, 2);
//			vecGenPos.y += GetRandomNum(-2, 2);
//			vecGenPos.z += GetRandomNum(-10, 10);
			SetParticleDefault(nCnt, vecGenPos);
		}
	}
*/
}

VOID CBoom::SetBoomParticle2(D3DVECTOR vecGenPos)
{
	INT nGenCnt = 0;
	for ( INT nCnt = 0; nCnt < m_nNum; nCnt++ )
	{
		if ( m_pxParticle[nCnt].m_bIsDead )
		{
			SetParticleDefault2(nCnt, vecGenPos);
			m_shPartNum++;

			nGenCnt++;

			if ( nGenCnt > 20 )
				return;
		}
	}
}

VOID CBoom::SetBoomParticle3(D3DVECTOR vecGenPos)
{
	INT nGenCnt = 0;

	for ( INT nCnt = 0; nCnt < m_nNum; nCnt++ )
	{
		if ( m_pxParticle[nCnt].m_bIsDead )
		{
			ZeroMemory(&m_pxParticle[nCnt], sizeof(CParticle));
			m_pxParticle[nCnt].m_vecPos		= vecGenPos;
			m_pxParticle[nCnt].m_vecVel		= D3DVECTOR((FLOAT)GetRandomNum(-300, 300), (FLOAT)GetRandomNum(-300, 150), 0.0f);
			m_pxParticle[nCnt].m_nLife		= GetRandomNum(300, 1600);
			m_pxParticle[nCnt].m_fMass		= 10.0f;
			m_pxParticle[nCnt].m_fSize		= m_pxParticle[nCnt].m_fOriSize = (FLOAT)GetRandomNum(2, 2) + _RAND_FLOAT;

			m_pxParticle[nCnt].m_bRed		= m_pxParticle[nCnt].m_bFstRed	= (BYTE)GetRandomNum( 30,  60);
			m_pxParticle[nCnt].m_bGreen		= m_pxParticle[nCnt].m_bFstGreen= (BYTE)GetRandomNum(130, 160);
			m_pxParticle[nCnt].m_bBlue		= m_pxParticle[nCnt].m_bFstBlue	= (BYTE)GetRandomNum(130, 160);

			m_pxParticle[nCnt].m_nDelay		= GetRandomNum(200, 300);

			m_shPartNum++;

			nGenCnt++;

			if ( nGenCnt > 20 )
				return;
		}
	}
}

VOID CBoom::SetBoomParticle4(D3DVECTOR vecGenPos)
{
	INT nGenCnt = 0;
	for ( INT nCnt = 0; nCnt < m_nNum; nCnt++ )
	{
		if ( m_pxParticle[nCnt].m_bIsDead )
		{
			SetParticleDefault3(nCnt, vecGenPos);
			m_shPartNum++;

			nGenCnt++;

			if ( nGenCnt > 10 )
				return;
		}
	}
}

VOID CBoom::SetBoomParticle5(D3DVECTOR vecGenPos)
{
	INT nGenCnt = 0;
	for ( INT nCnt = 0; nCnt < m_nNum; nCnt++ )
	{
		if ( m_pxParticle[nCnt].m_bIsDead )
		{
			SetParticleDefault2(nCnt, vecGenPos);
			m_shPartNum++;

			nGenCnt++;

			if ( nGenCnt > 5 )
				return;
		}
	}
}

VOID CBoom::SetBoomParticle6(D3DVECTOR vecGenPos)
{
	INT nGenCnt = 0;
	for ( INT nCnt = 0; nCnt < 600; nCnt++ )
	{
		if ( m_pxParticle[nCnt].m_bIsDead )
		{
			ZeroMemory(&m_pxParticle[nCnt], sizeof(CParticle));
			m_pxParticle[nCnt].m_vecPos		= vecGenPos;
			m_pxParticle[nCnt].m_vecVel		= D3DVECTOR((FLOAT)GetRandomNum(-80, 80), (FLOAT)GetRandomNum(-180, 10), 0.0f);
			m_pxParticle[nCnt].m_nLife		= GetRandomNum(800, 1200);
			m_pxParticle[nCnt].m_fMass		= 5.0f;
			m_pxParticle[nCnt].m_fSize		= m_pxParticle[nCnt].m_fOriSize = (FLOAT)GetRandomNum(4, 5) + _RAND_FLOAT;

			m_pxParticle[nCnt].m_bRed		= m_pxParticle[nCnt].m_bFstRed	= (BYTE)GetRandomNum( 120, 130);
			m_pxParticle[nCnt].m_bGreen		= m_pxParticle[nCnt].m_bFstGreen= (BYTE)GetRandomNum( 200, 210);
			m_pxParticle[nCnt].m_bBlue		= m_pxParticle[nCnt].m_bFstBlue	= (BYTE)GetRandomNum( 230, 240);

			m_pxParticle[nCnt].m_nDelay		= GetRandomNum(0, 0);

			m_shPartNum++;

			nGenCnt++;

			if ( nGenCnt > 5 )
				return;
		}
	}
}


VOID CBoom::SetParticleDefault(INT nNum, D3DVECTOR vecGenPos)
{
/*	ZeroMemory(&m_pxParticle[nNum], sizeof(CParticle));
    m_pxParticle[nNum].m_vecPos		= vecGenPos;
    m_pxParticle[nNum].m_vecVel		= D3DVECTOR((FLOAT)GetRandomNum(-75, 75), (FLOAT)GetRandomNum(-180, -50), 0.0f);
	m_pxParticle[nNum].m_nLife		= GetRandomNum(800, 1200);
	m_pxParticle[nNum].m_fMass		= 1.0f;
	m_pxParticle[nNum].m_fSize		= m_pxParticle[nNum].m_fOriSize = (FLOAT)GetRandomNum(5, 40) + _RAND_FLOAT;
	m_pxParticle[nNum].m_bRed		= m_pxParticle[nNum].m_bFstRed   = (BYTE)GetRandomNum(0, 255);
	m_pxParticle[nNum].m_bGreen		= m_pxParticle[nNum].m_bFstGreen = (BYTE)(m_pxParticle[nNum].m_bFstRed/2);
	m_pxParticle[nNum].m_bBlue		= m_pxParticle[nNum].m_bFstBlue  = 0;
	m_pxParticle[nNum].m_nDelay		= GetRandomNum(200, 300);
*/
	ZeroMemory(&m_pxParticle[nNum], sizeof(CParticle));
    m_pxParticle[nNum].m_vecPos		= vecGenPos;
    m_pxParticle[nNum].m_vecVel		= D3DVECTOR((FLOAT)GetRandomNum(-75, 75), (FLOAT)GetRandomNum(-180, -50), 0.0f);
	m_pxParticle[nNum].m_nLife		= GetRandomNum(800, 1200);
	m_pxParticle[nNum].m_fMass		= 1.0f;
	m_pxParticle[nNum].m_fSize		= m_pxParticle[nNum].m_fOriSize = (FLOAT)GetRandomNum(2, 10) + _RAND_FLOAT;
	m_pxParticle[nNum].m_bRed		= m_pxParticle[nNum].m_bFstRed   = (BYTE)GetRandomNum( 50, 100);
	m_pxParticle[nNum].m_bGreen		= m_pxParticle[nNum].m_bFstGreen = (BYTE)GetRandomNum(100, 200);
	m_pxParticle[nNum].m_bBlue		= m_pxParticle[nNum].m_bFstBlue  = (BYTE)GetRandomNum(200, 255);
	m_pxParticle[nNum].m_nDelay		= GetRandomNum(200, 300);
}

VOID CBoom::SetParticleDefault2(INT nNum, D3DVECTOR vecGenPos)
{
	ZeroMemory(&m_pxParticle[nNum], sizeof(CParticle));
    m_pxParticle[nNum].m_vecPos		= vecGenPos;
    m_pxParticle[nNum].m_vecVel		= D3DVECTOR((FLOAT)GetRandomNum(-175, 175), (FLOAT)GetRandomNum(-180, 10), 0.0f);
	m_pxParticle[nNum].m_nLife		= GetRandomNum(800, 2000);
	m_pxParticle[nNum].m_fMass		= 1.0f;
	m_pxParticle[nNum].m_fSize		= m_pxParticle[nNum].m_fOriSize = (FLOAT)GetRandomNum(6, 7) + _RAND_FLOAT;

	m_pxParticle[nNum].m_bRed		= m_pxParticle[nNum].m_bFstRed	= (BYTE)GetRandomNum( 40, 50);
	m_pxParticle[nNum].m_bGreen		= m_pxParticle[nNum].m_bFstGreen= (BYTE)GetRandomNum( 40, 50);
	m_pxParticle[nNum].m_bBlue		= m_pxParticle[nNum].m_bFstBlue	= (BYTE)GetRandomNum( 40, 50);

	m_pxParticle[nNum].m_nDelay		= GetRandomNum(200, 300);
}

VOID CBoom::SetParticleDefault3(INT nNum, D3DVECTOR vecGenPos)
{
	ZeroMemory(&m_pxParticle[nNum], sizeof(CParticle));
    m_pxParticle[nNum].m_vecPos		= vecGenPos;
    m_pxParticle[nNum].m_vecVel		= D3DVECTOR((FLOAT)GetRandomNum(-120, 120), (FLOAT)GetRandomNum(-180, 0), 0.0f);
	m_pxParticle[nNum].m_nLife		= GetRandomNum(200, 1200);
	m_pxParticle[nNum].m_fMass		= 1.0f;
	m_pxParticle[nNum].m_fSize		= m_pxParticle[nNum].m_fOriSize = (FLOAT)GetRandomNum(3, 12) + _RAND_FLOAT;

	m_pxParticle[nNum].m_bRed		= m_pxParticle[nNum].m_bFstRed	= (BYTE)GetRandomNum( 40, 50);
	m_pxParticle[nNum].m_bGreen		= m_pxParticle[nNum].m_bFstGreen= (BYTE)GetRandomNum( 40, 50);
	m_pxParticle[nNum].m_bBlue		= m_pxParticle[nNum].m_bFstBlue	= (BYTE)GetRandomNum( 40, 50);

	m_pxParticle[nNum].m_nDelay		= GetRandomNum(200, 300);
}











/******************************************************************************************************************

	CFlyingTail Class	

*******************************************************************************************************************/
CFlyingTail::CFlyingTail()
{
	InitSystem();
}


CFlyingTail::~CFlyingTail()
{
	DestroySystem();
}


VOID CFlyingTail::InitSystem()
{
	CParticleSystem::InitSystem();
	m_shPartNum = 0;
}


VOID CFlyingTail::DestroySystem()
{
	CParticleSystem::DestroySystem();

	InitSystem();
}


VOID CFlyingTail::SetupSystem(WORD wCnt)
{
	InitSystem();
	CParticleSystem::SetupSystem(wCnt);
	SetEnvFactor(-0.05f, D3DVECTOR(100, 1000, 0));
}


VOID CFlyingTail::SetFlyTailParticle(D3DVECTOR vecGenPos)
{
	INT nGenCnt = 0;
	for ( INT nCnt = 0; nCnt < m_nNum; nCnt++ )
	{
		if ( m_pxParticle[nCnt].m_bIsDead )
		{
			SetParticleDefault(nCnt, vecGenPos);
			m_shPartNum++;

			nGenCnt++;

			if ( nGenCnt > 2 )
				return;
		}
	}
}

VOID CFlyingTail::SetFlyTailParticleEx(D3DVECTOR vecGenPos)
{
	INT nGenCnt = 0;

//	CONST FLOAT rSinVal[8] = { 0.0f, 0.707107f, 1.0f, 0.707107f, 0.0f, -0.707107f, -1.0f, -0.707107f };
//	CONST FLOAT rCosVal[8] = { 1.0f, 0.707107f, 0.0f, -0.707107f, -1.0f, -0.707107f, 0.0f, 0.707107f };
//	m_pxParticle[nCnt].m_vecPos	= D3DVECTOR(vecGenPos.x+rCosVal[nGenCnt]*20, vecGenPos.y+(FLOAT)+rSinVal[nGenCnt]*20, 0.0f);

	for ( INT nCnt = 0; nCnt < m_nNum; nCnt++ )
	{
		if ( m_pxParticle[nCnt].m_bIsDead )
		{
			ZeroMemory(&m_pxParticle[nCnt], sizeof(CParticle));
			m_pxParticle[nCnt].m_vecPos		= D3DVECTOR(vecGenPos.x+(FLOAT)GetRandomNum(-10, 10), vecGenPos.y+(FLOAT)GetRandomNum(-10, 10), 0.0f);
			m_pxParticle[nCnt].m_vecVel		= D3DVECTOR((FLOAT)GetRandomNum(-150, 150), (FLOAT)GetRandomNum(-80, 150), 0.0f);
			m_pxParticle[nCnt].m_nLife		= GetRandomNum(200, 1200);
			m_pxParticle[nCnt].m_fMass		= 1000.0f;
			m_pxParticle[nCnt].m_fSize		= m_pxParticle[nCnt].m_fOriSize = (FLOAT)GetRandomNum(5, 10) + _RAND_FLOAT;
			m_pxParticle[nCnt].m_bRed		= m_pxParticle[nCnt].m_bFstRed	= (BYTE)GetRandomNum(100, 110);
			m_pxParticle[nCnt].m_bGreen		= m_pxParticle[nCnt].m_bFstGreen= (BYTE)GetRandomNum(100, 110);
			m_pxParticle[nCnt].m_bBlue		= m_pxParticle[nCnt].m_bFstBlue	= (BYTE)GetRandomNum(245, 255);
			m_pxParticle[nCnt].m_nDelay		= GetRandomNum(200, 300);

			m_shPartNum++;

			nGenCnt++;

			if ( nGenCnt > 1 )
				return;
		}
	}
}

VOID CFlyingTail::SetFlyTailParticleEx2(D3DVECTOR vecGenPos)
{
	INT nGenCnt = 0;

	for ( INT nCnt = 0; nCnt < m_nNum; nCnt++ )
	{
		if ( m_pxParticle[nCnt].m_bIsDead )
		{
			ZeroMemory(&m_pxParticle[nCnt], sizeof(CParticle));
			m_pxParticle[nCnt].m_vecPos		= D3DVECTOR(vecGenPos.x+(FLOAT)GetRandomNum(-10, 10), vecGenPos.y+(FLOAT)GetRandomNum(-10, 10), 0.0f);
			m_pxParticle[nCnt].m_vecVel		= D3DVECTOR((FLOAT)GetRandomNum(-30, 30), (FLOAT)GetRandomNum(-30, 30), 0.0f);
			m_pxParticle[nCnt].m_nLife		= GetRandomNum(200, 1200);
			m_pxParticle[nCnt].m_fMass		= 1000.0f;
			m_pxParticle[nCnt].m_fSize		= m_pxParticle[nCnt].m_fOriSize = (FLOAT)GetRandomNum(3, 7) + _RAND_FLOAT;
			m_pxParticle[nCnt].m_bRed		= m_pxParticle[nCnt].m_bFstRed	= (BYTE)GetRandomNum(100, 150);
			m_pxParticle[nCnt].m_bGreen		= m_pxParticle[nCnt].m_bFstGreen= (BYTE)GetRandomNum(150, 255);
			m_pxParticle[nCnt].m_bBlue		= m_pxParticle[nCnt].m_bFstBlue	= (BYTE)GetRandomNum(100, 150);
			m_pxParticle[nCnt].m_nDelay		= GetRandomNum(200, 300);

			m_shPartNum++;

			nGenCnt++;

			if ( nGenCnt > 2 )
				return;
		}
	}
}

VOID CFlyingTail::SetFlyTailParticleEx3(D3DVECTOR vecGenPos)
{
	INT nGenCnt = 0;

	for ( INT nCnt = 0; nCnt < m_nNum; nCnt++ )
	{
		if ( m_pxParticle[nCnt].m_bIsDead )
		{
			ZeroMemory(&m_pxParticle[nCnt], sizeof(CParticle));
			m_pxParticle[nCnt].m_vecPos		= vecGenPos;
			m_pxParticle[nCnt].m_vecVel		= D3DVECTOR((FLOAT)GetRandomNum(-50, 50), (FLOAT)GetRandomNum(-30, 60), 0.0f);
			m_pxParticle[nCnt].m_nLife		= GetRandomNum(500, 900);
			m_pxParticle[nCnt].m_fMass		= 1000.0f;
			m_pxParticle[nCnt].m_fSize		= m_pxParticle[nCnt].m_fOriSize = (FLOAT)GetRandomNum(3, 5) + _RAND_FLOAT;
			m_pxParticle[nCnt].m_bRed		= m_pxParticle[nCnt].m_bFstRed   = (BYTE)GetRandomNum(200, 255);
			m_pxParticle[nCnt].m_bGreen		= m_pxParticle[nCnt].m_bFstGreen = (BYTE)(m_pxParticle[nCnt].m_bFstRed/2);//(FLOAT)GetRandomNum(0, 125);
			m_pxParticle[nCnt].m_bBlue		= m_pxParticle[nCnt].m_bFstBlue  = (BYTE)GetRandomNum(0, 30);
			m_pxParticle[nCnt].m_nDelay		= GetRandomNum(200, 300);

			m_shPartNum++;

			nGenCnt++;

			if ( nGenCnt > 4 )
				return;
		}
	}
}

VOID CFlyingTail::SetFlyTailParticleEx4(D3DVECTOR vecGenPos)
{
	INT nGenCnt = 0;
	for ( INT nCnt = 0; nCnt < m_nNum; nCnt++ )
	{
		if ( m_pxParticle[nCnt].m_bIsDead )
		{
			ZeroMemory(&m_pxParticle[nCnt], sizeof(CParticle));
			m_pxParticle[nCnt].m_vecPos		= vecGenPos;
			m_pxParticle[nCnt].m_vecVel		= D3DVECTOR((FLOAT)GetRandomNum(-50, 50), (FLOAT)GetRandomNum(-30, 60), 0.0f);
		
			m_pxParticle[nCnt].m_nLife		= GetRandomNum(20, 500);

			m_pxParticle[nCnt].m_fMass		= 10000.0f;
			m_pxParticle[nCnt].m_fSize		= (FLOAT)GetRandomNum(5, 15) + _RAND_FLOAT;
			m_pxParticle[nCnt].m_bRed		= m_pxParticle[nCnt].m_bFstRed   = (BYTE)GetRandomNum(70, 90);
			m_pxParticle[nCnt].m_bGreen		= m_pxParticle[nCnt].m_bFstGreen = (BYTE)GetRandomNum(70, 90);
			m_pxParticle[nCnt].m_bBlue		= m_pxParticle[nCnt].m_bFstBlue  = (BYTE)GetRandomNum(100, 150);
			m_pxParticle[nCnt].m_nDelay		= GetRandomNum(0, 10);

			m_shPartNum++;

			nGenCnt++;

			if ( nGenCnt > 2 )
				return;
		}
	}
}

VOID CFlyingTail::SetFlyTailParticleEx5(D3DVECTOR vecGenPos)
{
	INT nGenCnt = 0;
	for ( INT nCnt = 0; nCnt < m_nNum; nCnt++ )
	{
		if ( m_pxParticle[nCnt].m_bIsDead )
		{
			ZeroMemory(&m_pxParticle[nCnt], sizeof(CParticle));
			m_pxParticle[nCnt].m_vecPos		= vecGenPos;
			m_pxParticle[nCnt].m_vecVel		= D3DVECTOR((FLOAT)GetRandomNum(-45, 45), (FLOAT)GetRandomNum(-150, -250), 0.0f);
		
			m_pxParticle[nCnt].m_nLife		= GetRandomNum(1000, 1600);

			m_pxParticle[nCnt].m_fMass		= 100.0f;
			m_pxParticle[nCnt].m_fSize		= 4.0f;
			m_pxParticle[nCnt].m_bRed		= m_pxParticle[nCnt].m_bFstRed   = 255;
			m_pxParticle[nCnt].m_bGreen		= m_pxParticle[nCnt].m_bFstGreen = 125;;
			m_pxParticle[nCnt].m_bBlue		= m_pxParticle[nCnt].m_bFstBlue  = 0;
			m_pxParticle[nCnt].m_nDelay		= GetRandomNum(200, 300);

			m_shPartNum++;

			nGenCnt++;

			if ( nGenCnt > 2 )
				return;
		}
	}
}

VOID CFlyingTail::UpdateSystem(INT nLoopTime, D3DVECTOR vecGenPos)
{
	BYTE bRate;
	INT  nSpeedRate, nPartCnt;
	
	nPartCnt = 0;

	nSpeedRate = (nLoopTime / 17);

	if ( nSpeedRate < 1 ) 	nSpeedRate = 1;

	m_fDeltaTime = 0.02f*nSpeedRate;

    for ( INT nCnt = 0; nCnt < m_nNum; nCnt++ )
    {
		if ( !m_pxParticle[nCnt].m_bIsDead )
		{
			m_pxParticle[nCnt].m_nCurrLife += nLoopTime;

			if ( m_pxParticle[nCnt].m_nCurrLife > m_pxParticle[nCnt].m_nLife )
			{				
				m_pxParticle[nCnt].Init();
				m_shPartNum--;
				nPartCnt--;
			}
			else
			{
				m_pxParticle[nCnt].m_nCurrDelay += nLoopTime;
				if ( m_pxParticle[nCnt].m_nCurrDelay > m_pxParticle[nCnt].m_nDelay )
				{
					m_pxParticle[nCnt].m_nCurrDelay = 0;
					m_pxParticle[nCnt].m_nCurrFrame++;

					if ( m_pxParticle[nCnt].m_nCurrFrame >= _MAX_FLYTAILFRAME )
					{
						m_pxParticle[nCnt].m_nCurrFrame = 0;
					}
				}

				if ( m_pxParticle[nCnt].m_nLife == 0 )
				{
					m_pxParticle[nCnt].m_nLife = 1;
				}

				bRate = (BYTE)(m_pxParticle[nCnt].m_bFstRed - m_pxParticle[nCnt].m_bFstRed*(FLOAT)((FLOAT)m_pxParticle[nCnt].m_nCurrLife/(FLOAT)m_pxParticle[nCnt].m_nLife));
				m_pxParticle[nCnt].m_bRed	= bRate;
				bRate = (BYTE)(m_pxParticle[nCnt].m_bFstGreen - m_pxParticle[nCnt].m_bFstGreen*(FLOAT)((FLOAT)m_pxParticle[nCnt].m_nCurrLife/(FLOAT)m_pxParticle[nCnt].m_nLife));
				m_pxParticle[nCnt].m_bGreen = bRate;
				bRate = (BYTE)(m_pxParticle[nCnt].m_bFstBlue - m_pxParticle[nCnt].m_bFstBlue*(FLOAT)((FLOAT)m_pxParticle[nCnt].m_nCurrLife/(FLOAT)m_pxParticle[nCnt].m_nLife));
				m_pxParticle[nCnt].m_bBlue	= bRate;

				UpdateAirFiction(nCnt);
				UpdateMove(nCnt);
				nPartCnt++;
			}
		}
		if ( nPartCnt >= m_shPartNum )
		{
			return;
		}
    }
}


HRESULT CFlyingTail::RenderSystem(LPDIRECT3DDEVICE9 lpDevice)
{
	// [FIX-R3] soft blit (see CScatter). Fixed texture 520, additive blend.
	(void)lpDevice;

	INT nPartCnt = 0;

	for ( INT nCnt = 0; nCnt < m_nNum; nCnt++ )
	{
		CParticle* p = &m_pxParticle[nCnt];
		if ( p->m_bIsDead ) continue;

		IDirect3DTexture9* lpddsTextr = g_xGameProc.m_xImage.GetTextrImg(_TEXTR_FILE_PARTICLE, _IMAGE_PROGUSE, 520);
		if ( lpddsTextr )
		{
			g_xGameProc.m_xImage.DrawTexQuadSoft( lpddsTextr, p->m_vecPos.x, p->m_vecPos.y,
												  p->m_fSize, p->m_fSize, 0.0f,
												  _BLEND_LIGHT, p->m_bRed, p->m_bGreen, p->m_bBlue );
		}

		nPartCnt++;
		if ( nPartCnt >= m_shPartNum ) break;
	}

	return S_OK;
}


VOID CFlyingTail::SetParticleDefault(INT nNum, D3DVECTOR vecGenPos)
{
	ZeroMemory(&m_pxParticle[nNum], sizeof(CParticle));
    m_pxParticle[nNum].m_vecPos		= vecGenPos;
    m_pxParticle[nNum].m_vecVel		= D3DVECTOR((FLOAT)GetRandomNum(-50, 50), (FLOAT)GetRandomNum(-30, 60), 0.0f);

//	m_pxParticle[nNum].m_nLife		= (FLOAT)GetRandomNum(500, 1000);
	m_pxParticle[nNum].m_nLife		= GetRandomNum(150, 400);

	m_pxParticle[nNum].m_fMass		= 1000.0f;
	m_pxParticle[nNum].m_fSize		= (FLOAT)GetRandomNum(5, 15) + _RAND_FLOAT;
	m_pxParticle[nNum].m_bRed		= m_pxParticle[nNum].m_bFstRed   = (BYTE)GetRandomNum(200, 255);
	m_pxParticle[nNum].m_bGreen		= m_pxParticle[nNum].m_bFstGreen = (BYTE)(m_pxParticle[nNum].m_bFstRed/2);//(FLOAT)GetRandomNum(0, 125);
	m_pxParticle[nNum].m_bBlue		= m_pxParticle[nNum].m_bFstBlue  = (BYTE)GetRandomNum(0, 30);
/*
	m_pxParticle[nNum].m_bRed		= m_pxParticle[nNum].m_bFstRed   = (FLOAT)GetRandomNum(200, 255);
	m_pxParticle[nNum].m_bGreen		= m_pxParticle[nNum].m_bFstGreen = (FLOAT)GetRandomNum(0, 150);
	m_pxParticle[nNum].m_bBlue		= m_pxParticle[nNum].m_bFstBlue  = (FLOAT)GetRandomNum(0, 150);
*/	m_pxParticle[nNum].m_nDelay		= GetRandomNum(200, 300);
}











/******************************************************************************************************************

	CSmoke Class	

*******************************************************************************************************************/
CSmoke::CSmoke()
{
	InitSystem();
}


CSmoke::~CSmoke()
{
	DestroySystem();
}


VOID CSmoke::InitSystem()
{
	CParticleSystem::InitSystem();
	m_shPartNum = 0;
}


VOID CSmoke::DestroySystem()
{
	CParticleSystem::DestroySystem();

	InitSystem();
}


VOID CSmoke::SetupSystem(WORD wCnt)
{
	InitSystem();
	CParticleSystem::SetupSystem(wCnt);
	SetEnvFactor(-0.05f, D3DVECTOR(0, 1000, 0));
}


VOID CSmoke::UpdateSystem(INT nLoopTime, D3DVECTOR vecGenPos)
{
	BYTE bRate;
	INT  nSpeedRate, nPartCnt;
	D3DVECTOR vecAddVel;
	
	nPartCnt = 0;

	nSpeedRate = (nLoopTime / 17);

	if ( nSpeedRate < 1 ) 	nSpeedRate = 1;

	m_fDeltaTime = 0.02f*nSpeedRate;

//	SetSmokeParticle(vecGenPos);

    for ( INT nCnt = 0; nCnt < m_nNum; nCnt++ )
    {
		if ( !m_pxParticle[nCnt].m_bIsDead )
		{
			m_pxParticle[nCnt].m_nCurrLife += nLoopTime;

			if ( m_pxParticle[nCnt].m_nCurrLife > m_pxParticle[nCnt].m_nLife )
			{				
				m_pxParticle[nCnt].Init();
				m_shPartNum--;
				nPartCnt--;
			}
			else
			{
				m_pxParticle[nCnt].m_nCurrDelay += nLoopTime;
				if ( m_pxParticle[nCnt].m_nCurrDelay > m_pxParticle[nCnt].m_nDelay )
				{
					m_pxParticle[nCnt].m_nCurrDelay = 0;
					m_pxParticle[nCnt].m_nCurrFrame++;

					if ( m_pxParticle[nCnt].m_nCurrFrame >= _MAX_SMOKEFRAME )
					{
						m_pxParticle[nCnt].m_nCurrFrame = 0;
					}
				}

//				m_pxParticle[nCnt].m_fSize = m_pxParticle[nCnt].m_fOriSize + m_pxParticle[nCnt].m_fOriSize*15.0f*(FLOAT)((FLOAT)m_pxParticle[nCnt].m_nCurrLife/(FLOAT)m_pxParticle[nCnt].m_nLife);
//				vecAddVel = D3DVECTOR((FLOAT)GetRandomNum(-1, 1), GetRandomNum(-2, 0), 0.0f);

				if ( m_pxParticle[nCnt].m_nLife == 0 )
				{
					m_pxParticle[nCnt].m_nLife = 1;
				}

				m_pxParticle[nCnt].m_fSize = m_pxParticle[nCnt].m_fOriSize + m_pxParticle[nCnt].m_fOriSize*7.0f*(FLOAT)((FLOAT)m_pxParticle[nCnt].m_nCurrLife/(FLOAT)m_pxParticle[nCnt].m_nLife);
				vecAddVel = D3DVECTOR((FLOAT)GetRandomNum(-1, 1), 0.0f, 0.0f);
				m_pxParticle[nCnt].m_vecVel += vecAddVel;

				bRate = (BYTE)(m_pxParticle[nCnt].m_bFstRed - m_pxParticle[nCnt].m_bFstRed*(FLOAT)((FLOAT)m_pxParticle[nCnt].m_nCurrLife/(FLOAT)m_pxParticle[nCnt].m_nLife));
				m_pxParticle[nCnt].m_bRed	= bRate;
				bRate = (BYTE)(m_pxParticle[nCnt].m_bFstGreen - m_pxParticle[nCnt].m_bFstGreen*(FLOAT)((FLOAT)m_pxParticle[nCnt].m_nCurrLife/(FLOAT)m_pxParticle[nCnt].m_nLife));
				m_pxParticle[nCnt].m_bGreen = bRate;
				bRate = (BYTE)(m_pxParticle[nCnt].m_bFstBlue - m_pxParticle[nCnt].m_bFstBlue*(FLOAT)((FLOAT)m_pxParticle[nCnt].m_nCurrLife/(FLOAT)m_pxParticle[nCnt].m_nLife));
				m_pxParticle[nCnt].m_bBlue	= bRate;

				UpdateAirFiction(nCnt);
				UpdateMove(nCnt);
				nPartCnt++;
			}
		}
		if ( nPartCnt >= m_shPartNum )
			return;
    }
}


VOID CSmoke::SetSmokeParticle(D3DVECTOR vecGenPos)
{
	INT nGenCnt = 0;
	for ( INT nCnt = 0; nCnt < m_nNum; nCnt++ )
	{
		if ( m_pxParticle[nCnt].m_bIsDead )
		{
			SetParticleDefault(nCnt, vecGenPos);
			m_shPartNum++;

			nGenCnt++;

			if ( nGenCnt > 0 )
				return;
		}
	}
}


HRESULT CSmoke::RenderSystem(LPDIRECT3DDEVICE9 lpDevice)
{
	// [FIX-R3] soft blit (see CScatter). Original: unit quad * fSize, LIGHTINV,
	// material colour = particle colour.
	(void)lpDevice;

	INT nPartCnt = 0;

	for ( INT nCnt = 0; nCnt < m_nNum; nCnt++ )
	{
		CParticle* p = &m_pxParticle[nCnt];
		if ( p->m_bIsDead ) continue;

		IDirect3DTexture9* lpddsTextr = g_xGameProc.m_xImage.GetTextrImg(_TEXTR_FILE_PARTICLE, _IMAGE_PROGUSE, 530);
		if ( lpddsTextr )
		{
			g_xGameProc.m_xImage.DrawTexQuadSoft( lpddsTextr, p->m_vecPos.x, p->m_vecPos.y,
												  p->m_fSize, p->m_fSize, 0.0f,
												  _BLEND_LIGHTINV, p->m_bRed, p->m_bGreen, p->m_bBlue );
		}

		nPartCnt++;
		if ( nPartCnt >= m_shPartNum ) break;
	}

	return S_OK;
}


VOID CSmoke::SetParticleDefault(INT nNum, D3DVECTOR vecGenPos)
{
	ZeroMemory(&m_pxParticle[nNum], sizeof(CParticle));
//	vecGenPos.x += (FLOAT)GetRandomNum(-5, 5);
//	vecGenPos.y += (FLOAT)GetRandomNum(-8, 2);
    m_pxParticle[nNum].m_vecPos		= vecGenPos;
    m_pxParticle[nNum].m_vecVel		= D3DVECTOR((FLOAT)GetRandomNum(-8, 8), 0.0f, 0.0f);
//	m_pxParticle[nNum].m_nLife		= (FLOAT)GetRandomNum(500, 1000);
	m_pxParticle[nNum].m_nLife		= GetRandomNum(150, 400);
	m_pxParticle[nNum].m_fMass		= 1000.0f;
//	m_pxParticle[nNum].m_fSize		= m_pxParticle[nNum].m_fOriSize = (FLOAT)GetRandomNum(5, 10) + _RAND_FLOAT;
	m_pxParticle[nNum].m_fSize		= m_pxParticle[nNum].m_fOriSize = (FLOAT)GetRandomNum(5, 10) + _RAND_FLOAT;
	m_pxParticle[nNum].m_bRed		= m_pxParticle[nNum].m_bFstRed = m_pxParticle[nNum].m_bGreen = m_pxParticle[nNum].m_bFstGreen = m_pxParticle[nNum].m_bBlue = m_pxParticle[nNum].m_bFstBlue = (BYTE)GetRandomNum(100, 150);
	m_pxParticle[nNum].m_nDelay		= GetRandomNum(200, 300);
}


VOID CSmoke::SetSmokeParticleEx(D3DVECTOR vecGenPos)
{
	INT nGenCnt = 0;
	for ( INT nCnt = 0; nCnt < m_nNum; nCnt++ )
	{
		if ( m_pxParticle[nCnt].m_bIsDead )
		{
			ZeroMemory(&m_pxParticle[nCnt], sizeof(CParticle));
			m_pxParticle[nCnt].m_vecPos		= vecGenPos;
			m_pxParticle[nCnt].m_vecVel		= D3DVECTOR((FLOAT)GetRandomNum(-12, 12), 0.0f, 0.0f);
			m_pxParticle[nCnt].m_nLife		= GetRandomNum(200, 1200);
			m_pxParticle[nCnt].m_fMass		= 1000.0f;
			m_pxParticle[nCnt].m_fSize		= m_pxParticle[nCnt].m_fOriSize = (FLOAT)GetRandomNum(20, 25) + _RAND_FLOAT;
			m_pxParticle[nCnt].m_bRed		= m_pxParticle[nCnt].m_bFstRed	= (BYTE)GetRandomNum(80, 120);
			m_pxParticle[nCnt].m_bGreen		= m_pxParticle[nCnt].m_bFstGreen= (BYTE)GetRandomNum(80, 120);
			m_pxParticle[nCnt].m_bBlue		= m_pxParticle[nCnt].m_bFstBlue	= (BYTE)GetRandomNum(220, 255);
			m_pxParticle[nCnt].m_nDelay		= GetRandomNum(200, 300);

			m_shPartNum++;

			nGenCnt++;

			if ( nGenCnt > 0 )
				return;
		}
	}
}

VOID CSmoke::SetSmokeParticleEx2(D3DVECTOR vecGenPos)
{
	INT nGenCnt = 0;
	for ( INT nCnt = 0; nCnt < m_nNum; nCnt++ )
	{
		if ( m_pxParticle[nCnt].m_bIsDead )
		{
			ZeroMemory(&m_pxParticle[nCnt], sizeof(CParticle));
			m_pxParticle[nCnt].m_vecPos		= vecGenPos;
			m_pxParticle[nCnt].m_vecVel		= D3DVECTOR((FLOAT)GetRandomNum(-20, 20), 0.0f, 0.0f);
			m_pxParticle[nCnt].m_nLife		= GetRandomNum(400, 800);
			m_pxParticle[nCnt].m_fMass		= 1000.0f;
			m_pxParticle[nCnt].m_fSize		= m_pxParticle[nCnt].m_fOriSize = (FLOAT)GetRandomNum(20, 25) + _RAND_FLOAT;
			m_pxParticle[nCnt].m_bRed		= m_pxParticle[nCnt].m_bFstRed	= (BYTE)GetRandomNum( 50, 100);
			m_pxParticle[nCnt].m_bGreen		= m_pxParticle[nCnt].m_bFstGreen= (BYTE)GetRandomNum( 50, 100);
			m_pxParticle[nCnt].m_bBlue		= m_pxParticle[nCnt].m_bFstBlue	= (BYTE)GetRandomNum(255, 255);
			m_pxParticle[nCnt].m_nDelay		= GetRandomNum(200, 300);

			m_shPartNum++;

			nGenCnt++;

			if ( nGenCnt > 0 )
				return;
		}
	}
}

VOID CSmoke::SetSmokeParticleEx3(D3DVECTOR vecGenPos)
{
	INT nGenCnt = 0;
	for ( INT nCnt = 0; nCnt < m_nNum; nCnt++ )
	{
		if ( m_pxParticle[nCnt].m_bIsDead )
		{
			ZeroMemory(&m_pxParticle[nCnt], sizeof(CParticle));
			m_pxParticle[nCnt].m_vecPos		= vecGenPos;
			m_pxParticle[nCnt].m_vecVel		= D3DVECTOR((FLOAT)0, 0.0f, 0.0f);
			m_pxParticle[nCnt].m_nLife		= GetRandomNum(400, 500);
			m_pxParticle[nCnt].m_fMass		= 10000.0f;
			m_pxParticle[nCnt].m_fSize		= m_pxParticle[nCnt].m_fOriSize = 7.0f;
			m_pxParticle[nCnt].m_bRed		= m_pxParticle[nCnt].m_bFstRed	= (BYTE)GetRandomNum(100, 150);
			m_pxParticle[nCnt].m_bGreen		= m_pxParticle[nCnt].m_bFstGreen= (BYTE)GetRandomNum(150, 200);
			m_pxParticle[nCnt].m_bBlue		= m_pxParticle[nCnt].m_bFstBlue	= (BYTE)GetRandomNum(255, 255);
			m_pxParticle[nCnt].m_nDelay		= GetRandomNum(200, 300);

			m_shPartNum++;

			nGenCnt++;

			if ( nGenCnt > 0 )
				return;
		}
	}
}

VOID CSmoke::SetSmokeParticleEx4(D3DVECTOR vecGenPos)
{
	INT nGenCnt = 0;
	for ( INT nCnt = 0; nCnt < m_nNum; nCnt++ )
	{
		if ( m_pxParticle[nCnt].m_bIsDead )
		{
			ZeroMemory(&m_pxParticle[nCnt], sizeof(CParticle));
			m_pxParticle[nCnt].m_vecPos		= vecGenPos;
			m_pxParticle[nCnt].m_vecVel		= D3DVECTOR((FLOAT)GetRandomNum(-12, 12), (FLOAT)GetRandomNum(-120, -80), 0.0f);
			m_pxParticle[nCnt].m_nLife		= GetRandomNum(300, 600);
			m_pxParticle[nCnt].m_fMass		= 1000.0f;
			m_pxParticle[nCnt].m_fSize		= m_pxParticle[nCnt].m_fOriSize = (FLOAT)GetRandomNum(10, 30);
			m_pxParticle[nCnt].m_bRed		= m_pxParticle[nCnt].m_bFstRed	= (BYTE)GetRandomNum(100, 150);
			m_pxParticle[nCnt].m_bGreen		= m_pxParticle[nCnt].m_bFstGreen= (BYTE)GetRandomNum(150, 200);
			m_pxParticle[nCnt].m_bBlue		= m_pxParticle[nCnt].m_bFstBlue	= (BYTE)GetRandomNum(255, 255);
			m_pxParticle[nCnt].m_nDelay		= GetRandomNum(200, 300);

			m_shPartNum++;

			nGenCnt++;

			if ( nGenCnt > 0 )
				return;
		}
	}
}

VOID CSmoke::SetSmokeParticleEx5(D3DVECTOR vecGenPos)
{
	INT nGenCnt = 0;
	for ( INT nCnt = 0; nCnt < m_nNum; nCnt++ )
	{
		if ( m_pxParticle[nCnt].m_bIsDead )
		{
			ZeroMemory(&m_pxParticle[nCnt], sizeof(CParticle));
			m_pxParticle[nCnt].m_vecPos		= vecGenPos;
			m_pxParticle[nCnt].m_vecVel		= D3DVECTOR((FLOAT)GetRandomNum(-2, 2), 0.0f, 0.0f);
			m_pxParticle[nCnt].m_nLife		= GetRandomNum(200, 400);
			m_pxParticle[nCnt].m_fMass		= 1000.0f;
			m_pxParticle[nCnt].m_fSize		= m_pxParticle[nCnt].m_fOriSize = (FLOAT)GetRandomNum(8, 12) + _RAND_FLOAT;
			m_pxParticle[nCnt].m_bRed		= m_pxParticle[nCnt].m_bFstRed	= (BYTE)GetRandomNum(220, 255);
			m_pxParticle[nCnt].m_bGreen		= m_pxParticle[nCnt].m_bFstGreen= (BYTE)GetRandomNum(140, 160);
			m_pxParticle[nCnt].m_bBlue		= m_pxParticle[nCnt].m_bFstBlue	= (BYTE)GetRandomNum(60, 80);
			m_pxParticle[nCnt].m_nDelay		= GetRandomNum(200, 300);

			m_shPartNum++;

			nGenCnt++;

			if ( nGenCnt > 0 )
				return;
		}
	}
}



VOID CSmoke::SetSmokeParticleEx6(D3DVECTOR vecGenPos)
{
	INT nGenCnt = 0;
	for ( INT nCnt = 0; nCnt < m_nNum; nCnt++ )
	{
		if ( m_pxParticle[nCnt].m_bIsDead )
		{
			ZeroMemory(&m_pxParticle[nCnt], sizeof(CParticle));
			m_pxParticle[nCnt].m_vecPos		= vecGenPos;
			m_pxParticle[nCnt].m_vecVel		= D3DVECTOR((FLOAT)GetRandomNum(-30, 0), (FLOAT)GetRandomNum(-60, -60), 0.0f);
			m_pxParticle[nCnt].m_nLife		= GetRandomNum(800, 1200);
			m_pxParticle[nCnt].m_fMass		= 2000.0f;
			m_pxParticle[nCnt].m_fSize		= m_pxParticle[nCnt].m_fOriSize = (FLOAT)GetRandomNum(20, 40);
			m_pxParticle[nCnt].m_bRed		= m_pxParticle[nCnt].m_bFstRed	= (BYTE)GetRandomNum( 50, 50);
			m_pxParticle[nCnt].m_bGreen		= m_pxParticle[nCnt].m_bFstGreen= (BYTE)GetRandomNum( 50, 50);
			m_pxParticle[nCnt].m_bBlue		= m_pxParticle[nCnt].m_bFstBlue	= (BYTE)GetRandomNum( 50, 50);
			m_pxParticle[nCnt].m_nDelay		= GetRandomNum(200, 300);

			m_shPartNum++;

			nGenCnt++;

			if ( nGenCnt > 0 )
				return;
		}
	}

}

VOID CSmoke::SetSmokeParticleEx7(D3DVECTOR vecGenPos)
{
	INT nGenCnt = 0;
	for ( INT nCnt = 0; nCnt < m_nNum; nCnt++ )
	{
		if ( m_pxParticle[nCnt].m_bIsDead )
		{
			ZeroMemory(&m_pxParticle[nCnt], sizeof(CParticle));
			m_pxParticle[nCnt].m_vecPos		= vecGenPos;
			m_pxParticle[nCnt].m_vecVel		= D3DVECTOR(-20.0f, -20.0f, 0.0f);
			m_pxParticle[nCnt].m_nLife		= 1000;
			m_pxParticle[nCnt].m_fMass		= 10000.0f;
			m_pxParticle[nCnt].m_fSize		= m_pxParticle[nCnt].m_fOriSize = 18.0f;
			m_pxParticle[nCnt].m_bRed		= m_pxParticle[nCnt].m_bFstRed	= (BYTE)GetRandomNum(60, 70);
			m_pxParticle[nCnt].m_bGreen		= m_pxParticle[nCnt].m_bFstGreen= (BYTE)GetRandomNum(60, 70);
			m_pxParticle[nCnt].m_bBlue		= m_pxParticle[nCnt].m_bFstBlue	= (BYTE)GetRandomNum(70, 80);
			m_pxParticle[nCnt].m_nDelay		= GetRandomNum(200, 300);

			m_shPartNum++;

			nGenCnt++;

			if ( nGenCnt > 0 )
				return;
		}
	}
}

VOID CSmoke::SetSmokeParticleEx8(D3DVECTOR vecGenPos)
{
	INT nGenCnt = 0;
	for ( INT nCnt = 0; nCnt < m_nNum; nCnt++ )
	{
		if ( m_pxParticle[nCnt].m_bIsDead )
		{
			ZeroMemory(&m_pxParticle[nCnt], sizeof(CParticle));
			m_pxParticle[nCnt].m_vecPos		= vecGenPos;
			m_pxParticle[nCnt].m_vecVel		= D3DVECTOR((FLOAT)GetRandomNum(-12, 12), 0.0f, 0.0f);
			m_pxParticle[nCnt].m_nLife		= GetRandomNum(200, 400);
			m_pxParticle[nCnt].m_fMass		= 1000.0f;
			m_pxParticle[nCnt].m_fSize		= m_pxParticle[nCnt].m_fOriSize = (FLOAT)GetRandomNum(20, 20) + _RAND_FLOAT;
			m_pxParticle[nCnt].m_bRed		= m_pxParticle[nCnt].m_bFstRed	= (BYTE)GetRandomNum(60, 70);
			m_pxParticle[nCnt].m_bGreen		= m_pxParticle[nCnt].m_bFstGreen= (BYTE)GetRandomNum(220, 230);
			m_pxParticle[nCnt].m_bBlue		= m_pxParticle[nCnt].m_bFstBlue	= (BYTE)GetRandomNum(210, 220);
			m_pxParticle[nCnt].m_nDelay		= GetRandomNum(200, 300);

			m_shPartNum++;

			nGenCnt++;

			if ( nGenCnt > 0 )
				return;
		}
	}
}



VOID CSmoke::SetSmokeParticleEx9(D3DVECTOR vecGenPos)
{
	INT nGenCnt = 0;
	for ( INT nCnt = 0; nCnt < m_nNum; nCnt++ )
	{
		if ( m_pxParticle[nCnt].m_bIsDead )
		{
			ZeroMemory(&m_pxParticle[nCnt], sizeof(CParticle));
			m_pxParticle[nCnt].m_vecPos		= vecGenPos;
			m_pxParticle[nCnt].m_vecVel		= D3DVECTOR((FLOAT)GetRandomNum(-20, 0), (FLOAT)GetRandomNum(-90, -150), 0.0f);
			m_pxParticle[nCnt].m_nLife		= GetRandomNum(1000, 3000);
			m_pxParticle[nCnt].m_fMass		= 2000.0f;
			m_pxParticle[nCnt].m_fSize		= m_pxParticle[nCnt].m_fOriSize = (FLOAT)GetRandomNum(8, 10) + _RAND_FLOAT;
			m_pxParticle[nCnt].m_bRed		= m_pxParticle[nCnt].m_bFstRed  = m_pxParticle[nCnt].m_bGreen = m_pxParticle[nCnt].m_bFstGreen = m_pxParticle[nCnt].m_bBlue = m_pxParticle[nCnt].m_bFstBlue = (FLOAT)GetRandomNum(120, 150);
			m_pxParticle[nCnt].m_nDelay		= GetRandomNum(200, 300);

			m_shPartNum++;

			nGenCnt++;

			if ( nGenCnt > 1 )
				return;
		}
	}
}


VOID CSmoke::SetSmokeParticleEx10(D3DVECTOR vecGenPos)
{
	INT nGenCnt = 0;
	for ( INT nCnt = 0; nCnt < m_nNum; nCnt++ )
	{
		if ( m_pxParticle[nCnt].m_bIsDead )
		{
			ZeroMemory(&m_pxParticle[nCnt], sizeof(CParticle));
			m_pxParticle[nCnt].m_vecPos		= vecGenPos;
			m_pxParticle[nCnt].m_vecVel		= D3DVECTOR((FLOAT)GetRandomNum(-10, 10), (FLOAT)GetRandomNum(-80, 0), 0.0f);
			m_pxParticle[nCnt].m_nLife		= GetRandomNum(800, 1200);
			m_pxParticle[nCnt].m_fMass		= 2000.0f;
			m_pxParticle[nCnt].m_fSize		= m_pxParticle[nCnt].m_fOriSize = (FLOAT)GetRandomNum(20, 40);
			m_pxParticle[nCnt].m_bRed		= m_pxParticle[nCnt].m_bFstRed	= (BYTE)GetRandomNum( 50, 50);
			m_pxParticle[nCnt].m_bGreen		= m_pxParticle[nCnt].m_bFstGreen= (BYTE)GetRandomNum( 50, 50);
			m_pxParticle[nCnt].m_bBlue		= m_pxParticle[nCnt].m_bFstBlue	= (BYTE)GetRandomNum( 50, 50);
			m_pxParticle[nCnt].m_nDelay		= GetRandomNum(200, 300);

			m_shPartNum++;

			nGenCnt++;

			if ( nGenCnt > 0 )
				return;
		}
	}
}

VOID CSmoke::SetSmokeParticleEx11(D3DVECTOR vecGenPos)
{
	INT nGenCnt = 0;
	for ( INT nCnt = 0; nCnt < m_nNum; nCnt++ )
	{
		if ( m_pxParticle[nCnt].m_bIsDead )
		{
			ZeroMemory(&m_pxParticle[nCnt], sizeof(CParticle));
			m_pxParticle[nCnt].m_vecPos		= vecGenPos;
			m_pxParticle[nCnt].m_vecVel		= D3DVECTOR((FLOAT)GetRandomNum(-55, 5), (FLOAT)GetRandomNum(-100, -50), 0.0f);
			m_pxParticle[nCnt].m_nLife		= GetRandomNum(900, 1000);
			m_pxParticle[nCnt].m_fMass		= 2000.0f;
			m_pxParticle[nCnt].m_fSize		= m_pxParticle[nCnt].m_fOriSize = (FLOAT)GetRandomNum(20, 40);
			m_pxParticle[nCnt].m_bRed		= m_pxParticle[nCnt].m_bFstRed	= (BYTE)GetRandomNum( 50, 50);
			m_pxParticle[nCnt].m_bGreen		= m_pxParticle[nCnt].m_bFstGreen= (BYTE)GetRandomNum( 50, 50);
			m_pxParticle[nCnt].m_bBlue		= m_pxParticle[nCnt].m_bFstBlue	= (BYTE)GetRandomNum( 50, 50);
			m_pxParticle[nCnt].m_nDelay		= GetRandomNum(200, 300);

			m_shPartNum++;

			nGenCnt++;

			if ( nGenCnt > 0 )
				return;
		}
	}
}

VOID CSmoke::SetSmokeParticleEx12(D3DVECTOR vecGenPos)
{
	INT nGenCnt = 0;
	for ( INT nCnt = 0; nCnt < m_nNum; nCnt++ )
	{
		if ( m_pxParticle[nCnt].m_bIsDead )
		{
			ZeroMemory(&m_pxParticle[nCnt], sizeof(CParticle));
		//	vecGenPos.x += (FLOAT)GetRandomNum(-5, 5);
		//	vecGenPos.y += (FLOAT)GetRandomNum(-8, 2);
			m_pxParticle[nCnt].m_vecPos		= vecGenPos;
			m_pxParticle[nCnt].m_vecVel		= D3DVECTOR((FLOAT)GetRandomNum(-10, 10), 0.0f, 0.0f);
			m_pxParticle[nCnt].m_nLife		= (FLOAT)GetRandomNum(500, 500);
//			m_pxParticle[nCnt].m_nLife		= GetRandomNum(150, 400);
			m_pxParticle[nCnt].m_fMass		= 1000.0f;
		//	m_pxParticle[nCnt].m_fSize		= m_pxParticle[nCnt].m_fOriSize = (FLOAT)GetRandomNum(5, 10) + _RAND_FLOAT;
			m_pxParticle[nCnt].m_fSize		= m_pxParticle[nCnt].m_fOriSize = (FLOAT)GetRandomNum(10, 30) + _RAND_FLOAT;
			m_pxParticle[nCnt].m_bRed		= m_pxParticle[nCnt].m_bFstRed = m_pxParticle[nCnt].m_bGreen = m_pxParticle[nCnt].m_bFstGreen = m_pxParticle[nCnt].m_bBlue = m_pxParticle[nCnt].m_bFstBlue = (BYTE)GetRandomNum(100, 150);
			m_pxParticle[nCnt].m_nDelay		= GetRandomNum(200, 300);

			m_shPartNum++;

			nGenCnt++;

			if ( nGenCnt > 0 )
				return;
		}
	}
}


VOID CSmoke::SetSmokeParticleEx13(D3DVECTOR vecGenPos)
{
	INT nGenCnt = 0;
	for ( INT nCnt = 0; nCnt < m_nNum; nCnt++ )
	{
		if ( m_pxParticle[nCnt].m_bIsDead )
		{
			ZeroMemory(&m_pxParticle[nCnt], sizeof(CParticle));
			m_pxParticle[nCnt].m_vecPos		= vecGenPos;
			m_pxParticle[nCnt].m_vecVel		= D3DVECTOR((FLOAT)GetRandomNum(-30, 0), (FLOAT)GetRandomNum(-130, -100), 0.0f);
			m_pxParticle[nCnt].m_nLife		= GetRandomNum(800, 900);
			m_pxParticle[nCnt].m_fMass		= 1000.0f;
			m_pxParticle[nCnt].m_fSize		= m_pxParticle[nCnt].m_fOriSize = (FLOAT)GetRandomNum(20, 30);
			m_pxParticle[nCnt].m_bRed		= m_pxParticle[nCnt].m_bFstRed	= (BYTE)GetRandomNum( 0, 10);
			m_pxParticle[nCnt].m_bGreen		= m_pxParticle[nCnt].m_bFstGreen= (BYTE)GetRandomNum( 65, 75);
			m_pxParticle[nCnt].m_bBlue		= m_pxParticle[nCnt].m_bFstBlue	= (BYTE)GetRandomNum( 140, 150);
			m_pxParticle[nCnt].m_nDelay		= GetRandomNum(1000, 1500);

			m_shPartNum++;

			nGenCnt++;

			if ( nGenCnt > 0 )
				return;
		}
	}
}


/******************************************************************************************************************

	CElec Class	

*******************************************************************************************************************/
CElec::CElec()
{
	InitElec();
}

CElec::~CElec()
{
	DestroyElec();
}


VOID CElec::SetupElec()
{
	D3DVECTOR vNorm(0, 0, -1);

	for ( INT nCntY = 0; nCntY < (_MAX_ELEC_NODEY+1); nCntY++ )
	{
		for ( INT nCntX = 0; nCntX < (_MAX_ELEC_NODEX+1); nCntX++ )
		{
			FLOAT fXPos = -0.5f+(FLOAT)nCntX/(FLOAT)(_MAX_ELEC_NODEX);
			FLOAT fYPos = /*-0.5f+*/(FLOAT)nCntY/(FLOAT)(_MAX_ELEC_NODEY);
			FLOAT fu = (FLOAT)nCntX/(FLOAT)(_MAX_ELEC_NODEX);
			FLOAT fv = (FLOAT)nCntY/(FLOAT)(_MAX_ELEC_NODEY);

			m_avElec[nCntY][nCntX] = D3DVERTEX(D3DVECTOR(fXPos, fYPos, 0), vNorm, fu, fv);
//			m_avElec[nCntY][nCntX] = D3DLVERTEX(D3DVECTOR(fXPos, fYPos, 0), RGB(0, 255, 0), RGB(255, 0, 0), fu, fv);
		}
	}

	memcpy(m_avSaveElec, m_avElec, (_MAX_ELEC_NODEY+1)*(_MAX_ELEC_NODEX+1)*sizeof(D3DVERTEX));

	// Ÿ���������� �ε��� ����.
    WORD* pwIndex = m_pwElecIndices;
	for ( INT nCntY = 0; nCntY < _MAX_ELEC_NODEY; nCntY++ )
	{
		for ( INT nCntX = 0; nCntX < _MAX_ELEC_NODEX; nCntX++ )
		{
			*pwIndex++ = (WORD)((nCntX+1)+((_MAX_ELEC_NODEX+1)*nCntY));
			*pwIndex++ = (WORD)(nCntX+((_MAX_ELEC_NODEX+1)*(nCntY+1)));
			*pwIndex++ = (WORD)((nCntX+1)+((_MAX_ELEC_NODEX+1)*(nCntY+1)));
			*pwIndex++ = (WORD)(nCntX+((_MAX_ELEC_NODEX+1)*(nCntY+1)));
			*pwIndex++ = (WORD)((nCntX+1)+((_MAX_ELEC_NODEX+1)*nCntY));
			*pwIndex++ = (WORD)((nCntX)+((_MAX_ELEC_NODEX+1)*nCntY));
		}
	}
}

VOID CElec::InitElec()
{
	ZeroMemory(m_pwElecIndices, _ELEC_INDICES*sizeof(WORD));
	ZeroMemory(m_avElec, sizeof(D3DVERTEX)*_MAX_ELEC_NODEY+1*_MAX_ELEC_NODEX+1);
	ZeroMemory(m_avSaveElec, (_MAX_ELEC_NODEY+1)*(_MAX_ELEC_NODEX+1)*sizeof(D3DVERTEX));

	m_fSize		= 0;
	m_fRadian	= 0;
	m_shPosX	= 0;
	m_shPosY	= 0;
	m_shDstPosX = 0;
	m_shDstPosY = 0;

}

VOID CElec::DestroyElec()
{
	InitElec();
}

VOID CElec::UpdateElec(SHORT shPosX, SHORT shPosY, SHORT shDstPosX, SHORT shDstPosY)
{
	FLOAT	fDiv;

	fDiv = 0.0f;

	m_shPosX = shPosX;
	m_shPosY = shPosY;
	m_shDstPosX = shDstPosX;
	m_shDstPosY = shDstPosY;

	INT nAbsX = abs(m_shDstPosX - m_shPosX);
	INT nAbsY = abs(m_shDstPosY - m_shPosY);

	m_fSize = (FLOAT)sqrt(nAbsX*nAbsX + nAbsY*nAbsY);

//	fDiv = (FLOAT)(nAbsY)/(FLOAT)(nAbsX);

	if ( m_fSize == 0.0f )
	{
		m_fSize = 1.0f;
	}

	fDiv = (FLOAT)-(m_shDstPosX - m_shPosX)/m_fSize;

	m_fRadian = (FLOAT)asin(fDiv);

	if ( m_shDstPosY - m_shPosY > 0 )
		m_fRadian = g_PI/2 + g_PI/2 - m_fRadian;


//	for ( INT nCntY = 0; nCntY < (_MAX_ELEC_NODEY+1); nCntY++ )

	static int nCnt = 0;

	nCnt++;

	if ( nCnt >= 2 )
		nCnt = 0;

	if ( nCnt == 1 )
	{
	
	for ( INT nCntY = 1; nCntY < _MAX_ELEC_NODEY; nCntY++ )
	{
//		FLOAT fRandX = _RAND_FLOAT*1.3f	 - 1.0f;
		FLOAT fRandX = _RAND_FLOAT*0.8f - 0.4f;
//		FLOAT fRandY = _RAND_FLOAT*0.2f;
		for ( INT nCntX = 0; nCntX < (_MAX_ELEC_NODEX+1); nCntX++ )
		{
			m_avElec[nCntY][nCntX].x = m_avSaveElec[nCntY][nCntX].x+fRandX;
//			m_avElec[nCntY][nCntX].y = m_avSaveElec[nCntY][nCntX].y+fRandY;
		}
	}

	}
	
/*
	CHAR szBuff[MAX_PATH];
	sprintf(szBuff, "->����:[%f] %f", m_fRadian, fDiv);
	g_xMainWnd.PutsHan(g_xMainWnd.GetBackBuffer(), 0, 100, RGB(250, 125, 0), RGB(0, 0, 0), szBuff);
*/
}

HRESULT CElec::RenderElec(LPDIRECT3DDEVICE9 lpDevice)
{
	// [FIX-R3] The lightning ribbon is a (3 x 11) vertex grid scaled by
	// (128/2.4, 512), rotated about Z and translated - it cannot be expressed as
	// an axis aligned billboard, so rasterise every cell as two affine textured
	// triangles straight into the 565 soft buffer.
	(void)lpDevice;

	IDirect3DTexture9* lpddsTextr = g_xGameProc.m_xImage.GetTextrImg(_TEXTR_FILE_PARTICLE, _IMAGE_PROGUSE, 540);
	if ( !lpddsTextr ) return E_FAIL;

	FLOAT fCos = (FLOAT)cos( m_fRadian );
	FLOAT fSin = (FLOAT)sin( m_fRadian );
	FLOAT fSclX = 128.0f / 2.4f;
	FLOAT fSclY = 512.0f;
	FLOAT fTrX  = (FLOAT)m_shPosX - 400.0f;
	FLOAT fTrY  = (FLOAT)-m_shPosY + 300.0f;

	FLOAT fPx[_MAX_ELEC_NODEY+1][_MAX_ELEC_NODEX+1];
	FLOAT fPy[_MAX_ELEC_NODEY+1][_MAX_ELEC_NODEX+1];

	for ( INT nCntY = 0; nCntY <= _MAX_ELEC_NODEY; nCntY++ )
	{
		for ( INT nCntX = 0; nCntX <= _MAX_ELEC_NODEX; nCntX++ )
		{
			FLOAT fLx = m_avElec[nCntY][nCntX].x * fSclX;
			FLOAT fLy = m_avElec[nCntY][nCntX].y * fSclY;
			FLOAT fWx = fLx * fCos - fLy * fSin + fTrX;
			FLOAT fWy = fLx * fSin + fLy * fCos + fTrY;

			fPx[nCntY][nCntX] = fWx + 400.0f;
			fPy[nCntY][nCntX] = 300.0f - fWy;
		}
	}

	for ( INT nCntY = 0; nCntY < _MAX_ELEC_NODEY; nCntY++ )
	{
		FLOAT fV0 = (FLOAT)nCntY / (FLOAT)_MAX_ELEC_NODEY;
		FLOAT fV1 = (FLOAT)( nCntY + 1 ) / (FLOAT)_MAX_ELEC_NODEY;

		for ( INT nCntX = 0; nCntX < _MAX_ELEC_NODEX; nCntX++ )
		{
			FLOAT fU0 = (FLOAT)nCntX / (FLOAT)_MAX_ELEC_NODEX;
			FLOAT fU1 = (FLOAT)( nCntX + 1 ) / (FLOAT)_MAX_ELEC_NODEX;

			g_xGameProc.m_xImage.DrawTexTriangleSoft( lpddsTextr,
				fPx[nCntY][nCntX],     fPy[nCntY][nCntX],     fU0, fV0,
				fPx[nCntY+1][nCntX],   fPy[nCntY+1][nCntX],   fU0, fV1,
				fPx[nCntY+1][nCntX+1], fPy[nCntY+1][nCntX+1], fU1, fV1,
				_BLEND_LIGHTINV, 200, 200, 255 );

			g_xGameProc.m_xImage.DrawTexTriangleSoft( lpddsTextr,
				fPx[nCntY][nCntX],     fPy[nCntY][nCntX],     fU0, fV0,
				fPx[nCntY+1][nCntX+1], fPy[nCntY+1][nCntX+1], fU1, fV1,
				fPx[nCntY][nCntX+1],   fPy[nCntY][nCntX+1],   fU1, fV0,
				_BLEND_LIGHTINV, 200, 200, 255 );
		}
	}

	return S_OK;
}






CMist::CMist()
{
 	D3DVECTOR vNorm(0, 0, -1);
    m_avBillboard[0] = D3DVERTEX(D3DVECTOR(-400.0f, 300.0f, 0), vNorm, 0, 0);
    m_avBillboard[1] = D3DVERTEX(D3DVECTOR(-400.0f,-300.0f, 0), vNorm, 0, 1);
    m_avBillboard[2] = D3DVERTEX(D3DVECTOR( 400.0f, 300.0f, 0), vNorm, 1, 0);
    m_avBillboard[3] = D3DVERTEX(D3DVECTOR( 400.0f,-300.0f, 0), vNorm, 1, 1);

	m_vTrans.x = 0;
	m_vTrans.y = 0;
	m_vTrans.z = 0;
}

CMist::~CMist()
{
}

BOOL CMist::Create()
{
	return TRUE;
}

VOID CMist::Init()
{
}

VOID CMist::Destory()
{
}

HRESULT CMist::DrawMist()
{
	if ( g_xMainWnd.Get3DDevice() )
	{
		IDirect3DTexture9* lpddsParticleTextr = g_xGameProc.m_xImage.GetTextrImg(_TEXTR_FILE_PARTICLE, _IMAGE_PROGUSE, 550);

		D3DMATRIX matWorldOriginal;
		D3DMATRIX matTrans;
		D3DMATRIX matScale;
		D3DMATRIX matWorld;
		D3DMATERIAL9 mtrl;

		static int nCnt = 0;

		nCnt++;

		if ( nCnt < 800 )
		{
 			m_vTrans.x -= 1.9f; 
			m_vTrans.y -= 0.2f; 
			m_vTrans.z = (FLOAT)0;
		}
		else if ( nCnt >= 800 &&  nCnt < 1600 )
		{
 			m_vTrans.x += 1.9f; 
			m_vTrans.y += 0.2f; 
			m_vTrans.z = (FLOAT)0;
		}
		else
		{
			nCnt = 0;
		}

		D3DUtil_InitMaterial(mtrl, (FLOAT)100/255.0f, (FLOAT)200/255.0f, (FLOAT)200/255.0f);
		mtrl.Diffuse.a = 0.4f;
		g_xMainWnd.Get3DDevice()->SetMaterial(&mtrl);

		if( SUCCEEDED(g_xMainWnd.Get3DDevice()->BeginScene()) )
		{
			g_xMainWnd.Get3DDevice()->GetTransform(D3DTS_WORLD, &matWorldOriginal);

			D3DUtil_SetTranslateMatrix(matTrans, m_vTrans);
			D3DUtil_SetScaleMatrix(matScale, 5.0f, 2.0f, 1.0f);
			D3DMath_MatrixMultiply(matWorld, matScale, matTrans);
			g_xMainWnd.Get3DDevice()->SetTransform(D3DTS_WORLD, &matWorld);

			SetBlendRenderState(g_xMainWnd.Get3DDevice(), 2, &mtrl);

			g_xMainWnd.Get3DDevice()->SetTexture(0, lpddsParticleTextr);

			g_xMainWnd.Get3DDevice()->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, m_avBillboard, sizeof(D3DVERTEX));

			ResetBlendenderState(g_xMainWnd.Get3DDevice());

			g_xMainWnd.Get3DDevice()->SetTransform(D3DTS_WORLD, &matWorldOriginal);

			g_xMainWnd.Get3DDevice()->EndScene();
			return S_OK;
		}
	}
	return E_FAIL;
}

VOID CMist::ProgressMist()
{
	DrawMist();
}


/***************** [weather] soft-render particles onto soft backbuffer *****************/
// colorkey: transparent pixel == 0x0000 (after DrawWithImageForCompToMem)


/***************** [weather] soft-render via DrawBillBoard (software blit to soft backbuffer) *****************/
VOID CSnow::RenderSoft()
{
	// [FIX-W1] 对齐原版 CSnow::RenderSystem:
	//   中心坐标 (x-400, -y+300)(原版把粒子世界坐标映射到 800x600 屏幕),
	//   四边形尺寸 = m_fSize(原版 m_avShape 是 -0.5..0.5 单位四边形),
	//   混合 = SRCBLEND_ONE / DESTBLEND_INVSRCALPHA, 即加色(雪花发亮)。
	//   原实现用 (x,y)、m_fSize*4、普通 alpha, 坐标/尺寸/混合三项都不对。
	if ( !m_nNum || !m_pxParticle ) return;

	for ( INT nCnt = 0; nCnt < m_nNum; nCnt++ )
	{
		CParticle* p = &m_pxParticle[nCnt];
		if ( p->m_bIsDead ) continue;

		IDirect3DTexture9* tex = g_xGameProc.m_xImage.GetTextrImg(_TEXTR_FILE_PARTICLE, _IMAGE_PROGUSE, 500 + p->m_nCurrFrame);
		if ( !tex ) continue;

		FLOAT fS = (FLOAT)p->m_fSize;
		if ( fS < 1.0f ) fS = 1.0f;

		// [FIX-W4] 直接用粒子屏幕坐标。原版 DX7 的 (x-400, -y+300) 是"中心原点 +
		//   Y 轴向上"的 3D 世界坐标, 经正交投影后正好还原为 (x, y)。
		//   软渲染本身就在屏幕空间作图, 再套一次该变换会导致 x 左移 400(只剩左半屏)
		//   且 Y 翻转(下落变上升)。
		FLOAT cx = (FLOAT)p->m_vecPos.x;
		FLOAT cy = (FLOAT)p->m_vecPos.y;

		D3DVECTOR vTrans(cx - fS * 0.5f, cy - fS * 0.5f, 0.0f);
		D3DVECTOR vScale(fS, fS, 1.0f);

		D3DMATERIAL9 mtrl;
		D3DUtil_InitMaterial(mtrl, (FLOAT)p->m_bRed / 255.0f, (FLOAT)p->m_bGreen / 255.0f, (FLOAT)p->m_bBlue / 255.0f);
		mtrl.Diffuse.a = 1.0f;

		g_xGameProc.m_xImage.DrawBillBoard(NULL, &vTrans, &vScale, &mtrl, tex, _BLEND_LIGHT);
	}
}

VOID CRain::RenderSoft()
{
	// [FIX-W2] 对齐原版 CRain::RenderSystem:
	//   雨丝 = 宽 2 x 高 m_fSize, 混合 _BLEND_LIGHTINV(ONE / INVSRCCOLOR);
	//   落地水花(生命周期差 0..510) = 32x32, 纹理 510+frame, 其余 500+frame。
	//   原实现用 NULL 纹理画纯白矩形, 雨丝/水花都没有纹理。
	if ( !m_nNum || !m_pxParticle ) return;

	for ( INT nCnt = 0; nCnt < m_nNum; nCnt++ )
	{
		CParticle* p = &m_pxParticle[nCnt];
		if ( p->m_bIsDead ) continue;

		FLOAT fS = (FLOAT)p->m_fSize;
		if ( fS < 2.0f ) fS = 2.0f;

		// [FIX-W4] 直接用粒子屏幕坐标。原版 DX7 的 (x-400, -y+300) 是"中心原点 +
		//   Y 轴向上"的 3D 世界坐标, 经正交投影后正好还原为 (x, y)。
		//   软渲染本身就在屏幕空间作图, 再套一次该变换会导致 x 左移 400(只剩左半屏)
		//   且 Y 翻转(下落变上升)。
		FLOAT cx = (FLOAT)p->m_vecPos.x;
		FLOAT cy = (FLOAT)p->m_vecPos.y;

		D3DMATERIAL9 mtrl;
		D3DUtil_InitMaterial(mtrl, (FLOAT)p->m_bRed / 255.0f, (FLOAT)p->m_bGreen / 255.0f, (FLOAT)p->m_bBlue / 255.0f);
		mtrl.Diffuse.a = 1.0f;

		INT nLifeDiff = p->m_nCurrLife - p->m_nLife;

		if ( nLifeDiff > 0 && nLifeDiff < 510 )
		{
			IDirect3DTexture9* tex = g_xGameProc.m_xImage.GetTextrImg(_TEXTR_FILE_PARTICLE, _IMAGE_PROGUSE, 510 + p->m_nCurrFrame);
			if ( !tex ) continue;

			D3DVECTOR vTrans(cx - 16.0f, cy - 16.0f, 0.0f);
			D3DVECTOR vScale(32.0f, 32.0f, 1.0f);
			g_xGameProc.m_xImage.DrawBillBoard(NULL, &vTrans, &vScale, &mtrl, tex, _BLEND_LIGHTINV);
		}
		else
		{
			IDirect3DTexture9* tex = g_xGameProc.m_xImage.GetTextrImg(_TEXTR_FILE_PARTICLE, _IMAGE_PROGUSE, 500 + p->m_nCurrFrame);
			if ( !tex ) continue;

			D3DVECTOR vTrans(cx - 1.0f, cy - fS * 0.5f, 0.0f);
			D3DVECTOR vScale(2.0f, fS, 1.0f);
			g_xGameProc.m_xImage.DrawBillBoard(NULL, &vTrans, &vScale, &mtrl, tex, _BLEND_LIGHTINV);
		}
	}
}

VOID CMist::RenderSoft()
{
	// [FIX-W7] 雾 = 纹理 550 的**双线性拉伸** + _BLEND_LIGHTINV + 缓慢漂移。
	//
	// 此前的三种做法都不对:
	//   W3  DrawBillBoard(纹理550)  -> 它是 1:1 拷贝且裁剪到纹理尺寸 => 硬边矩形
	//   W5  最近邻拉伸              -> 放大 3 倍后呈块状
	//   W6  纯色全屏叠加            -> 均匀"滤色布", 没有纹理的浓淡层次
	//
	// 原版 CMist::DrawMist 是"一个四边形 + SetScaleMatrix(5,2) + 纹理 550 + LIGHTINV",
	// 本质就是把云状纹理放大铺满屏幕。这里用双线性采样平滑放大, 保留雾的层次。
	//
	// 可调参数:
	//   MIST_STRENGTH : 雾浓度 0..255 (越大越浓)
	//   sfDriftX/Y    : 漂移速度(像素/帧)

	const DWORD MIST_STRENGTH = 170;		// <== 调这里改浓度

	CDX9SoftSurface* dst = g_xMainWnd.GetBackBuffer();
	if ( !dst || !dst->m_pBits ) return;

	LONG  W = (LONG)dst->m_dwWidth;
	LONG  H = (LONG)dst->m_dwHeight;
	WORD* pBits = dst->m_pBits;
	LONG  pitch = dst->m_lPitch >> 1;

	IDirect3DTexture9* tex = g_xGameProc.m_xImage.GetTextrImg(_TEXTR_FILE_PARTICLE, _IMAGE_PROGUSE, 550);

	D3DLOCKED_RECT lr;
	DWORD* pTex = NULL;
	LONG   texPitch = 0, texW = 0, texH = 0;

	if ( tex && SUCCEEDED(tex->LockRect(0, &lr, NULL, D3DLOCK_READONLY)) )
	{
		D3DSURFACE_DESC sd;
		if ( SUCCEEDED(tex->GetLevelDesc(0, &sd)) )
		{
			pTex = (DWORD*)lr.pBits;
			texPitch = (LONG)(lr.Pitch / sizeof(DWORD));
			texW = (LONG)sd.Width;
			texH = (LONG)sd.Height;
		}
	}

	// ---- 回退: 纹理不可用时就退回均匀叠加, 至少不是硬边矩形 ----
	if ( !pTex || texW <= 0 || texH <= 0 )
	{
		if ( tex ) tex->UnlockRect(0);
		const DWORD OPA = ( MIST_STRENGTH * 63 ) / 255;
		for ( LONG y = 0; y < H; y++ )
		{
			WORD* row = pBits + (size_t)y * pitch;
			for ( LONG x = 0; x < W; x++ )
			{
				WORD  d  = row[x];
				DWORD dr = ( d >> 11 ) & 0x1F, dg = ( d >> 5 ) & 0x3F, db = d & 0x1F;
				row[x] = (WORD)( ( ( 12 * OPA + dr * ( 255 - OPA ) ) / 255 ) << 11 |
				                 ( ( 50 * OPA + dg * ( 255 - OPA ) ) / 255 ) << 5  |
				                   ( 24 * OPA + db * ( 255 - OPA ) ) / 255 );
			}
		}
		return;
	}

	static FLOAT sfDriftX = 0.0f;
	static FLOAT sfDriftY = 0.0f;
	sfDriftX += 0.35f; if ( sfDriftX >= (FLOAT)texW ) sfDriftX -= (FLOAT)texW;
	sfDriftY += 0.08f; if ( sfDriftY >= (FLOAT)texH ) sfDriftY -= (FLOAT)texH;

	FLOAT sxScale = (FLOAT)texW / (FLOAT)W;
	FLOAT syScale = (FLOAT)texH / (FLOAT)H;

	for ( LONG y = 0; y < H; y++ )
	{
		FLOAT v = (FLOAT)y * syScale + sfDriftY;
		if ( v >= (FLOAT)texH ) v -= (FLOAT)texH;
		LONG  iy0 = (LONG)v;
		FLOAT ty  = v - (FLOAT)iy0;
		LONG  iy1 = iy0 + 1; if ( iy1 >= texH ) iy1 = 0;

		DWORD* row0 = pTex + (size_t)iy0 * texPitch;
		DWORD* row1 = pTex + (size_t)iy1 * texPitch;
		WORD*  drow = pBits + (size_t)y * pitch;

		for ( LONG x = 0; x < W; x++ )
		{
			FLOAT u = (FLOAT)x * sxScale + sfDriftX;
			if ( u >= (FLOAT)texW ) u -= (FLOAT)texW;
			LONG  ix0 = (LONG)u;
			FLOAT tx  = u - (FLOAT)ix0;
			LONG  ix1 = ix0 + 1; if ( ix1 >= texW ) ix1 = 0;

			// 双线性采样(A8R8G8B8)
			DWORD c00 = row0[ix0], c10 = row0[ix1], c01 = row1[ix0], c11 = row1[ix1];
			DWORD w00 = (DWORD)( ( 1.0f - tx ) * ( 1.0f - ty ) * 256.0f );
			DWORD w10 = (DWORD)( (         tx ) * ( 1.0f - ty ) * 256.0f );
			DWORD w01 = (DWORD)( ( 1.0f - tx ) * (         ty ) * 256.0f );
			DWORD w11 = (DWORD)( (         tx ) * (         ty ) * 256.0f );

			DWORD a = ( ( ( c00 >> 24 ) * w00 + ( c10 >> 24 ) * w10 + ( c01 >> 24 ) * w01 + ( c11 >> 24 ) * w11 ) >> 8 );
			if ( a == 0 ) continue;

			DWORD r8 = ( ( ( ( c00 >> 16 ) & 0xFF ) * w00 + ( ( c10 >> 16 ) & 0xFF ) * w10 +
			               ( ( c01 >> 16 ) & 0xFF ) * w01 + ( ( c11 >> 16 ) & 0xFF ) * w11 ) >> 8 );
			DWORD g8 = ( ( ( ( c00 >>  8 ) & 0xFF ) * w00 + ( ( c10 >>  8 ) & 0xFF ) * w10 +
			               ( ( c01 >>  8 ) & 0xFF ) * w01 + ( ( c11 >>  8 ) & 0xFF ) * w11 ) >> 8 );
			DWORD b8 = ( ( (   c00        & 0xFF ) * w00 + (   c10        & 0xFF ) * w10 +
			               (   c01        & 0xFF ) * w01 + (   c11        & 0xFF ) * w11 ) >> 8 );

			// 强度缩放
			r8 = r8 * MIST_STRENGTH / 255;
			g8 = g8 * MIST_STRENGTH / 255;
			b8 = b8 * MIST_STRENGTH / 255;

			DWORD sr = r8 >> 3;
			DWORD sg = g8 >> 2;
			DWORD sb = b8 >> 3;
			if ( sr > 31 ) sr = 31;
			if ( sg > 63 ) sg = 63;
			if ( sb > 31 ) sb = 31;

			WORD  d  = drow[x];
			DWORD dr = ( d >> 11 ) & 0x1F;
			DWORD dg = ( d >>  5 ) & 0x3F;
			DWORD db =  d         & 0x1F;

			// _BLEND_LIGHTINV: dst = src + dst * (1 - srcColor)
			DWORD nr = sr + dr * ( 31 - sr ) / 31;
			DWORD ng = sg + dg * ( 63 - sg ) / 63;
			DWORD nb = sb + db * ( 31 - sb ) / 31;
			if ( nr > 31 ) nr = 31;
			if ( ng > 63 ) ng = 63;
			if ( nb > 31 ) nb = 31;

			drow[x] = (WORD)( ( nr << 11 ) | ( ng << 5 ) | nb );
		}
	}

	tex->UnlockRect(0);
}
