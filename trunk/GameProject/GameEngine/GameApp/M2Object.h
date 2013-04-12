#pragma once 

#include <ModelLib/M2Loader.h>


class M2Object : public M2Loader
{
public:
	M2Object(Renderer* _renderer, class WowActor* owner);
	~M2Object();

	virtual void CreateRenderPass();	
	
	void DrawShadow(const mat4& world, const mat4& view, const mat4& proj);
	void DrawZOnly(const noMat4& world, const mat4& view, const mat4& proj);
	
		
	virtual void DrawModel(const mat4& world, const mat4& view, const mat4& proj);
	void ToggleBlendState(int index, int blendIndex);
	void update(size_t deltaTime, size_t accumTime);
	void DrawShadowModel( const mat4& world, const mat4& view, const mat4& proj );
	void DrawCascaded();
	void ComputeTangetSpace();
	void LoadModelData();
	void LoadModelData2();
	void DrawSSAO( const mat4& world, const mat4& view, const mat4& proj );
	WowActor* Owner_;
		
		
	noVec3* Tangents_;
	noVec3* Binormals_;
};