#include "stdafx.h"
#include "M2Edit.h"
#include "SceneNode.h"
#include "Mesh.h"
#include "M2Mesh.h"
#include "Common.h"

#include "ModelLib/M2Loader.h"
#include "ModelLib/M2Particle.h"

#include "M2Equipment.h"

M2Edit::M2Edit( M2Loader* m2 )
	:M2ModelEdit(m2), Equip_(NULL)
{

}

void M2Edit::Init( Renderer* renderer, M2Mesh* pMesh, WowActor* pActor)
{
	M2ModelEdit::Init(renderer);
	m2_ = pMesh->m2_;

	if (pActor)
		Equip_ = new M2EquipSystem(this, pActor, renderer);	
}

bool slotHasModel(size_t i)
{
	return (i==CS_HEAD || i==CS_SHOULDER || i==CS_HAND_LEFT || i==CS_HAND_RIGHT || i==CS_QUIVER);
}

void M2Edit::UpdateEquipment()
{
	if (!Equip_) return;
	for (ssize_t i=0; i<NUM_CHAR_SLOTS; i++) {
		if (slotHasModel(i)) 
			Equip_->RefreshEquip(i);

		/*if (g_canvas->model->modelType != MT_NPC) {
			if (labels[i]) {
				labels[i]->SetLabel(CSConv(items.getById(cd.equipment[i]).name));
				labels[i]->SetForegroundColour(ItemQualityColour(items.getById(cd.equipment[i]).quality));
			}
		}*/		
	}
	Equip_->Execute();	
	Equip_->CreateRenderPass();
}

M2Edit::~M2Edit()
{
	SAFE_DELETE(Equip_);
}

