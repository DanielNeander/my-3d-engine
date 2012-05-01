#include <U2_3D/src/U23DLibPCH.h>
#include "U2Dx9RenderStateMgr.h"


U2Dx9RenderStateMgr::RenderStateInitValue
U2Dx9RenderStateMgr::ms_initRenderStates[] = 
{
	D3DRS_ZENABLE,                      D3DZB_FALSE,        //**
	D3DRS_FILLMODE,                     D3DFILL_SOLID,
	D3DRS_SHADEMODE,                    D3DSHADE_GOURAUD,
	D3DRS_ZWRITEENABLE,                 TRUE,
	D3DRS_ALPHATESTENABLE,              FALSE,
	D3DRS_LASTPIXEL,                    TRUE,
	D3DRS_SRCBLEND,                     D3DBLEND_ONE,
	D3DRS_DESTBLEND,                    D3DBLEND_ZERO,
	D3DRS_CULLMODE,                     D3DCULL_CCW,
	D3DRS_ZFUNC,                        D3DCMP_LESSEQUAL,
	D3DRS_ALPHAREF,                     0x00000000,
	D3DRS_ALPHAFUNC,                    D3DCMP_ALWAYS,
	D3DRS_DITHERENABLE,                 FALSE,
	D3DRS_ALPHABLENDENABLE,             FALSE,
	D3DRS_FOGENABLE,                    FALSE,
	D3DRS_SPECULARENABLE,               FALSE,
	D3DRS_FOGCOLOR,                     0x00000000,
	D3DRS_FOGTABLEMODE,                 D3DFOG_NONE,
	D3DRS_FOGSTART,                     0x00000000,
	D3DRS_FOGEND,                       0x3F800000,
	D3DRS_FOGDENSITY,                   0x3F800000,
	D3DRS_RANGEFOGENABLE,               FALSE,
	D3DRS_STENCILENABLE,                FALSE,
	D3DRS_STENCILFAIL,                  D3DSTENCILOP_KEEP,
	D3DRS_STENCILZFAIL,                 D3DSTENCILOP_KEEP,
	D3DRS_STENCILPASS,                  D3DSTENCILOP_KEEP,
	D3DRS_STENCILFUNC,                  D3DCMP_ALWAYS,
	D3DRS_STENCILREF,                   0,
	D3DRS_STENCILMASK,                  0xFFFFFFFF,
	D3DRS_STENCILWRITEMASK,             0xFFFFFFFF,
	D3DRS_TEXTUREFACTOR,                0xFFFFFFFF,
	D3DRS_WRAP0,                        0,
	D3DRS_WRAP1,                        0,
	D3DRS_WRAP2,                        0,
	D3DRS_WRAP3,                        0,
	D3DRS_WRAP4,                        0,
	D3DRS_WRAP5,                        0,
	D3DRS_WRAP6,                        0,
	D3DRS_WRAP7,                        0,
	D3DRS_CLIPPING,                     TRUE,
	D3DRS_LIGHTING,                     TRUE,
	D3DRS_AMBIENT,                      0x00000000,
	D3DRS_FOGVERTEXMODE,                D3DFOG_NONE,
	D3DRS_COLORVERTEX,                  TRUE,
	D3DRS_LOCALVIEWER,                  TRUE,
	D3DRS_NORMALIZENORMALS,             FALSE,
	D3DRS_DIFFUSEMATERIALSOURCE,        D3DMCS_COLOR1,
	D3DRS_SPECULARMATERIALSOURCE,       D3DMCS_COLOR2,
	D3DRS_AMBIENTMATERIALSOURCE,        D3DMCS_MATERIAL,
	D3DRS_EMISSIVEMATERIALSOURCE,       D3DMCS_MATERIAL,
	D3DRS_VERTEXBLEND,                  D3DVBF_DISABLE,
	D3DRS_CLIPPLANEENABLE,              0,
	D3DRS_POINTSIZE,                    0x3F800000,
	D3DRS_POINTSIZE_MIN,                0x3F800000,
	D3DRS_POINTSPRITEENABLE,            FALSE,
	D3DRS_POINTSCALEENABLE,             FALSE,
	D3DRS_POINTSCALE_A,                 0x3F800000,
	D3DRS_POINTSCALE_B,                 0x00000000,
	D3DRS_POINTSCALE_C,                 0x00000000,
	D3DRS_MULTISAMPLEANTIALIAS,         TRUE,
	D3DRS_MULTISAMPLEMASK,              0xFFFFFFFF,
	D3DRS_PATCHEDGESTYLE,               D3DPATCHEDGE_DISCRETE,
	D3DRS_DEBUGMONITORTOKEN,            D3DDMT_ENABLE,
	D3DRS_POINTSIZE_MAX,                0x42800000,
	D3DRS_INDEXEDVERTEXBLENDENABLE,     FALSE,
	D3DRS_COLORWRITEENABLE,             0x0000000F,
	D3DRS_TWEENFACTOR,                  0x00000000,
	D3DRS_BLENDOP,                      D3DBLENDOP_ADD,
	D3DRS_POSITIONDEGREE,               D3DDEGREE_CUBIC,
	D3DRS_NORMALDEGREE,                 D3DDEGREE_LINEAR,
	D3DRS_SCISSORTESTENABLE,            FALSE,
	D3DRS_SLOPESCALEDEPTHBIAS,          0x00000000,
	D3DRS_ANTIALIASEDLINEENABLE,        FALSE, 
	D3DRS_MINTESSELLATIONLEVEL,         0x3F800000,
	D3DRS_MAXTESSELLATIONLEVEL,         0x3F800000,
	D3DRS_ADAPTIVETESS_X,               0x00000000,
	D3DRS_ADAPTIVETESS_Y,               0x00000000,
	D3DRS_ADAPTIVETESS_Z,               0x3F800000,
	D3DRS_ADAPTIVETESS_W,               0x00000000,
	D3DRS_ENABLEADAPTIVETESSELLATION,   FALSE,
	D3DRS_TWOSIDEDSTENCILMODE,          FALSE,
	D3DRS_CCW_STENCILFAIL,              0x00000001,
	D3DRS_CCW_STENCILZFAIL,             0x00000001,
	D3DRS_CCW_STENCILPASS,              0x00000001,
	D3DRS_CCW_STENCILFUNC,              0x00000008,
	D3DRS_COLORWRITEENABLE1,            0x0000000F,
	D3DRS_COLORWRITEENABLE2,            0x0000000F,
	D3DRS_COLORWRITEENABLE3,            0x0000000F,
	D3DRS_BLENDFACTOR,                  0xFFFFFFFF,
	D3DRS_SRGBWRITEENABLE,              0x00000000,
	D3DRS_DEPTHBIAS,                    0x00000000,
	D3DRS_WRAP8,                        0,
	D3DRS_WRAP9,                        0,
	D3DRS_WRAP10,                       0,
	D3DRS_WRAP11,                       0,
	D3DRS_WRAP12,                       0,
	D3DRS_WRAP13,                       0,
	D3DRS_WRAP14,                       0,
	D3DRS_WRAP15,                       0,
	D3DRS_SEPARATEALPHABLENDENABLE,     FALSE,
	D3DRS_SRCBLENDALPHA,                D3DBLEND_ONE,
	D3DRS_DESTBLENDALPHA,               D3DBLEND_ZERO,
	D3DRS_BLENDOPALPHA,                 D3DBLENDOP_ADD,
	(D3DRENDERSTATETYPE)0xFFFFFFFF,     0xFFFFFFFF
};



