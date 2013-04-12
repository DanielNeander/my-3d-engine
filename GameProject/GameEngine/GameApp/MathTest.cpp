#include "stdafx.h"
#include "stdafx.h"
#include <EngineCore/Util/BSP.h>
#include <EngineCore/Util/Model.h>

#include "GameApp.h"
#include "CascadedShadowsManager.h"
#include "GameApp/Mesh.h"
#include "GameApp/SceneNode.h"
#include "GameApp/M2Mesh.h"
#include "GameApp/Mesh_D3D11.h"
#include "GameApp/TestObjects.h"
#include "GameApp/GameObjectManager.h"
#include "GameApp/GameObjectUtil.h"
#include "GameApp/Common.h"
#include "ModelLib/M2Loader.h"
#include "GameApp/Actor.h"
#include "GameApp/Font/Dx11TextHelper.h"
#include "SDKMeshLoader.h"
#include "Utility.h"
#include "ShaderCompilation.h"
#include "PostProcessor.h"
#include "DeviceStates.h"

#include "Terrain/AdaptiveModelDX11Render.h"
#include "LIDRenderer.h"
#include "U2Light.h"

class Vector
{
public:
	
	FLOAT X,Y,Z, W;
	
	Vector() {}
	Vector(float x, float y, float z, float w)
		:X(x), Y(y), Z(z), W(w)
	{

	}
};


class FMatrix2
{
public:

	union
	{
		MS_ALIGN(16) FLOAT M[4][4] GCC_ALIGN(16);
	};
	//MS_ALIGN(16) static const FMatrix Identity GCC_ALIGN(16);

	// Constructors.

	FORCEINLINE FMatrix2() {}
	//FORCEINLINE FMatrix(const FPlane& InX,const FPlane& InY,const FPlane& InZ,const FPlane& InW);
	FORCEINLINE FMatrix2(const Vector& InX,const Vector& InY,const Vector& InZ,const Vector& InW);

	FORCEINLINE FMatrix2		operator* (const FMatrix2& Other) const;
};

FORCEINLINE FMatrix2::FMatrix2( const Vector& InX,const Vector& InY,const Vector& InZ,const Vector& InW )
{
	M[0][0] = InX.X; M[0][1] = InX.Y;  M[0][2] = InX.Z;  M[0][3] = InX.W;
	M[1][0] = InY.X; M[1][1] = InY.Y;  M[1][2] = InY.Z;  M[1][3] = InY.W;
	M[2][0] = InZ.X; M[2][1] = InZ.Y;  M[2][2] = InZ.Z;  M[2][3] = InZ.W;
	M[3][0] = InW.X; M[3][1] = InW.Y;  M[3][2] = InW.Z;  M[3][3] = InW.W;
}



FORCEINLINE FMatrix2 FMatrix2::operator*(const FMatrix2& Other) const
{
	FMatrix2 Result;
	VectorMatrixMultiply( &Result, this, &Other );
	return Result;
}

class Quat 
{
public:
	Quat() {}

	Quat( const FMatrix2& M );
	//static const FQuat Identity;
	Quat( const noMat4& m);

	// Variables.
	FLOAT X,Y,Z,W;
};

Quat::Quat( const FMatrix2& M )
{
	//const MeReal *const t = (MeReal *) tm;
	FLOAT	s;

	// Check diagonal (trace)
	const FLOAT tr = M.M[0][0] + M.M[1][1] + M.M[2][2];

	if (tr > 0.0f) 
	{
		FLOAT InvS = appInvSqrt(tr + 1.f);
		this->W = 0.5f * (1.f / InvS);
		s = 0.5f * InvS;

		this->X = (M.M[1][2] - M.M[2][1]) * s;
		this->Y = (M.M[2][0] - M.M[0][2]) * s;
		this->Z = (M.M[0][1] - M.M[1][0]) * s;
	} 
	else 
	{
		// diagonal is negative
		INT i = 0;

		if (M.M[1][1] > M.M[0][0])
			i = 1;

		if (M.M[2][2] > M.M[i][i])
			i = 2;

		static const INT nxt[3] = { 1, 2, 0 };
		const INT j = nxt[i];
		const INT k = nxt[j];

		s = M.M[i][i] - M.M[j][j] - M.M[k][k] + 1.0f;

		FLOAT InvS = appInvSqrt(s);

		FLOAT qt[4];
		qt[i] = 0.5f * (1.f / InvS);

		s = 0.5f * InvS;

		qt[3] = (M.M[j][k] - M.M[k][j]) * s;
		qt[j] = (M.M[i][j] + M.M[j][i]) * s;
		qt[k] = (M.M[i][k] + M.M[k][i]) * s;

		this->X = qt[0];
		this->Y = qt[1];
		this->Z = qt[2];
		this->W = qt[3];
	}
}

