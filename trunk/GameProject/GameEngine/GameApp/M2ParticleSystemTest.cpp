#include "stdafx.h"
#include "EngineCore/Util/GameLog.h"
#include <ModelLib/mpq_libmpq.h>
#include <ModelLib/Model_M2.h>
#include <ModelLib/M2Particle.h>
#include <ModelLib/M2Loader.h>
#include "GameApp.h"
#include <GameApp/Util/EffectUtil.h>
#include "M2EffectRender.h"



// Particle
ID3DX11EffectShaderResourceVariable* gptxDiffuse = NULL;
ID3D11InputLayout*                  gpVertexLayout = NULL;
ID3D11Buffer*                       gpParticleBuffer = NULL;
ID3D11ShaderResourceView*           gpParticleTextureSRV = NULL;
ID3DX11Effect*                       gpEffect10 = NULL;
ID3DX11EffectTechnique*              gpRenderParticles = NULL;
ID3DX11EffectMatrixVariable*         gpmWorldViewProjection = NULL;
ID3DX11EffectMatrixVariable*         gpmWorld = NULL;
ID3DX11EffectMatrixVariable*         gpmInvViewProj = NULL;
ID3DX11EffectScalarVariable*         gpfTime = NULL;
ID3DX11EffectVectorVariable*         gpvEyePt = NULL;
ID3DX11EffectVectorVariable*         gpvRight = NULL;
ID3DX11EffectVectorVariable*         gpvUp = NULL;
ID3DX11EffectVectorVariable*         gpvForward = NULL;

noMat4 testWorld;
UINT        gNumActiveParticles = 0;


template <class T> void QuickDepthSort( T* indices, float* depths, int lo, int hi )
{
	//  lo is the lower index, hi is the upper index
	//  of the region of array a that is to be sorted
	int i = lo, j = hi;
	float h;
	T index;
	float x = depths[( lo + hi ) / 2];

	//  partition
	do
	{
		while( depths[i] < x ) i++;
		while( depths[j] > x ) j--;
		if( i <= j )
		{
			h = depths[i]; depths[i] = depths[j]; depths[j] = h;
			index = indices[i]; indices[i] = indices[j]; indices[j] = index;
			i++; j--;
		}
	} while( i <= j );

	//  recursion
	if( lo < j ) QuickDepthSort( indices, depths, lo, j );
	if( i < hi ) QuickDepthSort( indices, depths, i, hi );
}



static void SortParticles()
{
	M2ParticleSystem* ps = GetApp()->loader->particleSystems;

	for (size_t i=0; i<GetApp()->loader->header.nParticleEmitters; i++) 
	{
		//for (ParticleList::iterator it = ps[i].particles.begin(); it != ps[i].particles.end(); ++it) {
	//	std::sort(ps[i].particles.begin(), ps[i].particles.end(), );
	}
}

