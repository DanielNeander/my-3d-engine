#include "stdafx.h"
#include "GameApp/GameApp.h"
#include <GameApp/Util/EffectUtil.h>

#include <ModelLib/Model_M2.h>
#include <ModelLib/M2Particle.h>
#include <ModelLib/M2Loader.h>
#include "M2EffectRender.h"




struct ParticleRenderData {
	ID3DX11EffectShaderResourceVariable* gptxDiffuse;
	ID3D11InputLayout*                  gpVertexLayout;
	ID3D11Buffer*                       gpParticleBuffer;
	ID3D11ShaderResourceView*           gpParticleTextureSRV;
	ID3DX11Effect*                       gpEffect10;
	ID3DX11EffectTechnique*              gpRenderParticles;
	ID3DX11EffectMatrixVariable*         gpmWorldViewProjection;
	ID3DX11EffectMatrixVariable*         gpmWorld;
	ID3DX11EffectMatrixVariable*         gpmInvViewProj;
	ID3DX11EffectScalarVariable*         gpfTime;
	ID3DX11EffectVectorVariable*         gpvEyePt;
	ID3DX11EffectVectorVariable*         gpvRight;
	ID3DX11EffectVectorVariable*         gpvUp;
	ID3DX11EffectVectorVariable*         gpvForward;
	int									gNumActiveParticles;
};

ParticleRenderData RibbonRD;

static void CopyParticlesToVertexBuffer( RibbonVertex* pVB )
{
	RibbonRD.gNumActiveParticles = 0;


	M2ParticleSystem* ps = GetApp()->loader->particleSystems;
		
	noVec3 vRight(0,0,-1);
	noVec3 vUp(0,1,0);

	UINT iVBIndex = 0;
		
	int num = GetApp()->loader->header.nRibbonEmitters;					
		

	M2RibbonEmitter* Ribbons = GetApp()->loader->ribbons;
	for (size_t i=0; i< num ; i++) 
	{
		std::list<M2RibbonSegment>::iterator it = Ribbons[i].segs.begin();
		float l = 0;
		for (; it != Ribbons[i].segs.end(); ++it) {
			float u = l/Ribbons[i].length;

			pVB[iVBIndex].pos = it->pos + Ribbons[i].tabove * it->up;
			pVB[iVBIndex].uv = noVec2(u, 0);
			pVB[iVBIndex].color = Ribbons[i].tcolor;
			
			pVB[++iVBIndex].pos = it->pos - Ribbons[i].tbelow * it->up;
			pVB[iVBIndex].uv = noVec2(u,1);
			pVB[iVBIndex].color = Ribbons[i].tcolor;

			++iVBIndex;
			RibbonRD.gNumActiveParticles += 2;
			l += it->len;
		}			

		if (Ribbons[i].segs.size() > 1) {
			// last segment...?
			--it;
			pVB[iVBIndex].pos = it->pos + Ribbons[i].tabove * it->up + (it->len/it->len0) * it->back;
			pVB[iVBIndex].uv = noVec2(1, 0);
			pVB[iVBIndex].color = Ribbons[i].tcolor;

			pVB[++iVBIndex].pos = it->pos - Ribbons[i].tbelow * it->up + (it->len/it->len0) * it->back;
			pVB[iVBIndex].uv = noVec2(1,1);
			pVB[iVBIndex].color = Ribbons[i].tcolor;
			
			RibbonRD.gNumActiveParticles += 2;
		}
	}	
		
}

static void CreateShaders()
{
	M2ParticleSystem*  ps = GetApp()->loader->particleSystems;

	/*D3D11_SAMPLER_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.Filter		  = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sd.AddressU		  = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.AddressV		  = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.AddressW		  = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.MipLODBias	  = 0.0f;
	sd.MaxAnisotropy  = 1;
	sd.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	sd.BorderColor[0] = 0.0f;
	sd.BorderColor[1] = 0.0f;
	sd.BorderColor[2] = 0.0f;
	sd.BorderColor[3] = 0.0f;
	sd.MinLOD		  = 0.0f;
	sd.MaxLOD		  = D3D11_FLOAT32_MAX;

	ID3D11SamplerState* pSampleState;
	GetApp()->GetDevice()->CreateSamplerState(&sd, &pSampleState);*/
	std::string texname = GetApp()->loader->GetTextureName(ps->texture);
	size_t len = strlen(texname.c_str());
	texname[len - 3] = 'd';
	texname[len - 2] = 'd';
	texname[len - 1] = 's';

	D3DX11CreateShaderResourceViewFromFile( GetApp()->GetDevice(), texname.c_str(), NULL, NULL, 
		&RibbonRD.gpParticleTextureSRV, NULL );

	LoadEffectFromFile(GetApp()->GetDevice(), "Data/Shaders/Particle.fx", &RibbonRD.gpEffect10);
	RibbonRD.gpRenderParticles = RibbonRD.gpEffect10->GetTechniqueByName( "RenderParticles" );

	
	RibbonRD.gptxDiffuse = RibbonRD.gpEffect10->GetVariableByName( "g_txMeshTexture" )->AsShaderResource();
	RibbonRD.gpmWorld = RibbonRD.gpEffect10->GetVariableByName( "g_mWorld" )->AsMatrix();

	//gpfTime = gpEffect10->GetVariableByName( "g_fTime" )->AsScalar();
	RibbonRD.gpvEyePt = RibbonRD.gpEffect10->GetVariableByName( "g_vEyePt" )->AsVector();
	//gpvRight = gpEffect10->GetVariableByName( "g_vRight" )->AsVector();
	//gpvUp = gpEffect10->GetVariableByName( "g_vUp" )->AsVector();
	//gpvForward = gpEffect10->GetVariableByName( "g_vForward" )->AsVector();
	//gpmViewProj = gpEffect10->GetVariableByName( "gmViewProj" )->AsMatrix();

	RibbonRD.gpmWorldViewProjection = RibbonRD.gpEffect10->GetVariableByName( "g_mWorldViewProjection" )->AsMatrix();	
}

