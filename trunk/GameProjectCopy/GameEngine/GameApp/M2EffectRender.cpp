#include "stdafx.h"
#include <EngineCore/Util/GameLog.h>
//#undef MAX_PARTICLES 

//#define MAX_PARTICLES  000
#include "GameApp/GameApp.h"
#include <GameApp/Util/EffectUtil.h>

#include "ModelLib/M2Loader.h"
#include "ModelLib/M2Particle.h"
#include "M2Object.h"
#include "M2EffectRender.h"
#include "DeviceStates.h"

M2EffectRender::M2EffectRender( M2Loader* pM2 )
	:Owner_(pM2),ExistRibbon_(false), ExistParticle_(false)
{

}

void M2EffectRender::UpdateEffects()
{
	if (ExistParticle_)
		UpdateParicles();
	if (ExistRibbon_)
		UpdateRibbons();
}

void M2EffectRender::Render()
{
	if (ExistParticle_) {
		RenderParticles();

		ID3DX11EffectBlendVariable* pBlendVar = Effect_->GetVariableByName("SrcColorBlendingAdd")->AsBlend();
		pBlendVar->AsBlend()->SetBlendState(0, GetApp()->blendStates_->BlendDisabled());		
		ParticleData_.gpRenderParticles->GetPassByIndex(0)->Apply(0, D3D11Context());
	}
	
	if (ExistRibbon_) {
		RenderRibbons();	

		ID3DX11EffectBlendVariable* pBlendVar = Effect_->GetVariableByName("SrcColorBlendingAdd")->AsBlend();
		pBlendVar->AsBlend()->UndoSetBlendState(0);
		ID3DX11EffectDepthStencilVariable* pDepthStencilVar = Effect_->GetVariableByName("EnableDepth")->AsDepthStencil();
		pDepthStencilVar->AsDepthStencil()->UndoSetDepthStencilState(0);
		RibbonData_.gpRenderParticles->GetPassByIndex(0)->Apply(0, D3D11Context());
	}



	
	

	
	//pBlendVar->SetBlendState(0, GetApp()->blendStates_->BlendDisabled());*/	
}

void M2EffectRender::CreateResources()
{
	ExistParticle_ = Owner_->particleSystems != NULL;
	ExistRibbon_  = Owner_->ribbons != NULL;
		
	LoadShaders();	

	if (ExistParticle_)
		CreateParticles();

	if (ExistRibbon_)
		CreateRibbons();		
}