static void CopyParticlesToVertexBuffer( ParticleVertex* pVB, D3DXVECTOR3 vEye, D3DXVECTOR3 _vRight, D3DXVECTOR3 _vUp )
{

	M2ParticleSystem* ps = GetApp()->loader->particleSystems;
	
	gNumActiveParticles = 0;


	noVec3 vRight(0,0,-1);
	noVec3 vUp(0,1,0);

	// position stuff
	const float f = 1;//0.707106781f; // sqrt(2)/2
	noVec3 bv0 = noVec3(-f,+f,0);
	noVec3 bv1 = noVec3(-f,-f,0);		
	noVec3 bv2 = noVec3(+f,-f,0);
	noVec3 bv3 = noVec3(+f,+f,0);		

	UINT iVBIndex = 0;
		
	int num = GetApp()->loader->header.nParticleEmitters;					
	//num = 1;
	for (size_t i=0; i< num ; i++) 
	{
		if (ps[i].billboard) 
		{
			vRight.Set(_vRight.x, _vRight.y, _vRight.z);
			vUp.Set(_vUp.x, _vUp.y, _vUp.z);
		}

		if (ps[i].ParticleType==0 || ps[i].ParticleType==2)
		{
			for (ParticleList::iterator it = ps[i].particles.begin(); it != ps[i].particles.end(); ++it) {
				if (ps[i].tiles.size() - 1 < it->tile) // Alfred, 2009.08.07, error prevent
					break;

				M2Particle &p = *it;	
				const float size = it->size;// / 2;
									

				if (ps[i].billboard)
				{						
					
					// 0 ---- 3
					//
					//
					// 1 ---- 2
					//	// Tri 0 (0,1,3)
					
					noVec3 v3 = it->pos + (vRight + vUp) * size; 
					pVB[iVBIndex+2].pos = noVec3(v3.x, v3.y, v3.z);
					pVB[iVBIndex+2].color = it->color;
					pVB[iVBIndex+2].uv = ps[i].tiles[it->tile].tc[1];				//ps[i].tiles[it->tile].tc[0];				

					noVec3 v1 = it->pos - (vRight + vUp) * size;
					pVB[iVBIndex+1].pos = noVec3(v1.x, v1.y, v1.z);
					pVB[iVBIndex+1].color = it->color;
					pVB[iVBIndex+1].uv = ps[i].tiles[it->tile].tc[3];//	ps[i].tiles[it->tile].tc[1];

					noVec3 v0 = it->pos - (vRight - vUp) * size;
					pVB[iVBIndex].pos = noVec3(v0.x, v0.y, v0.z);
					pVB[iVBIndex].color = it->color;
					pVB[iVBIndex].uv = ps[i].tiles[it->tile].tc[0];
					
					//	// Tri 1 (3,1,2)					
					noVec3 v2 =  it->pos + (vRight - vUp) * size;
					pVB[iVBIndex+5].pos =  noVec3(v2.x, v2.y, v2.z);
					pVB[iVBIndex+5].color = it->color;
					pVB[iVBIndex+5].uv = ps[i].tiles[it->tile].tc[2];

					pVB[iVBIndex+4].pos = noVec3(v1.x, v1.y, v1.z);   //it->pos - (vRight + vUp) * size;
					pVB[iVBIndex+4].color = it->color;
					pVB[iVBIndex+4].uv = ps[i].tiles[it->tile].tc[3];//ps[i].tiles[it->tile].tc[1];
					
					
					pVB[iVBIndex+3].pos = noVec3(v3.x, v3.y, v3.z);
					pVB[iVBIndex+3].color = it->color;
					pVB[iVBIndex+3].uv = ps[i].tiles[it->tile].tc[1];				
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
					pVB[iVBIndex+2].pos = it->pos + it->corners[1] * size;
					pVB[iVBIndex+2].color = it->color;
					pVB[iVBIndex+2].uv = ps[i].tiles[it->tile].tc[1];

					pVB[iVBIndex+1].pos = it->pos + it->corners[2] * size;
					pVB[iVBIndex+1].color = it->color;
					pVB[iVBIndex+1].uv = ps[i].tiles[it->tile].tc[3];

					pVB[iVBIndex].pos = it->pos + it->corners[0] * size;
					pVB[iVBIndex].color = it->color;
					pVB[iVBIndex].uv = ps[i].tiles[it->tile].tc[0];

					//	// Tri 1 (3,1,2)					
					pVB[iVBIndex+5].pos = it->pos + it->corners[3] * size;
					pVB[iVBIndex+5].color = it->color;
					pVB[iVBIndex+5].uv = ps[i].tiles[it->tile].tc[2];

					pVB[iVBIndex+4].pos = it->pos + it->corners[2] * size;
					pVB[iVBIndex+4].color = it->color;
					pVB[iVBIndex+4].uv = ps[i].tiles[it->tile].tc[3];

					pVB[iVBIndex+3].pos = it->pos + it->corners[1] * size;
					pVB[iVBIndex+3].color = it->color;
					pVB[iVBIndex+3].uv = ps[i].tiles[it->tile].tc[1];
				}

				iVBIndex+=6;
				gNumActiveParticles+=6;
			}		
		}
		else if (ps[i].ParticleType==1) { // Sphere particles

						
			for (ParticleList::iterator it = ps[i].particles.begin(); it != ps[i].particles.end(); ++it) {
				if (ps[i].tiles.size() - 1 < it->tile) // Alfred, 2009.08.07, error prevent
					break;

			//	//	// Tri 0 (0,1,3)
			//	pVB[iVBIndex+2].pos = it->pos + it->corners[1] * size;
			//	pVB[iVBIndex+2].color = it->color;
			//	pVB[iVBIndex+2].uv = ps[i].tiles[it->tile].tc[1];

			//	pVB[iVBIndex+1].pos = it->pos + bv1 * it->size;
			//	pVB[iVBIndex+1].color = it->color;
			//	pVB[iVBIndex+1].uv = ps[i].tiles[it->tile].tc[3];

			//	pVB[iVBIndex].pos = it->pos + bv0 * it->size;
			//	pVB[iVBIndex].color = it->color;
			//	pVB[iVBIndex].uv = ps[i].tiles[it->tile].tc[0];

			//	//	// Tri 1 (3,1,2)					
			//	pVB[iVBIndex+5].pos = it->pos + it->corners[3] * size;
			//	pVB[iVBIndex+5].color = it->color;
			//	pVB[iVBIndex+5].uv = ps[i].tiles[it->tile].tc[2];

			//	pVB[iVBIndex+4].pos = it->pos + it->corners[2] * size;
			//	pVB[iVBIndex+4].color = it->color;
			//	pVB[iVBIndex+4].uv = ps[i].tiles[it->tile].tc[3];

			//	pVB[iVBIndex+3].pos = it->pos + it->corners[1] * size;
			//	pVB[iVBIndex+3].color = it->color;
			//	pVB[iVBIndex+3].uv = ps[i].tiles[it->tile].tc[1];

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
		&gpParticleTextureSRV, NULL );

	LoadEffectFromFile(GetApp()->GetDevice(), "Data/Shaders/Particle.fx", &gpEffect10);
	gpRenderParticles = gpEffect10->GetTechniqueByName( "RenderParticles" );

	
	gptxDiffuse = gpEffect10->GetVariableByName( "g_txMeshTexture" )->AsShaderResource();
	gpmWorld = gpEffect10->GetVariableByName( "g_mWorld" )->AsMatrix();

	//gpfTime = gpEffect10->GetVariableByName( "g_fTime" )->AsScalar();
	gpvEyePt = gpEffect10->GetVariableByName( "g_vEyePt" )->AsVector();
	//gpvRight = gpEffect10->GetVariableByName( "g_vRight" )->AsVector();
	//gpvUp = gpEffect10->GetVariableByName( "g_vUp" )->AsVector();
	//gpvForward = gpEffect10->GetVariableByName( "g_vForward" )->AsVector();
	//gpmViewProj = gpEffect10->GetVariableByName( "gmViewProj" )->AsMatrix();

	gpmWorldViewProjection = gpEffect10->GetVariableByName( "g_mWorldViewProjection" )->AsMatrix();	
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
	gpRenderParticles->GetPassByIndex( 0 )->GetDesc( &PassDesc ) ;
	device->CreateInputLayout( layout, 3, PassDesc.pIAInputSignature,
		PassDesc.IAInputSignatureSize, &gpVertexLayout ) ;
		
	D3D11_BUFFER_DESC BDesc;
	BDesc.ByteWidth = sizeof( ParticleVertex ) * 6 * MAX_PARTICLES;
	BDesc.Usage = D3D11_USAGE_DYNAMIC;
	BDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	BDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	BDesc.MiscFlags = 0;
	device->CreateBuffer( &BDesc, NULL, &gpParticleBuffer );	
}

void App::LoadM2Particles()
{
	CreateShaders();
	CreateMeshes();	
}

void App::UpdateM2Particles( float fElapsedTime )
{
	if (!gpEffect10) return;

	if (fElapsedTime > 0.1f) fElapsedTime = 0.1f;

	D3DXVECTOR3 vEye;
	DefCam_->getFrom(vEye);
	D3DXMATRIX mView;
	memcpy(&mView, DefCam_->getViewMatrix(), SIZE16);	

	D3DXVECTOR3 vRight( mView._11, mView._21, mView._31 );
	D3DXVECTOR3 vUp( mView._12, mView._22, mView._32 );
	D3DXVECTOR3 vFoward( mView._13, mView._23, mView._33 );

	
	memcpy(&testWorld, &g_World, SIZE16);
		

	D3DXVec3Normalize( &vRight, &vRight );
	D3DXVec3Normalize( &vUp, &vUp );
	D3DXVec3Normalize( &vFoward, &vFoward );

	//gpvRight->SetFloatVector( ( float* )&vRight );
	//gpvUp->SetFloatVector( ( float* )&vUp );
	//gpvForward->SetFloatVector( ( float* )&vFoward );
	gpvEyePt->SetFloatVector( ( float* )&vEye );

	ParticleVertex* pVerts = NULL;

	ID3D11Resource* pRes = gpParticleBuffer;
	D3D11_MAPPED_SUBRESOURCE Data;

	HRESULT hr = GetApp()->GetContext()->Map( pRes, 0, D3D11_MAP_WRITE_DISCARD, 0, &Data );	
	
	if (FAILED(hr))
		assert(false);

	pVerts = (ParticleVertex*)Data.pData;
	CopyParticlesToVertexBuffer( pVerts, vEye, -vFoward, vUp );

	GetApp()->GetContext()->Unmap(pRes, 0);

}

void App::RenderM2Particles( float fTime, const mat4& world, const mat4& view, const mat4& proj )
{
	if (gNumActiveParticles == 0) return;

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

	gpmWorld->SetMatrix( (float*)mWorld);	
	mWorldViewProjection = mWorld * mView * mProj;
	
		
	gpmWorldViewProjection->SetMatrix( ( float* )&mWorldViewProjection );

	context->IASetInputLayout( gpVertexLayout );
	UINT Strides[1];
	UINT Offsets[1];
	ID3D11Buffer* pVB[1];
	pVB[0] = gpParticleBuffer;
	Strides[0] = sizeof( ParticleVertex );
	Offsets[0] = 0;
	context->IASetVertexBuffers( 0, 1, pVB, Strides, Offsets );
	context->IASetIndexBuffer( NULL, DXGI_FORMAT_R16_UINT, 0 );
	context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	HRESULT hr = gptxDiffuse->SetResource( gpParticleTextureSRV );

	//Render
	D3DX11_TECHNIQUE_DESC techDesc;
	gpRenderParticles->GetDesc( &techDesc );
		
	for( UINT p = 0; p < techDesc.Passes; ++p )
	{
		gpRenderParticles->GetPassByIndex( p )->Apply( 0, context );
		context->Draw( gNumActiveParticles, 0 );
	}	
}