#pragma once 

#include "clipMapImageSource.h"
#include "clipMapCache.h"

class ClipMapBlenderCache : public IClipMapImageCache
{
public:
	ClipMapBlenderCache(IClipMapImageSource *opacitySource, IClipMapImageSource *lightmapSource, bool LM1 = false);
	ClipMapBlenderCache(IClipMapImageSource *lightmapSource);
	virtual ~ClipMapBlenderCache();

	virtual void initialize(ClipMap *cm);
	virtual void setInterestCenter(const Point2I &origin);
	virtual bool beginRectUpdates(ClipMap::ClipStackEntry &cse);
	virtual void doRectUpdate(U32 mipLevel, ClipMap::ClipStackEntry &cse, const RectI &srcRegion, const RectI &dstRegion);
	virtual void finishRectUpdates(ClipMap::ClipStackEntry &cse);
	virtual bool isDataAvailable(U32 mipLevel, const RectI &region) const;
	virtual bool isRenderToTargetCache();

	void clearSourceImages();
	void registerSourceImage(const String &imgPath);

	void clearOpacitySources();
	void registerOpacitySource(IClipMapImageSource *opacitySource);

	void createOpacityScratchTextures();

	// Lightmap support
	void setNewLightmapSource(IClipMapImageSource *lightmapSource);

private:
	/// References to image source for opacity and lightmap information.
	std::vector<IClipMapImageSource*> mOpacitySources;
	std::vector<IClipMapImageSource*> mLightmapSources;

	Point4F     mTempScaleFactors;
	std::vector<F32> mScales;
	std::vector<F32> mSourceWidths;

	/// Used for streaming video data to card for blend operations.
	std::vector<CTexture*> mOpacityScratchTextures;
	std::vector<CTexture*> mLightmapScratchTextures;

	/// Whether LM1 (lightmap blend factor 1) shaders should be used.
	bool mLM1;

	/// Whether mOpacityScratchTexturesFF should be used, amongst other things
	bool mFixedfunction;

	/// Shared shader constant blocks
	GFXShaderConstBufferRef mShaderConsts;
	GFXShaderConstHandle* mModelViewProjSC;
	GFXShaderConstHandle* mOpacityMapSC;
	GFXShaderConstHandle* mLightMapSC;
	GFXShaderConstHandle* mTex1SC;
	GFXShaderConstHandle* mTex2SC;
	GFXShaderConstHandle* mTex3SC;
	GFXShaderConstHandle* mTex4SC;
	GFXShaderConstHandle* mSourceTexScalesSC;

	/// SM2.0 shader used for one pass blending.
	GFXShader *mOnePass;
	GFXStateBlockRef mOnePassSB;
		

	/// FF
	GFXStateBlockRef mFFBaseLayerSB;
	GFXStateBlockRef mFFAdditionalLayersSB;
	GFXStateBlockRef mFFLightmapSB;

	/// Images that we're blending; in channel-usage order.
	std::vector<CTexture*> mSourceImages;

	U32 mClipMapSize;
	ClipMap *mOwningClipMap;

	/// Generate appropriate volatile geometry and set it as the current VB
	void setupGeometry( const RectF &srcRect0, const RectF &srcRect1,
		const RectF &masterCoords, const RectI &dstRect);
	void setupGeometryFF( const RectF &srcRect0, const RectF &masterCoords,
		const F32 scaleFactor, const RectI &dstRect);
};
