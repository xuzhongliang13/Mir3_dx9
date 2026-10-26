#include "StdAfx.h"


const GUID CLSID_AVIFile = { 0x00020000, 0, 0, {0xC0, 0,  0,  0,  0,  0,  0,  0X46} };


CAvi::CAvi()
{
	Init();
	InitializeCriticalSection(&m_AccessBuffer);
}


CAvi::~CAvi(void)
{
	Release();
	DeleteCriticalSection(&m_AccessBuffer);
}


VOID CAvi::Init()
{
	m_bPlaying		= FALSE;
	m_bRevPlay		= FALSE;
	m_bLoop			= FALSE;
	m_dwFps			= 0;
	m_nTimeTick		= 0;
	m_lLength		= 0;
	m_lLinePitch	= 0;

	m_pAviFile		= NULL;
	m_nTimerID		= 0;

	m_pAviStream	= NULL;
	m_pInputFormat	= NULL;
	m_pTargetFormat = NULL;
	m_lpddsAvi		= NULL;
	m_lIndex		= 0;
	m_lFrames		= 0;
	m_Decompressor	= 0;
	m_lpInput		= NULL;
	m_lpOutput		= NULL;

	m_pAviSound		= NULL;
	m_pSoundFormat	= NULL;
	m_lpdsbAvi	    = NULL;
	m_dwLoadPos		= 0;
	m_dwLoadSize	= 0;
	m_nSoundFramesAhead = 0;

	SetRect(&m_rcSrc, 0, 0, 0, 0);
	SetRect(&m_rcDst, 0, 0, 0, 0);

	ZeroMemory(&m_stSoundInfo, sizeof(AVISTREAMINFO));
	ZeroMemory(&m_stStreamInfo, sizeof(AVISTREAMINFO));
}


VOID CAvi::Release()
{
	Stop();

	if ( m_Decompressor )
	{
		ICDecompressEnd(m_Decompressor);
		ICClose(m_Decompressor);
		m_Decompressor = 0;
	}
	
	if ( m_pInputFormat ) 
	{
		SAFE_DELETE(m_pInputFormat);
	}

	if ( m_pTargetFormat ) 
	{
		SAFE_DELETE(m_pTargetFormat);
	}

	if ( m_lpInput )
	{
		SAFE_DELETE(m_lpInput);
	}

	if ( m_lpOutput ) 
	{
		SAFE_DELETE(m_lpOutput);
	}

	if ( m_pAviStream ) 
	{
		AVIStreamRelease( m_pAviStream);
		m_pAviStream = NULL;
	}

	if ( m_pAviSound )  
	{
		AVIStreamRelease( m_pAviSound );
		m_pAviSound = NULL;
	}

	if ( m_pAviFile )   
	{
		AVIFileRelease(m_pAviFile);
		m_pAviFile = NULL;
	}

	AVIFileExit();

	if ( m_pSoundFormat ) 
	{
		SAFE_DELETE(m_pSoundFormat);
	}

	if ( m_lpddsAvi ) 
	{
		SAFE_RELEASE(m_lpddsAvi );
	}

	if ( m_lpdsbAvi ) 
	{
		SAFE_RELEASE(m_lpdsbAvi );
	}
}


