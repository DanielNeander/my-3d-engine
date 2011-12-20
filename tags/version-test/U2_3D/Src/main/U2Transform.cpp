#include <U2_3D/src/U23DLibPCH.h>
#include "U2Transform.h"


D3DXMATRIX U2Transform::ZERO(
	0.0f,0.0f,0.0f,0.f,
	0.0f,0.0f,0.0f,0.f,
	0.0f,0.0f,0.0f,0.f,
	0.0f,0.0f,0.0f,0.f);

D3DXMATRIX U2Transform::IDENTITY(
	1.0f,0.0f,0.0f,0.f,
	0.0f,1.0f,0.0f,0.f,
	0.0f,0.0f,1.0f,0.f,
	0.0f,0.0f,0.0f,1.f);


U2Transform::U2Transform()
:m_vScale(1.f, 1.f, 1.f)
{
	m_uFlags = 0;
	m_matRot = U2Transform::IDENTITY;
	m_trans = VECTOR3_ZERO;
		
	SetBit(true, TM_IDENTITY);
	SetBit(true, TM_ROTSCALE);
	SetBit(true, UNIFORM_SCALE);	
}


void U2Transform::SetRot(const D3DXMATRIX& mat)
{
	m_matRot = mat;
	m_matRot._41 = m_matRot._42 = m_matRot._43 = 0.f;
	SetBit(false, TM_IDENTITY);
	SetBit(true, TM_ROTSCALE);
	
}

void U2Transform::SetTrans(const D3DXVECTOR3& trans)
{	
	m_trans = trans;	
	SetBit(false, TM_IDENTITY);
}


void U2Transform::SetScale(const D3DXVECTOR3 vScale)
{
	m_vScale = vScale;	
	SetBit(false, TM_IDENTITY);
	SetBit(false, UNIFORM_SCALE);	
}

void U2Transform::SetUniformScale(float scale)
{
	m_vScale.x = scale;
	m_vScale.y = scale;
	m_vScale.z = scale;
	SetBit(false, TM_IDENTITY);
	SetBit(true, UNIFORM_SCALE);	
}


const D3DXMATRIX& U2Transform::GetRot() const
{
	return m_matRot;
}


const D3DXVECTOR3& U2Transform::GetTrans() const
{
	return m_trans;
}


const D3DXVECTOR3& U2Transform::GetScale() const
{
	return m_vScale;
}


void U2Transform::MakeIdentity()
{
	m_matRot = U2Transform::IDENTITY;
	m_vScale = D3DXVECTOR3(1.f, 1.f, 1.f);
	m_trans = VECTOR3_ZERO;	

	SetBit(true, TM_IDENTITY);
	SetBit(true, TM_ROTSCALE);
	SetBit(true, UNIFORM_SCALE);	
}

bool U2Transform::IsIdentity() const 
{
	return GetBit(TM_IDENTITY);
}

bool U2Transform::IsRSMatrix () const
{
	return GetBit(TM_ROTSCALE);
}

bool U2Transform::IsUniformScale () const
{
	return GetBit(TM_ROTSCALE) && GetBit(UNIFORM_SCALE);
}



void U2Transform::Product (const U2Transform& rkA, const U2Transform& rkB)
{
	if(rkA.IsIdentity())
	{
		*this = rkB;
		return;
	}

	if(rkB.IsIdentity())
	{
		*this = rkA;
		return;
	}

	if(rkA.IsRSMatrix() && rkB.IsRSMatrix())
	{
		if(rkA.IsUniformScale())
		{
			SetRot(rkA.m_matRot * rkB.m_matRot);

			D3DXVECTOR3 output;
			D3DXMATRIX transpose;
			D3DXMatrixTranspose(&transpose, &rkA.m_matRot);
			D3DXVec3TransformCoord(&output, &rkB.m_trans, &transpose);	
			SetTrans(rkA.GetScale().x * output + rkA.m_trans);

			if(rkB.IsUniformScale())
			{
				SetUniformScale(rkA.m_vScale.x  * rkB.m_vScale.x);
			}
			else 
			{
				SetScale(rkA.m_vScale.x * rkB.GetScale());
			}

			return;
		}
	}

	D3DXMATRIX matA = (rkA.IsRSMatrix() ? 
		U2Math::TimesDiagonal(rkA.m_matRot, rkA.m_vScale) :
	rkA.m_matRot);
	D3DXMATRIX matB = (rkB.IsRSMatrix() ? 
		U2Math::TimesDiagonal(rkB.m_matRot, rkB.m_vScale) :
	rkB.m_matRot);

	SetRot(matA * matB);

	D3DXVECTOR3 output;
	D3DXMATRIX transpose;
	D3DXMatrixTranspose(&transpose, &matA);
	D3DXVec3TransformCoord(&output, &rkB.m_trans , &transpose);
	output += rkA.m_trans;
	SetTrans(output);
}