U2Dx9RenderStateMgr::TextureStageInitValue 
U2Dx9RenderStateMgr::ms_initTextureStages[] = 
{
	D3DTSS_COLOROP,                 D3DTOP_MODULATE,    //**
	D3DTSS_COLORARG1,               D3DTA_TEXTURE,
	D3DTSS_COLORARG2,               D3DTA_CURRENT,
	D3DTSS_ALPHAOP,                 D3DTOP_SELECTARG1,  //**
	D3DTSS_ALPHAARG1,               D3DTA_TEXTURE,
	D3DTSS_ALPHAARG2,               D3DTA_CURRENT,
	D3DTSS_BUMPENVMAT00,            0x00000000,
	D3DTSS_BUMPENVMAT01,            0x00000000,
	D3DTSS_BUMPENVMAT10,            0x00000000,
	D3DTSS_BUMPENVMAT11,            0x00000000,
	D3DTSS_TEXCOORDINDEX,           0,                  //**
	D3DTSS_BUMPENVLSCALE,           0x00000000,
	D3DTSS_BUMPENVLOFFSET,          0x00000000,
	D3DTSS_TEXTURETRANSFORMFLAGS,   D3DTTFF_DISABLE,
	D3DTSS_COLORARG0,               D3DTA_CURRENT,
	D3DTSS_ALPHAARG0,               D3DTA_CURRENT,
	D3DTSS_RESULTARG,               D3DTA_CURRENT,
	D3DTSS_RESULTARG,               D3DTA_CURRENT,
	//
	(D3DTEXTURESTAGESTATETYPE)0xFFFFFFFF, 0xFFFFFFFF
};


