#ifndef FRAMEWORK_ASSETCONVERTER_H
#define FRAMEWORK_ASSETCONVERTER_H

class noRenderSystemDX9;

class noAssetConverter {
public:

	struct SkinPaletteMap
	{
		//HKG_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( SkinPaletteMap);

		SkinPaletteMap() : m_skin(HK_NULL) { }
		class noRenderModel* m_skin;
		hkArray< class hkgBlendMatrixSet* > m_palettes;
	};

	struct IndexMapping
	{
		hkInt16* m_mapping;
		hkInt32 m_numMapping;
	};

	void setupHardwareSkin( noRenderSystemDX9* renderSystem, 
		noRenderModel* model, 
		IndexMapping* mappings,
		hkInt32 numMappings, 
		hkInt16 fullPaletteSize, hkBool allowCulling);

	static void ConvertM2();

};



#endif