void M2EffectRender::LoadShaders()
{
	if (ExistParticle_)
	{

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

		M2ParticleSystem*  ps;
		ID3D11ShaderResourceView* particleMapSRV;
		for (size_t i =0; i < Owner_->header.nParticleEmitters; ++i)
		{
			ps =&(Owner_->particleSystems[i]);

			std::string texname = Owner_->GetTextureName(ps->texture);
			size_t len = strlen(texname.c_str());
			texname[len - 3] = 'd';
			texname[len - 2] = 'd';
			texname[len - 1] = 's';

			if (!wxFile::Exists(texname.c_str()))
				MessageBox(NULL, texname.c_str(), "Texture", 0);
		
			D3DX11CreateShaderResourceViewFromFile( GetApp()->GetDevice(), texname.c_str(), NULL, NULL, 
				&particleMapSRV, NULL );

			ParticleData_.ParticleTextureSRVMaps.insert(std::make_pair(ps->texture, particleMapSRV));
		}

		ParticleData_.gNumActiveParticles.resize(Owner_->header.nParticleEmitters);

		LoadEffectFromFile(GetApp()->GetDevice(), "Data/Shaders/Particle.fx", &Effect_);
		ParticleData_.gpEffect10 = Effect_; 	
		ParticleData_.gpRenderParticles = ParticleData_.gpEffect10->GetTechniqueByName( "RenderParticles" );
	
		ParticleData_.gptxDiffuse = ParticleData_.gpEffect10->GetVariableByName( "g_txMeshTexture" )->AsShaderResource();
		ParticleData_.gpmWorld = ParticleData_.gpEffect10->GetVariableByName( "g_mWorld" )->AsMatrix();

		
		//gpfTime = gpEffect10->GetVariableByName( "g_fTime" )->AsScalar();
		ParticleData_.gpvEyePt = ParticleData_.gpEffect10->GetVariableByName( "g_vEyePt" )->AsVector();
		//gpvRight = gpEffect10->GetVariableByName( "g_vRight" )->AsVector();
		//gpvUp = gpEffect10->GetVariableByName( "g_vUp" )->AsVector();
		//gpvForward = gpEffect10->GetVariableByName( "g_vForward" )->AsVector();
		//gpmViewProj = gpEffect10->GetVariableByName( "gmViewProj" )->AsMatrix();

		ParticleData_.gpmWorldViewProjection = ParticleData_.gpEffect10->GetVariableByName( "g_mWorldViewProjection" )->AsMatrix();	
	}

	if (ExistRibbon_)
	{
		

		M2RibbonEmitter* ribbon;
		ID3D11ShaderResourceView* particleMapSRV;
		for (size_t i =0; i < Owner_->header.nRibbonEmitters; ++i)
		{
			ribbon = &(Owner_->ribbons[i]);

			std::string texname = Owner_->GetTextureName(ribbon->texture);
			size_t len = strlen(texname.c_str());
			texname[len - 3] = 'd';
			texname[len - 2] = 'd';
			texname[len - 1] = 's';

			if (!wxFile::Exists(texname.c_str()))
				MessageBox(NULL, texname.c_str(), "Texture", 0);

			D3DX11CreateShaderResourceViewFromFile( GetApp()->GetDevice(), texname.c_str(), NULL, NULL, 
				&particleMapSRV, NULL );

			RibbonData_.ParticleTextureSRVMaps.insert(std::make_pair(ribbon->texture, particleMapSRV));
		}

		RibbonData_.gNumActiveParticles.resize(Owner_->header.nRibbonEmitters);
				
		//LoadEffectFromFile(GetApp()->GetDevice(), "Data/Shaders/Particle.fx", &RibbonData_.gpEffect10);
		RibbonData_.gpEffect10 = Effect_;		
		RibbonData_.gpRenderParticles = RibbonData_.gpEffect10->GetTechniqueByName( "RenderParticles" );

		RibbonData_.gptxDiffuse = RibbonData_.gpEffect10->GetVariableByName( "g_txMeshTexture" )->AsShaderResource();
		RibbonData_.gpmWorld = RibbonData_.gpEffect10->GetVariableByName( "g_mWorld" )->AsMatrix();

		//gpfTime = gpEffect10->GetVariableByName( "g_fTime" )->AsScalar();
		RibbonData_.gpvEyePt = RibbonData_.gpEffect10->GetVariableByName( "g_vEyePt" )->AsVector();
		//gpvRight = gpEffect10->GetVariableByName( "g_vRight" )->AsVector();
		//gpvUp = gpEffect10->GetVariableByName( "g_vUp" )->AsVector();
		//gpvForward = gpEffect10->GetVariableByName( "g_vForward" )->AsVector();
		//gpmViewProj = gpEffect10->GetVariableByName( "gmViewProj" )->AsMatrix();

		RibbonData_.gpmWorldViewProjection = RibbonData_.gpEffect10->GetVariableByName( "g_mWorldViewProjection" )->AsMatrix();	
	}
}