BOOL CAvi::Create(LPDIRECTDRAW7 lpDD, LPDIRECTSOUND lpDS, const CHAR* pszFile, BOOL bLoop, BOOL bRevPlay)
{
	// [DX9][B] lpDD 允许为 NULL：纯内存解码(不创建 DX7 表面)，帧数据在 m_lpOutput
	if ( pszFile == NULL )
	{
		return FALSE;
	}

	Init();

	AVIFileInit();

	if ( AVIFileOpen(&m_pAviFile, pszFile, OF_READ, (LPGUID)&CLSID_AVIFile) )
	{
		AVIFileExit( );	
		return FALSE; 
	}

	if ( AVIFileGetStream(m_pAviFile, &m_pAviStream, streamtypeVIDEO, 0) )
	{	
		AVIFileExit( );
        return FALSE;
	}

	HRESULT hr;
	LONG	lFmtLenght;

	AVIStreamFormatSize(m_pAviStream, 0, &lFmtLenght);
	m_pInputFormat  = (LPBITMAPINFOHEADER) new BYTE[lFmtLenght];
	m_pTargetFormat = (LPBITMAPV4HEADER)   new BYTE[(max((DWORD)lFmtLenght, (DWORD)sizeof(BITMAPV4HEADER)))];
	ZeroMemory(m_pTargetFormat, sizeof(BITMAPV4HEADER));
	AVIStreamReadFormat(m_pAviStream, 0, m_pInputFormat, &lFmtLenght);
	m_lFrames = AVIStreamLength(m_pAviStream);
	AVIStreamInfo(m_pAviStream, &m_stStreamInfo, sizeof(AVISTREAMINFO));

	// [DX9][B] 纯内存解码：目标格式固定 24bit RGB(解码器普遍支持)，不创建 DX7 表面
	memcpy(m_pTargetFormat, m_pInputFormat, min((DWORD)lFmtLenght, (DWORD)sizeof(BITMAPV4HEADER)));
	m_pTargetFormat->bV4Size		  = (DWORD)sizeof(BITMAPV4HEADER);
	m_pTargetFormat->bV4Width		  = m_pInputFormat->biWidth;
	m_pTargetFormat->bV4Height		  = m_pInputFormat->biHeight;
	m_pTargetFormat->bV4Planes		  = 1;
	m_pTargetFormat->bV4BitCount	  = 24;
	m_pTargetFormat->bV4V4Compression = BI_RGB;
	m_pTargetFormat->bV4ClrUsed		  = 0;
	m_pTargetFormat->bV4RedMask		  = 0;
	m_pTargetFormat->bV4GreenMask	  = 0;
	m_pTargetFormat->bV4BlueMask	  = 0;
	m_pTargetFormat->bV4AlphaMask	  = 0;
	m_pTargetFormat->bV4SizeImage	  = ((m_pTargetFormat->bV4Width +3)&0xFFFFFFFC) *
										 m_pTargetFormat->bV4Height * (m_pTargetFormat->bV4BitCount>>3);

	m_lLength = m_pInputFormat->biWidth * m_pInputFormat->biHeight * (m_pInputFormat->biBitCount >> 3);

	if ( m_stStreamInfo.dwSuggestedBufferSize ) 
	{
		m_lLength = (LONG)m_stStreamInfo.dwSuggestedBufferSize;
	}

	m_Decompressor = ICDecompressOpen(ICTYPE_VIDEO,	m_stStreamInfo.fccHandler, m_pInputFormat,
									  (LPBITMAPINFOHEADER)m_pTargetFormat);

	if ( !m_Decompressor ) 
	{
		AVIFileExit();
		SAFE_DELETE(m_pInputFormat);
		SAFE_DELETE(m_pTargetFormat);
		return FALSE;
	}

	m_lpInput  = new BYTE[m_lLength];
	m_lpOutput = new BYTE[m_pTargetFormat->bV4SizeImage];
	ZeroMemory(m_lpInput, m_lLength);
	ZeroMemory(m_lpOutput, m_pTargetFormat->bV4SizeImage);

	m_lLinePitch = m_pTargetFormat->bV4Width * (m_pTargetFormat->bV4BitCount >> 3);
	
	ICDecompressBegin(m_Decompressor, m_pInputFormat, (LPBITMAPINFOHEADER)m_pTargetFormat);
	
	m_dwFps		= m_stStreamInfo.dwRate / m_stStreamInfo.dwScale;	
	m_nTimeTick = (1000 * m_stStreamInfo.dwScale + (m_stStreamInfo.dwRate >> 1)) / m_stStreamInfo.dwRate;

	SetRect(&m_rcSrc, 0, 0, m_pTargetFormat->bV4Width, m_pTargetFormat->bV4Height);
	SetRect(&m_rcDst, 0, 0, 640, 480);

	// ����κ� ����.(���� ���尡 ���ٸ� �̹����� �÷����ϰ� �����Ѵ�.)
	AVIFileGetStream(m_pAviFile, &m_pAviSound, streamtypeAUDIO, 0);
/*	if( AVIFileGetStream(m_pAviFile, &m_pAviSound, streamtypeAUDIO, 0) )
	{ 
		AVIFileExit();
		SAFE_DELETE(m_pInputFormat);
		SAFE_DELETE(m_pTargetFormat);
		SAFE_DELETE(m_lpInput);
		SAFE_DELETE(m_lpOutput);
		return FALSE;
	}*/

	if ( lpDS != NULL && m_pAviSound )
	{
		AVIStreamFormatSize(m_pAviSound, 0, &lFmtLenght);
		m_pSoundFormat = (WAVEFORMATEX*) new BYTE[lFmtLenght];

		AVIStreamReadFormat(m_pAviSound, 0, m_pSoundFormat, &lFmtLenght);
		AVIStreamInfo(m_pAviSound, &m_stSoundInfo, sizeof(AVISTREAMINFO));

		m_dwLoadSize = (m_pSoundFormat->nAvgBytesPerSec + m_dwFps - 1 ) / m_dwFps;

		DSBUFFERDESC dsbd;
		ZeroMemory(&dsbd, sizeof(DSBUFFERDESC));
		dsbd.dwSize		   = sizeof(DSBUFFERDESC);
		dsbd.dwFlags	   = 0;
		dsbd.dwReserved	   = 0;
		dsbd.lpwfxFormat   = m_pSoundFormat;
	//	dsbd.dwBufferBytes = m_dwLoadSize * m_dwFps;
		dsbd.dwBufferBytes = m_stSoundInfo.dwSuggestedBufferSize * m_dwFps * 2;

		if ( lpDS->CreateSoundBuffer(&dsbd, &m_lpdsbAvi, NULL) != DS_OK )
		{
			AVIFileExit();
			SAFE_DELETE(m_pInputFormat);
			SAFE_DELETE(m_pTargetFormat);
			SAFE_DELETE(m_lpInput);
			SAFE_DELETE(m_lpOutput);
			return FALSE;
		}
	}

	m_bLoop	   = bLoop;
	m_bRevPlay = bRevPlay;

	m_lIndex = (!m_bRevPlay ? 0 : m_lFrames-1);

	return TRUE;
}