unsigned int U2Dx9RenderStateMgr::ms_uSSMappings[
	U2Dx9RenderStateMgr::MAX_D3D_SAMPLERS] =
{
		D3DSAMP_ADDRESSU,  
		D3DSAMP_ADDRESSV,  
		D3DSAMP_ADDRESSW,   
		D3DSAMP_BORDERCOLOR,   
		D3DSAMP_MAGFILTER,     
		D3DSAMP_MINFILTER,     
		D3DSAMP_MIPFILTER,     
		D3DSAMP_MIPMAPLODBIAS, 
		D3DSAMP_MAXMIPLEVEL,   
		D3DSAMP_MAXANISOTROPY, 
		D3DSAMP_SRGBTEXTURE,   
		D3DSAMP_ELEMENTINDEX,  
		D3DSAMP_DMAPOFFSET     
};

	U2Dx9RenderStateMgr::SamplerStateInitValue 
		U2Dx9RenderStateMgr::ms_initSamplerStates[] = 
	{
		D3DSAMP_ADDRESSU,                D3DTADDRESS_WRAP, 
		D3DSAMP_ADDRESSV,                D3DTADDRESS_WRAP, 
		D3DSAMP_ADDRESSW,                D3DTADDRESS_WRAP, 
		D3DSAMP_BORDERCOLOR,             0x00000000, 
		D3DSAMP_MAGFILTER,               D3DTEXF_POINT, 
		D3DSAMP_MINFILTER,               D3DTEXF_POINT, 
		D3DSAMP_MIPFILTER,               D3DTEXF_NONE, 
		D3DSAMP_MIPMAPLODBIAS,           0, 
		D3DSAMP_MAXMIPLEVEL,             0,  
		D3DSAMP_MAXANISOTROPY,           1, 
		D3DSAMP_SRGBTEXTURE,             0, 
		D3DSAMP_ELEMENTINDEX,            0, 
		D3DSAMP_DMAPOFFSET,              256, 
		// 
		(D3DSAMPLERSTATETYPE)0xFFFFFFFF, 0xFFFFFFFF
	};



