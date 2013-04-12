//=================================================================================================
//
//	MJP's DX11 Sample Framework
//  http://mynameismjp.wordpress.com/
//
//  All code and content licensed under Microsoft Public License (Ms-PL)
//
//=================================================================================================

#include "stdafx.h"

#include "ShaderCompilation.h"

#include "Utility.h"
#include "Exceptions.h"
#include "InterfacePointers.h"


ID3D10Blob* CompileShader(LPCWSTR path,
                            LPCSTR functionName,
                            LPCSTR profile,
                            CONST D3D_SHADER_MACRO* defines,
                            ID3DInclude* includes)
{
    // Loop until we succeed, or an exception is thrown
    while (true)
    {

        UINT flags = 0;
        #ifdef _DEBUG
            flags |= D3D10_SHADER_DEBUG|D3D10_SHADER_SKIP_OPTIMIZATION|D3D10_SHADER_WARNINGS_ARE_ERRORS;
        #endif

        ID3D10Blob* compiledShader;
        ID3D10BlobPtr errorMessages;
        HRESULT hr = D3DX11CompileFromFileW(path, defines, includes, functionName, profile,
            flags, 0, NULL, &compiledShader, &errorMessages, NULL);

        if (FAILED(hr))
        {
            if (errorMessages)
            {
                WCHAR message[1024];
                message[0] = NULL;
                char* blobdata = reinterpret_cast<char*>(errorMessages->GetBufferPointer());

                MultiByteToWideChar(CP_ACP, 0, blobdata, static_cast<int>(errorMessages->GetBufferSize()), message, 1024);
                std::wstring fullMessage = L"Error compiling shader file \"";
                fullMessage += path;
                fullMessage += L"\" - ";
                fullMessage += message;

                #ifdef _DEBUG
                    // Pop up a message box allowing user to retry compilation
                    int retVal = MessageBoxW(NULL, fullMessage.c_str(), L"Shader Compilation Error", MB_RETRYCANCEL);
                    if(retVal != IDRETRY)
                        throw DXException(hr, fullMessage.c_str());
                #else
                    throw DXException(hr, fullMessage.c_str());
                #endif
            }
            else
            {
                _ASSERT(false);
                throw DXException(hr);
            }
        }
        else
            return compiledShader;
    }
}

ID3D11VertexShader* CompileVSFromFile(ID3D11Device* device,
                                        LPCWSTR path,
                                        LPCSTR functionName,
                                        LPCSTR profile,
                                        CONST D3D_SHADER_MACRO* defines,
                                        ID3DInclude* includes,
                                        ID3DBlob** byteCode)
{
    ID3DBlobPtr compiledShader = CompileShader(path, functionName, profile, defines, includes);
    ID3D11VertexShader* shader = NULL;
    DXCall(device->CreateVertexShader(compiledShader->GetBufferPointer(),
        compiledShader->GetBufferSize(),
        NULL,
        &shader));

    if (byteCode != NULL)
        *byteCode = compiledShader.Detach();

    return shader;
}

ID3D11PixelShader* CompilePSFromFile(ID3D11Device* device,
                                        LPCWSTR path,
                                        LPCSTR functionName,
                                        LPCSTR profile,
                                        CONST D3D_SHADER_MACRO* defines,
                                        ID3DInclude* includes)
{
    ID3DBlobPtr compiledShader = CompileShader(path, functionName, profile, defines, includes);
    ID3D11PixelShader* shader = NULL;
    DXCall(device->CreatePixelShader(compiledShader->GetBufferPointer(),
        compiledShader->GetBufferSize(),
        NULL,
        &shader));

    return shader;
}

ID3D11GeometryShader* CompileGSFromFile(ID3D11Device* device,
                                        LPCWSTR path,
                                        LPCSTR functionName,
                                        LPCSTR profile,
                                        CONST D3D_SHADER_MACRO* defines,
                                        ID3DInclude* includes)
{
    ID3DBlobPtr compiledShader = CompileShader(path, functionName, profile, defines, includes);
    ID3D11GeometryShader* shader = NULL;
    DXCall(device->CreateGeometryShader(compiledShader->GetBufferPointer(),
        compiledShader->GetBufferSize(),
        NULL,
        &shader));

    return shader;
}

ID3D11HullShader* CompileHSFromFile(ID3D11Device* device,
                                    LPCWSTR path,
                                    LPCSTR functionName,
                                    LPCSTR profile,
                                    CONST D3D_SHADER_MACRO* defines,
                                    ID3DInclude* includes)
{
    ID3DBlobPtr compiledShader = CompileShader(path, functionName, profile, defines, includes);
    ID3D11HullShader* shader = NULL;
    DXCall(device->CreateHullShader(compiledShader->GetBufferPointer(),
        compiledShader->GetBufferSize(),
        NULL,
        &shader));

    return shader;
}

ID3D11DomainShader* CompileDSFromFile(ID3D11Device* device,
                                        LPCWSTR path,
                                        LPCSTR functionName,
                                        LPCSTR profile,
                                        CONST D3D_SHADER_MACRO* defines,
                                        ID3DInclude* includes)
{
    ID3DBlobPtr compiledShader = CompileShader(path, functionName, profile, defines, includes);
    ID3D11DomainShader* shader = NULL;
    DXCall(device->CreateDomainShader(compiledShader->GetBufferPointer(),
        compiledShader->GetBufferSize(),
        NULL,
        &shader));

    return shader;
}

ID3D11ComputeShader* CompileCSFromFile(ID3D11Device* device,
                                        LPCWSTR path,
                                        LPCSTR functionName,
                                        LPCSTR profile,
                                        CONST D3D_SHADER_MACRO* defines,
                                        ID3DInclude* includes)
{
    ID3DBlobPtr compiledShader = CompileShader(path, functionName, profile, defines, includes);
    ID3D11ComputeShader* shader = NULL;
    DXCall(device->CreateComputeShader(compiledShader->GetBufferPointer(),
        compiledShader->GetBufferSize(),
        NULL,
        &shader));

    return shader;
}

// == CompileOptions ==============================================================================

CompileOptions::CompileOptions()
{
    Reset();
}

void CompileOptions::Add(const std::string& name, UINT32 value)
{
    _ASSERT(numDefines < MaxDefines);

    defines[numDefines].Name = buffer + bufferIdx;
    for(UINT32 i = 0; i < name.length(); ++i)
        buffer[bufferIdx++] = name[i];
    ++bufferIdx;

    std::string stringVal = ToAnsiString(value);
    defines[numDefines].Definition = buffer + bufferIdx;
    for(UINT32 i = 0; i < stringVal.length(); ++i)
        buffer[bufferIdx++] = stringVal[i];
    ++bufferIdx;
    
    ++numDefines;
}

void CompileOptions::Reset()
{
    numDefines = 0;
    bufferIdx = 0;

    for(UINT32 i = 0; i < MaxDefines; ++i)
    {
        defines[i].Name = NULL;
        defines[i].Definition = NULL;
    }

    ZeroMemory(buffer, BufferSize);
}

const D3D_SHADER_MACRO* CompileOptions::Defines() const
{
    return defines;
}

