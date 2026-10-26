// **************************************************************************************
//
//  Euyheon's Queue Class
//
//  All written by Euy-heon, Jeong.
//  Copyright(C) 1999-2001 Euy-heon, Jeong. All rights reserved.
//
//	Win32 Version : Compiled by Microsoft Visual C++ 6.0
//
//	Problems & History
//	------------------
//
// **************************************************************************************

#include "stdafx.h"

CWHQueue::CWHQueue()
{
	InitializeCriticalSection(&m_cs);

	m_btQPushPos	= 0;
	m_btQPopPos		= 0; 

	m_btCount		= 0;
}

CWHQueue::~CWHQueue()
{
	DeleteCriticalSection(&m_cs);
}

BOOL CWHQueue::PushQ(BYTE *lpbtQ)
{
	BOOL fRet = TRUE;

	EnterCriticalSection(&m_cs);

	if (m_btQPushPos + 1 == m_btQPopPos || (m_btQPushPos + 1 == QUEUE_MAX && !m_btQPopPos))
	{
		fRet = FALSE;
		LeaveCriticalSection(&m_cs);
		return fRet;		// Queue is full.
	}

	m_lpCircularQ[m_btQPushPos] = lpbtQ;
	m_btQPushPos++;
	m_btCount++;

	if (m_btQPushPos == QUEUE_MAX)
		m_btQPushPos = 0;

	LeaveCriticalSection(&m_cs);
	
	return fRet;
}

BYTE *CWHQueue::PopQ()
{
	BYTE *pbt = NULL;

	EnterCriticalSection(&m_cs);

	if (m_btQPopPos == QUEUE_MAX)
	{
		m_btQPopPos = 0;
	}

	if (m_btQPopPos == m_btQPushPos)
	{
		LeaveCriticalSection(&m_cs);
		return pbt;		// No event.
	}

	m_btQPopPos++;
	m_btCount--;

	pbt = m_lpCircularQ[m_btQPopPos - 1];

	LeaveCriticalSection(&m_cs);

	return pbt;
}
