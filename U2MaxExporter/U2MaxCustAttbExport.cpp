#undef V
#include "U2MaxCustAttbExport.h"
#include <icustattribcontainer.h> 


//-------------------------------------------------------------------------------------------------
void U2MaxCustAttbExport::ExportCustAttb(Animatable *obj, U2Object *pU2Obj)
{
	if(obj && obj->SuperClassID() == GEN_DERIVOB_CLASS_ID)
	{
		IDerivedObject* pDerived = (IDerivedObject*)obj;
		for(int i=0; i < pDerived->NumModifiers(); ++i)
		{
			Modifier* pMod = pDerived->GetModifier(i);
			if(pMod)
			{
				ExportCustAttb(pMod, pU2Obj);
			}
		}

		Object* pObjRef = pDerived->GetObjRef();
		if(pObjRef != obj)
		{
			ExportCustAttb(pObjRef, pU2Obj);
		}

		BaseObject* pBaseObj = pDerived->FindBaseObject();
		if(pBaseObj != obj && pBaseObj != pObjRef)
		{
			ExportCustAttb(pBaseObj, pU2Obj);
		}
	}

	if(!obj || !pU2Obj)
		return;

	ICustAttribContainer* cc =obj->GetCustAttribContainer();
	if(cc)
	{
		int numAttbs = cc->GetNumCustAttribs();
		CustAttrib *pAttb = NULL;

		for(int attb=0; attb < numAttbs; ++attb)
		{
			pAttb = (CustAttrib*)cc->GetCustAttrib(attb);
			if(pAttb)
			{
				Class_ID cId = pAttb->ClassID();
				FILE_LOG(logDEBUG) << TSTR("Converting Custom Attrib: ");
				FILE_LOG(logDEBUG) << ToUnicode(pAttb->GetName());

				for(int iPB = 0; iPB < pAttb->NumParamBlocks(); ++iPB)
				{
					IParamBlock2* pParamBlock = 
						pAttb->GetParamBlock(iPB);
					if(pParamBlock != NULL)
					{
						if(!ConvertParamBlock(pParamBlock, pU2Obj))
						{
							
						}
					}
				}
			}
		}
	}
}

//-------------------------------------------------------------------------------------------------
bool U2MaxCustAttbExport::ConvertParamBlock(IParamBlock2* pParamBlock, U2Object* pU2Obj)
{
	int numParams = pParamBlock->NumParams();

	int paramType;

	for(int i=0; i < numParams; ++i)
	{
		paramType = pParamBlock->GetParameterType(pParamBlock->IndextoID(i));

		switch(paramType)
		{
		case TYPE_INODE:
			break;

		case TYPE_STRING:
			break;

		case TYPE_FLOAT:
			break;

		case TYPE_INT:
			break;

		case TYPE_RGBA:
			break;

		case TYPE_POINT3:
			break;

		case TYPE_POINT4:
			break;
		}

	}

	return true;
}