VOID CALLBACK GetFrameTimer(UINT, UINT, DWORD dwAviSurface, DWORD, DWORD)
{
	((CAvi*)dwAviSurface)->NextFrame();
}


BOOL CAvi::Start()
{
	if ( m_nTimerID == 0 )
	{
		m_nTimerID = timeSetEvent(m_nTimeTick, 5, GetFrameTimer, (ULONG)this, TIME_PERIODIC);
		m_bPlaying = (m_nTimerID != NULL);
		CAvi::ReadFrame(0);
	}

	if ( m_lpdsbAvi )
	{
		m_nSoundFramesAhead = m_stSoundInfo.dwInitialFrames / m_stSoundInfo.dwScale;

		if ( m_lpdsbAvi->Play(0, 0, DSBPLAY_LOOPING) != DS_OK ) 
		{
			return FALSE;
		}
	}

	if ( m_pAviSound )
	{
		if ( !GetSound() ) 
		{
			return FALSE;
		}
	}

	Sleep(50);

	return m_bPlaying;
}


VOID CAvi::Stop()
{
	if ( m_lpdsbAvi )
	{
		m_lpdsbAvi->Stop();
	}

	if ( m_nTimerID )
	{
		timeKillEvent(m_nTimerID);
		m_nTimerID = 0;

		Sleep(50);
	}

	m_bPlaying = FALSE;	
}


VOID CAvi::NextFrame()
{
	m_lIndex = m_lIndex + (!m_bRevPlay ? 1 : -1);

	if ( m_lIndex == (!m_bRevPlay ? m_lFrames : 0) )
	{
		Stop();

		if ( m_bLoop )
		{
			m_nTimerID   = timeSetEvent(m_nTimeTick, 5, GetFrameTimer, (ULONG)this, TIME_PERIODIC);
			m_lIndex	 = (!m_bRevPlay ? 0 : m_lFrames);
			m_dwLoadPos  = 0;
			m_bPlaying   = (m_nTimerID != NULL);

			if ( m_lpdsbAvi )
			{
				m_lpdsbAvi->SetCurrentPosition(0);
				GetSound();
				m_lpdsbAvi->Play(0, 0, DSBPLAY_LOOPING);
			}
		}
	}

	GetSound();
	ReadFrame(m_lIndex);
}