Quat::Quat( const noMat4& M )
{
	//const MeReal *const t = (MeReal *) tm;
	FLOAT	s;

	// Check diagonal (trace)
	const FLOAT tr = M.mat[0][0] + M.mat[1][1] + M.mat[2][2];

	if (tr > 0.0f) 
	{
		FLOAT InvS = appInvSqrt(tr + 1.f);
		this->W = 0.5f * (1.f / InvS);
		s = 0.5f * InvS;

		this->X = (M.mat[1][2] - M.mat[2][1]) * s;
		this->Y = (M.mat[2][0] - M.mat[0][2]) * s;
		this->Z = (M.mat[0][1] - M.mat[1][0]) * s;
	} 
	else 
	{
		// diagonal is negative
		INT i = 0;

		if (M.mat[1][1] > M.mat[0][0])
			i = 1;

		if (M.mat[2][2] > M.mat[i][i])
			i = 2;

		static const INT nxt[3] = { 1, 2, 0 };
		const INT j = nxt[i];
		const INT k = nxt[j];

		s = M.mat[i][i] - M.mat[j][j] - M.mat[k][k] + 1.0f;

		FLOAT InvS = appInvSqrt(s);

		FLOAT qt[4];
		qt[i] = 0.5f * (1.f / InvS);

		s = 0.5f * InvS;

		qt[3] = (M.mat[j][k] - M.mat[k][j]) * s;
		qt[j] = (M.mat[i][j] + M.mat[j][i]) * s;
		qt[k] = (M.mat[i][k] + M.mat[k][i]) * s;

		this->X = qt[0];
		this->Y = qt[1];
		this->Z = qt[2];
		this->W = qt[3];
	}
}



struct FBoneAtom
{
	Quat	Rotation;
	Vector	Translation;
	FLOAT	Scale;

	FBoneAtom()
	{

	}

	void ToTransform(FMatrix2& OutMatrix) const
	{
		OutMatrix.M[3][0] = Translation.X;
		OutMatrix.M[3][1] = Translation.Y;
		OutMatrix.M[3][2] = Translation.Z;

		const FLOAT x2 = Rotation.X + Rotation.X;	
		const FLOAT y2 = Rotation.Y + Rotation.Y;  
		const FLOAT z2 = Rotation.Z + Rotation.Z;
		{
			const FLOAT xx2 = Rotation.X * x2;
			const FLOAT yy2 = Rotation.Y * y2;			
			const FLOAT zz2 = Rotation.Z * z2;

			OutMatrix.M[0][0] = (1.0f - (yy2 + zz2)) * Scale;	
			OutMatrix.M[1][1] = (1.0f - (xx2 + zz2)) * Scale;
			OutMatrix.M[2][2] = (1.0f - (xx2 + yy2)) * Scale;
		}
		{
			const FLOAT yz2 = Rotation.Y * z2;   
			const FLOAT wx2 = Rotation.W * x2;	

			OutMatrix.M[2][1] = (yz2 - wx2) * Scale;
			OutMatrix.M[1][2] = (yz2 + wx2) * Scale;
		}
		{
			const FLOAT xy2 = Rotation.X * y2;
			const FLOAT wz2 = Rotation.W * z2;

			OutMatrix.M[1][0] = (xy2 - wz2) * Scale;
			OutMatrix.M[0][1] = (xy2 + wz2) * Scale;
		}
		{
			const FLOAT xz2 = Rotation.X * z2;
			const FLOAT wy2 = Rotation.W * y2;   

			OutMatrix.M[2][0] = (xz2 + wy2) * Scale;
			OutMatrix.M[0][2] = (xz2 - wy2) * Scale;
		}

		OutMatrix.M[0][3] = 0.0f;
		OutMatrix.M[1][3] = 0.0f;
		OutMatrix.M[2][3] = 0.0f;
		OutMatrix.M[3][3] = 1.0f;
	}