static void CreateMeshes()
{
	const D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		//{ "LIFE",      0, DXGI_FORMAT_R32_FLOAT,       0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		//{ "THETA",     0, DXGI_FORMAT_R32_FLOAT,       0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",     0, DXGI_FORMAT_R8G8B8A8_UNORM,  0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	ID3D11Device* device = GetApp()->GetDevice();
	D3DX11_PASS_DESC PassDesc;
	RibbonRD.gpRenderParticles->GetPassByIndex( 0 )->GetDesc( &PassDesc ) ;
	device->CreateInputLayout( layout, 3, PassDesc.pIAInputSignature,
		PassDesc.IAInputSignatureSize, &RibbonRD.gpVertexLayout ) ;
		
	D3D11_BUFFER_DESC BDesc;
	BDesc.ByteWidth = sizeof( RibbonVertex ) * 4 * MAX_PARTICLES;
	BDesc.Usage = D3D11_USAGE_DYNAMIC;
	BDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	BDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	BDesc.MiscFlags = 0;
	device->CreateBuffer( &BDesc, NULL, &RibbonRD.gpParticleBuffer );	
}

void App::LoadM2Ribbons()
{
	CreateShaders();
	CreateMeshes();	
}


void App::UpdateRibbonEmitters()
{
	if (!GetApp()->loader->ribbons) return;

	RibbonVertex* pVerts = NULL;

	ID3D11Resource* pRes = RibbonRD.gpParticleBuffer;
	D3D11_MAPPED_SUBRESOURCE Data;

	HRESULT hr = GetApp()->GetContext()->Map( pRes, 0, D3D11_MAP_WRITE_DISCARD, 0, &Data );	

	if (FAILED(hr))
		assert(false);

	pVerts = (RibbonVertex*)Data.pData;
	CopyParticlesToVertexBuffer( pVerts );

	GetApp()->GetContext()->Unmap(pRes, 0);
}

void App::RenderRibbions(float fTime, const mat4& world, const mat4& view, const mat4& proj )
{
	if (RibbonRD.gNumActiveParticles == 0) return;

	//renderer->changeToMainFramebuffer();
	//loader->reset();

	ID3D11RenderTargetView* pRTV;
	ID3D11DepthStencilView* pDSV;

	context->OMGetRenderTargets(1, &pRTV, &pDSV);

	ID3D11DeviceContext* pd3dContext = GetApp()->GetContext();

	float ClearColor[4] =
	{
		0.0f, 0.0f, 0.0f, 1.0f
	};
	//pd3dContext->ClearRenderTargetView( pRTV, ClearColor);
	//pd3dContext->ClearDepthStencilView( pDSV, D3D11_CLEAR_DEPTH, 1.0, 0);

	D3DXVECTOR3 vEyePt;
	D3DXMATRIX mWorldViewProjection;	
	D3DXMATRIX mWorld;
	D3DXMATRIX mView;
	D3DXMATRIX mProj;	
	DefCam_->getFrom(vEyePt);
	const unsigned int size16 = sizeof(float) * 16;


	memcpy(&mView, &view, size16);	 
	memcpy(&mProj, &proj, size16);
	memcpy(&mWorld, &world, size16);	

	RibbonRD.gpmWorld->SetMatrix( (float*)mWorld);	
	mWorldViewProjection = mWorld * mView * mProj;


	RibbonRD.gpmWorldViewProjection->SetMatrix( ( float* )&mWorldViewProjection );

	context->IASetInputLayout( RibbonRD.gpVertexLayout );
	UINT Strides[1];
	UINT Offsets[1];
	ID3D11Buffer* pVB[1];
	pVB[0] = RibbonRD.gpParticleBuffer;
	Strides[0] = sizeof( RibbonVertex );
	Offsets[0] = 0;
	context->IASetVertexBuffers( 0, 1, pVB, Strides, Offsets );
	context->IASetIndexBuffer( NULL, DXGI_FORMAT_R16_UINT, 0 );
	context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

	HRESULT hr = RibbonRD.gptxDiffuse->SetResource( RibbonRD.gpParticleTextureSRV );

	//Render
	D3DX11_TECHNIQUE_DESC techDesc;
	RibbonRD.gpRenderParticles->GetDesc( &techDesc );

	for( UINT p = 0; p < techDesc.Passes; ++p )
	{
		RibbonRD.gpRenderParticles->GetPassByIndex( p )->Apply( 0, context );
		context->Draw( RibbonRD.gNumActiveParticles, 0 );
	}	
}