U2Dx9RenderStateMgr::U2Dx9RenderStateMgr(U2Dx9Renderer* pRenderer, const D3DCAPS9& d3dCaps9)
	:m_pD3DDev(0), 
	m_pRenderer(pRenderer)
{
	if(m_pRenderer)
	{
		if(m_pRenderer->GetD3DDevice())
		{
			m_pD3DDev = m_pRenderer->GetD3DDevice();
			m_pD3DDev->AddRef();
		}
	}

	m_d3dVertBlendFlags[0] = D3DVBF_DISABLE;
	m_d3dVertBlendFlags[1] = D3DVBF_1WEIGHTS;
	m_d3dVertBlendFlags[2] = D3DVBF_1WEIGHTS;
	m_d3dVertBlendFlags[3] = D3DVBF_2WEIGHTS;
	m_d3dVertBlendFlags[4] = D3DVBF_3WEIGHTS;

	m_uSrcAlphaCaps = 0;
	m_uDestAlphaCaps = 0;

	m_uAlphaBlendModes[U2AlphaState::BLEND_ZERO] = D3DBLEND_ZERO;
	m_uAlphaBlendModes[U2AlphaState::BLEND_ONE] = D3DBLEND_ONE;	
	m_uAlphaBlendModes[U2AlphaState::BLEND_SRC_COLOR] = D3DBLEND_SRCCOLOR;
	m_uAlphaBlendModes[U2AlphaState::BLEND_ONE_MINUS_SRC_COLOR] = D3DBLEND_INVSRCCOLOR;
	m_uAlphaBlendModes[U2AlphaState::BLEND_DEST_COLOR] = D3DBLEND_DESTCOLOR;
	m_uAlphaBlendModes[U2AlphaState::BLEND_ONE_MINUS_DEST_COLOR] = D3DBLEND_INVDESTCOLOR;
	m_uAlphaBlendModes[U2AlphaState::BLEND_SRC_ALPHA] = D3DBLEND_SRCALPHA;
	m_uAlphaBlendModes[U2AlphaState::BLEND_ONE_MINUS_SRC_ALPHA] = D3DBLEND_INVSRCALPHA;
	m_uAlphaBlendModes[U2AlphaState::BLEND_DEST_ALPHA] = D3DBLEND_DESTALPHA;
	m_uAlphaBlendModes[U2AlphaState::BLEND_ONE_MINUS_DEST_ALPHA] = D3DBLEND_INVDESTALPHA;
	m_uAlphaBlendModes[U2AlphaState::BLEND_SRC_ALPHA_SATURATE] = D3DBLEND_SRCALPHASAT;
	
	m_uAlphaTestModes[U2AlphaState::TEST_ALWAYS] = D3DCMP_ALWAYS;
	m_uAlphaTestModes[U2AlphaState::TEST_LESS] = D3DCMP_LESS;
	m_uAlphaTestModes[U2AlphaState::TEST_EQUAL] = D3DCMP_EQUAL;
	m_uAlphaTestModes[U2AlphaState::TEST_LESSEQUAL] = D3DCMP_LESSEQUAL;
	m_uAlphaTestModes[U2AlphaState::TEST_GREATER] = D3DCMP_GREATER;
	m_uAlphaTestModes[U2AlphaState::TEST_NOTEQUAL] = D3DCMP_NOTEQUAL;
	m_uAlphaTestModes[U2AlphaState::TEST_GREATEREQUAL] = D3DCMP_GREATEREQUAL;
	m_uAlphaTestModes[U2AlphaState::TEST_NEVER] = D3DCMP_NEVER;

	m_uStencilTestModes[U2StencilState::CF_ALWAYS] = D3DCMP_ALWAYS;
	m_uStencilTestModes[U2StencilState::CF_LESS] = D3DCMP_LESS;
	m_uStencilTestModes[U2StencilState::CF_EQUAL] = D3DCMP_EQUAL;
	m_uStencilTestModes[U2StencilState::CF_LESSEQUAL] = D3DCMP_LESSEQUAL;
	m_uStencilTestModes[U2StencilState::CF_GREATER] = D3DCMP_GREATER;
	m_uStencilTestModes[U2StencilState::CF_NOTEQUAL] = D3DCMP_NOTEQUAL;
	m_uStencilTestModes[U2StencilState::CF_GREATEREQUAL] = D3DCMP_GREATEREQUAL;
	m_uStencilTestModes[U2StencilState::CF_NEVER] = D3DCMP_NEVER;

	m_uSteinlOps[U2StencilState::OP_KEEP] = D3DSTENCILOP_KEEP; // default 
	m_uSteinlOps[U2StencilState::OP_ZERO] = D3DSTENCILOP_ZERO; 
	m_uSteinlOps[U2StencilState::OP_REPLACE] = D3DSTENCILOP_REPLACE; 
	m_uSteinlOps[U2StencilState::OP_INCREMENT] = D3DSTENCILOP_INCRSAT; 
	m_uSteinlOps[U2StencilState::OP_DECREMENT] = D3DSTENCILOP_DECRSAT; 
	m_uSteinlOps[U2StencilState::OP_INVERT] = D3DSTENCILOP_INVERT; 

	m_uCullModes[U2StencilState::CM_CCW] = D3DCULL_CCW;
	m_uCullModes[U2StencilState::CM_CW] = D3DCULL_CW;
	m_uCullModes[U2StencilState::CM_BOTH] = D3DCULL_NONE;

	

		
	m_uZBufferModes[U2ZBufferState::CF_ALWAYS] = D3DCMP_ALWAYS;
	m_uZBufferModes[U2ZBufferState::CF_LESS] = D3DCMP_LESS;
	m_uZBufferModes[U2ZBufferState::CF_GREATEREQUAL] = D3DCMP_EQUAL;
	m_uZBufferModes[U2ZBufferState::CF_LESSEQUAL] = D3DCMP_LESSEQUAL;
	m_uZBufferModes[U2ZBufferState::CF_GREATER] = D3DCMP_GREATER;
	m_uZBufferModes[U2ZBufferState::CF_NOTEQUAL] = D3DCMP_NOTEQUAL;
	m_uZBufferModes[U2ZBufferState::CF_GREATEREQUAL] = D3DCMP_GREATEREQUAL;
	m_uZBufferModes[U2ZBufferState::CF_NEVER] = D3DCMP_NEVER;

	m_bForceNormalizeNormals = false;

	m_fCameraNear = 0.0f;
	m_fCameraFar = 0.0f;
	m_fCameraDepthRange = 0.0f;
	m_fMaxFogFactor = 0.0f;
	m_fMaxFogValue = 1.0f;
	
	memcpy((void*)&m_d3dCaps9, (const void*)&d3dCaps9, sizeof(m_d3dCaps9));
}