VOID CAvi::ReadFrame(LONG lFrame)
{
	lFrame -= m_nSoundFramesAhead;

	if ( lFrame < 0 ) 
	{
		return;
	}

	if ( lFrame < m_lFrames )
	{
		if ( m_pAviStream )
		{
			AVIStreamRead(m_pAviStream, lFrame, 1, m_lpInput, m_lLength, NULL, NULL);

			if ( m_Decompressor )
			{
				EnterCriticalSection(&m_AccessBuffer);
				ICDecompress(m_Decompressor, 0, m_pInputFormat, m_lpInput, (LPBITMAPINFOHEADER)m_pTargetFormat, m_lpOutput);
				LeaveCriticalSection(&m_AccessBuffer);
			}
		}
	}
}


VOID CAvi::DrawFrame565(WORD* pDst, LONG lDstPitch, INT nDstW, INT nDstH)
{
	// [DX9][B] m_lpOutput 24bit BGR(自底向上 DIB) → 565 最近邻缩放写 pDst
	if ( !m_lpOutput || !pDst ) return;
	INT sw = m_pTargetFormat ? m_pTargetFormat->bV4Width : 0;
	INT sh = m_pTargetFormat ? m_pTargetFormat->bV4Height : 0;
	if ( sw <= 0 || sh <= 0 ) return;
	LONG lw = lDstPitch >> 1;
	for ( INT dy = 0; dy < nDstH; dy++ )
	{
		INT sy = (INT)(((FLOAT)dy * (FLOAT)sh) / (FLOAT)nDstH);
		if ( sy >= sh ) sy = sh - 1;
		if ( sy < 0 ) sy = 0;
		WORD* dRow = pDst + (LONG)dy * lw;
		BYTE* sRow = m_lpOutput + (LONG)(sh - 1 - sy) * (sw * 3);   // DIB 自底向上
		for ( INT dx = 0; dx < nDstW; dx++ )
		{
			INT sx = (INT)(((FLOAT)dx * (FLOAT)sw) / (FLOAT)nDstW);
			if ( sx >= sw ) sx = sw - 1;
			if ( sx < 0 ) sx = 0;
			BYTE* p = sRow + sx * 3;   // BGR
			dRow[dx] = (WORD)(((p[2] >> 3) << 11) | ((p[1] >> 2) << 5) | (p[0] >> 3));
		}
	}
}


BOOL CAvi::GetSound()
{
	if ( m_lpdsbAvi )
	{
		HRESULT hr;
		DWORD	dwSize1, dwSize2;
		LPVOID	Data1, Data2;

/*		hr = m_lpdsbAvi->Lock(m_dwLoadPos * m_dwLoadSize, m_dwLoadSize, &Data1, &dwSize1, &Data2, &dwSize2, 0);

		if ( hr != DS_OK )
		{
			return FALSE;
		}

		AVIStreamRead(m_pAviSound, m_lIndex * m_dwLoadSize >> 1, m_dwLoadSize >> 1, Data1, m_dwLoadSize, NULL, NULL);

		hr = m_lpdsbAvi->Unlock(Data1, dwSize1, Data2, dwSize2);

		m_dwLoadPos++;
		m_dwLoadPos %= m_dwFps;
		*/

		hr = m_lpdsbAvi->Lock(m_dwLoadPos * m_stSoundInfo.dwSuggestedBufferSize, m_stSoundInfo.dwSuggestedBufferSize, &Data1, &dwSize1, &Data2, &dwSize2, 0);

		if ( hr != DS_OK )
		{
			return FALSE;
		}

		if ( m_pSoundFormat->nBlockAlign == 0 )
		{
			return FALSE;
		}

		AVIStreamRead(m_pAviSound, 
			          m_lIndex * m_stSoundInfo.dwSuggestedBufferSize / m_pSoundFormat->nBlockAlign,
		              m_stSoundInfo.dwSuggestedBufferSize / m_pSoundFormat->nBlockAlign, 
					  Data1, m_stSoundInfo.dwSuggestedBufferSize, NULL, NULL);

		hr = m_lpdsbAvi->Unlock(Data1, dwSize1, Data2, dwSize2);

		m_dwLoadPos++;
		m_dwLoadPos %= m_dwFps * 2;

		return TRUE;
	}

	return FALSE;
}


