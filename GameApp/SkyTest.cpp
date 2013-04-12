#include "stdafx.h"
#include "GameApp.h"
#include "Sky/BasicController.h"
#include "Sky/VCloudSkySystem.h"

using namespace VCloud;

BasicController* BCtrl_;
uint32 mCurrentPreset = 0;

struct SkyXSettings
{
	/** Constructor
	    @remarks Skydome + vol. clouds + lightning settings
	 */
	SkyXSettings(const noVec3 t, const float& tm, const float& mp, const AtmosphereManager::Options& atmOpt,
		const bool& lc, const bool& vc, const float& vcws, const bool& vcauto, const float& vcwd, 
		const noVec3& vcac, const noVec4& vclr,  const noVec4& vcaf, const noVec2& vcw,
		const bool& vcl, const float& vclat, const noVec3& vclc, const float& vcltm)
		: time(t), timeMultiplier(tm), moonPhase(mp), atmosphereOpt(atmOpt), layeredClouds(lc), volumetricClouds(vc), vcWindSpeed(vcws)
		, vcAutoupdate(vcauto), vcWindDir(vcwd), vcAmbientColor(vcac), vcLightResponse(vclr), vcAmbientFactors(vcaf), vcWheater(vcw)
		, vcLightnings(vcl), vcLightningsAT(vclat), vcLightningsColor(vclc), vcLightningsTM(vcltm)
	{}

	/** Constructor
	    @remarks Skydome + vol. clouds
	 */
	SkyXSettings(const noVec3 t, const float& tm, const float& mp, const AtmosphereManager::Options& atmOpt,
		const bool& lc, const bool& vc, const float& vcws, const bool& vcauto, const float& vcwd, 
		const noVec3& vcac, const noVec4& vclr,  const noVec4& vcaf, const noVec2& vcw)
		: time(t), timeMultiplier(tm), moonPhase(mp), atmosphereOpt(atmOpt), layeredClouds(lc), volumetricClouds(vc), vcWindSpeed(vcws)
		, vcAutoupdate(vcauto), vcWindDir(vcwd), vcAmbientColor(vcac), vcLightResponse(vclr), vcAmbientFactors(vcaf), vcWheater(vcw), vcLightnings(false)
	{}

	/** Constructor
	    @remarks Skydome settings
	 */
	SkyXSettings(const noVec3 t, const float& tm, const float& mp, const AtmosphereManager::Options& atmOpt, const bool& lc)
		: time(t), timeMultiplier(tm), moonPhase(mp), atmosphereOpt(atmOpt), layeredClouds(lc), volumetricClouds(false), vcLightnings(false)
	{}

	/// Time
	noVec3 time;
	/// Time multiplier
	float timeMultiplier;
	/// Moon phase
	float moonPhase;
	/// Atmosphere options
	AtmosphereManager::Options atmosphereOpt;
	/// Layered clouds?
	bool layeredClouds;
	/// Volumetric clouds?
	bool volumetricClouds;
	/// VClouds wind speed
	float vcWindSpeed;
	/// VClouds autoupdate
	bool vcAutoupdate;
	/// VClouds wind direction
	float vcWindDir;
	/// VClouds ambient color
	noVec3 vcAmbientColor;
	/// VClouds light response
	noVec4 vcLightResponse;
	/// VClouds ambient factors
	noVec4 vcAmbientFactors;
	/// VClouds wheater
	noVec2 vcWheater;
	/// VClouds lightnings?
	bool vcLightnings;
	/// VClouds lightnings average aparition time
	float vcLightningsAT;
	/// VClouds lightnings color
	noVec3 vcLightningsColor;
	/// VClouds lightnings time multiplier
	float vcLightningsTM;
};

/** Demo presets
    @remarks The best way of determinate each parameter value is by using a real-time editor.
			 These presets have been quickly designed using the Paradise Editor, which is a commercial solution.
			 At the time I'm writting these lines, SkyX 0.1 is supported by Ogitor. Hope that the Ogitor team will 
			 support soon SkyX 0.4, this way you all are going to be able to quickly create cool SkyX configurations.
 */
