//=================================================================================================
//
//  MJP's DX11 Sample Framework
//  http://mynameismjp.wordpress.com/
//
//  All code and content licensed under Microsoft Public License (Ms-PL)
//
//=================================================================================================

#pragma once

#include "Exceptions.h"

#pragma warning(push)
#pragma  warning(disable :  4238 )

#define CHECK_R(a) { HRESULT hr = (a);  if(FAILED(hr)) OutputDebugString("failed: "#a); }
#define DX_ASSERT(x)           { if( !(x) ) { DXUTTrace( __FILE__, (DWORD)__LINE__, 0, L#x, true ); } }

// Constants
const float Pi = 3.14159f;
const float PiOver2 = Pi / 2.0f;
const float PiOver4 = Pi / 4.0f;

// Throws a DXException on failing HRESULT
inline void DXCall(HRESULT hr)
{
  if (FAILED(hr))
    {
        _ASSERT(false);
        throw DXException(hr);
    }
}

// Throws a Win32Exception on failing return value
inline void Win32Call(BOOL retVal)
{
    if (retVal == 0)
  {
        _ASSERT(false);
        throw Win32Exception(GetLastError());
    }
}

// Throws a GdiPlusException on failing Status value
//inline void GdiPlusCall(Gdiplus::Status status)
//{
//    if (status != Gdiplus::Ok)
//    {
//        _ASSERT(false);
//        throw GdiPlusException(status);
//    }
//}

// Returns a size suitable for creating a constant buffer, by rounding up
// to the next multiple of 16
inline UINT CBSize(UINT size)
{
    UINT cbsize = size + (16 - (size % 16));
    return cbsize;
}

// Returns the forward vector from a transform matrix
inline XMVECTOR ForwardVec(const XMMATRIX& matrix)
{
    return XMLoadFloat3(&XMFLOAT3(matrix._31, matrix._32, matrix._33));
}

// Returns the forward vector from a transform matrix
inline XMVECTOR BackVec(const XMMATRIX& matrix)
{
    return XMLoadFloat3(&XMFLOAT3(-matrix._31, -matrix._32, -matrix._33));
}

// Returns the forward vector from a transform matrix
inline XMVECTOR RightVec(const XMMATRIX& matrix)
{
    return XMLoadFloat3(&XMFLOAT3(matrix._11, matrix._12, matrix._13));
}

// Returns the forward vector from a transform matrix
inline XMVECTOR LeftVec(const XMMATRIX& matrix)
{
    return XMLoadFloat3(&XMFLOAT3(-matrix._11, -matrix._12, -matrix._13));
}

// Returns the forward vector from a transform matrix
inline XMVECTOR UpVec(const XMMATRIX& matrix)
{
    return XMLoadFloat3(&XMFLOAT3(matrix._21, matrix._22, matrix._23));
}

// Returns the forward vector from a transform matrix
inline XMVECTOR DownVec(const XMMATRIX& matrix)
{
    return XMLoadFloat3(&XMFLOAT3(-matrix._21, -matrix._22, -matrix._23));
}

// Returns the translation vector from a transform matrix
inline XMVECTOR TranslationVec(const XMMATRIX& matrix)
{
    return XMLoadFloat3(&XMFLOAT3(matrix._41, matrix._42, matrix._43));
}

// Sets the translation vector in a transform matrix
inline void SetTranslationVec(XMMATRIX& matrix, const XMVECTOR& translation)
{
    matrix._41 = XMVectorGetX(translation);
    matrix._42 = XMVectorGetY(translation);
    matrix._43 = XMVectorGetZ(translation);
}

// Clamps a value to the specified range
inline float Clamp(float val, float min, float max)
{
    _ASSERT(max >= min);

    if (val < min)
        val = min;
    else if (val > max)
        val = max;
    return val;
}

inline XMFLOAT2 Clamp(const XMFLOAT2& val, const XMFLOAT2 min, const XMFLOAT2 max)
{
    XMFLOAT2 retVal;
    retVal.x = Clamp(val.x, min.x, max.x);
    retVal.y = Clamp(val.y, min.y, max.y);
}

inline XMFLOAT3 Clamp(const XMFLOAT3& val, const XMFLOAT3 min, const XMFLOAT3 max)
{
    XMFLOAT3 retVal;
    retVal.x = Clamp(val.x, min.x, max.x);
    retVal.y = Clamp(val.y, min.y, max.y);
    retVal.z = Clamp(val.z, min.z, max.z);
}