U2Dx9RenderStateMgr::~U2Dx9RenderStateMgr()
{
	m_pRenderer = NULL;
	if(m_pD3DDev)
	{
		m_pD3DDev->Release();
		m_pD3DDev = NULL;
	}
}


U2Dx9RenderStateMgr* U2Dx9RenderStateMgr::Create(U2Dx9Renderer* pRenderer, bool bZBuffer)
{
	U2Dx9RenderStateMgr* pRSMgr = U2_NEW U2Dx9RenderStateMgr(pRenderer, 
		*pRenderer->GetDeviceCaps());

	// CheckCaps()

	for(uint32 i = 0; i < MAX_RENDER_STATES; ++i)
	{
		if(ms_initRenderStates[i].m_eType == (D3DRENDERSTATETYPE)0xffffffff)
		{
			break;	
		}
		else if(ms_initRenderStates[i].m_eType == D3DRS_ZENABLE)
		{
			ms_initRenderStates[i].m_uVal = (bZBuffer ? D3DZB_TRUE : D3DZB_FALSE);
		}

	}

	pRSMgr->m_spShaderConstMgr = 
		U2Dx9ShaderConstantMgr::Create(pRenderer, pRSMgr->m_d3dCaps9);

	pRSMgr->InitAllStates();

	return pRSMgr;
}


void U2Dx9RenderStateMgr::InitRenderState()
{
	RenderStateInitValue *pInit = &ms_initRenderStates[0];

	uint32 idx = 0;
	while(pInit->m_eType != 0xffffffff)
	{
		m_RSBlocks[pInit->m_eType].InitValue(pInit->m_uVal);
		m_pD3DDev->SetRenderState(pInit->m_eType, pInit->m_uVal);

		idx++;
		pInit = &ms_initRenderStates[idx];
		U2ASSERT(pInit);
	}
}


void U2Dx9RenderStateMgr::SaveRenderState()
{
	for(uint32 i = 0; i < MAX_RENDER_STATES; ++i)
	{
		m_RSBlocks[i].SaveValue();
	}
}


void U2Dx9RenderStateMgr::RestoreRenderState()
{
	for(uint32 i = 0; i < MAX_RENDER_STATES; ++i)
	{
		RestoreRenderState((D3DRENDERSTATETYPE)i);
	}
}


void U2Dx9RenderStateMgr::InitTextureStageState()
{
	for(uint32 i = 0; i < MAX_TEXBLEND_STAGES; ++i)
	{
		uint32 idx = 0;

		TextureStageInitValue* pTSSInit = &ms_initTextureStages[0];

		D3DTEXTURESTAGESTATETYPE eTSSType = pTSSInit->m_eType;
		while(eTSSType != 0xffffffff)
		{
			if(eTSSType == D3DTSS_TEXCOORDINDEX)
			{
				pTSSInit->m_uVal = i;
			}
			else if(eTSSType == D3DTSS_COLOROP || eTSSType == D3DTSS_ALPHAOP)
			{
				if(i != 0)
					pTSSInit->m_uVal = D3DTOP_DISABLE;
			}

			m_TSRSBlocks[i][eTSSType].InitValue(pTSSInit->m_uVal);
			m_pD3DDev->SetTextureStageState(i, eTSSType, pTSSInit->m_uVal);

			idx++;
			pTSSInit = &ms_initTextureStages[idx];
			U2ASSERT(pTSSInit);
			eTSSType = pTSSInit->m_eType;
		}
	}

	for(unsigned int i = 0; i < MAX_SAMPLERS; ++i)
	{
		unsigned int idx = 0;
		
		SamplerStateInitValue* pSSInit = &ms_initSamplerStates[0];

		D3DSAMPLERSTATETYPE eSSType = pSSInit->m_eType;
		while(eSSType != 0xffffffff)
		{
			m_SSRSBlocks[i][eSSType].InitValue(pSSInit->m_uVal);
			m_pD3DDev->SetSamplerState(i, eSSType, pSSInit->m_uVal);

			idx++;
			pSSInit = &ms_initSamplerStates[idx];
			U2ASSERT(pSSInit);
			eSSType = pSSInit->m_eType;
		}		
	}	
}


