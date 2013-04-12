#include "stdafx.h"
#include "EngineCore/Util/GameLog.h"
#include "GameApp/GameApp.h"

#include "SceneNode.h"
#include "Mesh.h"
#include "M2Mesh.h"
#include "Common.h"

#include "ModelLib/M2Loader.h"
#include "ModelLib/M2Particle.h"
#include "M2Object.h"
#include "TestObjects.h"

#include "M2Equipment.h"

static const char* WeaponTypeNames[] = {
	"Axe",
	"Bow",
	"Club",
	"Firearm",
	"Glave",
	"Hammer",
	"Hand",
	"Knife",
	"Mace",
	"Misc",
	"Offhand",
	"Polearm",
	"Shield",
	"Staff",
	"Stave",
	"Sword",
	"Thrown",
	"Totem",
	"Wand"
};



#define Slice(str, start, end) (str.Mid(start, end-start))
//Split (Tokenize) string at specified intervals
//s == string to split
//retArray == split up string (out)
//cpszExp == expression to split at
//crnStart == start postion to split 
//crnCount == max number of split of strings
//crbCIComp == true if case insensitive
static void Split(  const wxString& s, wxArrayString& retArray,  const wxChar* cpszExp,  
	const size_t& crnStart = 0,    const size_t& crnCount = (size_t)-1, 
	const bool& crbCIComp = false)
{
	//sanity checks
	wxASSERT_MSG(cpszExp != NULL, wxT("Invalid value for First Param of wxString::Split (cpszExp)"));
	//    wxASSERT_MSG(crnCount >= (size_t)-1, wxT("Invalid value for Third Param of wxString::Split (crnCount)"));

	retArray.Clear();

	size_t  nOldPos = crnStart,      //Current start position in this string
		nPos = crnStart;      //Current end position in this string 

	wxString szComp,            //this string as-is (if bCIComp is false) or converted to lowercase
		szExp = cpszExp;   //Expression string, normal or lowercase

	if (crbCIComp)
	{
		szComp = s.Lower();
		szExp.MakeLower();
	}
	else
		szComp = s;

	if(crnCount == (size_t)-1)
	{
		for (; (nPos = szComp.find(szExp, nPos)) != wxString::npos;)//Is there another token in the string 
		{
			retArray.Add(Slice(s, nOldPos, nPos)); //Insert the token in the array
			nOldPos = nPos += szExp.Length();//Move up the start slice position
		}

	}
	else
	{
		for (int i = crnCount; 
			(nPos = szComp.find(szExp, nPos)) != wxString::npos &&
			i != 0;
		--i)//Is there another token in the string && have we met nCount?
		{
			retArray.Add(Slice(s, nOldPos, nPos)); //Insert the token in the array
			nOldPos = nPos += szExp.Length();//Move up the start slice position
		}
	}
	if (nOldPos != s.Length())
		retArray.Add( Slice(s, nOldPos, s.Length()) ); //Add remaining characters in string
}