//bool U2Transform::operator ==(const U2Transform & xform)  const
//{
//	return m_mat == xform;
//}


//U2Transform U2Transform::operator*(const U2Transform &xform) const
//{
//	return m_mat * xform;
//}
//
//D3DXVECTOR3	U2Transform::operator*(const D3DXVECTOR3 &pt) const
//{
//	return Apply(pt);
//}

U2Plane U2Transform::Apply(const U2Plane& input) const
{
	if(GetBit(TM_IDENTITY))
	{
		return input;
	}

	U2Plane output;
	if(GetBit(TM_ROTSCALE))
	{
		if(GetBit(UNIFORM_SCALE))
		{

			output.m_vNormal = U2Math::MatProductVec(m_matRot, input.m_vNormal);
			output.m_fDistance = m_vScale.x * input.m_fDistance + 
				D3DXVec3Dot(&output.m_vNormal , &m_trans);
			return output;
		}

		output.m_vNormal = input.m_vNormal;

		float fScaleXY = m_vScale.x * m_vScale.y;
		float fScaleXZ = m_vScale.x * m_vScale.z;
		float fScaleYZ = m_vScale.y * m_vScale.z;
		float fInvDet = 1.f/(fScaleXY * m_vScale.z);
		output.m_vNormal.x *= fInvDet * fScaleYZ;
		output.m_vNormal.y *= fInvDet * fScaleXZ;
		output.m_vNormal.z *= fInvDet * fScaleXY;
		output.m_vNormal = U2Math::MatProductVec(m_matRot, output.m_vNormal);
	}
	else 
	{
		D3DXMATRIX inverseRot;
		D3DXMatrixInverse(&inverseRot, NULL, &m_matRot);
		output.m_vNormal = U2Math::MatProductVec(inverseRot, input.m_vNormal);
	}

	float fInvLen = 1.0f/ D3DXVec3Length(&output.m_vNormal);
	output.m_vNormal *= fInvLen;
	output.m_fDistance = fInvLen * input.m_fDistance +
		D3DXVec3Dot(&output.m_vNormal, &m_trans);

	return output;
}




D3DXVECTOR3 U2Transform::Apply(const D3DXVECTOR3& vIn) const
{
	D3DXVECTOR3 output(m_vScale.x * vIn.x, m_vScale.y * vIn.y, 
		m_vScale.z * vIn.z);
	D3DXMATRIX transpose;
	D3DXMatrixTranspose(&transpose, &m_matRot);
	D3DXVec3TransformCoord(&output, &output, &transpose);
	return output;
}


D3DXVECTOR3 U2Transform::InverseVector(const D3DXVECTOR3& input)
{
	if(GetBit(TM_IDENTITY))
	{
		return input;
	}

	D3DXVECTOR3 output;
	if(GetBit(TM_ROTSCALE))
	{
		 // X = S^{-1}*R^t*Y
		output = U2Math::MatProductVec(m_matRot, input);
		if(GetBit(UNIFORM_SCALE))
		{
			output /= m_vScale.x;
		}
		else 
		{
			// The direct inverse scaling is
			//   kOutput.X() /= m_kScale.X();
			//   kOutput.Y() /= m_kScale.Y();
			//   kOutput.Z() /= m_kScale.Z();
			// When division is much more expensive than multiplication,
			// three divisions are replaced by one division and ten
			// multiplications.
			float fSXY = m_vScale.x * m_vScale.y;
			float fSXZ = m_vScale.x * m_vScale.z;
			float fSYZ = m_vScale.y * m_vScale.z;
			float fInvDet = 1.0f/(fSXY * m_vScale.z);
			output.x *= fInvDet * fSYZ;
			output.y *= fInvDet * fSXZ;
			output.z *= fInvDet * fSXY;
		}
	}
	else 
	{
		// X = M^{-1}*Y
		D3DXMATRIX invMat;
		D3DXMatrixInverse(&invMat, 0, &m_matRot);
		D3DXMATRIX transpose;
		D3DXMatrixTranspose(&transpose, &invMat);
		D3DXVec3TransformCoord(&output, &input, &transpose);
	}

	return output;
}


D3DXVECTOR3 U2Transform::ApplyInverse(const D3DXVECTOR3& vec3)
{
	
	if(GetBit(TM_IDENTITY))
	{
		return vec3;
	}
		
	D3DXVECTOR3 output = vec3 - m_trans;

	if(GetBit(TM_ROTSCALE))
	{	
		D3DXVECTOR3 temp = output;
		D3DXVec3TransformCoord(&output, &temp, &m_matRot);
		if(GetBit(UNIFORM_SCALE))
		{
			output /= m_vScale.x;
		}
		else 
		{	

			float xy = m_vScale.x * m_vScale.y;
			float xz = m_vScale.x * m_vScale.z;
			float yz = m_vScale.y * m_vScale.z;
			float invDet = 1.0f/(xy * m_vScale.z);
			output.x *= invDet * yz;
			output.y *= invDet * xz;
			output.z *= invDet * xy;
		}
		
	}
	else 
	{
		D3DXMATRIX invMat;
		D3DXVECTOR3 temp = output;
		D3DXMatrixInverse(&invMat, 0, &m_matRot);
		D3DXMATRIX transpose;
		D3DXMatrixTranspose(&transpose, &invMat);
		D3DXVec3TransformCoord(&output, &temp, &transpose);
	}
	
	return output;	
}