/*
HRESULT CAvi::DrawBuffer()
{
	HRESULT		   hr;
	DDSURFACEDESC2 ddsd;

	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	hr = m_lpddsAvi->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);

	if ( hr != DD_OK )
	{
		return hr;
	}

	LPBYTE lpSource = m_lpOutput + m_lLinePitch * (m_pTargetFormat->bV4Height - 1);
	LPBYTE lpDest   = (LPBYTE)ddsd.lpSurface;

	if ( hr == DD_OK )
	{
		EnterCriticalSection(&m_AccessBuffer);

		for ( INT nCnt = 0; nCnt < (INT)m_pTargetFormat->bV4Height; nCnt++)
		{
			memcpy(lpDest, lpSource, m_lLinePitch);
			lpDest   += ddsd.lPitch;
			lpSource -= m_lLinePitch;
		}

		LeaveCriticalSection(&m_AccessBuffer);

		hr = m_lpddsAvi->Unlock(NULL);

		while ( hr == DDERR_SURFACELOST ) 
		{
			hr = m_lpddsAvi->Unlock(NULL);
		}
	}

	return hr;
}
*/
HRESULT CAvi::DrawBuffer(VOID)
{
	HRESULT hr;
	DDSURFACEDESC2 ddsd;

	if ( m_lpddsAvi )
	{
		ZeroMemory(&ddsd, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);
		hr = m_lpddsAvi->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
		
		if ( hr == DD_OK && m_lpOutput )
		{
			LPBYTE lpSource = m_lpOutput + m_lLinePitch * (m_pTargetFormat->bV4Height - 1);
			LPBYTE lpDest   = (LPBYTE)ddsd.lpSurface;

			INT xs = m_lLinePitch>>2,
				xm = m_lLinePitch*2,
				y  = m_pTargetFormat->bV4Height,
				z  = ddsd.lPitch-m_lLinePitch;

			EnterCriticalSection(&m_AccessBuffer);

			if ( m_pTargetFormat && m_pTargetFormat->bV4Width%4 )
			{	
				for( INT nCnt = 0; nCnt < (INT)m_pTargetFormat->bV4Height; nCnt++ )
				{	
					memcpy(lpDest, lpSource, m_lLinePitch);
					lpDest   += ddsd.lPitch;
					lpSource -= m_lLinePitch;
				}
			}
			else
			{
				char *psrc = (char*)lpSource;
				char *pdst = (char*)lpDest;
				for ( int _n = (int)y; _n > 0; _n-- )
				{
					memcpy( pdst, psrc, (size_t)xs * 4 );
					psrc -= (int)xm;
					pdst += (int)z;
				}
			}

			LeaveCriticalSection(&m_AccessBuffer);

			hr = m_lpddsAvi->Unlock(NULL);

			while ( hr == DDERR_SURFACELOST ) 
			{
				hr = m_lpddsAvi->Unlock(NULL);
			}
		}

		return hr;
	}

	return E_FAIL;
}


BOOL CAvi::Draw(LPDIRECTDRAWSURFACE7 lpddsDest)
{
	if ( DrawBuffer() != DD_OK )
	{
		return FALSE;
	}
	
	if ( lpddsDest )
	{
		lpddsDest->Blt(&m_rcDst, m_lpddsAvi, &m_rcSrc, DDBLT_WAIT, NULL);

	//	if ( hr == DDERR_SURFACELOST )
	//	{
	//		Restore();
	//	}

		return m_bPlaying;
	}

	return FALSE;
}


HRESULT CAvi::DrawFast(INT nX, INT nY, LPDIRECTDRAWSURFACE7 lpddsDest)
{
	HRESULT hr;

	hr = DrawBuffer();

	if ( hr != DD_OK ) 
	{
		return hr;
	}

	if ( lpddsDest )
	{
		hr = lpddsDest->BltFast(nX, nY, m_lpddsAvi, &m_rcSrc, DDBLTFAST_WAIT);

	//	if ( hr == DDERR_SURFACELOST )
	//	{
	//		Restore();
	//	}

		return hr;
	}

	return E_FAIL;
}