void M2EffectRender::CreateParticles()
{

	const D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		//{ "LIFE",      0, DXGI_FORMAT_R32_FLOAT,       0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		//{ "THETA",     0, DXGI_FORMAT_R32_FLOAT,       0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",     0, DXGI_FORMAT_R32G32B32A32_FLOAT,  0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	ID3D11Device* device = GetApp()->GetDevice();
	D3DX11_PASS_DESC PassDesc;
	ParticleData_.gpRenderParticles->GetPassByIndex( 0 )->GetDesc( &PassDesc ) ;
	device->CreateInputLayout( layout, 3, PassDesc.pIAInputSignature,
		PassDesc.IAInputSignatureSize, &ParticleData_.gpVertexLayout ) ;


	ID3D11Buffer* pBuffer;
	ParticleData_.ParticleBuffers.resize(Owner_->header.nParticleEmitters);
	for (size_t i =0; i < Owner_->header.nParticleEmitters; ++i)
	{
		D3D11_BUFFER_DESC BDesc;
		BDesc.ByteWidth = sizeof( ParticleVertex ) * 6 * MAX_PARTICLES;
		BDesc.Usage = D3D11_USAGE_DYNAMIC;
		BDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		BDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		BDesc.MiscFlags = 0;
		device->CreateBuffer( &BDesc, NULL, &pBuffer );	
		ParticleData_.ParticleBuffers.at(i) = pBuffer;
	}
}

void M2EffectRender::CreateRibbons()
{
	const D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		//{ "LIFE",      0, DXGI_FORMAT_R32_FLOAT,       0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		//{ "THETA",     0, DXGI_FORMAT_R32_FLOAT,       0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",     0, DXGI_FORMAT_R32G32B32A32_FLOAT,  0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	ID3D11Device* device = GetApp()->GetDevice();
	D3DX11_PASS_DESC PassDesc;
	RibbonData_.gpRenderParticles->GetPassByIndex( 0 )->GetDesc( &PassDesc ) ;
	device->CreateInputLayout( layout, 3, PassDesc.pIAInputSignature,
		PassDesc.IAInputSignatureSize, &RibbonData_.gpVertexLayout ) ;

	ID3D11Buffer* pBuffer;
	RibbonData_.ParticleBuffers.resize(Owner_->header.nRibbonEmitters);
	for (size_t i =0; i < Owner_->header.nRibbonEmitters; ++i)
	{
		D3D11_BUFFER_DESC BDesc;
		BDesc.ByteWidth = sizeof( RibbonVertex ) * 4 * MAX_PARTICLES;
		BDesc.Usage = D3D11_USAGE_DYNAMIC;
		BDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		BDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		BDesc.MiscFlags = 0;
		device->CreateBuffer( &BDesc, NULL, &pBuffer );	
		RibbonData_.ParticleBuffers.at(i) = pBuffer;
	}
}

void M2EffectRender::UpdateParicles()
{

	if (!ParticleData_.gpEffect10) return;

	//if (fElapsedTime > 0.1f) fElapsedTime = 0.1f;

	D3DXVECTOR3 vEye;
	GetCamera()->getFrom(vEye);
	D3DXMATRIX mView;
	memcpy(&mView, GetCamera()->getViewMatrix(), SIZE16);	
		
	D3DXVECTOR3 vRight( mView._13, mView._23, mView._33 );
	D3DXVECTOR3 vUp( mView._12, mView._22, mView._32 );
	D3DXVECTOR3 vFoward( mView._11, mView._21, mView._31 );

	

	D3DXVec3Normalize( &vRight, &vRight );
	D3DXVec3Normalize( &vUp, &vUp );
	D3DXVec3Normalize( &vFoward, &vFoward );

	//gpvRight->SetFloatVector( ( float* )&vRight );
	//gpvUp->SetFloatVector( ( float* )&vUp );
	//gpvForward->SetFloatVector( ( float* )&vFoward );
	ParticleData_.gpvEyePt->SetFloatVector( ( float* )&vEye );

	ParticleVertex* pVerts = NULL;

	M2ParticleSystem* ps ;
	for (size_t i=0; i < Owner_->header.nParticleEmitters; ++i)
	{
		ps = &Owner_->particleSystems[i];
		ID3D11Resource* pRes = ParticleData_.ParticleBuffers.at(i);
		D3D11_MAPPED_SUBRESOURCE Data;

		HRESULT hr = GetApp()->GetContext()->Map( pRes, 0, D3D11_MAP_WRITE_DISCARD, 0, &Data );	

		if (FAILED(hr))
			assert(false);

		pVerts = (ParticleVertex*)Data.pData;	


		CopyParticlesToVertexBuffer(i, ps, pVerts, vEye, -vFoward, vUp );

		GetApp()->GetContext()->Unmap(pRes, 0);
	}

}

