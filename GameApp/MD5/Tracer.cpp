// Tracer.cpp:
// Created by: Aurelio Reis

#include "stdafx.h"

#include "Tracer.h"


//////////////////////////////////////////////////////////////////////////
// CArTracer
//////////////////////////////////////////////////////////////////////////

CArTracer *CArTracer::s_pHead = NULL;
CArTracer *CArTracer::s_pTail = NULL;
int CArTracer::s_iNumTracers = 0;

CArTracer::CArTracer() : m_pNext( NULL ), m_fExpireTime( 0.0f ), m_uiBirthFrame( 0 )
{

}

CArTracer::~CArTracer()
{

}

void CArTracer::AddTracer( CArTracer *pTracer )
{
	s_iNumTracers++;

	pTracer->m_fExpireTime = (float)g_fTime + TRACER_LIFETIME;
	pTracer->m_uiBirthFrame = g_uiFrameNum;

	if ( !CArTracer::s_pHead )
	{
		CArTracer::s_pHead = CArTracer::s_pTail = pTracer;	

		return;
	}

	CArTracer::s_pTail->m_pNext = pTracer;
	CArTracer::s_pTail = pTracer;
}

void CArTracer::RemoveExpiredTracers()
{
	// Old tracers should be up in the front so keep deleting
	// until we hit an un-expired tracer.
	CArTracer *pCurTracer = CArTracer::s_pHead;
	CArTracer *pNxtTracer;
	for ( ; pCurTracer != NULL; pCurTracer = pNxtTracer )
	{
		pNxtTracer = pCurTracer->m_pNext;

		if ( pCurTracer->m_fExpireTime <= g_fTime )
		{
			CArTracer::s_pHead = pNxtTracer;

			if ( pCurTracer == CArTracer::s_pTail )
			{
				CArTracer::s_pTail = NULL;
			}

			SAFE_DELETE( pCurTracer );
			s_iNumTracers--;
		}
	}
}

void CArTracer::Update()
{
	static float fTraceDebounce = 0.0f;

	if ( fTraceDebounce < g_fTime && GetAsyncKeyState( VK_LBUTTON ) )
	{
		POINT pt;
		GetCursorPos( &pt );
		ScreenToClient( g_hWnd, &pt );

		if ( pt.x > 0 && pt.y > 0 &&
			 pt.x < (int)g_pDeviceSettings->d3d9.pp.BackBufferWidth &&
			 pt.y < (int)g_pDeviceSettings->d3d9.pp.BackBufferHeight )
		{
			CArTracer *pTracer = new CArTracer();
			CArTracer::AddTracer( pTracer );

			ProjectScreenToWorld( pTracer->m_vStart, pt.x, pt.y, 0.0f );
			ProjectScreenToWorld( pTracer->m_vEnd, pt.x, pt.y, 1.0f );

			// FIXME: Offset from the view axis a little so we can see the tracers
			pTracer->m_vStart.z += 1.0f;

			fTraceDebounce = (float)g_fTime + 0.05f;	// five hundredths of a second debounce
		}
	}
}
