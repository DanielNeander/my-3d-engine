#pragma once 


//#define terrain_geometry_scale				1.0f
#define terrain_maxheight					30.0f 
#define terrain_minheight					-30.0f 

#define shadowmap_resource_buffer_size_xy				4096
#define water_normalmap_resource_buffer_size_xy			2048
#define	 terrain_layerdef_map_texture_size				128
#define terrain_depth_shadow_map_texture_size			512


#define terrain_height_underwater_start		-100.0f
#define terrain_height_underwater_end		-8.0f
#define terrain_height_sand_start			-30.0f
#define terrain_height_sand_end				1.7f
#define terrain_height_grass_start			1.7f
#define terrain_height_grass_end			30.0f
#define terrain_height_rocks_start			-2.0f
#define terrain_height_trees_start			4.0f
#define terrain_height_trees_end			30.0f
#define terrain_slope_grass_start			0.96f
#define terrain_slope_rocks_start			0.85f

#define main_buffer_size_multiplier			1.0f //1.1f
#define reflection_buffer_size_multiplier   1.0f //1.1f
#define refraction_buffer_size_multiplier   1.0f //1.1f

struct TerrainShaderParam {
	idStr BindShaderVarName;
	idStr BindShaderMapName;
};


class SNBTerrainMaterial {

public:
	SNBTerrainMaterial();
	virtual ~SNBTerrainMaterial();
	void	Destory();
	void	SetDetailMapName(const char* texname) { DetailMapName.BindShaderMapName = texname; }
	void	SetNormalMapName(const char* texname) { NormalMapName.BindShaderMapName = texname; }
	void	SetNoiseMapName(const char* texname) { NoiseMapName.BindShaderMapName = texname; }
	void	SetDispMapName(const char* texname) { DispMapName.BindShaderMapName = texname; }

	bool	Init(ID3DX11Effect* effect);

	void CommitChanges();
	void Update(float fDeltaTime);
	void CreateOpacityMap(const BYTE* layerdef_map_texture_pixels);
	void CreateDepthMap(byte * depth_shadow_map_texture_pixels);
	void ReCreateBuffers();
	void CommitCommonShaderConstants();
	void CommitPerInstanceShaderConstants();

	void SetupLightView(class BaseCamera *cam);
	void EndPass();
	
	void RenderPass_Caustics(ID3D11InputLayout* trianglestrip_inputlayout, ID3D11Buffer* pathVB);
	void SetupNormalView(BaseCamera *cam);
	void RenderPass_Reflection();
	void RenderPass_Reraction();
	void RenderPass_Shadow();
	void RenderPass_Main();
	void EndPass_Main();
	static const int MAX_TEX_LAYER = 4;
		
	bool									Initialized;
	ID3DX11Effect*							Effect;
	class Clipmap*							clipmap[MAX_TEX_LAYER];
	ID3DX11EffectShaderResourceVariable*	DetailMapVar;
	ID3DX11EffectShaderResourceVariable*	NormalMapVar;
	ID3DX11EffectShaderResourceVariable*	NoiseMapVar;
	ID3DX11EffectShaderResourceVariable*	DispMapVar;

	CComPtr<ID3D11Texture2D>	layerdef_texture;
	CComPtr<ID3D11ShaderResourceView> layerdef_textureSRV;

	CComPtr<ID3D11Texture2D>	depthmap_texture;
	CComPtr<ID3D11ShaderResourceView> depthmap_texSRV;		;
	
	ID3DX11EffectShaderResourceVariable*	DiffuseMap1Var;
	ID3DX11EffectShaderResourceVariable*	 BumpMap1Var;
	ID3DX11EffectShaderResourceVariable*	DiffuseMap2Var;
	ID3DX11EffectShaderResourceVariable*	 BumpMap2Var;
	ID3DX11EffectShaderResourceVariable*	DiffuseMap3Var;
	ID3DX11EffectShaderResourceVariable*	 BumpMap3Var;
	ID3DX11EffectShaderResourceVariable*	DiffuseMap4Var;
	ID3DX11EffectShaderResourceVariable*	 BumpMap4Var;

	CComPtr<ID3D11ShaderResourceView>		DiffuseMap1SRV;
	CComPtr<ID3D11ShaderResourceView>		DiffuseMap2SRV;
	CComPtr<ID3D11ShaderResourceView>		DiffuseMap3SRV;
	CComPtr<ID3D11ShaderResourceView>		DiffuseMap4SRV;

	CComPtr<ID3D11ShaderResourceView>		BumpMap1SRV;
	CComPtr<ID3D11ShaderResourceView>		BumpMap2SRV;
	CComPtr<ID3D11ShaderResourceView>		BumpMap3SRV;
	CComPtr<ID3D11ShaderResourceView>		BumpMap4SRV;


	ID3DX11EffectShaderResourceVariable*	 SlopeMapVar;
	CComPtr<ID3D11ShaderResourceView>		SlopeTexSRV;



	TerrainShaderParam SplattSrcMap[MAX_TEX_LAYER];
	TerrainShaderParam	SplattBumpMap[MAX_TEX_LAYER];
	TerrainShaderParam	DetailMapName;
	TerrainShaderParam	NormalMapName;
	TerrainShaderParam	NoiseMapName;
	TerrainShaderParam	DispMapName;
	TerrainShaderParam	SlopeMapName;
	
	float BackbufferWidth;
	float BackbufferHeight;

	UINT MultiSampleCount;
	UINT MultiSampleQuality;

	int TerrainFarRange;
	float	terrain_geometry_scale;

	ID3D11Texture2D			 *reflection_color_resource;
	ID3D11ShaderResourceView *reflection_color_resourceSRV;
	ID3D11RenderTargetView   *reflection_color_resourceRTV;
	ID3D11Texture2D			 *refraction_color_resource;
	ID3D11ShaderResourceView *refraction_color_resourceSRV;
	ID3D11RenderTargetView   *refraction_color_resourceRTV;
		
		
	ID3D11Texture2D			 *shadowmap_resource;
	ID3D11ShaderResourceView *shadowmap_resourceSRV;
	ID3D11DepthStencilView   *shadowmap_resourceDSV;

	ID3D11Texture2D			 *reflection_depth_resource;
	ID3D11DepthStencilView   *reflection_depth_resourceDSV;


	ID3D11Texture2D			 *refraction_depth_resource;
	ID3D11RenderTargetView   *refraction_depth_resourceRTV;
	ID3D11ShaderResourceView *refraction_depth_resourceSRV;

	ID3D11Texture2D			 *water_normalmap_resource;
	ID3D11ShaderResourceView *water_normalmap_resourceSRV;
	ID3D11RenderTargetView   *water_normalmap_resourceRTV;

	ID3D11Texture2D			 *main_color_resource;
	ID3D11ShaderResourceView *main_color_resourceSRV;
	ID3D11RenderTargetView   *main_color_resourceRTV;
	ID3D11Texture2D			 *main_depth_resource;
	ID3D11DepthStencilView   *main_depth_resourceDSV;
	ID3D11ShaderResourceView *main_depth_resourceSRV;
	ID3D11Texture2D			 *main_color_resource_resolved;
	ID3D11ShaderResourceView *main_color_resource_resolvedSRV;


};