void M2EffectRender::UpdateRibbons()
{
	RibbonVertex* pVerts = NULL;

	M2RibbonEmitter* ribbon;

	for (size_t i=0; i < Owner_->header.nRibbonEmitters; ++i)
	{
		ribbon = &Owner_->ribbons[i];		

		ID3D11Resource* pRes = RibbonData_.ParticleBuffers.at(i);
		D3D11_MAPPED_SUBRESOURCE Data;

		HRESULT hr = GetApp()->GetContext()->Map( pRes, 0, D3D11_MAP_WRITE_DISCARD, 0, &Data );	

		if (FAILED(hr))
			assert(false);

		pVerts = (RibbonVertex*)Data.pData;
		CopyRibbonsToVertexBuffer(i, *ribbon, pVerts );

		GetApp()->GetContext()->Unmap(pRes, 0);
	}
}

void M2EffectRender::RenderParticles()
{	

	//renderer->changeToMainFramebuffer();
	//loader->reset();

	ID3D11RenderTargetView* pRTV;
	ID3D11DepthStencilView* pDSV;

	D3D11Context()->OMGetRenderTargets(1, &pRTV, &pDSV);

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
	GetCamera()->getFrom(vEyePt);
	const unsigned int size16 = sizeof(float) * 16;
		
	noMat4 tm(worldTM_.rotate_ * worldTM_.scale_, worldTM_.trans_);
	tm.TransposeSelf();
	ParticleData_.gpmWorld->SetMatrix(tm.ToFloatPtr());

	memcpy(&mView, &ViewMat(), size16);	 
	memcpy(&mProj, &ProjMat(), size16);
	memcpy(&mWorld, tm.ToFloatPtr(), size16);	

	//ParticleData_.gpmWorld->SetMatrix( (float*)mWorld);		
	mWorldViewProjection = mWorld * mView * mProj;


	ParticleData_.gpmWorldViewProjection->SetMatrix( ( float* )&mWorldViewProjection );

	D3D11Context()->IASetInputLayout( ParticleData_.gpVertexLayout );

	for (size_t i=0; i < Owner_->header.nParticleEmitters; ++i)
	{
		if (ParticleData_.gNumActiveParticles.at(i) == 0)
			continue;

		UINT Strides[1];
		UINT Offsets[1];
		ID3D11Buffer* pVB[1];
		pVB[0] = ParticleData_.ParticleBuffers.at(i);
		Strides[0] = sizeof( ParticleVertex );
		Offsets[0] = 0;
		pd3dContext->IASetVertexBuffers( 0, 1, pVB, Strides, Offsets );
		pd3dContext->IASetIndexBuffer( NULL, DXGI_FORMAT_R16_UINT, 0 );
		pd3dContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
			

		HRESULT hr = ParticleData_.gptxDiffuse->SetResource( ParticleData_.ParticleTextureSRVMaps.find(Owner_->particleSystems[i].texture)->second);

		//Render
		D3DX11_TECHNIQUE_DESC techDesc;
		ParticleData_.gpRenderParticles->GetDesc( &techDesc );

		for( UINT p = 0; p < techDesc.Passes; ++p )
		{
			ParticleData_.gpRenderParticles->GetPassByIndex( p )->Apply( 0, pd3dContext );
			pd3dContext->Draw( ParticleData_.gNumActiveParticles.at(i) * 6, 0 );
		}			
	}
}