void M2EquipSystem::LoadAndAttachEquip( const ItemDisplayDB::Record& r, const wxString& path, int id1, int slot )
{
	TextureID tex;
	bool succ = false;
	M2Equipment *att = NULL;
	wxString mp;
	M2Loader *m = NULL;
	M2Loader* model = pMesh_->m2_;

	wxString quiverFname;
	switch(weaponType_)
	{
	case WEAPONT_1H:
		quiverFname = "Quiver_A.m2";
		break;
	case WEAPONT_2H:
		quiverFname = "Quiver_Bolt_A_01.m2";
		break;		
	}

	mp = path + quiverFname;

	if (mp.length()) {
		att = AddEquipment(mp, id1, slot);
		if (att) {
			m = att->data_->m2_;
			if (m) {
				mp = path + "QuiverSkin_A.tga";				
				tex = renderer_->addTexture(TexPath(mp.wx_str()).c_str(), true);
				for (size_t x=0;x<m->TextureList.size();x++){
					if (m->TextureList[x] == wxString(wxT("Special_2"))){
						wxLogMessage(wxT("Replacing ID1's %s with %s"),m->TextureList[x].c_str(),mp.c_str());
						m->TextureList[x] = mp;
					}
				}
				m->replaceTextures[TEXTURE_CAPE] = tex;							
				succ = true;

				// hardcode fix
				//wxLogMessage(model->name + wxT(" id1"));
				wxString modelname(model->name.c_str());
				if (modelname.CmpNoCase(wxT("character\\human\\male\\humanmale.m2")) == 0 ||
					modelname.CmpNoCase(wxT("character\\human\\female\\humanfemale.m2")) == 0 ||
					modelname.CmpNoCase(wxT("character\\gnome\\male\\gnomemale.m2")) == 0 ||
					modelname.CmpNoCase(wxT("character\\gnome\\female\\gnomefemale.m2")) == 0 ||
					modelname.CmpNoCase(wxT("character\\scourge\\male\\scourgemale.m2")) == 0 ||
					modelname.CmpNoCase(wxT("character\\scourge\\female\\scourgefemale.m2")) == 0 ||
					modelname.CmpNoCase(wxT("character\\dwarf\\male\\dwarfmale.m2")) == 0 ||
					modelname.CmpNoCase(wxT("character\\dwarf\\female\\dwarffemale.m2")) == 0 ||
					modelname.CmpNoCase(wxT("character\\tauren\\male\\taurenmale.m2")) == 0) {
						if (id1 == ATT_LEFT_BACK_SHEATH)
						{			
							noAngles angle(90.0, 135.0, 90.0);
							att->data_->localTM_.rotate_ = angle.ToMat3();
							//m->rot = noVec3(135.0, 90.0, 90.0);
						}
						else if (id1 == ATT_RIGHT_BACK_SHEATH)
						{
							noAngles angle(90.0, 225.0, 90.0);
							att->data_->localTM_.rotate_ = angle.ToMat3();
							//m->rot = noVec3(225.0, 90.0, 90.0);
						}
						else if (id1 == ATT_MIDDLE_BACK_SHEATH)
						{
							noAngles angle(90.0, 0.0,  90.0);
							att->data_->localTM_.rotate_ = angle.ToMat3();
							//m->rot = noVec3(0.0, 90.0, 90.0);
						}
				} else if (modelname.CmpNoCase(wxT("character\\nightelf\\female\\nightelffemale.m2")) == 0) {
					if (id1 == ATT_RIGHT_BACK_SHEATH)
					{
						noAngles angle(90.0, 225.0,  90.0);
						att->data_->localTM_.rotate_ = angle.ToMat3();
						//m->rot = noVec3(225.0, 90.0, 90.0);
					}
				} else if (modelname.CmpNoCase(wxT("character\\bloodelf\\female\\bloodelffemale")) == 0) {
					//if (id1 == ATT_MIDDLE_BACK_SHEATH)
					{
						att->data_->id_ = ATT_MIDDLE_BACK_SHEATH;
						noAngles angle(0.0f, 0.0f,  0.0f);
						att->data_->SetRotate(angle.ToMat3());												
						//m->rot = noVec3(0.0, 90.0, 90.0);
					}
				} else if (modelname.CmpNoCase(wxT("character\\tauren\\female\\taurenfemale.m2")) == 0) {
					if (id1 == ATT_LEFT_BACK_SHEATH)
					{
						noAngles angle(90.0, 135.0,  90.0);
						att->data_->localTM_.rotate_ = angle.ToMat3();
						//m->rot = noVec3(135.0, 90.0, 90.0);
					}
					if (id1 == ATT_MIDDLE_BACK_SHEATH)
					{
						noAngles angle(90.0, 0.0,  90.0);
						att->data_->localTM_.rotate_ = angle.ToMat3();
						//m->rot = noVec3(0.0, 90.0, 90.0);
					}
				}
			}
		}
	}
}