void U2Dx9RenderStateMgr::SaveTextureStageState()
{
	for(unsigned int i = 0; i < MAX_TEXBLEND_STAGES; ++i)
	{
		unsigned int j = 0;
		for(;j < MAX_TEXSTAGE_STATES; ++j)
		{
			m_TSRSBlocks[i][j].SaveValue();
		}
		for(j =0; j < MAX_SAMPLER_STATES; ++j)
		{
			m_TSRSBlocks[i][j].SaveValue();
		}
	}
}


void U2Dx9RenderStateMgr::RestoreTextureStageState()
{
	for(unsigned int i = 0; i < MAX_TEXBLEND_STAGES; ++i)
	{
		unsigned int j = 0;
		for(;j < MAX_TEXSTAGE_STATES; ++j)
		{
			RestoreTextureStageState(i, (D3DTEXTURESTAGESTATETYPE)j);
		}
		for(j =0; j < MAX_SAMPLER_STATES; ++j)
		{
			RestoreSamplerState(i, (D3DSAMPLERSTATETYPE)j);
		}
	}
}


void U2Dx9RenderStateMgr::InitShaderState()
{
	m_bVertDecl = false;
	m_uCurrFVF = 0;
	m_uPrevFVF = 1;
	m_pCurrVertDecl = (LPDIRECT3DVERTEXDECLARATION9)0;
	m_pPrevVertDecl = (LPDIRECT3DVERTEXDECLARATION9)1;
	m_pCurrVS = (LPDIRECT3DVERTEXSHADER9)0;
	m_pPrevVS = (LPDIRECT3DVERTEXSHADER9)1;
	m_pCurrPS = (LPDIRECT3DPIXELSHADER9)0;
	m_pPrevPS = (LPDIRECT3DPIXELSHADER9)1;

}


void U2Dx9RenderStateMgr::SaveShaderState()
{
	m_uPrevFVF = m_uCurrFVF;
	m_pPrevVertDecl = m_pCurrVertDecl;
	m_pPrevVS = m_pCurrVS;
	m_pPrevPS = m_pCurrPS;

}


void U2Dx9RenderStateMgr::RestoreShaderState()
{
	RestoreFVF();
	RestoreVertDecl();

	RestorePixelShader();
	RestoreVertexShader();
}

unsigned int U2Dx9RenderStateMgr::GetSamplerStateMapping(unsigned int uSamplerState)
{
	if(uSamplerState < MAX_D3D_SAMPLERS)
		return ms_uSSMappings[uSamplerState];
	else 
		return 0xffffffff;	
}


void U2Dx9RenderStateMgr::InitAllStates()
{
	InitRenderState();
	InitShaderState();
	InitTextureStageState();
	InitTextures();
	
	SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);	// default
	SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
	SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	SetRenderState(D3DRS_SRCBLEND, 
		m_uAlphaBlendModes[U2AlphaState::BLEND_SRC_ALPHA]);
	SetRenderState(D3DRS_DESTBLEND, 
		m_uAlphaBlendModes[U2AlphaState::BLEND_ONE_MINUS_SRC_ALPHA]);
	SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_ALWAYS);
	SetRenderState(D3DRS_ALPHAREF, 0);
	SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
	SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	SetRenderState(D3DRS_DITHERENABLE, FALSE);
	SetRenderState(D3DRS_SPECULARENABLE, FALSE);
	SetRenderState(D3DRS_FOGENABLE, FALSE);
	SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_NONE);
	SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR);
	SetRenderState(D3DRS_RANGEFOGENABLE, FALSE);
	SetRenderState(D3DRS_FOGCOLOR, 0);
	SetRenderState(D3DRS_NORMALIZENORMALS, TRUE);
	SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
	SetRenderState(D3DRS_COLORVERTEX, TRUE);

}