SkyXSettings mPresets[] = {
	// Sunset
	SkyXSettings(noVec3(8.85f, 7.5f, 20.5f),  -0.08f, 0, AtmosphereManager::Options(9.77501f, 10.2963f, 0.01f, 0.0022f, 0.000675f, 30, noVec3(0.57f, 0.52f, 0.44f), -0.991f, 3, 4), false, true, 300, false, float(270), noVec3(0.63f,0.63f,0.7f), noVec4(0.35, 0.2, 0.92, 0.1), noVec4(0.4, 0.7, 0, 0), noVec2(0.8,1)),
	// Clear
	SkyXSettings(noVec3(17.16f, 7.5f, 20.5f), 0, 0, AtmosphereManager::Options(9.77501f, 10.2963f, 0.01f, 0.0017f, 0.000675f, 30, noVec3(0.57f, 0.54f, 0.44f), -0.991f, 2.5f, 4), false),
	// Thunderstorm 1
	SkyXSettings(noVec3(12.23, 7.5f, 20.5f),  0, 0, AtmosphereManager::Options(9.77501f, 10.2963f, 0.01f, 0.00545f, 0.000375f, 30, noVec3(0.55f, 0.54f, 0.52f), -0.991f, 1, 4), false, true, 300, false, float(0), noVec3(0.63f,0.63f,0.7f), noVec4(0.25, 0.4, 0.5, 0.1), noVec4(0.45, 0.3, 0.6, 0.1), noVec2(1,1), true, 0.5, noVec3(1,0.976,0.92), 2),
	// Thunderstorm 2
	SkyXSettings(noVec3(10.23, 7.5f, 20.5f),  0, 0, AtmosphereManager::Options(9.77501f, 10.2963f, 0.01f, 0.00545f, 0.000375f, 30, noVec3(0.55f, 0.54f, 0.52f), -0.991f, 0.5, 4), false, true, 300, false, float(0), noVec3(0.63f,0.63f,0.7f), noVec4(0, 0.02, 0.34, 0.24), noVec4(0.29, 0.3, 0.6, 1), noVec2(1,1), true, 0.5, noVec3(0.95,1,1), 2),
	// Desert
	SkyXSettings(noVec3(7.59f, 7.5f, 20.5f), 0, -0.8f, AtmosphereManager::Options(9.77501f, 10.2963f, 0.01f, 0.0072f, 0.000925f, 30, noVec3(0.71f, 0.59f, 0.53f), -0.997f, 2.5f, 1), true),
	// Night
	SkyXSettings(noVec3(21.5f, 7.5, 20.5), 0.03, -0.25, AtmosphereManager::Options(), true)
};

std::string buildInfoStr()
{
	std::string str = "SkyX 0.4 demo - Select preset: [1] Desert, [2] Clear, [3] Thunderstorm (1), [4] Thunderstorm (2), [5] Sunset, [6] Night\nCurrent preset: ";

	switch (mCurrentPreset)
	{
		case 0: {str += "1 - Sunset";} break;
		case 1: {str += "2 - Clear";} break;
		case 2: {str += "3 - Thunderstorm (1)";} break;
		case 3: {str += "4 - Thunderstorm (2)";} break;
		case 4: {str += "5 - Desert";} break;
		case 5: {str += "6 - Night";} break;
	}

	return str;
}


//
//
//
//

