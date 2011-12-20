#include "U2MaxCameraExport.h"


U2MaxCameraExport::U2MaxCameraExport(TimeValue animStart, TimeValue animEnd)
:m_animStart(animStart),
m_animEnd(animEnd)
{

}

void U2MaxCameraExport::Export(Interface* pIf, INode *pMaxNode, Object *obj)
{

	CameraObject* camObj = (CameraObject*)obj;

	// get camera state
	Interval interval;
	CameraState camState;
	camObj->EvalCameraState(m_animStart, interval, &camState);

	bool bFixedCam = false;
	if(camObj->GetManualClip())
		bFixedCam = true;

	float fFov = camState.fov;
	BOOL bIsOrtho = camState.isOrtho;

	float fInvAspectRatio = 1.0f / pIf->GetRendImageAspect();

	if(bIsOrtho)
	{
		float fWidth = 640.0f / 4.0f;
		float fHeight = 480.0f / 4.0f;
		float fScalar = camObj->GetTDist(0) / 200.0f;
	}
	else 
	{
		// add
	}

	if(bFixedCam)
	{
		float fNearPlane = camState.hither;
		float farPlane = camState.yon;
	}
	else 
	{
		float nearPlane = 1.0f;
		float farPlane = 6000.0f;
	}

	ViewExp* viewport = pIf->GetActiveViewport();
	if(viewport)
	{
		INode* pMaxViewCam = viewport->GetViewCamera();

		if(pMaxNode == pMaxViewCam)
		{
			; 
		}

		pIf->ReleaseViewport(viewport);
	}
	else 
	{
		;
	}
}

void U2MaxCameraExport::ExtractFromViewport(Interface* pIf)
{
	ViewExp* viewport = pIf->GetActiveViewport();
	if(viewport)
	{
		Matrix3 modelView, invModelView;

		viewport->GetAffineTM(modelView);

		invModelView = Inverse(modelView);

		Point3 up = invModelView.GetRow(0);	// up
		Point3 right = invModelView.GetRow(1); // right
		Point3 look = invModelView.GetRow(2);
		Point3 eye = invModelView.GetRow(3);

		float fFov = viewport->GetFOV();
		BOOL bIsPerp = viewport->IsPerspView();

		pIf->ReleaseViewport(viewport);

		if(bIsPerp)
		{
			;
		}
		else 
		{
			;
		}
	}
}