inline XMFLOAT4 Clamp(const XMFLOAT4& val, const XMFLOAT4 min, const XMFLOAT4 max)
{
    XMFLOAT4 retVal;
    retVal.x = Clamp(val.x, min.x, max.x);
    retVal.y = Clamp(val.y, min.y, max.y);
    retVal.z = Clamp(val.z, min.z, max.z);
    retVal.w = Clamp(val.w, min.w, max.w);
}

inline XMFLOAT3 Normalize(const XMFLOAT3& val)
{
    float len = std::sqrtf(val.x * val.x + val.y * val.y + val.z * val.z);
    XMFLOAT3 ret(val.x / len, val.y / len, val.z / len);
    return ret;
}

inline float Distance(const XMFLOAT3& a, const XMFLOAT3& b)
{
    XMVECTOR x = XMLoadFloat3(&a);
    XMVECTOR y = XMLoadFloat3(&b);
    XMVECTOR length = XMVector3Length(XMVectorSubtract(x, y));
    return XMVectorGetX(length);
}

// Converts an ANSI string to a std::wstring
inline std::wstring AnsiToWString(const char* ansiString)
{
    WCHAR buffer[512];
    Win32Call(MultiByteToWideChar(CP_ACP, 0, ansiString, -1, buffer, 512));
    return std::wstring(buffer);
}

// Returns true if a file exits
inline bool FileExists(const WCHAR* fileName)
{
    _ASSERT(fileName);
    if(fileName == NULL)
        return false;

    DWORD fileAttr;
    fileAttr = GetFileAttributesW(fileName);
    if (fileAttr == INVALID_FILE_ATTRIBUTES)
        return false;

    return true;
}

// Returns the directory containing a file
inline std::wstring GetDirectoryFromFileName(const WCHAR* fileName)
{
    _ASSERT(fileName);

    std::wstring filePath(fileName);
    size_t idx = filePath.rfind(L'\\');
    if (idx != std::wstring::npos)
        return filePath.substr(0, idx + 1);
    else
        return std::wstring(L"");
}

// Returns the given filename, minus the extension
inline std::wstring GetFileNameWithoutExtension(const WCHAR* fileName)
{
    _ASSERT(fileName);

    std::wstring filePath(fileName);
    size_t idx = filePath.rfind(L'.');
    if (idx != std::wstring::npos)
        return filePath.substr(0, idx);
    else
        return std::wstring(L"");
}


// Returns the extension of the filename
inline std::wstring GetFileExtension(const WCHAR* fileName)
{
    _ASSERT(fileName);

    std::wstring filePath(fileName);
    size_t idx = filePath.rfind(L'.');
    if (idx != std::wstring::npos)
        return filePath.substr(idx + 1, filePath.length() - idx - 1);
    else
        return std::wstring(L"");
}

// Splits up a string using a delimiter
inline void Split(const std::wstring& str, std::vector<std::wstring>& parts, const std::wstring& delimiters = L" ")
{
    // Skip delimiters at beginning
    std::wstring::size_type lastPos = str.find_first_not_of(delimiters, 0);

    // Find first "non-delimiter"
    std::wstring::size_type pos = str.find_first_of(delimiters, lastPos);

    while (std::wstring::npos != pos || std::wstring::npos != lastPos)
    {
        // Found a token, add it to the vector
        parts.push_back(str.substr(lastPos, pos - lastPos));

        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);

        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
}

// Splits up a string using a delimiter
inline std::vector<std::wstring> Split(const std::wstring& str, const std::wstring& delimiters = L" ")
{
    std::vector<std::wstring> parts;
    Split(str, parts, delimiters);
    return parts;
}

// Parses a string into a number
template<typename T> inline T Parse(const std::wstring& str)
{
    std::wistringstream stream(str);
    wchar_t c;
    T x;
    if (!(str >> x) || stream.get(c))
        throw Exception(L"Can't parse string \"" + str + L"\"");
    return x;
}

// Converts a number to a string
template<typename T> inline std::wstring ToString(const T& val)
{
    std::wostringstream stream;
    if (!(stream << val))
        throw Exception(L"Error converting value to string");
    return stream.str();
}

// Converts a number to an ansi string
template<typename T> inline std::string ToAnsiString(const T& val)
{
    std::ostringstream stream;
    if (!(stream << val))
        throw Exception(L"Error converting value to string");
    return stream.str();
}

// Outputs a string to the debugger output
inline void DebugPrint(const std::wstring& str)
{
    std::wstring output = str + L"\n";
    OutputDebugStringW(output.c_str());
}

