#pragma once 

#include "VClouds.h"
#include "ColorGradient.h"

namespace VCloud {

	class VCloudSkySystem;

	class VCloudsManager {
	public:
		VCloudsManager(VCloudSkySystem* s);

		~VCloudsManager();

		void Create(const float& readius = -1);

		void Update(const float& deltaTime);

		void Destroy();

		
		/** Set ambient gradient
		    @param AmbientGradient Ambient color gradient
		 */
		inline void setAmbientGradient(const ColorGradient& AmbientGradient)
		{
			mAmbientGradient = AmbientGradient;
		}

		/** Get ambient color gradient
		    @return Ambient color gradient
		 */
		inline const ColorGradient& getAmbientGradient() const
		{
			return mAmbientGradient;
		}

		/** Set sun gradient
		    @param SunGradient Sun color gradient
		 */
		inline void setSunGradient(const ColorGradient& SunGradient)
		{
			mSunGradient = SunGradient;
		}

		/** Get sun color gradient
		    @return Sun color gradient
		 */
		inline const ColorGradient& getSunGradient() const
		{
			return mSunGradient;
		}

		/** Set height parameters
		    @param Height x = Cloud field y-coord start, y: Field height (both in world coordinates)
			@remarks Call it before create(), for now...
			         For autocalculated height based on the radius length set (-1,-1) as height params
		 */
		inline void setHeight(const noVec2& Height)
		{
			mHeight = Height;
		}

		/** Get height parameters
		    @return Height: x = Cloud field y-coord start, y: Field height (both in world coordinates)
		 */
		inline const noVec2& getHeight() const
		{
			return mHeight;
		}

		inline void setAutoupdate(const bool& Autoupdate) 
		{
			mAutoupdate = Autoupdate;
			_updateWindSpeedConfig();
		}

		/** Set wind speed
		    @param WindSpeed Wind speed
		 */
		inline void setWindSpeed(const float& WindSpeed)
		{
			mWindSpeed = WindSpeed;
			_updateWindSpeedConfig();
		}

		/** Get wind speed
		    @return Wind speed
		 */
		inline const float& getWindSpeed() const
		{
			return mWindSpeed;
		}
	
		/** Get VClouds
		 */
		inline VClouds* getVClouds()
		{
			return mVClouds;
		}

		/** Is moon manager created?
		    @return true if yes, false if not
		 */
		inline const bool& isCreated() const
		{
			return mCreated;
		}

		/** Update wind speed config
		    @remarks Only for internal use
		 */
		void _updateWindSpeedConfig();
		void VCloudsManager::remove();
		void VCloudsManager::create(const float& radius);
		void VCloudsManager::update(const float& timeSinceLastFrame);

		void VCloudsManager::Render(BaseCamera* c);

	private:
		void _setLightParameters();
		/// Ambient and Sun color gradients
		ColorGradient mAmbientGradient;
		ColorGradient mSunGradient;

		/// VClouds pointer
		VClouds* mVClouds;

		/// Height parameters, x = Cloud field y-coord start, y: Field height (both in world coordinates)
		noVec2 mHeight;

		/// Autoupdate wind speed depending of skyx time multiplier?
		bool mAutoupdate;
		/// Wind speed
		float mWindSpeed;

		/// Is vclouds manager created?
		bool mCreated;

		/// Current time since last frame
		float mCurrentTimeSinceLastFrame;

		/// SkyX parent pointer
		VCloudSkySystem *mSkyX;
	};

}