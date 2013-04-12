//=================================================================================================
//
//  MJP's DX11 Sample Framework
//  http://mynameismjp.wordpress.com/
//
//  All code and content licensed under Microsoft Public License (Ms-PL)
//
//=================================================================================================

#include "stdafx.h"

#include "Utility.h"
#include "Dx11Util.h"
#include "Exceptions.h"
#include "InterfacePointers.h"

using namespace std;


// Converts from cartesian to barycentric coordinates
XMFLOAT3 CartesianToBarycentric(float x, float y, const XMFLOAT2& pos1, const XMFLOAT2& pos2, const XMFLOAT2& pos3)
{
    float r1 = (pos2.y - pos3.y) * (x - pos3.x) + (pos3.x - pos2.x) * (y - pos3.y);
    r1 /= (pos2.y - pos3.y) * (pos1.x - pos3.x) + (pos3.x - pos2.x) * (pos1.y - pos3.y);

    float r2 = (pos3.y - pos1.y) * (x - pos3.x) + (pos1.x - pos3.x) * (y - pos3.y);
    r2 /= (pos3.y - pos1.y) * (pos2.x - pos3.x) + (pos1.x - pos3.x) * (pos2.y - pos3.y);

    float r3 = 1.0f - r1 - r2;

    return XMFLOAT3(r1, r2, r3);
}

// Converts from barycentric to cartesian coordinates
XMFLOAT2 BarycentricToCartesian(const XMFLOAT3& r, const XMFLOAT2& pos1, const XMFLOAT2& pos2, const XMFLOAT2& pos3)
{
    float x = r.x * pos1.x + r.y * pos2.x + r.z * pos3.x;
    float y = r.x * pos1.y + r.y * pos2.y + r.z * pos3.y;

    return XMFLOAT2(x, y);
}

// Converts from barycentric to cartesian coordinates
XMVECTOR BarycentricToCartesian(const XMFLOAT3& r, FXMVECTOR pos1, FXMVECTOR pos2, FXMVECTOR pos3)
{
    XMVECTOR rvec;
    rvec = XMVectorScale(pos1, r.x);
    rvec += XMVectorScale(pos2, r.y);
    rvec += XMVectorScale(pos3, r.z);

    return rvec;
}

// Returns true if the given barycentric coordinate is in the triangle
BOOL PointIsInTriangle(const XMFLOAT3& r, float epsilon)
{
    float minr = 0.0f - epsilon;
    float maxr = 1.0f + epsilon;

    if(r.x < minr || r.x > maxr)
        return false;

    if(r.y < minr || r.y > maxr)
        return false;

    if(r.z < minr || r.z > maxr)
        return false;

    float rsum = r.x + r.y + r.z;
    return rsum >= minr && rsum <= maxr;
}

// Computes a compute shader dispatch size given a thread group size, and number of elements to process
UINT32 DispatchSize(UINT32 tgSize, UINT32 numElements)
{
    UINT32 dispatchSize = numElements / tgSize;
    dispatchSize += numElements % tgSize > 0 ? 1 : 0;
    return dispatchSize;
}

void SetCSInputs(ID3D11DeviceContext* context, ID3D11ShaderResourceView* srv0, ID3D11ShaderResourceView* srv1,
                    ID3D11ShaderResourceView* srv2, ID3D11ShaderResourceView* srv3)
{
    ID3D11ShaderResourceView* srvs[4] = { srv0, srv1, srv2, srv3 };
    context->CSSetShaderResources(0, 4, srvs);
}

void ClearCSInputs(ID3D11DeviceContext* context)
{
    SetCSInputs(context, NULL, NULL, NULL, NULL);
}

void SetCSOutputs(ID3D11DeviceContext* context, ID3D11UnorderedAccessView* uav0, ID3D11UnorderedAccessView* uav1,
                    ID3D11UnorderedAccessView* uav2, ID3D11UnorderedAccessView* uav3, ID3D11UnorderedAccessView* uav4,
                    ID3D11UnorderedAccessView* uav5)
{
    ID3D11UnorderedAccessView* uavs[6] = { uav0, uav1, uav2, uav3, uav4, uav5 };
    context->CSSetUnorderedAccessViews(0, 6, uavs, NULL);
}

void ClearCSOutputs(ID3D11DeviceContext* context)
{
    SetCSOutputs(context, NULL, NULL, NULL, NULL);
}

void SetCSSamplers(ID3D11DeviceContext* context, ID3D11SamplerState* sampler0, ID3D11SamplerState* sampler1,
                    ID3D11SamplerState* sampler2, ID3D11SamplerState* sampler3)
{
    ID3D11SamplerState* samplers[4] = { sampler0, sampler1, sampler2, sampler3 };
    context->CSSetSamplers(0, 4, samplers);
}

void SetCSShader(ID3D11DeviceContext* context, ID3D11ComputeShader* shader)
{
    context->CSSetShader(shader, NULL, 0);
}

void SetCSConstants(ID3D11DeviceContext* context, ID3D11Buffer* constantBuffer, UINT32 slot)
{
    ID3D11Buffer* constants[1] = { constantBuffer };
    context->CSSetConstantBuffers(slot, 1, constants);
}


bool DirectoryFilter::Initialize(const string& strPath, const string& strMask, bool bFiles, bool bDirectories)
{
	WIN32_FIND_DATAA findData;
	string strFind = strPath + strMask;

	HANDLE hFile = FindFirstFileA(strFind.c_str(), &findData);
	if(hFile == INVALID_HANDLE_VALUE)
		return false;
	BOOL bFoundOne = TRUE;
	while(bFoundOne)
	{
		bool bIsDir = !!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
		bool bAdd	= false;

		if(bIsDir) 
		{
			bAdd = bDirectories;
			if(bDirectories)
			{
				if(findData.cFileName[0] == '.')
					bAdd = false;
			}
		}
		else
			bAdd = bFiles;

		if(bAdd)
			m_lstFiles.push_back(findData.cFileName);
		bFoundOne = FindNextFileA(hFile, &findData);
	}
	return true;
}

#pragma warning (disable:4996)
namespace Utility
{
	using namespace std;

	void	DebugPrint(const char* format, ...)
	{
		va_list args;
		size_t     len;
		char    *buffer;

		// retrieve the variable arguments
		va_start( args, format );

		len = _vscprintf( format, args ) // _vscprintf doesn't count
			+ 1; // terminating '\0'

		buffer = (char*)malloc( len * sizeof(char) );

		vsnprintf( buffer,len, format, args ); // C4996
		// Note: vsprintf is deprecated; consider using vsprintf_s instead
		OutputDebugStringA(buffer);
		free( buffer );
	}

	float Random(float lower, float upper)
	{
		return lower  + (upper - lower) * std::rand() / RAND_MAX;
	}

	std::wstring LocateFile(const wchar_t* name)
	{
		WCHAR str[MAX_PATH];
		if( SUCCEEDED( FindMediaFileCch( str, 
			MAX_PATH,
			name
			)
			)
			)
			return str;
		else
			return L"";
	}



	namespace Effect
	{
		UINT NumPasses(ID3DX11EffectTechnique* technique)
		{
			D3DX11_TECHNIQUE_DESC td;
			technique->GetDesc(&td);
			return td.Passes;
		}
	}
}