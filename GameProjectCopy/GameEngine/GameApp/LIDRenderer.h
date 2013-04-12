#pragma once 

#include "DeviceStates.h"
#include "GraphicsTypes.h"
#include "AppSettings.h"
#include "SH.h"

class Mesh;
class CDXUTSDKMesh;
class BaseCamera;

struct MeshPSConstants
{
	Float4Align XMFLOAT3 LightDirWS;
	Float4Align XMFLOAT3 LightColor;
	Float4Align XMFLOAT3 CameraPosWS;
	Float4Align XMFLOAT3 DiffuseAlbedo;
	Float4Align XMFLOAT3 SpecularAlbedo;
	float Balance;
	float Roughness;
	UINT32 NumTilesX;
	UINT32 NumTilesY;
	UINT32 EnableNormalMapping;
};

class LIDRenderer : public Singleton<LIDRenderer>
{
public:
	void Initialize(ID3D11Device* device, ID3D11DeviceContext* context);

	void Setup(const XMFLOAT3& lightDir,
		const XMFLOAT3& lightColor, const XMMATRIX& world);


	void RenderDepth(Mesh* pMesh,  BaseCamera* pCam, ID3D11DeviceContext* context, const XMMATRIX& world);
	void RenderGBuffer(Mesh* pMesh,  BaseCamera* pCam, ID3D11DeviceContext* context, const XMMATRIX& world);
	void Render(Mesh* pMesh,  BaseCamera* pCam, ID3D11DeviceContext* context, const XMMATRIX& world, ID3D11ShaderResourceView* lightsBuffer, ID3D11ShaderResourceView* lightIndicesBuffer, UINT32 numTilesX, UINT32 numTilesY);

	void RenderDepth2(CDXUTSDKMesh* pMesh, BaseCamera* pCam,  ID3D11DeviceContext* context, const XMMATRIX& world);
	void RenderGBuffer2(CDXUTSDKMesh* pMesh, BaseCamera* pCam,  ID3D11DeviceContext* context, const XMMATRIX& world);
	void Render2(CDXUTSDKMesh* pMesh,  BaseCamera* pCam, ID3D11DeviceContext* context, const XMMATRIX& world, ID3D11ShaderResourceView* lightsBuffer, ID3D11ShaderResourceView* lightIndicesBuffer, UINT32 numTilesX, UINT32 numTilesY);
	BOOL DoFrustumTests(CDXUTSDKMesh* pMesh, BaseCamera* pCam);
	//void BeginRender( BaseCamera* pCam, ID3D11DeviceContext* context, const XMMATRIX& world, ID3D11ShaderResourceView* lightsBuffer, ID3D11ShaderResourceView* lightIndicesBuffer, UINT32 numTilesX, UINT32 numTilesY );
	void BeginRender(BaseCamera* pCam, ID3D11DeviceContext* context, const XMMATRIX& world, UINT32 numTilesX, UINT32 numTilesY );
	void BeginRender(ID3D11DeviceContext* context);
	void EndRender();

	SamplerStates& GetSamplerState()  { return samplerStates; }
	
	void BeginDepthRender( BaseCamera* pCam, ID3D11DeviceContext* context, const XMMATRIX& world, bool Skinned = false );
private:
	static const UINT NumCascades = 4;

	ID3D11DevicePtr device;


	XMFLOAT3 lightDir;

	BlendStates blendStates;
	RasterizerStates rasterizerStates;
	DepthStencilStates depthStencilStates;
	SamplerStates samplerStates;

	DepthStencilBuffer shadowMap;

	std::vector<ID3D11InputLayoutPtr> meshInputLayouts;
	ID3D10BlobPtr compiledMeshVS;
	ID3D11VertexShaderPtr meshVS[NumLightsGUI::NumValues];
	ID3D11PixelShaderPtr meshPS[NumLightsGUI::NumValues];

	std::vector<ID3D11InputLayoutPtr> meshDepthInputLayouts;
	ID3D11VertexShaderPtr meshDepthVS;
	ID3D10BlobPtr compiledMeshDepthVS;

	ID3D11VertexShaderPtr meshSkinDepthVS;
	ID3D10BlobPtr compiledMeshSkinDepthVS;

	std::vector<ID3D11InputLayoutPtr> meshGBufferInputLayouts;
	ID3D11VertexShaderPtr meshGBufferVS;
	ID3D11PixelShaderPtr meshGBufferPS;
	ID3D10BlobPtr compiledMeshGBufferVS;

	ID3D11InputLayoutPtr	meshDepthInputLayout;
	ID3D11InputLayoutPtr	meshGBufferInputLayout;
	ID3D11InputLayoutPtr	meshInputLayout;

	// Constant buffers
	struct MeshVSConstants
	{
		Float4Align XMMATRIX World;
		Float4Align XMMATRIX View;
		Float4Align XMMATRIX WorldViewProjection;
	};

	struct MeshVSSkinConstants
	{
		Float4Align XMMATRIX World;
		Float4Align XMMATRIX View;
		Float4Align XMMATRIX WorldViewProjection;
		Float4Align XMMATRIX BomeMatrices[165];
	};

	struct MaterialConstants
	{
		MaterialConstants()
			:Diffuse(0.5f, 0.5f, 0.5f,1.0f),
			Amibient(0.5f, 0.5f, 0.5f, 1.0f),
			Specular(0.0f, 0.0f, 0.0f, 1.0f),
			Emissive(0.0f, 0.0f, 0.0f, 1.0f),
			EmissiveIntensity(0.0f),
			SpecularPower(4.0f)			
		{
			
		}

		noVec4	Diffuse;
		noVec4	Amibient;
		noVec4	Specular;
		noVec4	Emissive;
		float	EmissiveIntensity;
		float	SpecularPower;		
	};

	struct SHConstants
	{
		SH9Color AmbientDiffuse;
	};

	ConstantBuffer<MeshVSConstants> meshVSConstants;
	ConstantBuffer<MeshPSConstants> meshPSConstants;
	ConstantBuffer<SHConstants> shConstants;
	ConstantBuffer<MeshVSSkinConstants>	meshVSSkinConstants;
	ConstantBuffer<MaterialConstants>	matConstants;

public:
	ID3D11Buffer* GetD3DPSBuffer() { return meshPSConstants.Buffer(); }
	ConstantBuffer<MeshPSConstants>*	GetPSConstBuffer()  { return &meshPSConstants; }
	void BeginDepthSkinRender( BaseCamera* pCam, ID3D11DeviceContext* context, const XMMATRIX& world, noMat4* boneMatrices, int nBone /*= false */ );	
	void UpdateMaterial(ID3D11DeviceContext* context,  const noVec4 e, const noVec4 o = noVec4(1.0f, 1.0f, 1.0f, 1.0f) , const noVec3& diffuse = noVec3(0.5f, 0.5f, 0.5f), int nSlot = 2);
};

extern CDXUTSDKMesh gSample;