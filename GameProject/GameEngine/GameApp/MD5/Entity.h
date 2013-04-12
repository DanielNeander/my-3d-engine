// Entity.h:
// Created by: Aurelio Reis

#ifndef __AR__ENTITY__H__
#define __AR__ENTITY__H__

#include "Model.h"
#include "Animator.h"


//////////////////////////////////////////////////////////////////////////
// Forward decls.
//////////////////////////////////////////////////////////////////////////

class CArRay;
class CArBaseModel;
class CArModel_MD5;
class CArMD5Anim;


//////////////////////////////////////////////////////////////////////////
// CArEntity
//////////////////////////////////////////////////////////////////////////

class CArEntity
{
public:
	struct SAngles
	{
		float m_fYaw, m_fPitch, m_fRoll;
		
		void Zero()
		{
			m_fYaw = 0.0f;
			m_fPitch = 0.0f;
			m_fRoll = 0.0f;
		}

		void MatrixToAngles( const D3DXMATRIXA16 &JointMatrix );
	};

	CArModelInstance			*m_pModel;
	CArAnimator					*m_pAnimator;
	bool						m_bDismembered;

	CArEntity();
	~CArEntity();

	void Initialize( IArModelFile *pModelFile );
	void Update();
	void Draw();

	void CreateDismemberedLimbEntity( const CArDamageZone &Dz, const CArRay &Ray );

	void SetOrigin( D3DXVECTOR3 &vOrg ) { m_vOrigin = vOrg; }
	D3DXVECTOR3 &GetOrigin() { return m_vOrigin; }

	void SetAngles( const SAngles &Angles ) { m_Angles = Angles; }
	SAngles &GetAngles() { return m_Angles; }

	void PlayAnimation( CArMD5Anim *pAnim, bool bLoop );
	void DrawDamageZones( vector< CArDamageZone * > &DamageZones, const CArAnimator &Animator );

	static void AddEntity( CArEntity *pEntity );
	static void PresentEntities();
	static void DeleteEntities();

private:
	// Angles are used to generate an entity transform before drawing it.
	SAngles m_Angles;

	D3DXVECTOR3 m_vOrigin;
	D3DXVECTOR3 m_vVelocity;
	D3DXVECTOR3 m_vForce;

	// Linked list of entities stuff.
	CArEntity				*m_pNext;
	static CArEntity		*s_pHead;
	static int				s_iNumEntities;
};


#endif // __AR__ENTITY__H__
