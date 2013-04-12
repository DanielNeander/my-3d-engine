#include "stdafx.h"
#include "Model_M2Format.h"
#include "M2Loader.h"

extern int gameVersion;

bool ModelRenderPass::init( M2Loader *m )
{
	if (!m->showGeosets[geoset])
		return false;

	// COLOUR
	// Get the colour and transparency and check that we should even render


	/*uint32 bindtex = 0;
	if (m->specialTextures[tex]==-1) 
		bindtex = m->textureIds[tex];
	else 
		bindtex = m->replaceTextures[m->specialTextures[tex]];*/


	//// Texture
	//// ALPHA BLENDING
	//// blend mode
	//switch (blendmode) {
	//	case BM_OPAQUE:	// 0
	//		break;
	//	case BM_TRANSPARENT: // 1
	//		//glEnable(GL_ALPHA_TEST);
	//		//glAlphaFunc(GL_GEQUAL,0.7f); // Dx10 ÀÌÈÄ Alpha Test ¾ø¾îÁü. 
	//		break;
	//	case BM_ALPHA_BLEND:
	//		m->renderer->addBlendState(D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, 
	//			D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, 0, D3D11_BLEND_OP_ADD);
	//		break;
	//	case BM_ADDITIVE: // 3
	//		m->renderer->addBlendState(D3D11_BLEND_SRC_COLOR, D3D11_BLEND_ONE, 
	//			D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, 0, D3D11_BLEND_OP_ADD);
	//		break;
	//	case BM_ADDITIVE_ALPHA:
	//		m->renderer->addBlendState(D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, 
	//			D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_ONE, 0, D3D11_BLEND_OP_ADD);
	//		break;
	//	case BM_MODULATE:
	//		m->renderer->addBlendState(D3D11_BLEND_DEST_COLOR, D3D11_BLEND_SRC_COLOR, 
	//			D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, 0, D3D11_BLEND_OP_ADD);
	//		break;
	//	case BM_MODULATEX2:	// 6, not sure if this is right
	//		m->renderer->addBlendState(D3D11_BLEND_DEST_COLOR, D3D11_BLEND_SRC_COLOR, 
	//			D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, 0, D3D11_BLEND_OP_ADD);
	//		break;
	//	default:
	//		m->renderer->addBlendState(D3D11_BLEND_DEST_COLOR, D3D11_BLEND_SRC_COLOR, 
	//			D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, 0, D3D11_BLEND_OP_ADD);
	//		break;
	//}

	//if (noZWrite)
	//	m->renderer->addDepthState(true, false);


	if (unlit)
		;

	if (blendmode<=1 && ocol.w<1.0f)
		;//glEnable(GL_BLEND);

	return true;
}
