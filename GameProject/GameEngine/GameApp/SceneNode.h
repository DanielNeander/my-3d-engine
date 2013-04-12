#pragma once 

#include <EngineCore/RefCount.h>

class SceneNode : public RefCounter
{
public:
	enum TransformSpace
	{
		/// Transform is relative to the local space
		TS_LOCAL,
		/// Transform is relative to the space of the parent node
		TS_PARENT,
		/// Transform is relative to world space
		TS_WORLD
	};

	SceneNode();
	~SceneNode();
	
	static SceneNode* NewNode();
		
	noTransform localTM_;	
	noTransform worldTM_;
	

	void SetTrans(const noVec3& in) { localTM_.trans_ = in; localTM_.isDirty = true; }
	void SetRotate(const noMat3& in) { localTM_.rotate_ = in; localTM_.isDirty = true;}
	const noVec3& GetTrans() const { return localTM_.trans_; }
	const noMat3& GetRotate() const { return localTM_.rotate_; }
	const float	  GetScale() const { return localTM_.scale_; }
	const noVec3& WorldTrans() const { return worldTM_.trans_; }
	const noMat3& WorldRotate() const { return worldTM_.rotate_; }
	const float	  WorldScale() const { return worldTM_.scale_; }

	void SetName(const char* name) { name_ = name; }

	SceneNode* GetChildByName(const char* name) const;

	void setVisible(bool bVisible);
	bool IsVisible() const { return visible_; }

	void SetScale(float scale) { localTM_.scale_ = scale; }		
	void AddChild(SceneNode* pChild);
	
	virtual void PreUpdate() {}
	void Update(float fDelta);
	virtual void PostUpdate() {}
	void UpdateDownward();
	void UpdateUpward();

	SceneNode* GetParent() const { return parent_; }
	
	virtual void UpdateTM();
	void SetDir(const noVec3& vec, TransformSpace relativeTo, const noVec3& localDirectionVector);
	SceneNode* parent_;
	std::vector<SceneNode*> childNodes_;

	std::string name_;

	BoundingBox bbox_;
	bool visible_;

	bool IsInheritTM;

protected:
	virtual void UpdateData() {}

};

MSmartPointer(SceneNode);