#pragma once 

class Bone;
class M2Loader;
struct PickData;


struct M2Bone
{
	int boneId;
	noMat4* mat;
	mat4*	rdmat;	

	M2Bone*	pSibling;
	M2Bone* pFirstChild;
};


class M2Skeleton 
{
public:
	M2Skeleton(M2Loader* m) : model(m) {}

	M2Loader* model;
	//Bone* bones_;
	M2Bone* bones_;
	M2Bone* rootBone_;

	noMat4 *pBoneOffsetMatrices;    // this is the bind pose matrices
	noMat4 **pBoneMatrixPtrs;    // pointers into the skeleton for each bone, this gets updates as animation continues

	void extractSkeleton();
	void CreateBoneTree(int bone, M2Bone* childBone);
};
// X Axis : Forward 
// Y Axis : Up
// Z Axis : Right
class M2Mesh : public Mesh
{
public:
	M2Mesh();
	virtual ~M2Mesh();
	
	void Destory();
	virtual bool CreateBuffers(void);
	virtual void Draw(void);
	virtual void DrawShadow(void);
	virtual void DrawCascadedShadow(void);
	virtual void DrawCascaded(void);
	virtual void DrawZOnly(void);

	virtual void PreUpdate();
	virtual void PostUpdate();
	
	Bone& GetBone(int bone);
	const Bone& GetBone(int bone) const;	
	void UpdateBoneMatrix(int parent, const noMat4& parentMat);
	void UpdateIK();
	void UpdateParticle();
		
	void DrawBones();
	void ToggleDebug();
	void ToggleBones(int index);

	Mesh* GetBbox() const { return pBBox_; }

	void UpdateTM();

private:
	virtual void UpdateData();	
	void DrawBoundingVolumes(RenderData& rd);
public:
	M2Loader* m2_;
	class WowActor* owner_;
	struct NavDebugDraw*	dd_;	
	class InverseKinematics *m_pIK;
	RenderData box_rd;

	M2Skeleton* gSkel;	
	int id_;	// Attach id
	bool showBones_[164];
	bool drawDebug_;

	Mesh* pBBox_;
	Mesh* pDebugBBox_;

	class M2EffectRender* EffectRender_;
};