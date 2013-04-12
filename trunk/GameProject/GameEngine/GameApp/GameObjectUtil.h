#pragma once 

//#include "GameObjectManager.h"




#define GetGameObjectComponent(pkGameObject,type)   \
	noDynamicCast(type,pkGameObject->GetComponentOfType(type::ms_RTTI))

#define GetGameObjectAttribute(pkGameObject,type)   \
	noDynamicCast(type,pkGameObject->GetAttributeOfType(type::ms_RTTI))

extern noVec3 FORWARD_VECTOR;
extern noVec3 UP_VECTOR;
extern noVec3 DOWN_VECTOR;
extern noVec3 RIGHT_VECTOR;
extern noVec3 LEFT_VECTOR;

class MathHelpers
{
public:
	static float &UP_COMPONENT(noVec3 &pos);
	static float UP_COMPONENT(const noVec3 &pos);

	static float &FORWARD_COMPONENT(noVec3 &pos);
	static float FORWARD_COMPONENT(const noVec3 &pos);
	
	static float GetOneMeter();
	static void SetOneMeter(float oneMeter);

	static void SetYUp(bool b);
	static bool IsYUp()                     { return ms_bYisUp; }
	static bool IsZUp()                     { return !ms_bYisUp; }

	static bool ms_bYisUp;      // Y - up, X - Forward if true, Z up, Y forward if false
	static noVec3 GetForwardVectorFromMatrix( const noMat3& rMatrix )
	{
		noVec3 vRetVal;		
		vRetVal = rMatrix[AXIS_FORWARD()];

		if (IsYUp())
		{
			return vRetVal;
		}
		else
		{
			return vRetVal * -1.0f;
		}
	}
	static noVec3 GetRightVectorFromMatrix( const noMat3& rMatrix )
	{
		noVec3 vRetVal;		
		vRetVal = rMatrix[AXIS_RIGHT()];
		return vRetVal;
	}

	static noVec3 GetUpVectorFromMatrix( const noMat3& rMatrix )
	{
		noVec3 vRetVal;
		vRetVal = rMatrix[AXIS_UP()];
		return vRetVal;
	}

	static float GetCurrentYaw( const noMat3 &rotation );
	static int AXIS_UP()
	{
		if( ms_bYisUp )
			return 1;

		return 2;
	}

	static int AXIS_FORWARD()
	{
		if( ms_bYisUp )
			return 0;

		return 1;
	}

	static int AXIS_RIGHT()
	{
		if( ms_bYisUp )
			return 2;
		return 0;
	}


	static void FaceDirection(noMat3 &matrix, const noVec3 &cvDesiredFacing);
	static void FaceDirectionLight(noMat3 &Matrix, const noVec3 &cvDesiredFacing);
	static float GetAngleBetweenVectors (const noVec3 &u, const noVec3 &v);
	static float GetAngleBetweenActorDirections ( const noVec3 &vDir1, const noVec3 &vDir2 );
	static int GetRandomInteger()
	{
		return rand();
	}

	static int GetRandomIntegerRange(int iMin, int iMax)
	{
		return (GetRandomInteger() % (iMax - iMin + 1)) + iMin;
	}

	static int GetRandomIntegerRangeZeroTo(int iMax)
	{
		return GetRandomInteger() / (RAND_MAX / iMax + 1);
	}

	static noVec3 NormalizeVector(const noVec3 &vector);

	static void Slerp(noQuat &dest, const noQuat &q1, const noQuat &q2, float t);


	static float Dist2D(const noVec3 &pos1, const noVec3 &pos2);

	static bool IsZero(const noVec3 &point, float fValue=0.0001f);

	static VOID ComputeFrustumFromProjection( class Frustum* pOut, float* pProjection );
	static noVec4 ReciprocalEst( noVec4 V);

private:
	static float ms_oneMeter;
	static void Slerp(float *pDest, const float *pQ1, const float *pQ2, float t);
};

#define ONE_METER   MathHelpers::GetOneMeter()


