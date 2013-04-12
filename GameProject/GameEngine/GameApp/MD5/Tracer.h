// Tracer.h:
// Created by: Aurelio Reis

#ifndef __AR__TRACER__H__
#define __AR__TRACER__H__


static const float TRACER_LIFETIME = 3.0f;

class CArTracer
{
public:
	static CArTracer *s_pHead, *s_pTail;
	static int s_iNumTracers;

	D3DXVECTOR3		m_vStart, m_vEnd;
	CArTracer		*m_pNext;
	float			m_fExpireTime;
	UINT32			m_uiBirthFrame;

	CArTracer();
	~CArTracer();

	static void AddTracer( CArTracer *pTracer );
	static void RemoveExpiredTracers();
	static void Update();
};


#endif // __AR__TRACER__H__