void M2EquipSystem::RefreshEquip( int slot )
{
	if (slot==CS_HEAD && bHideHelmet == true)
		return;

	RemoveEquipment( slot );

	M2ActorDetails& cd = pEdit_->cd;
	M2Loader* model = pMesh_->m2_;
	

	int itemnum = cd.equipment[slot];
	if (itemnum!=0) {
		// load new model(s)
		int id1=-1, id2=-1;
		wxString path;
		//float sc = 1.0f;
		if (slot==CS_HEAD) {
			id1 = ATT_HELMET;
			path = wxT("Item\\ObjectComponents\\Head\\");
		} else if (slot==CS_SHOULDER) {
			id1 = ATT_LEFT_SHOULDER;
			id2 = ATT_RIGHT_SHOULDER;
			path = wxT("Item\\ObjectComponents\\Shoulder\\");
		} else if (slot == CS_HAND_LEFT) {
			id1 = ATT_LEFT_PALM;
			model->charModelDetails.closeLHand = true;
		} else if (slot == CS_HAND_RIGHT) {
			id1 = ATT_RIGHT_PALM;
			model->charModelDetails.closeRHand = true;
		} else if (slot == CS_QUIVER) {
			id1 = ATT_RIGHT_BACK_SHEATH;
			path = wxT("Item\\ObjectComponents\\Quiver\\");
		} else 
			return;

		if (slot==CS_HAND_LEFT || slot==CS_HAND_RIGHT) {
			if (items.getById(itemnum).type == IT_SHIELD) {
				path = wxT("Item\\ObjectComponents\\Shield\\");
				id1 = ATT_LEFT_WRIST;
				hasShield_ = true;
			} else {
				path = wxT("Item\\ObjectComponents\\Weapon\\");
			}

			// If we're sheathing our weapons, relocate the items to
			// their correct positions
			if (bSheathe && items.getById(itemnum).sheath>SHEATHETYPE_NONE) {	
				id1 = items.getById(itemnum).sheath;

				if (slot == CS_HAND_LEFT) {
					// make the weapon cross
					if (id1 == ATT_LEFT_BACK_SHEATH)
						id1 = ATT_RIGHT_BACK_SHEATH;
					if (id1 == ATT_LEFT_BACK)
						id1 = ATT_RIGHT_BACK;
					if (id1 == ATT_LEFT_HIP_SHEATH)
						id1 = ATT_RIGHT_HIP_SHEATH;
					model->charModelDetails.closeLHand = false;
				} else
					model->charModelDetails.closeRHand = false;

				/* in itemcache.wdb & item.dbc
				0	 None	 Used on Armor, non-weapon items.
				1	 Angled Back	 Used on two-handed swords/axes, and some one-handers such as Thunderfury.
				2	 Upper Back	 Used on staffs and polearms, positioned higher and straighter.
				3	 Side	 Used on one-handed maces,swords,axes,daggers.
				4	 Back	 Used for shields
				5	 ?	
				6	 	
				7	 Invisible	 Used for fist weapons and offhands.
				*/
				/*
				26 = upper right back, two-handed sword(2:8) sheath1
				27 = upper left back, 
				28 = center back, shield(4:6), sheath4
				30 = upside down, upper left back -- staff(2:10) sheath2, spears
				32 = left hip, mace(2:4) sheath3, sword(2:7) sheath3
				33 = right hip
				*/
			}
		}

		try {

			// This corrects the problem with trying to automatically load equipment on NPC's
			int ItemID = 0;
			//if (g_canvas->model->modelType == MT_NPC)
			//	ItemID = itemnum;
			//else {
				const ItemRecord &item = items.getById(itemnum);
				ItemID = item.model;
			//}

			ItemDisplayDB::Record r = itemdisplaydb.getById(ItemID);
						
			TextureID tex;
			wxString mp;
			bool succ = false;
			M2Equipment *att = NULL;
			M2Loader *m = NULL;

			if (id1>=0) {
				wxString weaponFname = r.getString(ItemDisplayDB::Model);
				if (id1 != ATT_LEFT_WRIST && slot==CS_HAND_LEFT || slot==CS_HAND_RIGHT)
				{
					AnalysizeWeaponType(weaponFname);

					if (weaponKind_ == WEAPONK_BOW)
						LoadAndAttachEquip(r, wxString("Item\\ObjectComponents\\Quiver\\"), ATT_RIGHT_BACK_SHEATH, CS_QUIVER);
				}
				mp = (path + weaponFname);			

				if (slot==CS_HEAD) {
					// sigh, head items have more crap to sort out
					mp = mp.substr(0, mp.length()-4); // delete .mdx
					mp.append(wxT("_"));
					try {
						CharRacesDB::Record race = racedb.getById(cd.race);
						mp.append(race.getString(CharRacesDB::ShortName));
						mp.append(cd.gender?wxT("F"):wxT("M"));
						mp.append(wxT(".m2"));
					} catch (CharRacesDB::NotFound) {
						mp = wxT("");
					}
				}

				if (mp.length()) {
					att = AddEquipment(mp, id1, slot);
					if (att) {
						m = att->data_->m2_;
						if (m) {
							mp = (path + r.getString(ItemDisplayDB::Skin));
							mp.append(wxT(".tga"));
							tex = renderer_->addTexture(TexPath(mp.wx_str()).c_str(), true);
							for (size_t x=0;x<m->TextureList.size();x++){
								if (m->TextureList[x] == wxString(wxT("Special_2"))){
									wxLogMessage(wxT("Replacing ID1's %s with %s"),m->TextureList[x].c_str(),mp.c_str());
									m->TextureList[x] = mp;
								}
							}
							m->replaceTextures[TEXTURE_CAPE] = tex;							
							succ = true;

							// hardcode fix
							//wxLogMessage(model->name + wxT(" id1"));
							wxString modelname(model->name.c_str());
							if (modelname.CmpNoCase(wxT("character\\human\\male\\humanmale.m2")) == 0 ||
								modelname.CmpNoCase(wxT("character\\human\\female\\humanfemale.m2")) == 0 ||
								modelname.CmpNoCase(wxT("character\\gnome\\male\\gnomemale.m2")) == 0 ||
								modelname.CmpNoCase(wxT("character\\gnome\\female\\gnomefemale.m2")) == 0 ||
								modelname.CmpNoCase(wxT("character\\scourge\\male\\scourgemale.m2")) == 0 ||
								modelname.CmpNoCase(wxT("character\\scourge\\female\\scourgefemale.m2")) == 0 ||
								modelname.CmpNoCase(wxT("character\\dwarf\\male\\dwarfmale.m2")) == 0 ||
								modelname.CmpNoCase(wxT("character\\dwarf\\female\\dwarffemale.m2")) == 0 ||
								modelname.CmpNoCase(wxT("character\\tauren\\male\\taurenmale.m2")) == 0) {
									if (id1 == ATT_LEFT_BACK_SHEATH)
									{			
										noAngles angle(90.0, 135.0, 90.0);
										att->data_->localTM_.rotate_ = angle.ToMat3();
										//m->rot = noVec3(135.0, 90.0, 90.0);
									}
									else if (id1 == ATT_RIGHT_BACK_SHEATH)
									{
										noAngles angle(90.0, 225.0, 90.0);
										att->data_->localTM_.rotate_ = angle.ToMat3();
										//m->rot = noVec3(225.0, 90.0, 90.0);
									}
									else if (id1 == ATT_MIDDLE_BACK_SHEATH)
									{
										noAngles angle(90.0, 0.0,  90.0);
										att->data_->localTM_.rotate_ = angle.ToMat3();
										//m->rot = noVec3(0.0, 90.0, 90.0);
									}
							} else if (modelname.CmpNoCase(wxT("character\\nightelf\\female\\nightelffemale.m2")) == 0) {
								if (id1 == ATT_RIGHT_BACK_SHEATH)
								{
									noAngles angle(90.0, 225.0,  90.0);
									att->data_->localTM_.rotate_ = angle.ToMat3();
									//m->rot = noVec3(225.0, 90.0, 90.0);
								}
							} else if (modelname.CmpNoCase(wxT("character\\bloodelf\\female\\bloodelffemale.m2")) == 0) {
								if (id1 == ATT_MIDDLE_BACK_SHEATH)
								{
									noAngles angle(0.0, 90.0,  90.0);
									att->data_->localTM_.rotate_ = angle.ToMat3();
									//m->rot = noVec3(0.0, 90.0, 90.0);
								}
							} else if (modelname.CmpNoCase(wxT("character\\tauren\\female\\taurenfemale.m2")) == 0) {
								if (id1 == ATT_LEFT_BACK_SHEATH)
								{
									noAngles angle(90.0, 135.0,  90.0);
									att->data_->localTM_.rotate_ = angle.ToMat3();
									//m->rot = noVec3(135.0, 90.0, 90.0);
								}
								if (id1 == ATT_MIDDLE_BACK_SHEATH)
								{
									noAngles angle(90.0, 0.0,  90.0);
									att->data_->localTM_.rotate_ = angle.ToMat3();
									//m->rot = noVec3(0.0, 90.0, 90.0);
								}
							}
						}
					}
				}
			}
			if (id2>=0) {
				mp = (path + r.getString(ItemDisplayDB::Model2));
				if (mp.length()) {
					att = AddEquipment(mp, id2, slot);
					if (att) {
						m = att->data_->m2_;
						if (m) {
							mp = (path + r.getString(ItemDisplayDB::Skin2));
							mp.append(wxT(".tga"));
							tex = renderer_->addTexture(TexPath(mp.wx_str()).c_str(), true);							
							for (size_t x=0;x<m->TextureList.size();x++){
								if (m->TextureList[x] == wxString(wxT("Special_2"))){
									wxLogMessage(wxT("Replacing ID2's %s with %s"),m->TextureList[x].c_str(),mp.c_str());
									m->TextureList[x] = mp;
								}
							}
							m->replaceTextures[TEXTURE_CAPE] = tex;							
							succ = true;
						}
					}
				}
			}

			if (succ) {
				// Manual position correction of items equipped on the back, staves, 2h weapons, quivers, etc.
				//if (id1 >= ATT_RIGHT_BACK_SHEATH && id1 <= ATT_RIGHT_BACK)
				//	att->pos = noVec3(0.0f, 0.0f, 0.06f);

				// okay, see if we have any glowy effects
				int visualid = r.getInt(ItemDisplayDB::Visuals);

				if (visualid == 0) {
					/*if ((g_modelViewer->enchants->RHandEnchant > -1) && (slot == CS_HAND_RIGHT)) {
						visualid = g_modelViewer->enchants->RHandEnchant;
					} else if ((g_modelViewer->enchants->LHandEnchant > -1) && (slot == CS_HAND_LEFT)) {
						visualid = g_modelViewer->enchants->LHandEnchant;
					}*/
				}

				if (m == NULL)
					m = att->data_->m2_;

				if (visualid > 0) {
					try {
						ItemVisualDB::Record vis = visualdb.getById(visualid);
						for (size_t i=0; i<5; i++) {
							// try all five visual slots
							int effectid = vis.getInt(ItemVisualDB::Effect1 + i);
							if (effectid==0 || m->attLookup[i]<0) continue;

							try {
								ItemVisualEffectDB::Record eff = effectdb.getById(effectid);
								wxString filename = eff.getString(ItemVisualEffectDB::Model);

								AddEquipment(att->data_, filename, (int)i, -1);
								

							} catch (ItemVisualEffectDB::NotFound) {}
						}
					} catch (ItemVisualDB::NotFound) {}
				}

			} else {
				cd.equipment[slot] = 0; // no such model? :(
			}

		} catch (ItemDisplayDB::NotFound) {}
	}
}