/*void				ApplyInverse(int quantiy, const D3DXVECTOR3& inVec3,
D3DXVECTOR3& outVec3 );*/

void U2Transform::Inverse(U2Transform& invTM) const
{
	if(GetBit(TM_IDENTITY))
	{
		invTM = *this;
		return;
	}
	
	if(GetBit(TM_ROTSCALE))
	{	
		if(GetBit(UNIFORM_SCALE))
		{

			D3DXMATRIX transpose;
			D3DXMatrixTranspose(&transpose, &m_matRot);
			invTM.m_matRot = transpose / m_vScale.x;
		}
		else 
		{
		
			D3DXMATRIX RS = U2Math::TimesDiagonal(m_matRot, m_vScale);
			D3DXMATRIX invRS;
			D3DXMatrixInverse(&invRS, NULL, &RS);
			invTM.m_matRot = invRS;
		}
	}
	else 
	{
		D3DXMATRIX invMatRot;
		D3DXMatrixInverse(&invMatRot, NULL, &m_matRot);
		invTM.m_matRot = invMatRot;
	}



	D3DXVECTOR3 temp; 
	D3DXMATRIX transpose;
	D3DXMatrixTranspose(&transpose, &invTM.m_matRot);
	D3DXVec3TransformCoord(&temp, &m_trans, &transpose);		
	invTM.SetTrans(-temp);	

	invTM.SetBit(false, TM_IDENTITY);
	invTM.SetBit(false, TM_ROTSCALE);
	invTM.SetBit(false, UNIFORM_SCALE);	
}


float U2Transform::GetNorm() const
{

	if(GetBit(TM_ROTSCALE))
	{
		float fMax = U2Math::FAbs(m_vScale.x);
		if(U2Math::FAbs(m_vScale.y) > fMax)
		{
			fMax = U2Math::FAbs(m_vScale.y);
		}
		if(U2Math::FAbs(m_vScale.z) > fMax)
		{
			fMax = U2Math::FAbs(m_vScale.z);
		}
		return fMax;
	}

	

	float fMaxRowSum = 
		U2Math::FAbs(m_matRot._11) + 
		U2Math::FAbs(m_matRot._12) + 
		U2Math::FAbs(m_matRot._13);

	float fRowSum = 
		U2Math::FAbs(m_matRot._21) +
		U2Math::FAbs(m_matRot._22) +
		U2Math::FAbs(m_matRot._23);
	if(fRowSum > fMaxRowSum)
		fMaxRowSum = fRowSum;

	fRowSum = 
		U2Math::FAbs(m_matRot._31) +
		U2Math::FAbs(m_matRot._32) +
		U2Math::FAbs(m_matRot._33);

	if(fRowSum > fMaxRowSum)
		fMaxRowSum = fRowSum;

	return fMaxRowSum;		
}


void U2Transform::GetHomogeneousMat(D3DXMATRIX& HMat) const 
{
	if(GetBit(TM_ROTSCALE))
	{
		HMat._11 = m_vScale.x * m_matRot._11;
		HMat._12 = m_vScale.x * m_matRot._21;
		HMat._13 = m_vScale.x * m_matRot._31;
		HMat._14 = 0.f;
		HMat._21 = m_vScale.y * m_matRot._12;
		HMat._22 = m_vScale.y * m_matRot._22;
		HMat._23 = m_vScale.y * m_matRot._32;
		HMat._24 = 0.f;
		HMat._31 = m_vScale.z * m_matRot._13;
		HMat._32 = m_vScale.z * m_matRot._23;
		HMat._33 = m_vScale.z * m_matRot._33;
		HMat._34 = 0.f;
	}
	else 
	{
		HMat._11 = m_matRot._11;
		HMat._12 = m_matRot._21;
		HMat._13 = m_matRot._31;
		HMat._14 = 0.f;
		HMat._21 = m_matRot._12;
		HMat._22 = m_matRot._22;
		HMat._23 = m_matRot._32;
		HMat._24 = 0.f;
		HMat._31 = m_matRot._13;
		HMat._32 = m_matRot._23;
		HMat._33 = m_matRot._33;
		HMat._34 = 0.f;
	}
	HMat._41 = m_trans.x;
	HMat._42 = m_trans.y;
	HMat._43 = m_trans.z;
	HMat._44 = 1.0f;

}