void	U2Dx9RenderStateMgr::Reset(const D3DCAPS9& d3dCaps9, bool bZBuffer)
{
	memcpy((void*)&m_d3dCaps9, (const void*)&d3dCaps9,
		sizeof(m_d3dCaps9));


	// Initial render state values
	for (unsigned int i = 0; i < MAX_RENDER_STATES; i++)
	{
		if (ms_initRenderStates[i].m_eType == 
			(D3DRENDERSTATETYPE)0xFFFFFFFF)
		{
			break;
		}
		else if (ms_initRenderStates[i].m_eType == D3DRS_ZENABLE)
		{
			ms_initRenderStates[i].m_uVal = 
				(bZBuffer ? D3DZB_TRUE : D3DZB_FALSE);
		}
	}

	m_spShaderConstMgr = U2Dx9ShaderConstantMgr::Create(m_pRenderer, d3dCaps9);

	InitAllStates();
}


void U2Dx9RenderStateMgr::UpdateRenderState(const U2RenderStateCollection* pRSC)
{	
	SetAlphaState(pRSC->GetAlpha());
	SetZBufferState(pRSC->GetZBuffer());
	SetStencilState(pRSC->GetStencil());
	SetMaterialState(pRSC->GetMaterial());	
	SetWireframeState(pRSC->GetWireframe());
}



void U2Dx9RenderStateMgr::SetAlphaState(const U2AlphaState* pState)
{
	if(!pState)
		return;

	bool bNewAlpha = pState->GetAlphaBlending();
	if(bNewAlpha)
	{
		SetRenderState(D3DRS_ALPHABLENDENABLE, true);
		SetRenderState(D3DRS_SRCBLEND, 
			m_uAlphaBlendModes[pState->GetSrcBlendMode()]);
		SetRenderState(D3DRS_DESTBLEND, 
			m_uAlphaBlendModes[pState->GetDestBlendMode()]);
	}
	else
	{
		SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	}

	if (pState->GetAlphaTesting())
	{
		SetRenderState(D3DRS_ALPHATESTENABLE, true);
		SetRenderState(D3DRS_ALPHAFUNC, 
			m_uAlphaBlendModes[pState->GetTestMode()]);
		SetRenderState(D3DRS_ALPHAREF, (unsigned int)pState->GetTestRef());
	}
	else
	{
		SetRenderState(D3DRS_ALPHATESTENABLE, false);
	}

}


void U2Dx9RenderStateMgr::SetMaterialState(U2MaterialState * pState)
{
	if(!pState)
		return;
	m_matMgr.SetCurrMaterial(*pState);
	m_matMgr.UpdateToDevice(m_pD3DDev);
}


void U2Dx9RenderStateMgr::SetStencilState(const U2StencilState* pState)
{
	if(!pState)
		return;

	if(pState->GetStencilFlag())
	{
		SetRenderState(D3DRS_STENCILENABLE, TRUE);
		SetRenderState(D3DRS_STENCILFUNC, 
			m_uStencilTestModes[pState->GetStencilFunc()]);
		SetRenderState(D3DRS_STENCILREF, pState->GetStencilReference());
		SetRenderState(D3DRS_STENCILMASK, pState->GetStencilMask());
		SetRenderState(D3DRS_STENCILFAIL, 
			m_uSteinlOps[pState->GetStencilFailOp()]);
		SetRenderState(D3DRS_STENCILZFAIL, 
			m_uSteinlOps[pState->GetStencilZFailOp()]);
		SetRenderState(D3DRS_STENCILPASS, 
			m_uSteinlOps[pState->GetStencilZPassOp()]);
	}
	else
	{
		SetRenderState(D3DRS_STENCILENABLE, FALSE);
	}

	// CullMode 	
	SetRenderState(D3DRS_CULLMODE, m_uCullModes[pState->GetCullMode()]);
}


void U2Dx9RenderStateMgr::SetZBufferState(const U2ZBufferState* pState)
{
	if(!pState)
		return;

	if (pState->GetZEnable() || pState->GetZWritable())
	{
		SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);

		if (pState->GetZEnable())
		{
			SetRenderState(D3DRS_ZFUNC, 
				m_uZBufferModes[pState->GetZTestFunc()]);
		}
		else
		{
			SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
		}
	}
	else
	{
		SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
	}

	SetRenderState(D3DRS_ZWRITEENABLE, pState->GetZWritable());

}


void U2Dx9RenderStateMgr::SetWireframeState(const U2WireframeState* pState)
{
	if(!pState)
		return;

	SetRenderState(D3DRS_FILLMODE, 
		pState->GetWireFrame() ? D3DFILL_WIREFRAME : D3DFILL_SOLID);
}
