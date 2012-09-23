#ifndef MATH_QTRANSFORM_H
#define MATH_QTRANSFORM_H

#include <Math/Vector.h>
#include <Math/Quaternion.h>

class noQTransform {
public:
	//HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_MATH, noQTransform);

	enum 	{
		IDENTITY,
		ZERO,
	};

	NO_INLINE noQTransform() {}
	NO_INLINE noQTransform(int init);
	NO_INLINE noQTransform(const noVec4& trans, const noQuat& rot, const noVec4& scl);
	NO_INLINE noQTransform(const noVec4& trans, const noQuat& rot);
	NO_INLINE noQTransform(const noQTransform& other);
	NO_INLINE void	Set(const noVec4& trans, const noQuat& rot, const noVec4& scl);
	NO_INLINE void  Identity();
	NO_INLINE static const noQTransform& __cdecl GetIdentity();
	NO_INLINE const noVec4&	GetTrans() const;
	NO_INLINE void	SetTrans(const noVec4& t) const;
	NO_INLINE const noVec4&	GetScale() const;
	NO_INLINE void	SetScale(const noVec4& t) const;
	NO_INLINE void	SetRotation(const noQuat& rot);
	//NO_INLINE void SetRotation(const )
	bool	IsOk() const;

public:
	noVec4 m_trans;
	noQuat m_rot;
	noVec4 m_scale;
	
};

#endif