M2Equipment* M2EquipSystem::AddEquipment(wxString modelfn, int id, int slot, float scale, float rot, noVec3 pos)
{
	if (!modelfn || modelfn.Len() == 0 || id<0) 
		return 0;


	M2Loader* loader = new M2Object(GetApp()->getRenderer(), Owner_); 
	loader->SetViewMatrix(ViewMat());
	loader->Load(modelfn.wx_str(), true);	

	// 장비 파티클 때문에 애니메이션 루프 반복 필요..
	if( slot != CS_HAND_LEFT || slot != CS_HAND_RIGHT  && loader->animManager)		
	{
		loader->animManager->SetAnim(0, 0, 1);
		loader->animManager->Play();
		loader->animManager->SetLoop(1);
	}

	if (loader) {
		return AddEquipment(loader, id, slot, scale, rot, pos);
	}
	else {
		SAFE_DELETE(loader);	
		return 0;
	}
}

M2Equipment* M2EquipSystem::AddEquipment(M2Mesh* pParent, wxString modelfn, int id, int slot, float scale, float rot, noVec3 pos)
{
	if (!modelfn || modelfn.Len() == 0 || id<0) 
		return 0;


	M2Loader* loader = new M2Object(GetApp()->getRenderer(), Owner_); 
	loader->SetViewMatrix(ViewMat());
	loader->Load(modelfn.wx_str(), true);	

	

	LOG_INFO << "Equip Model Name : " << modelfn.wx_str() << " Id : " << id;

	if (loader) {		
		return AddEquipment(pParent, loader, id, slot, scale, rot, pos);
	}
	else {
		SAFE_DELETE(loader);	
		return 0;
	}
}