void M2EffectRender::RenderRibbons()
{

	//renderer->changeToMainFramebuffer();
	//loader->reset();

	ID3D11RenderTargetView* pRTV;
	ID3D11DepthStencilView* pDSV;

	D3D11Context()->OMGetRenderTargets(1, &pRTV, &pDSV);

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
	GetCamera()->getFrom(vEyePt);
	const unsigned int size16 = sizeof(float) * 16;


	memcpy(&mView, &ViewMat(), size16);	 
	memcpy(&mProj, &ProjMat(), size16);
	memcpy(&mWorld, &WorldMat(), size16);	

	RibbonData_.gpmWorld->SetMatrix( (float*)mWorld);	
	mWorldViewProjection = mWorld * mView * mProj;


	RibbonData_.gpmWorldViewProjection->SetMatrix( ( float* )&mWorldViewProjection );


	

	D3D11Context()->IASetInputLayout( RibbonData_.gpVertexLayout );
	for (size_t i=0; i < Owner_->header.nRibbonEmitters; ++i)
	{
		if (RibbonData_.gNumActiveParticles.at(i) == 0)
			continue;
			
		UINT Strides[1];
		UINT Offsets[1];
		ID3D11Buffer* pVB[1];
		pVB[0] = RibbonData_.ParticleBuffers.at(i);
		Strides[0] = sizeof( RibbonVertex );
		Offsets[0] = 0;
		pd3dContext->IASetVertexBuffers( 0, 1, pVB, Strides, Offsets );
		pd3dContext->IASetIndexBuffer( NULL, DXGI_FORMAT_R16_UINT, 0 );
		pd3dContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

		HRESULT hr = RibbonData_.gptxDiffuse->SetResource( RibbonData_.ParticleTextureSRVMaps.find(Owner_->ribbons[i].texture)->second );

		//Render
		D3DX11_TECHNIQUE_DESC techDesc;
		RibbonData_.gpRenderParticles->GetDesc( &techDesc );

		for( UINT p = 0; p < techDesc.Passes; ++p )
		{
			RibbonData_.gpRenderParticles->GetPassByIndex( p )->Apply( 0, pd3dContext );
			pd3dContext->Draw( RibbonData_.gNumActiveParticles.at(i) * 4, 0 );
		}		
	}
}

