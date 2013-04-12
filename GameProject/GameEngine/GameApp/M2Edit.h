#pragma once 


#include <ModelLib/M2ModelEdit.h>


class M2Edit : public M2ModelEdit
{
public:
	M2Edit() {}
	M2Edit(M2Loader* m2);
	~M2Edit();
	
	
	void Init( Renderer* renderer, class M2Mesh* pMesh, class WowActor* pActor);
	void UpdateEquipment();

	class M2EquipSystem* Equip_;

};