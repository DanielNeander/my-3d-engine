//=================================================================================================
//
//	MJP's DX11 Sample Framework
//  http://mynameismjp.wordpress.com/
//
//  All code and content licensed under Microsoft Public License (Ms-PL)
//
//=================================================================================================

#pragma once


// Compiles a shader from file and returns the compiled bytecode
ID3D10Blob* CompileShader(LPCWSTR path,
                            LPCSTR functionName,
                            LPCSTR profile,
                            CONST D3D_SHADER_MACRO* defines = NULL,
                            ID3DInclude* includes = NULL);

// Compiles a shader from file and creates the appropriate shader instance
ID3D11VertexShader* CompileVSFromFile(ID3D11Device* device,
                                        LPCWSTR path,
                                        LPCSTR functionName = "VS",
                                        LPCSTR profile = "vs_5_0",
                                        CONST D3D_SHADER_MACRO* defines = NULL,
                                        ID3DInclude* includes = NULL,
                                        ID3DBlob** byteCode = NULL);

ID3D11PixelShader* CompilePSFromFile(ID3D11Device* device,
                                        LPCWSTR path,
                                        LPCSTR functionName = "PS",
                                        LPCSTR profile = "ps_5_0",
                                        CONST D3D_SHADER_MACRO* defines = NULL,
                                        ID3DInclude* includes = NULL);

ID3D11GeometryShader* CompileGSFromFile(ID3D11Device* device,
                                        LPCWSTR path,
                                        LPCSTR functionName = "GS",
                                        LPCSTR profile = "gs_5_0",
                                        CONST D3D_SHADER_MACRO* defines = NULL,
                                        ID3DInclude* includes = NULL);

ID3D11HullShader* CompileHSFromFile(ID3D11Device* device,
                                    LPCWSTR path,
                                    LPCSTR functionName = "HS",
                                    LPCSTR profile = "hs_5_0",
                                    CONST D3D_SHADER_MACRO* defines = NULL,
                                    ID3DInclude* includes = NULL);

ID3D11DomainShader* CompileDSFromFile(ID3D11Device* device,
                                        LPCWSTR path,
                                        LPCSTR functionName = "DS",
                                        LPCSTR profile = "ds_5_0",
                                        CONST D3D_SHADER_MACRO* defines = NULL,
                                        ID3DInclude* includes = NULL);

ID3D11ComputeShader* CompileCSFromFile(ID3D11Device* device,
                                        LPCWSTR path,
                                        LPCSTR functionName = "CS",
                                        LPCSTR profile = "cs_5_0",
                                        CONST D3D_SHADER_MACRO* defines = NULL,
                                        ID3DInclude* includes = NULL);


class CompileOptions
{
public:

    // constants
    static const UINT32 MaxDefines = 16;
    static const UINT32 BufferSize = 1024;

    CompileOptions();

    void Add(const std::string& name, UINT32 value);
    void Reset();

    const D3D10_SHADER_MACRO* Defines() const;

protected:

    D3D_SHADER_MACRO defines[MaxDefines + 1];
    CHAR buffer[BufferSize];
    UINT32 numDefines;
    UINT32 bufferIdx;
};