void M2EffectRender::CopyParticlesToVertexBuffer( uint32 index, class M2ParticleSystem* ps, ParticleVertex* pVB, D3DXVECTOR3 vEye, D3DXVECTOR3 _vRight, D3DXVECTOR3 _vUp )
{

	noVec3 vRight(0,0,-1);
	noVec3 vUp(0,1,0);

	// position stuff
	const float f = 1;//0.707106781f; // sqrt(2)/2
	noVec3 bv0 = noVec3(-f,+f,0);
	noVec3 bv1 = noVec3(-f,-f,0);		
	noVec3 bv2 = noVec3(+f,-f,0);
	noVec3 bv3 = noVec3(+f,+f,0);		

	UINT iVBIndex = 0;

	uint32 gNumActiveParticles = 0;	
		
	for (size_t i=0; i< 1 ; i++) 
	{
		if (ps->billboard) 
		{
			vRight.Set(_vRight.x, _vRight.y, _vRight.z);
			vUp.Set(_vUp.x, _vUp.y, _vUp.z);
		}
			

		if (ps->ParticleType==0 || ps->ParticleType==2)
		{
			
						

			for (ParticleList::iterator it = ps->particles.begin(); it != ps->particles.end(); ++it) {
				if (ps->tiles.size() - 1 < it->tile) // Alfred, 2009.08.07, error prevent
					break;

				//if (gNumActiveParticles >= MAX_PARTICLES)
				//	return;
								
				M2Particle &p = *it;	
				const float size = it->size;// / 2;
													

				if (ps->billboard)
				{						
					
					// 0 ---- 3
					//
					//
					// 1 ---- 2
					//	// Tri 0 (0,1,3)	
					// it->posÀº center
					noVec3 v0 = it->pos - (vRight - vUp) * size;					
					noVec3 v1 = it->pos - (vRight + vUp) * size;
					noVec3 v2 = it->pos + (vRight - vUp) * size;
					noVec3 v3 = it->pos + (vRight + vUp) * size;

					pVB[iVBIndex].pos = v0;
					pVB[iVBIndex].color = it->color;
					pVB[iVBIndex].uv = ps->tiles[it->tile].tc[0];
					
					pVB[iVBIndex+1].pos = v1;
					pVB[iVBIndex+1].color = it->color;
					pVB[iVBIndex+1].uv = ps->tiles[it->tile].tc[3];
										
					pVB[iVBIndex+2].pos = v3;
					pVB[iVBIndex+2].color = it->color;
					pVB[iVBIndex+2].uv = ps->tiles[it->tile].tc[1];
					
					pVB[iVBIndex+3].pos = v2;
					pVB[iVBIndex+3].color = it->color;
					pVB[iVBIndex+3].uv = ps->tiles[it->tile].tc[2];

					pVB[iVBIndex+4].pos = v3;
					pVB[iVBIndex+4].color = it->color;
					pVB[iVBIndex+4].uv = ps->tiles[it->tile].tc[1];

					pVB[iVBIndex+5].pos = v1;
					pVB[iVBIndex+5].color = it->color;
					pVB[iVBIndex+5].uv = ps->tiles[it->tile].tc[3];

							
					//if (frame > 10 && frame < 20)
					{
					/*	for (int i = 0; i < 6; i++) {
							LOG_INFO << "Particle : " << iVBIndex +i  << " " <<  pVB[iVBIndex +i].pos.x << " " << pVB[iVBIndex +i].pos.y << 
							" " << pVB[iVBIndex +i].pos.z;*/
							/*noVec3 Pos = (mView * mProj) * pVB[iVBIndex +i].pos;
							LOG_INFO << "Particle : " << iVBIndex +i  << " " <<  Pos.x << " " << Pos.y << 
							" " << Pos.z;*/
						//}
					}

				}
				else 					
				{
					//	// Tri 0 (0,1,3)
					pVB[iVBIndex].pos = it->pos + it->corners[3] * size;
					pVB[iVBIndex].color = it->color;
					pVB[iVBIndex].uv = ps->tiles[it->tile].tc[3];

					pVB[iVBIndex+1].pos = it->pos + it->corners[0] * size;
					pVB[iVBIndex+1].color = it->color;
					pVB[iVBIndex+1].uv = ps->tiles[it->tile].tc[0];

					pVB[iVBIndex+2].pos = it->pos + it->corners[2] * size;
					pVB[iVBIndex+2].color = it->color;
					pVB[iVBIndex+2].uv = ps->tiles[it->tile].tc[2];

					//	// Tri 1 (3,1,2)					
					pVB[iVBIndex+3].pos = it->pos + it->corners[1] * size;
					pVB[iVBIndex+3].color = it->color;
					pVB[iVBIndex+3].uv = ps->tiles[it->tile].tc[1];

					pVB[iVBIndex+4].pos = it->pos + it->corners[2] * size;
					pVB[iVBIndex+4].color = it->color;
					pVB[iVBIndex+4].uv = ps->tiles[it->tile].tc[2];

					pVB[iVBIndex+5].pos = it->pos + it->corners[0] * size;
					pVB[iVBIndex+5].color = it->color;
					pVB[iVBIndex+5].uv = ps->tiles[it->tile].tc[0];
				}

				iVBIndex+=6;
				gNumActiveParticles++;
			}		
		}
		else if (ps->ParticleType==1) { // Sphere particles

						
			for (ParticleList::iterator it = ps->particles.begin(); it != ps->particles.end(); ++it) {
				if (ps->tiles.size() - 1 < it->tile) // Alfred, 2009.08.07, error prevent
					break;

			//	//	// Tri 0 (0,1,3)
			//	pVB[iVBIndex+2].pos = it->pos + it->corners[1] * size;
			//	pVB[iVBIndex+2].color = it->color;
			//	pVB[iVBIndex+2].uv = ps->tiles[it->tile].tc[1];

			//	pVB[iVBIndex+1].pos = it->pos + bv1 * it->size;
			//	pVB[iVBIndex+1].color = it->color;
			//	pVB[iVBIndex+1].uv = ps->tiles[it->tile].tc[3];

			//	pVB[iVBIndex].pos = it->pos + bv0 * it->size;
			//	pVB[iVBIndex].color = it->color;
			//	pVB[iVBIndex].uv = ps->tiles[it->tile].tc[0];

			//	//	// Tri 1 (3,1,2)					
			//	pVB[iVBIndex+5].pos = it->pos + it->corners[3] * size;
			//	pVB[iVBIndex+5].color = it->color;
			//	pVB[iVBIndex+5].uv = ps->tiles[it->tile].tc[2];

			//	pVB[iVBIndex+4].pos = it->pos + it->corners[2] * size;
			//	pVB[iVBIndex+4].color = it->color;
			//	pVB[iVBIndex+4].uv = ps->tiles[it->tile].tc[3];

			//	pVB[iVBIndex+3].pos = it->pos + it->corners[1] * size;
			//	pVB[iVBIndex+3].color = it->color;
			//	pVB[iVBIndex+3].uv = ps->tiles[it->tile].tc[1];

			//	iVBIndex+=6;
			//	gNumActiveParticles+=6;

			//	noVec3 bv0 = noVec3(-f,+f,0);
			//	noVec3 bv1 = noVec3(-f,-f,0);		
			//					
			//	glTexCoord2fv(tiles[it->tile].tc[0]);
			//	glVertex3fv(it->pos + bv0 * it->size);

			//	glTexCoord2fv(tiles[it->tile].tc[1]);
			//	glVertex3fv(it->pos + bv1 * it->size);

			//	glTexCoord2fv(tiles[it->tile].tc[2]);
			//	glVertex3fv(it->origin + bv1 * it->size);

			//	glTexCoord2fv(tiles[it->tile].tc[3]);
			//	glVertex3fv(it->origin + bv0 * it->size);
			}		
		}		
	}		
	ParticleData_.gNumActiveParticles.at(index) = gNumActiveParticles;
}