	void ToTransform2(noMat4& OutMatrix) const
	{
		OutMatrix.mat[3][0] = Translation.X;
		OutMatrix.mat[3][1] = Translation.Y;
		OutMatrix.mat[3][2] = Translation.Z;

		const FLOAT x2 = Rotation.X + Rotation.X;	
		const FLOAT y2 = Rotation.Y + Rotation.Y;  
		const FLOAT z2 = Rotation.Z + Rotation.Z;
		{
			const FLOAT xx2 = Rotation.X * x2;
			const FLOAT yy2 = Rotation.Y * y2;			
			const FLOAT zz2 = Rotation.Z * z2;

			OutMatrix.mat[0][0] = (1.0f - (yy2 + zz2)) * Scale;	
			OutMatrix.mat[1][1] = (1.0f - (xx2 + zz2)) * Scale;
			OutMatrix.mat[2][2] = (1.0f - (xx2 + yy2)) * Scale;
		}
		{
			const FLOAT yz2 = Rotation.Y * z2;   
			const FLOAT wx2 = Rotation.W * x2;	

			OutMatrix.mat[2][1] = (yz2 - wx2) * Scale;
			OutMatrix.mat[1][2] = (yz2 + wx2) * Scale;
		}
		{
			const FLOAT xy2 = Rotation.X * y2;
			const FLOAT wz2 = Rotation.W * z2;

			OutMatrix.mat[1][0] = (xy2 - wz2) * Scale;
			OutMatrix.mat[0][1] = (xy2 + wz2) * Scale;
		}
		{
			const FLOAT xz2 = Rotation.X * z2;
			const FLOAT wy2 = Rotation.W * y2;   

			OutMatrix.mat[2][0] = (xz2 + wy2) * Scale;
			OutMatrix.mat[0][2] = (xz2 - wy2) * Scale;
		}

		OutMatrix.mat[0][3] = 0.0f;
		OutMatrix.mat[1][3] = 0.0f;
		OutMatrix.mat[2][3] = 0.0f;
		OutMatrix.mat[3][3] = 1.0f;
	}
};

void App::TestMath()
{
	noMat4 test;
	noVec4 x = noVec4( 1, 4, 5, 0);
	noVec4 y = noVec4(0, 5,12, 0);
	noVec4 z = noVec4(0, 5,12, 0);
	noVec4 w = noVec4(12, 14, 12, 1);
	test.mat[0] = x;
	test.mat[1] = y;
	test.mat[2] = z;
	test.mat[3] = w;
	
	noMat4 test2;
	test2 = test;

	test = test * test2;
			
	Quat t(test);

	Vector x1( 1, 4, 5, 0);
	Vector y1(0, 5,12, 0);
	Vector z1(0, 5,12, 0);
	Vector w1(12, 14, 12, 1);

	FMatrix2 m(x1, y1, z1, w1);

	FMatrix2 m2;
	memcpy(&m2, &m, SIZE16);

	m = m * m2;

	Quat t2(m);
	
	FBoneAtom a;
	a.Rotation = t;
	a.Translation = Vector(3, 5, 7, 1);
	a.Scale = 1.0f;

	FMatrix2 out3;
	a.ToTransform(out3);
	
	

	int tps = 150;
	D3DXMATRIX rot;
	D3DXVECTOR3 axis(1, 0, 1);
	D3DXVECTOR3 arbitrary(0.5f, 0.5f, 0.0f);
	D3DXMatrixRotationAxis(&rot, &axis, noMath::PI*2.0f/(float)(tps-1));
			
	D3DXVec3TransformCoord(&arbitrary,&arbitrary,&rot);


	noRotation rot2;
	noVec3 axis2(1, 0, 1);
	noVec3 arbitrary2(0.5f, 0.5f, 0.0f);
	rot2.Set(vec3_zero, axis2, -RAD2DEG(noMath::PI/(float)(tps-1)));
	noMat3 mat = rot2.ToMat3();	
	arbitrary2 = mat * arbitrary2;
	

	noMat4 out4;
	a.ToTransform2(out4);
	int x2;
}