M2Equipment* M2EquipSystem::AddEquipment( M2Loader* m2, int id, int slot, float scale, float rot, noVec3 pos )
{
	M2Equipment* pNewEp = new M2Equipment(m2, id,  slot, scale, rot, pos);
	pMesh_->AddChild(pNewEp->data_);			
	pNewEp->data_->owner_ = pMesh_->owner_;
	equips_.push_back(pNewEp);			
	return pNewEp;
}

M2Equipment* M2EquipSystem::AddEquipment( M2Mesh* pParent, M2Loader* m2, int id, int slot, float scale, float rot, noVec3 pos )
{
	M2Equipment* pNewEp = new M2Equipment(m2, id, slot, scale, rot, pos);
	pParent->AddChild(pNewEp->data_);			
	pNewEp->data_->owner_ = pMesh_->owner_;
	equips_.push_back(pNewEp);	
	return pNewEp;
}

void M2EquipSystem::RemoveEquipment( int nSlot )
{

}

M2EquipSystem::M2EquipSystem( class M2ModelEdit* pEdit, WowActor* owner, Renderer* renderer )
	:pEdit_(pEdit), pMesh_(owner->mesh_), renderer_(renderer), Owner_(owner),
	weaponType_(WEAPONT_1H), weaponKind_(WEAPONK_HAND), hasShield_(false)
{
	
}

