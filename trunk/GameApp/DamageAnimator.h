// Animator.h:
// Created by: Aurelio Reis

#ifndef __AR__ANIMATOR__H__
#define __AR__ANIMATOR__H__

#include "DamageModel.h"

// Forward decls.
class CArDamageZone;

//////////////////////////////////////////////////////////////////////////
// CArAnimator
//////////////////////////////////////////////////////////////////////////

class CArAnimator
{
public:
	SMD5Joint				*m_pBlendedJoints;
	D3DXMATRIXA16			*m_pJointTransforms;
	D3DXMATRIXA16			*m_pJointTransformsSkel;
	int						m_iNumTransforms;
		

	bool					m_bLoop;
	float					m_fCurAnimFrame;
	float					m_fAnimScale;

	const CArDamageZone		*m_pDismemberedDz;

	CArAnimator();
	~CArAnimator();

	void Initialize( const SMD5Skeleton *pSkel );
	void Destroy();
	void Update( const SMD5Skeleton *pSkel, class idAnimator* animator );

	void Pause();

	void SetFrame( float fFrame );
	float GetFrame();
};


#endif // __AR__MD5ANIM__H__
