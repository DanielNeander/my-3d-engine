#ifndef FRAMEWORK_SCENE_H
#define FRAMEWORK_SCENE_H

class noViewport3D;
//class noD3DResource;
class noVec3;
class noTexture2D;
class noLinearColor;

class noSceneDrawInterface {
public:
	const noViewport3D* const m_viewport;

	noSceneDrawInterface(const noViewport3D* view) :
	m_viewport(view) {}

	virtual BOOL HitTest() = 0;
	
	virtual void DrawSprite(
		const noVec3& pos, 
		float sizeX, 
		float sizeY, 
		const noTexture2D* sprite,
		const noLinearColor& color 
		) = 0;	
};


#endif