void M2EffectRender::CopyRibbonsToVertexBuffer( int index, M2RibbonEmitter& Ribbon, RibbonVertex* pVB )
{
	
	noVec3 vRight(0,0,-1);
	noVec3 vUp(0,1,0);

	UINT iVBIndex = 0;

	uint32 gNumActiveParticles  = 0;	
			
	for (size_t i=0; i< 1 ; i++) 
	{
		std::list<M2RibbonSegment>::iterator it = Ribbon.segs.begin();
		float l = 0;
		for (; it != Ribbon.segs.end(); ++it) {
			float u = l/Ribbon.length;
						

			pVB[iVBIndex].pos = it->pos + Ribbon.tabove * it->up;
			pVB[iVBIndex].uv = noVec2(u, 0);
			pVB[iVBIndex].color = Ribbon.tcolor;

			pVB[++iVBIndex].pos = it->pos - Ribbon.tbelow * it->up;
			pVB[iVBIndex].uv = noVec2(u,1);
			pVB[iVBIndex].color = Ribbon.tcolor;

			++iVBIndex;
			gNumActiveParticles ++;
			l += it->len;
		}			

		if (Ribbon.segs.size() > 1) {
						
			// last segment...?
			--it;
			pVB[iVBIndex].pos = it->pos + Ribbon.tabove * it->up + (it->len/it->len0) * it->back;
			pVB[iVBIndex].uv = noVec2(1, 0);
			pVB[iVBIndex].color = Ribbon.tcolor;

			pVB[++iVBIndex].pos = it->pos - Ribbon.tbelow * it->up + (it->len/it->len0) * it->back;
			pVB[iVBIndex].uv = noVec2(1,1);
			pVB[iVBIndex].color = Ribbon.tcolor;

			gNumActiveParticles++;
		}
	}	

	RibbonData_.gNumActiveParticles.at(index) = gNumActiveParticles;
 }

 M2EffectRender::~M2EffectRender()
 {
	 SAFE_DELETE(Effect_);
 }

 M2EffectRender::RenderData::RenderData()
 {
	 
 }

 M2EffectRender::RenderData::~RenderData()
 {
	 for(size_t i = 0; i < ParticleBuffers.size(); ++i)
		 SAFE_RELEASE(ParticleBuffers.at(i));

	 std::map<TextureID, ID3D11ShaderResourceView*>::iterator itr = ParticleTextureSRVMaps.begin();
	 for ( ; itr != ParticleTextureSRVMaps.end(); ++itr)
		 SAFE_RELEASE(itr->second); 
	 
 }
