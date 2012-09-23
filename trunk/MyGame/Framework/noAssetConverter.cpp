#include "DXUT.h"
#include <Renderer/noRenderSystemDX9.h>
#include "noAssetConverter.h"

void noAssetConverter::setupHardwareSkin( noRenderSystemDX9* renderSystem, noRenderModel* model, IndexMapping* mappings, hkInt32 numMappings, hkInt16 fullPaletteSize, hkBool allowCulling )
{
	//SkinPaletteMap* smap = new SkinPaletteMap;
	//smap->m_skin = dispObj;

	//// trivial binding (just one pallete, same size and order as main pallete)
	//float ident[16]; hkgMat4Identity(ident);
	//int singlePaletteSize = (numMappings == 0)? fullPaletteSize : ((numMappings == 1)? mappings[0].m_numMapping : 0);
	//noBlendMatrixSet* mainPalette = HK_NULL;
	//if (singlePaletteSize > 0)
	//{
	//	mainPalette = noBlendMatrixSet::create( context );
	//	for (int i=0; i < singlePaletteSize; ++i)
	//	{
	//		mainPalette->addMatrix(ident, (numMappings == 0)? i : mappings[0].m_mapping[i] );
	//	}
	//	smap->m_palettes.pushBack(mainPalette);
	//}


	
}