void M2EquipSystem::CreateRenderPass()
{
	RightHandBone_ = new SceneNode;
	LeftHandBone_ = new SceneNode;
		
	for(size_t i=0; i < equips_.size(); ++i)
	{
		M2Equipment* pEquip = equips_.at(i);
		pEquip->data_->m2_->CreateRenderPass();
	}
}

void M2EquipSystem::Execute()
{
	if (weaponType_ == WEAPONT_1H)
	{	
		EquipWeapon1h();		
	}
	else if (weaponType_ == WEAPONT_2H)
	{

	}
	if (hasShield_)	
		EquipShield();
}

void M2EquipSystem::EquipShield()
{
	static const int boneid = 0;
	for (size_t i = 0; i < equips_.size() ; ++i)
	{
		if (equips_[i]->slot_ == CS_HAND_LEFT)
		{
			equips_[i]->id_ = boneid;
			equips_[i]->data_->id_ = boneid;			
		}
	}
}

void M2EquipSystem::EquipWeapon1h()
{
	static const int bone1 = 1; 
	static const int bone2 = 2; 
	for (size_t i = 0; i < equips_.size() ; ++i)
	{
		// Melee Weapon
		if (equips_[i]->slot_ == CS_HAND_RIGHT)
		{
			equips_[i]->id_ = bone1;
			equips_[i]->data_->id_ = bone1;
			equips_[i]->data_->SetName("Weapon1h");			

		} else if (equips_[i]->slot_ == CS_HAND_LEFT) //	 Bow
		{
			equips_[i]->id_ = bone2;
			equips_[i]->data_->id_ = bone2;
			equips_[i]->data_->SetName("Bow");			
		}
	}
}

void M2EquipSystem::AnalysizeWeaponType(const wxString& weaponname)
{
	wxArrayString res;
	Split(weaponname, res, "_");
	
	if (res[0] == "Bow")
	{
		if (res[1] == "1H")
		{
			weaponType_ = WEAPONT_1H;
		}
		else if (res[1] == "2H")
		{
			weaponType_ = WEAPONT_2H;
		}		
		weaponKind_ = WEAPONK_BOW;
	}
	else if (res[0] == "Firearm")
	{
		if (res[1] == "1H")
		{
			weaponType_ = WEAPONT_1H;
		}
		else if (res[1] == "2H")
		{
			weaponType_ = WEAPONT_2H;
		}

		weaponKind_ = WEAPONK_RIFLE;
	}
	else if (res[0] == "Sword")
	{
		if (res[1] == "1H")
		{
			weaponType_ = WEAPONT_1H;
		}
		else if (res[1] == "2H")
		{
			weaponType_ = WEAPONT_2H;
		}

		weaponKind_ = WEAPONK_SWORD;
	}
}

void M2EquipSystem::EquipSword2h()
{
	static const int boneid = 0;
	for (size_t i = 0; i < equips_.size() ; ++i)
	{
		if (equips_[i]->slot_ == CS_HAND_RIGHT)
		{

		}
	}
}

void M2EquipSystem::EquipRifle()
{
	static const int boneid = 0;
	for (size_t i = 0; i < equips_.size() ; ++i)
	{
		if (equips_[i]->slot_ == CS_HAND_RIGHT)
		{

		}
	}
}

M2EquipSystem::~M2EquipSystem()
{
	for(size_t i=0; i < equips_.size(); i++) {
		delete equips_[i];
	}
	equips_.clear();
}

M2Equipment::M2Equipment( M2Loader* m2, int id, int slot, float scale, float rot, noVec3 pos )
	:id_(id), slot_(slot)
{
	
	data_ = new M2Mesh;
	data_->m2_ = m2;
	data_->id_ = id_;
	data_->CreateBuffers();
	data_->localTM_.scale_ = scale;
	noAngles angle(rot, rot, rot);
	data_->localTM_.rotate_ = angle.ToMat3();
	data_->localTM_.trans_ = pos;	
}

M2Equipment::~M2Equipment()
{
	SAFE_DELETE(data_);
}
