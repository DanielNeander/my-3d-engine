#include <cs/bipexp.h>

#include "U2MaxUtil.h"


//-----------------------------------------------------------------------------
/**
Retrieves Object from the given node.
It checkes the returned object is derived object and if it is, 
tries to get its referenced object until it is not a derived object.
*/
Object* U2MaxUtil::GetBaseObject(INode* inode, TimeValue time)
{
	ObjectState objState = inode->EvalWorldState(time);
	Object* obj = objState.obj;

	if (obj)
	{
		SClass_ID sID;
		sID = obj->SuperClassID();
		while( sID == GEN_DERIVOB_CLASS_ID )
		{
			obj = ((IDerivedObject*)obj)->GetObjRef();
			sID = obj->SuperClassID();
		}
	}

	return obj;
}


//---------------------------------------------------------------------------
/**
It is used to find physique modifier or skin modifier.

@code
Object *obj = node->GetObjectRef();

Modifier* mod;
// find physique modifier
mod = nMaxUtil::FindModifier(obj, Class_ID(PHYSIQUE_CLASS_ID_A, PHYSIQUE_CLASS_ID_B));

// find skin modifier
mod = nMaxUtil::FindModifier(obj, SKIN_CLASSID);
@endcode
*/
Modifier* U2MaxUtil::FindModifier(Object* obj, Class_ID classID)
{
	if (!obj)
		return NULL;

	Modifier *mod;

	while (obj->SuperClassID() == GEN_DERIVOB_CLASS_ID)
	{
		IDerivedObject* derivObj = static_cast<IDerivedObject*>(obj);

		int modStack = 0;

		// iterate modifier stack.
		while (modStack < derivObj->NumModifiers())
		{
			// Get current modifier.
			mod = derivObj->GetModifier(modStack);
			// See this modifier is skin.
			Class_ID cId = mod->ClassID();
			if (cId == classID)
			{
				// found the modifier
				return mod;
			}
			modStack++;
		}
		obj = derivObj->GetObjRef();
	}

	return NULL;

}


//-----------------------------------------------------------------------------
/**
Retrieves the number of materials of the given node.
*/
int U2MaxUtil::GetMaterialCnt(INode* inode)
{
	Mtl* material = 0;
	material = inode->GetMtl();
	if (!material)
		return 1;

	if (material->ClassID() == Class_ID(MULTI_CLASS_ID, 0))
	{
		MultiMtl *mm = (MultiMtl*)material;

		return mm->NumSubMtls();
	}

	return 1;
}


//-----------------------------------------------------------------------------
/**
Retrieves super class id of the given Object.

@param obj pointer to the Object class instance.
@return super class id of the Object.
*/
SClass_ID U2MaxUtil::GetSuperClassID(Object* obj)
{
	U2ASSERT(obj != 0);

	SClass_ID sID;

	sID = obj->SuperClassID();

	while(sID == GEN_DERIVOB_CLASS_ID)
	{
		obj = ((IDerivedObject*)obj)->GetObjRef();
		sID = obj->SuperClassID();
	}

	return sID;
}


//-----------------------------------------------------------------------------
/**
*/
Modifier* U2MaxUtil::FindPhysique(INode* inode)
{
	Object *obj = inode->GetObjectRef();
	if (!obj)
		return NULL;

	// Is derived object
	if (obj->SuperClassID() == GEN_DERIVOB_CLASS_ID)
	{
		// casting object to derived object.
		IDerivedObject* deriveObj = static_cast<IDerivedObject*>(obj);

		// Iterate the modifier stack.
		int modStack = 0;
		while (modStack < deriveObj->NumModifiers())
		{
			// get current modifier.
			Modifier* modifier = deriveObj->GetModifier(modStack);

			// check the given modifier Is physique type.
			if (modifier->ClassID() == Class_ID(PHYSIQUE_CLASS_ID_A, PHYSIQUE_CLASS_ID_B))
			{
				// found the modifier.
				return modifier;
			}

			// move to the next modifier stack.
			modStack++;
		}
	}

	return NULL;
}