// Returns the number of mip levels given a texture size
inline UINT NumMipLevels(UINT width, UINT height)
{
    UINT numMips = 0;
    UINT size = max(width, height);
    while (1U << numMips <= size)
        ++numMips;

    if (1U << numMips < size)
        ++numMips;

    return numMips;
}

// Sets the viewport for a given render target size
inline void SetViewport(ID3D11DeviceContext* context, UINT rtWidth, UINT rtHeight)
{
    D3D11_VIEWPORT viewport;
    viewport.Width = static_cast<float>(rtWidth);
    viewport.Height = static_cast<float>(rtHeight);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;

    context->RSSetViewports(1, &viewport);
}

// Barycentric coordinate functions
XMFLOAT3 CartesianToBarycentric(float x, float y, const XMFLOAT2& pos1, const XMFLOAT2& pos2, const XMFLOAT2& pos3);
XMFLOAT2 BarycentricToCartesian(const XMFLOAT3& r, const XMFLOAT2& pos1, const XMFLOAT2& pos2, const XMFLOAT2& pos3);
XMVECTOR BarycentricToCartesian(const XMFLOAT3& r, FXMVECTOR pos1, FXMVECTOR pos2, FXMVECTOR pos3);
BOOL PointIsInTriangle(const XMFLOAT3& r, float epsilon = 0.0f);

// Compute shader helpers
UINT32 DispatchSize(UINT32 tgSize, UINT32 numElements);
void SetCSInputs(ID3D11DeviceContext* context, ID3D11ShaderResourceView* srv0, ID3D11ShaderResourceView* srv1 = NULL,
                    ID3D11ShaderResourceView* srv2 = NULL, ID3D11ShaderResourceView* srv3 = NULL);
void ClearCSInputs(ID3D11DeviceContext* context);
void SetCSOutputs(ID3D11DeviceContext* context, ID3D11UnorderedAccessView* uav0, ID3D11UnorderedAccessView* uav1 = NULL,
                    ID3D11UnorderedAccessView* uav2 = NULL, ID3D11UnorderedAccessView* uav3 = NULL,
                    ID3D11UnorderedAccessView* uav4 = NULL, ID3D11UnorderedAccessView* uav5 = NULL);
void ClearCSOutputs(ID3D11DeviceContext* context);
void SetCSSamplers(ID3D11DeviceContext* context, ID3D11SamplerState* sampler0, ID3D11SamplerState* sampler1 = NULL,
                    ID3D11SamplerState* sampler2 = NULL, ID3D11SamplerState* sampler3 = NULL);
void SetCSShader(ID3D11DeviceContext* context, ID3D11ComputeShader* shader);
void SetCSConstants(ID3D11DeviceContext* context, ID3D11Buffer* constantBuffer, UINT32 slot);

// Serialization helpers
template<typename T> void SerializeWrite(HANDLE fileHandle, const T& val)
{
    DWORD bytesWritten = 0;
    Win32Call(WriteFile(fileHandle, &val, sizeof(T), &bytesWritten, NULL));
}

template<typename T> void SerializeWriteVector(HANDLE fileHandle, const std::vector<T>& vec)
{
    DWORD bytesWritten = 0;
    UINT32 numElements = static_cast<UINT32>(vec.size());
    UINT32 vectorSize = numElements * sizeof(T);
    Win32Call(WriteFile(fileHandle, &numElements, sizeof(UINT32), &bytesWritten, NULL));
    if(numElements > 0)
        Win32Call(WriteFile(fileHandle, &vec[0], vectorSize, &bytesWritten, NULL));
}

template<typename T> void SerializeWriteString(HANDLE fileHandle, const std::basic_string<T>& str)
{
    DWORD bytesWritten = 0;
    UINT32 numChars = static_cast<UINT32>(str.length());
    UINT32 strSize = numChars * sizeof(T);
    Win32Call(WriteFile(fileHandle, &numChars, sizeof(UINT32), &bytesWritten, NULL));
    if(numChars > 0)
        Win32Call(WriteFile(fileHandle, str.c_str(), strSize, &bytesWritten, NULL));

}

template<typename T> void SerializeRead(HANDLE fileHandle, T& val)
{
    DWORD bytesRead = 0;
    Win32Call(ReadFile(fileHandle, &val, sizeof(T), &bytesRead, NULL));
}

