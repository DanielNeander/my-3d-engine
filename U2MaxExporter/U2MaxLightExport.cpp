#include "U2MaxLightExport.h"
#include "U2MaxOptions.h"
#include <U2_3D/src/collision/U2aabb.h>

U2MaxLightExport::U2MaxLightExport(TimeValue animStart, TimeValue animEnd)
:m_animStart(animStart),
m_animEnd(animEnd)
{

}

void U2MaxLightExport::ExportLight(Interface* pIf, INode* pMaxNode, 
								   U2Node* pCurNode,  Object* pObj)
{
	U2Light* pLight = U2_NEW U2Light(U2Light::LT_AMBIENT);
	U2LightNode *pLightNode = U2_NEW U2LightNode(pLight);
	U2ASSERT(pLightNode);
	pCurNode->AttachChild(pLightNode);

	if(!BuildLight(pIf, pObj, pLightNode))
	{
		U2_DELETE pLightNode;
		return;
	}
}


bool U2MaxLightExport::BuildLight(Interface* pIf, Object *obj, U2LightNode* pLightNode)
{
	U2ASSERT(pLightNode);

	LightState lightState;

	GenLight* genericLight = (GenLight*)obj;

	genericLight->EvalLightState(m_animStart, Interval(0, 0), &lightState);
	
	U2Light* pLight = pLightNode->GetLight();

	switch(lightState.type)
	{
	case OMNI_LIGHT:
		{
			float fSize = 0;
			pLight->m_eLitType = U2Light::LT_POINT;
			if(lightState.useAtten)
			{
				fSize = lightState.attenEnd;
			}
			else if(lightState.useNearAtten)
			{
				fSize = lightState.nearAttenEnd;
			}
			else 
			{
				fSize = 500.f; // use default
			}

			float fGeomScale = U2MaxOptions::ms_fGeomScale;
			fSize *= fGeomScale;

			pLightNode->GetShaderAttb().SetArg(U2FxShaderState::LightRange, 
				U2ShaderArg(fSize));

			U2Aabb bbox;
			bbox.Set(D3DXVECTOR3(), D3DXVECTOR3(fSize, fSize, fSize));
			
			pLightNode->SetBoundingBox(bbox);
		}
		break;

	case DIR_LIGHT:
		pLight->m_eLitType = U2Light::LT_DIRECTIONAL;
		break;
	case FSPOT_LIGHT:
	case TSPOT_LIGHT:
		{
			pLight->m_eLitType = U2Light::LT_SPOT;

			float fGeomScale = U2MaxOptions::ms_fGeomScale;

			float fHotSize = lightState.hotsize * fGeomScale;
			float fFallSize = lightState.fallsize * fGeomScale;
		}
		break;

	default:
		U2ASSERT(false);
		return false;
	}

	pLightNode->SetActive(lightState.on == TRUE);

	// cast shadows
	pLightNode->SetCastShadows(lightState.shadow != FALSE);

	float fIntensity = lightState.intens;

	D3DXVECTOR4 litColor;
	litColor.x = lightState.color.r;
	litColor.y = lightState.color.g;
	litColor.z = lightState.color.b;
	litColor.w = 1.0f;

	pLightNode->GetShaderAttb().SetArg(U2FxShaderState::LightDiffuse, U2ShaderArg(litColor));
	pLightNode->GetShaderAttb().SetArg(U2FxShaderState::LightSpecular, U2ShaderArg(litColor));

	Color ambient;
	ambient = pIf->GetAmbient(m_animStart, FOREVER);
	litColor.x = ambient.r;
	litColor.y = ambient.g;
	litColor.z = ambient.b;
	litColor.w = 1.0f;

	pLightNode->GetShaderAttb().SetArg(U2FxShaderState::LightAmbient, U2ShaderArg(litColor));

	//TODO: any good way to specifies secondary colors for a light node?
	//      such as the param of LightDiffuse1, LIghtSpecular1 etc.
	//      One possible solution is that provides a custom attribute for that parameters.
	//...

	// export diffuse/specular color animation of the light if it exist.
	Control* control;

	control = genericLight->GetColorControl();

	if(control)
	{
		
	

	}

	// export ambient color animation.
	control = pIf->GetAmbientController();

	if(control && control->NumKeys())
	{


	}

	return true;
}






		



				
				


	

