#pragma once 

class InverseKinematics
{
public:
	InverseKinematics(class M2Mesh* pSkinnedMesh);
	void UpdateHeadIK();
	void ApplyLookAtIK(noVec3 &lookAtTarget, float maxAngle);

	void DrawDebug();
private:
	M2Mesh *m_pSkinnedMesh;
	class Bone& m_pHeadBone;
	noVec3 m_headForward;
	class NavDebugDraw*	dd_;	

	noVec3 m_localRotationAxis;
	noVec3 m_localLookAt;	
};
