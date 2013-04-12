#pragma once 

class SHRenderer
{
public:
	enum { SH_ORDER = 5, SH_COEFFS=SH_ORDER*SH_ORDER };
	/// Simple vertex: position, normal
	struct SVertex {
		noVec3	p;
		noVec3	n;
	};
	/// SH vertex: position, normal, SH coeffs
	struct SSHVertex {
		noVec3	p;
		noVec3	n;
		float		sh[SH_COEFFS];
	};

	bool Init(ID3D11Device* dev, ID3D11DeviceContext* context);

	void Reset();

	void LoadShaders();

	//bool		mUseLocalSH = true; // true = localsh, false = normal shadow cubemap

	ID3D11Device* Dev;
	ID3D11DeviceContext* Context;
};