template<typename T> void SerializeReadVector(HANDLE fileHandle, std::vector<T>& vec)
{
    DWORD bytesRead = 0;
    UINT32 numElements = 0;
    Win32Call(ReadFile(fileHandle, &numElements, sizeof(UINT32), &bytesRead, NULL));

    vec.clear();
    if(numElements > 0)
    {
        vec.resize(numElements);
        Win32Call(ReadFile(fileHandle, &vec[0], sizeof(T) * numElements, &bytesRead, NULL));
    }
}

template<typename T> void SerializeReadString(HANDLE fileHandle, std::basic_string<T>& str)
{
    DWORD bytesRead = 0;
    UINT32 numChars = 0;
    Win32Call(ReadFile(fileHandle, &numChars, sizeof(UINT32), &bytesRead, NULL));

    str.clear();
    if(numChars > 0)
    {
        str.resize(numChars + 1, 0);
        Win32Call(ReadFile(fileHandle, &str[0], sizeof(T) * numChars, &bytesRead, NULL));
    }
}

template<typename T>
class ArrayDeleter
{
public:
    ArrayDeleter(T* ptr)
    {
        _ASSERT(ptr);
        array = ptr;
    }

    ~ArrayDeleter() { delete[] array; }

protected:
    T* array;
};


class DirectoryFilter
{
	typedef std::vector<std::string> LstFiles;
	LstFiles	m_lstFiles;

public:
	DirectoryFilter(){}
	~DirectoryFilter(){}

	bool Initialize(const std::string& strPath, const std::string& strMask, bool bFiles, bool bDirectories);

	uint32				Count() const			{ return (uint32)m_lstFiles.size();	}

	const std::string&	operator[](int i) const	{ return m_lstFiles[i];				}

	typedef LstFiles::iterator Iterator;

	Iterator		Begin()						{ return m_lstFiles.begin();		}
	Iterator		End()						{ return m_lstFiles.end();			}

};


namespace Utility
{

	void	DebugPrint(const char* format, ...);

	template <typename T>
	T int_pow(T a, unsigned int n)
	{
		T r = 1;
		for(unsigned int i = 0; i < n; ++i)
			r *= a;
		return r;
	}

	float Random(float lower = 0.0f, float upper = 1.0f);

	// Helper for automatically zeroing out structures
	// e.g. Zero<D3D11_WHATEVER_STRUCT> str;

	template <typename T>
	class Zero : T
	{
	public:
		Zero()
		{
			ZeroMemory(this, sizeof(T));
		}
	};

	std::wstring LocateFile(const wchar_t* name);

	namespace Effect
	{
		UINT NumPasses(ID3DX11EffectTechnique* technique);
	}

	// Pseudo constructors 
	inline D3D11_INPUT_ELEMENT_DESC InputElementDesc
		(
		LPCSTR SemanticName,
		UINT SemanticIndex,
		DXGI_FORMAT Format,
		UINT InputSlot,
		UINT AlignedByteOffset,
		D3D11_INPUT_CLASSIFICATION InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
		UINT InstanceDataStepRate = 0
		)
	{
		D3D11_INPUT_ELEMENT_DESC r;

		r.SemanticName = SemanticName;
		r.SemanticIndex = SemanticIndex;
		r.Format = Format;
		r.InputSlot = InputSlot;
		r.AlignedByteOffset = AlignedByteOffset;
		r.InputSlotClass = InputSlotClass;
		r.InstanceDataStepRate=InstanceDataStepRate;
		return r;
	}

	inline D3D11_TEXTURE2D_DESC Texture2DDesc
		(
		UINT Width,
		UINT Height,
		UINT MipLevels,
		UINT ArraySize,
		DXGI_FORMAT Format,
		DXGI_SAMPLE_DESC SampleDesc,
		D3D11_USAGE Usage,
		UINT BindFlags,
		UINT CPUAccessFlags,
		UINT MiscFlags
		)
	{
		D3D11_TEXTURE2D_DESC r;
		r.Width = Width;
		r.Height = Height;
		r.MipLevels = MipLevels;
		r.ArraySize = ArraySize;
		r.Format = Format;
		r.SampleDesc = SampleDesc;
		r.Usage = Usage;
		r.BindFlags = BindFlags;
		r.CPUAccessFlags = CPUAccessFlags;
		r.MiscFlags = MiscFlags;

		return r;
	}

	inline DXGI_SAMPLE_DESC SampleDesc
		(
		UINT Count,
		UINT Quality
		)
	{
		DXGI_SAMPLE_DESC r;
		r.Count = Count;
		r.Quality = Quality;
		return r;
	}
}



#pragma warning(pop)