//-----------------------------------------------------------------------------
/**
*/
Modifier* U2MaxUtil::FindSkin(INode* inode)
{
	// Get object from node. Abort if no object.
	Object* obj = inode->GetObjectRef();
	if (!obj) 
		return NULL;

	// Is derived object ?
	while (obj->SuperClassID() == GEN_DERIVOB_CLASS_ID)
	{
		// Yes -> Cast.
		IDerivedObject* derObj = static_cast<IDerivedObject*>(obj);

		// Iterate over all entries of the modifier stack.
		int ModStackIndex = 0;
		while (ModStackIndex < derObj->NumModifiers())
		{
			// Get current modifier.
			Modifier* mod = derObj->GetModifier(ModStackIndex);

			// Is this Skin ?
			if (mod->ClassID() == SKIN_CLASSID )
			{
				// Yes -> Exit.
				return mod;
			}

			// Next modifier stack entry.
			ModStackIndex++;
		}

		obj = derObj->GetObjRef();
	}

	// Not found.
	return NULL;
}


bool U2MaxUtil::IsGeomObject(INode *node)
{
	ObjectState os = node->EvalWorldState(0); 

	// only add geometry nodes
	if (os.obj)
	{
		if (os.obj->SuperClassID() == GEOMOBJECT_CLASS_ID)
		{
			if (os.obj->ClassID() != Class_ID(TARGET_CLASS_ID, 0))
			{
				return true;
			}
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
/**
Check the given node is bone or not.

@note
This method does not check 'FOOTPRINT_CLASS_ID' type.
see nMaxBone::IsFootStep()

@param inode pointer to INode
@return true, if the given INode is bone.
*/
bool U2MaxUtil::IsBone(INode *inode)
{
	if (NULL == inode)
		return false;

	if (inode->IsRootNode())
		return false;

	ObjectState os;
	os = inode->EvalWorldState(0);
	if (os.obj && os.obj->ClassID() == Class_ID(BONE_CLASS_ID, 0x00))
		return true;

#if MAX_RELEASE >= 4000
	if (os.obj && os.obj->ClassID() == Class_ID(BONE_OBJ_CLASSID))
		return true;
#endif
	if (os.obj && os.obj->ClassID() == Class_ID(DUMMY_CLASS_ID, 0x00))
		return false;

	// check for biped node
	Control *control;
	control = inode->GetTMController();

	if ((control->ClassID() == BIPSLAVE_CONTROL_CLASS_ID) ||
		(control->ClassID() == BIPBODY_CONTROL_CLASS_ID))
		return true;

	return false;
}

//-----------------------------------------------------------------------------
/**
Check the given node is Biped's foot step or not.
*/
bool U2MaxUtil::IsFootStep(INode* inode)
{
	if (NULL == inode)
		return false;

	if (inode->IsRootNode())
		return false;

	// check for biped node
	Control *control;
	control = inode->GetTMController();

	if ((control->ClassID() == FOOTPRINT_CLASS_ID))
		return true;

	return false;
}

//-----------------------------------------------------------------------------
/**
-27-Mar-07  kims  Added to check whether the dummy node has biped or not.
Thank Cho Jun Heung for the patch.
*/
bool U2MaxUtil::IsDummy(INode* inode)
{
	if (NULL == inode)
		return false;

	// optimizing
	if( inode->NumChildren() == 0 )
		return false;

	//HACK: check the node has biped constrol
	Control* control = 0;
	control = inode->GetTMController();

	if (control)
	{
		// if the node has biped, consider that it is not a dummy.
		if ((control->ClassID() == BIPSLAVE_CONTROL_CLASS_ID) ||
			(control->ClassID() == BIPBODY_CONTROL_CLASS_ID))
			return false;
	}

	ObjectState os;
	os = inode->EvalWorldState(0);

	if (os.obj && os.obj->ClassID() == Class_ID(DUMMY_CLASS_ID, 0x00))
	{
		return true;
	}
	return false;
}

void U2MaxUtil::SetFlags(ReferenceMaker *pRefMaker, int iStat)
{
	int i;

	for (i = 0; i < pRefMaker->NumRefs(); i++) 
	{
		ReferenceMaker *pChildRef = pRefMaker->GetReference(i);
		if (pChildRef) 
			SetFlags(pChildRef, iStat);
	}

	switch (iStat) 
	{
	case ND_CLEAR:
		pRefMaker->ClearAFlag(A_WORK1);
		break;
	case ND_SET:
		pRefMaker->SetAFlag(A_WORK1);
		break;
	}
}
