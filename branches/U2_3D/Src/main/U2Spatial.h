/**************************************************************************************************
module	:	U2Spatial
Author	:	Yun sangyong
Desc	:	현재 OOBB 기반의 BoundingVolume은 사용하지 않고
			AABB를 사용해 가시성 테스트를 한다.
*************************************************************************************************/
#ifndef	U2_SPATIAL_H
#define	U2_SPATIAL_H

#include <U2Lib/Src/U2SmartPtr.h>
#include <U2Lib/Src/U2Flags.h>

#include <U2_3D/src/U2_3DLibType.h>
#include <U2_3D/src/Main/U2Object.h>
#include <U2_3D/Src/Main/U2RenderContext.h>
#include <U2_3D/src/Dx9/U2RenderStateCollection.h>
#include <U2_3D/src/Effect/U2Effect.h>

#include "U2Transform.h"
#include "U2Culler.h"
#include "U2BoundingVolume.h"

U2SmartPointer(U2Spatial);
U2SmartPointer(U2Light);
class U2Node;

class U2_3D U2Spatial : public U2Object
{
	DECLARE_RTTI;
public:

	// Culling parameters.
	enum CullingMode
	{
		// Determine visibility state by comparing the world bounding volume
		// to culling planes.
		CULL_DYNAMIC,

		// Force the object to be culled.  If a Node is culled, its entire
		// subtree is culled.
		CULL_ALWAYS,

		// Never cull the object.  If a Node is never culled, its entire
		// subtree is never culled.  To accomplish this, the first time such
		// a Node is encountered, the bNoCull parameter is set to 'true' in
		// the recursive chain GetVisibleSet/OnGetVisibleSet.
		CULL_NEVER,

		MAX_CULLING_MODE
	};

	CullingMode m_eCulling;
	
	virtual ~U2Spatial();

	__forceinline U2Node* GetParent();
	__forceinline const U2Node* GetParent() const;

	void Update(float time =0.f, bool bInit = true);
	void UpdateBound();
	void UpdateSelected(float fTime);	
	virtual void UpdateRenderStates(U2RenderStateCollectionPtr spRSC = 0, 
		U2ObjVec<U2LightPtr>* pLightArray = 0);
	
	void OnVisibleSet(U2Culler& rkCuller, bool bNoCull);
	virtual void GetVisibleSet (U2Culler& rkCuller, bool bNoCull) = 0;


	virtual void UpdateRenderContext(U2RenderContext* pRCxt);


	
	// transform
	inline void					SetLocalTrans(const D3DXVECTOR3& trans);
	inline void					SetLocalTrans(float x, float y, float z);
	inline const D3DXVECTOR3&	GetLocalTrans() const;
	
	inline void					SetLocalRot(const D3DXMATRIX& rot);
	inline const D3DXMATRIX		GetLocalRot() const;
	inline void					SetLocalRot(float angle, float x, float y, float z);
	//inline void					SetLocalRot(float &angle, float &x, float &y, float &z);
	inline void					SetLocalRot(const D3DXQUATERNION& quat);
	inline void					GetLocalRot(D3DXQUATERNION& outQuat) const;
	inline void					GetLocalRot(float &angle, float &x, float &y, float &z);
	inline void					SetRotateAboutCenter(const D3DXMATRIX& rot, 
		const D3DXVECTOR3& cen, const D3DXVECTOR3& trans = VECTOR3_ZERO);
	
	inline void					SetLocalScale(float scale);
	inline void					SetLocalScale(float scaleX, float scaleY, float scaleZ);
	inline D3DXVECTOR3			GetLocalScale() const;
	
	inline const D3DXVECTOR3	GetWorldTranslate() const;
	inline const D3DXMATRIX		GetWorldRotate() const;
	inline D3DXVECTOR3			GetWorldScale() const;

	// for occlusion culling
	inline void					SetVisibleObject(bool bDisplay);
	inline bool					GetVisibleObject() const;

	inline void					SetWorldTranslate(const D3DXVECTOR3 trans);
	inline void					SetWorldRotate(const D3DXMATRIX& rot);
	inline void					SetWorldScale(float fScale);

	void						Cull();
	void						SetCulledFlag(bool bAppCulled);
	bool						IsCulled() const;

	U2RSList& GetRSList();
	const U2RSList& GetRSList() const;
	void AttachRenderState(U2RenderState* pRS);
	void DetachRenderState(U2RenderState* pRS);	
	void DetachAllRenderStates();
	U2RenderState* GetRenderState(int rsType) const;	

	uint32 GetLightCnt() const;
	U2Light* GetLight(uint32 idx);
	void AttachLight(U2Light* pLight);
	void DetachLight(U2Light* pLight);
	void DetachAllLights();

	int GetEffectCnt() const;
	U2Effect* GetEffect(uint32 idx);	
	const U2Effect* GetEffect(uint32 idx) const;	
	void AttachEffect(U2Effect* pEffect);
	void DetachEffect(U2Effect* pEffect);
	void DetachAllEffects();
	void SetStartEffect(uint32 i);
	uint32 GetStartEffect() const;
	
	void AttachParent(U2Node* pkParent);
	void DetachParent();

	void SetActive(bool bActive);
	bool GetActive() const;

	// Add Aabb
	void SetBoundingBox(const U2Aabb& box);
	const U2Aabb& GetBoundingBox() const;

	U2BoundingVolumePtr m_spWorldBound;		 

	U2Aabb m_bbox;

	bool m_bCurrWorldBound;
	U2Transform m_tmLocal;
	U2Transform m_tmWorld;
	bool m_bCurrWorldTM;

	U2RenderStateCollectionPtr m_spRSC;

	
protected:
	U2Spatial();

	enum 
	{
		CULLED_MASK				= 0x0001,		
		DISABLE_SORT			= 0x0002,
	};

	virtual void UpdateWorldData(float fTime);
	virtual void UpdateWorldBound() = 0;
	virtual void UpdateState(U2RenderStateCollectionPtr spRS , 
		U2ObjVec<U2LightPtr>* pLightArray) = 0;
	
	void PropagateStateFromRoot(U2RenderStateCollectionPtr& spRSC, U2ObjVec<U2LightPtr>* pLightArray);
	void PropagateBoundToRoot();
	U2RenderStateCollectionPtr PushRenderStates(
		U2RenderStateCollectionPtr spRSC, bool bCopyOnChange);

	uint32 PushLights(
		U2ObjVec<U2LightPtr>* pLightArray);
	
protected:

	// Bound	
	U2Node* m_pParent;
	bool m_bActive;
	uint32 m_uStartEffect;

	U2RSList m_renderStateList;
	U2ObjVec<U2LightPtr> m_aspLights;
	U2ObjVec<U2EffectPtr> m_aspEffects;
	
	U2DeclareFlags(unsigned short);	

};


#include "U2Spatial.inl"


#endif