void setPreset(VCloudSkySystem* Sky_,   const SkyXSettings& preset)
{
	Sky_->setTimeMultiplier(preset.timeMultiplier);
	BCtrl_->setTime(preset.time);
	BCtrl_->setMoonPhase(preset.moonPhase);
	Sky_->getAtmosphereManager()->setOptions(preset.atmosphereOpt);

	// Layered clouds
	if (preset.layeredClouds)
	{
		// Create layer cloud
		if (Sky_->getCloudsManager()->getCloudLayers().empty())
		{
			Sky_->getCloudsManager()->add(CloudLayer::Options(/* Default options */));
		}
	}
	else
	{
		// Remove layer cloud
		if (!Sky_->getCloudsManager()->getCloudLayers().empty())
		{
			Sky_->getCloudsManager()->removeAll();
		}
	}

	Sky_->getVCloudsManager()->setWindSpeed(preset.vcWindSpeed);
	Sky_->getVCloudsManager()->setAutoupdate(preset.vcAutoupdate);

	VClouds* vclouds = Sky_->getVCloudsManager()->getVClouds();

	vclouds->setWindDirection(preset.vcWindDir);
	vclouds->setAmbientColor(preset.vcAmbientColor);
	vclouds->setLightResponse(preset.vcLightResponse);
	vclouds->setAmbientFactors(preset.vcAmbientFactors);
	vclouds->setWheater(preset.vcWheater.x, preset.vcWheater.y, false);

	if (preset.volumetricClouds)
	{
		// Create VClouds
		if (!Sky_->getVCloudsManager()->isCreated())
		{
			// MeshManager::getSkydomeRadius(...) works for both finite and infinite(=0) camera far clip distances
			Sky_->getVCloudsManager()->create(Sky_->getMeshManager()->getSkydomeRadius(GetApp()->ActiveCam_));
		}
	}
	else
	{
		// Remove VClouds
		if (Sky_->getVCloudsManager()->isCreated())
		{
			Sky_->getVCloudsManager()->remove();
		}
	}

	vclouds->getLightningManager()->setEnabled(preset.vcLightnings);
	vclouds->getLightningManager()->setAverageLightningApparitionTime(preset.vcLightningsAT);
	vclouds->getLightningManager()->setLightningColor(preset.vcLightningsColor);
	vclouds->getLightningManager()->setLightningTimeMultiplier(preset.vcLightningsTM);

	

	// Reset camera position/orientation	
	GetApp()->ActiveCam_->setFrom(vec3_zero.ToFloatPtr());
	noVec3 eye = GetApp()->ActiveCam_->GetFrom();
	float to[] = {eye.x, eye.y, eye.z + 1.0f};
	GetApp()->ActiveCam_->setTo(to);
	GetApp()->ActiveCam_->computeModelView(true);	
	GetApp()->ActiveCam_->orthogonalize();

	Sky_->update(0);
}

void App::LoadSky()
{	
	BCtrl_ = new BasicController;
	Sky_ = new VCloudSkySystem(BCtrl_);
	Sky_->create();

	noVec3 eye(20000, 1800, 20000);
	noVec3 to(20001, 1800,  20001);
	ActiveCam_->setNear(20);
	ActiveCam_->setFar(30000);
	ActiveCam_->setFrom(eye.ToFloatPtr());
	ActiveCam_->setTo(to.ToFloatPtr());
	ActiveCam_->computeModelView();
	ActiveCam_->ComputeProjection();

	//g_World = translate(20000, 1800, 20000);

	//BCtrl_->setMoonPhase(0.75f);
	// Add a basic cloud layer
	//Sky_->getCloudsManager()->add(CloudLayer::Options(/* Default options */));


	// Distance geometry falling is a feature introduced in SkyX 0.2
	// When distance falling is enabled, the geometry linearly falls with the distance and the
	// amount of falling in world units is determinated by the distance between the cloud field "plane"
	// and the camera height multiplied by the falling factor.
	// For this demo, a falling factor of two is good enough for the point of view we're using. That means that if the camera
	// is at a distance of 100 world units from the cloud field, the fartest geometry will fall 2*100 = 200 world units.
	// This way the cloud field covers a big part of the sky even if the camera is in at a very low altitude.
	// The second parameter is the max amount of falling distance in world units. That's needed when for example, you've an 
	// ocean and you don't want to have the volumetric cloud field geometry falling into the water when the camera is underwater.
	// -1 means that there's not falling limit.
	Sky_->getVCloudsManager()->getVClouds()->setDistanceFallingParams(noVec2(2,-1));

	Sky_->getCloudsManager()->add(CloudLayer::Options(/* Default options */));

	setPreset(Sky_, mPresets[mCurrentPreset]);
}

void App::UpdateSKy()
{
	if(!Sky_) return;
	Sky_->setTimeMultiplier(0.01f);

}

void App::RenderSky( float fDeltaTime )
{
	if(!Sky_) return;

	UpdateSKy();
	Sky_->update(fDeltaTime);	

	Sky_->RenderSky(ActiveCam_);
}