#pragma once 

#include "ModelLib/database.h"

class WowActor;
class M2Loader;

class M2Equipment  
{
public:
	M2Equipment(M2Loader* m2, int id, int slot, float scale, float rot, noVec3 pos);
	~M2Equipment();

	int id_;
	int slot_;		
	class M2Mesh* data_;
};

enum WeaponType 
{
	WEAPONT_1H,
	WEAPONT_2H,	
	WEAPONT_LH,	// Hand
	WEAPONT_RH,	// Hand	
};

enum WeaponKind {

	WEAPONK_AXE,
	WEAPONK_BOW,
	WEAPONK_HAND,
	WEAPONK_RIFLE,	
	WEAPONK_SWORD,	

};

class M2EquipSystem
{
public:
	M2EquipSystem(class M2ModelEdit* pEdit, WowActor* owner, Renderer* renderer);;
	~M2EquipSystem();
	void RefreshEquip(int nSlot);
		
	M2Equipment* AddEquipment(M2Loader* m2, int id, int slot, float scale = 1.0f, float rot = 0.0f, noVec3 pos = noVec3(0.f, 0.f, 0.f));
	M2Equipment* AddEquipment(wxString modelfn, int id, int slot, float scale = 1.0f, float rot = 0.0f, noVec3 pos = noVec3(0.f, 0.f, 0.f));
	M2Equipment* AddEquipment(M2Mesh* pParent, wxString modelfn, int id, int slot, float scale = 1.0f, float rot = 0.0f, noVec3 pos = noVec3(0.f, 0.f, 0.f));
	M2Equipment* AddEquipment( M2Mesh* pParent, M2Loader* m2, int id, int slot, float scale = 1.0f, float rot = 0.0f, noVec3 pos = noVec3(0.f, 0.f, 0.f));
	void RemoveEquipment(int nSlot);

	int GetWeaponType() const { return weaponType_; }
	int GetWeaponKind() const { return weaponKind_; }

	void CreateRenderPass();

	void EquipShield();
	void EquipWeapon1h();
	void EquipSword2h();
	void EquipRifle();
	void Execute();
	void AnalysizeWeaponType(const wxString& weaponname);
	void LoadAndAttachEquip(const ItemDisplayDB::Record& r, const wxString& path, int id1, int slot);
	void EquipBow();
	int weaponType_;
	int weaponKind_;

	std::vector<M2Equipment*> equips_;

	bool bHideHelmet;
	bool bSheathe;
	bool hasShield_;

	M2Mesh*		pMesh_;
	M2ModelEdit* pEdit_;
	Renderer* renderer_;
	WowActor*	Owner_;


	SceneNode* RightHandBone_;
	SceneNode* LeftHandBone_;
};