#pragma once 

#include "DataManager.h"
#include "GeometryManager.h"
#include "LightningManager.h"
#include "GameApp/BaseCamera.h"

namespace VCloud {

	class VClouds
	{
	public:

		/** Geometry settings
		 */
		struct GeometrySettings 
		{
			/// Height: x = Altitude over the camera, y: Field height (both in world coordinates)
			noVec2 Height;
			/// Angles
			float Alpha, Beta;
			/// Radius
			float Radius;
			/// Number of blocks
			int NumberOfBlocks;
			/// Number of slices per geometry zone
			int Na, Nb, Nc;

			/** Default constructor
			 */
			GeometrySettings()
				: Height(noVec2(10,50))
				, Alpha(DEG2RAD(12)), Beta(DEG2RAD(40))
				, Radius(100)
				, NumberOfBlocks(12)
				, Na(10), Nb(8), Nc(6)
			{
			}

			/** Constructor
			    @param _Height x = Cloud field y-coord start, y: Field height (both in world coordinates)
				@param _Radius Radius
				@param _Alpha Alpha angle
				@param _Beta Beta angle
				@param _NumberOfBlocks Number of geometry blocks
				@param _Na Number of slices in A zone
				@param _Nb Number of slices in B zone
				@param _Nc Number of slices in C zone
			 */
			GeometrySettings(const noVec2& _Height, const float& _Radius,
					const float& _Alpha = DEG2RAD(12), const float& _Beta = DEG2RAD(40), 
					const int& _NumberOfBlocks = 12, const int& _Na = 10, const int& _Nb = 8, const int& _Nc = 6)
				: Height(_Height)
				, Alpha(_Radius), Beta(_Beta)
				, Radius(_Radius)
				, NumberOfBlocks(_NumberOfBlocks)
				, Na(_Na), Nb(_Nb), Nc(_Nc)
			{
			}
		};

		/** Camera data struct
		 */
		struct CameraData
		{
		public:
			/** Default constructor
			 */
			inline CameraData()
				: camera(0)
				, lastPosition(noVec3(0,0,0))
				, cameraOffset(noVec2(0,0))
				, geometryDisplacement(noVec3(0,0,0))
			{
			}

			/** Constructor
			    @param c Camera
			 */
			inline CameraData(BaseCamera* c)
				: camera(c)
				, lastPosition(c->GetFrom())
				, cameraOffset(noVec2(0,0))
				, geometryDisplacement(noVec3(0,0,0))
			{
			}

			/// Camera
			BaseCamera* camera;
			/// Last camera position
			noVec3 lastPosition;
			/// Camera offset
			noVec2 cameraOffset;
			/// Geometry displacement
			noVec3 geometryDisplacement;
		};
				
		VClouds();

		/** Destructor
		 */
		~VClouds();

		/** Create
		 */
		void create();

		/** Create
			@param gs Geometry settings
		 */
		void create(const GeometrySettings& gs);

		/** Create
			@param Height x = Cloud field y-coord start, y: Field height (both in world coordinates)
			@param Radius Radius
		 */
		void create(const noVec2& Height, const float& Radius);

		/** Remove
		 */
		void remove();

		/** Update, to be invoked per frame
		    @param timeSinceLastFrame Time since last frame
         */
        void update(const float& timeSinceLastFrame);

		
		/** Has been create() already called?
		    @return true if created() have been already called, false if not
		 */
		inline const bool& isCreated() const
		{
			return mCreated;
		}

		/** Set geometry settings
		    @param GeometrySettings Geometry settings
			@remarks Set geometry settings before call create(...)
		 */
		inline void setGeometrySettings(const GeometrySettings& gs)
		{
			mGeometrySettings = gs;
		}

		/** Get geometry settings
		    @return Geometry settings
		 */
		inline const GeometrySettings& getGeometrySettings() const
		{
			return mGeometrySettings;
		}

		/** Set distance falling params
		    @param DistanceFallingParams
				   DistanceFallingParams.x = Distance falling factor (How much the cloud field geometry falls with the distance)
									         Remember that the geometry falling is relative to the distance(height) between the camera
											 and the cloud field. Typical range is [0, ~2] 0 = no falling
				   DistanceFallingParams.y = Max falling (in world coords), useful when , i.e., you've water and you want to go in. 
									  	     That param will allow you to avoid the cloud field geometry falls into the ocean. 
											 -1 means not max falling. (default)
			@remarks See GoemetryBlock::_setVertexData(...) for more info
		*/
		inline void setDistanceFallingParams(const noVec2& DistanceFallingParams)
		{
			mDistanceFallingParams = DistanceFallingParams;
		}

		/** Get distance falling params
		    @return DistanceFallingParams.x = Distance falling factor (How much the cloud field geometry falls with the distance)
									          Remember that the geometry falling is relative to the distance(height) between the camera
											  and the cloud field.
											  Typical range is [0, ~2] 0 = no falling
				    DistanceFallingParams.y = Max falling (in world coords), useful when , i.e., you've water and you want to go in. 
									  	      That param will allow you to avoid the cloud field geometry falls into the ocean. 
											  -1 means not max falling. (default)
			@remarks See GoemetryBlock::_setVertexData(...) for more info
		 */
		inline const noVec2& getDistanceFallingParams() const
		{
			return mDistanceFallingParams;
		}

		/** Set wind direction
		    @param WindDirection Wind direction
		 */
		inline void setWindDirection(const float& WindDirection)
		{
			mWindDirection = WindDirection;
		}

		/** Get wind direction
		    @return Wind direction
		 */
		inline const float& getWindDirection() const
		{
			return mWindDirection;
		}

		/** Get wind direction as a Vector2
		    @return Wind direction
		 */
		inline const noVec2 getWindDirectionV2() const
		{
			return noVec2(noMath::Cos(mWindDirection), noMath::Sin(mWindDirection));
		}

		/** Set wind speed
		    @param WindSpeed Wind speed
		 */
		inline void setWindSpeed(const float& WindSpeed)
		{
			mWindSpeed = WindSpeed;
		}

		/** Get wind speed
		    @return Wind speed
		 */
		inline const float& getWindSpeed() const
		{
			return mWindSpeed;
		}

		/** Set sun direction
		    @param SunDirection Sun direction
		 */
		inline void setSunDirection(const noVec3& SunDirection)
		{
			mSunDirection = SunDirection;
		}

		/** Get sun direction
		    @return Sun direction
		 */
		inline const noVec3& getSunDirection() const
		{
			return mSunDirection;
		}

		/** Set sun color
		    @param SunColor Sun color
		 */
		void setSunColor(const noVec3& SunColor);

		/** Get sun color
		    @return Sun color
		 */
		inline const noVec3& getSunColor() const
		{
			return mSunColor;
		}

		/** Set ambient color
		    @param AmbientColor Ambient color
		 */
		void setAmbientColor(const noVec3& AmbientColor);

		/** Get Ambient color
		    @return Ambient color
		 */
		inline const noVec3& getAmbientColor() const
		{
			return mAmbientColor;
		}

		/** Set light response
		    @param LightResponse
				   x - Sun light power
				   y - Sun beta multiplier
				   z - Ambient color multiplier
				   w - Distance attenuation
	     */
		void setLightResponse(const noVec4& LightResponse);

		/** Get light response
		    @return Light response
		 */
		inline const noVec4& getLightResponse() const
		{
			return mLightResponse;
		}

		/** Set ambient factors
		    @param AmbientFactors x - constant, y - linear, z - cuadratic, w - cubic
	     */
		void setAmbientFactors(const noVec4& AmbientFactors);

		/** Get ambient factors
		    @return Ambient factors
		 */
		inline const noVec4& getAmbientFactors() const
		{
			return mAmbientFactors;
		}

		/** Set global opacity
		    @param GlobalOpacity Global opacity: [0,1] range 0->Transparent cloud field
		 */
		inline void setGlobalOpacity(const float& GlobalOpacity)
		{
			mGlobalOpacity = GlobalOpacity;
		}

		/** Get global opacity
		    @return Global opacity
		 */
		inline const float& getGlobalOpacity() const
		{
			return mGlobalOpacity;
		}

		/** Set cloud field scale
		    @param CloudFieldScale Cloud field scale
		 */
		inline void setCloudFieldScale(const float& CloudFieldScale)
		{
			mCloudFieldScale = CloudFieldScale;
		}

		/** Get cloud field scale
		    @return Cloud field scale
		 */
		inline const float& getCloudFieldScale() const
		{
			return mCloudFieldScale;
		}

		/** Set noise scale
		    @param NoiseScale Noise scale
		 */
		inline void setNoiseScale(const float& NoiseScale)
		{
			mNoiseScale = NoiseScale;
		}

		/** Get noise scale
		    @return Noise scale
		 */
		inline const float& getNoiseScale() const
		{
			return mNoiseScale;
		}

		/** Set wheater parameters
		    Use this funtion to update the cloud field parameters, you'll get a smart and smooth transition from your old 
			setting to your new ones.
			@param Humidity Humidity, in other words: the percentage of clouds in [0,1] range.
			@param AverageCloudsSize Average clouds size, for example: if previous wheater clouds size parameter was very different from new one(i.e: more little)
			       only the old biggest clouds are going to be keept and the little ones are going to be replaced
		    @param DelayedResponse false to change wheather conditions over several updates, true to change it at the moment
		 */
		void setWheater(const float& Humidity, const float& AverageCloudsSize, const bool& DelayedResponse);

		/** Get wheater
		    @return Wheater parameters: x = Humidity, y = Average clouds size, both un [0,1] range
		 */
		inline const noVec2& getWheater() const
		{
			return mWheater;
		}

		/** Set visible
		    @param visible true to set VClouds visible, false to hide it
		 */
		void setVisible(const bool& visible);

		/** Is VClouds visible?
		    @return true if VClouds is visible, false otherwise
		 */
		inline const bool& isVisible() const
		{
			return mVisible;
		}

		inline BaseCamera* getCamera()
		{
			return mCamera;
		}
		
		/** Get data manager
		    @return Data manager
		 */
		inline DataManager* getDataManager()
		{
			return mDataManager;
		}

		/** Get geometry manager
		    @return Geometry manager
		 */
		inline GeometryManager* getGeometryManager()
		{
			return mGeometryManager;
		}

		/** Get lightning manager
		    @return Lightning manager
		 */
		inline LightningManager* getLightningManager()
		{
			return mLightningManager;
		}

		/** Get cameras data
		    @return Cameras data
			@remarks Only for internal use
		 */
		inline std::vector<CameraData>& _getCamerasData()
		{
			return mCamerasData;
		}
		void VClouds::unregisterCamera(BaseCamera* c);
		void VClouds::registerCamera(BaseCamera* c);		
		void VClouds::RenderCloud( BaseCamera* c, float timeSinceLastCameraFrame);
		/// Vol. clouds material
		ShaderID mVolCloudsMaterial;
		/// Vol. clouds + lightning material
		ShaderID mVolCloudsLightningMaterial;

	private:
		/// Has been create(...) already called?
		bool mCreated;

		/// Geometry settings
		GeometrySettings mGeometrySettings;

		/// Geometry distance falling params
		noVec2 mDistanceFallingParams;

			

		/// Wind direction
		float mWindDirection;
		/// Wind speed
		float mWindSpeed;

		/// Wheater parameters: x = Humidity, y = Average clouds size, both un [0,1] range
		noVec2 mWheater;
		/// Delayed response (This param is stored to allow the user call setWheater(...) before create() )
		bool mDelayedResponse;

		/// Sun direction
		noVec3 mSunDirection;

		/// Sun color
		noVec3 mSunColor;
		/// Ambient color
		noVec3 mAmbientColor;

		/** Light response:
		    x - Sun light power
			y - Sun beta multiplier
		    z - Ambient color multiplier
		    w - Distance attenuation
	     */
		noVec4 mLightResponse;
		/** Ambient factors
		    x - constant, y - linear, z - cuadratic, w - cubic
		 */
		noVec4 mAmbientFactors;

		/// Global opacity
		float mGlobalOpacity;

		/// Cloud field scale
		float mCloudFieldScale;
		/// Noise scale
		float mNoiseScale;

		/// Is VClouds visible?
		bool mVisible;

		/// Data manager
		DataManager *mDataManager;
		/// Geometry manager
		GeometryManager *mGeometryManager;
		/// Lightning manager
		LightningManager *mLightningManager;

		/// SceneManager pointer
        
		/// Current rendering camera
		 BaseCamera* mCamera;

		

		/// Cameras data
		std::vector<CameraData> mCamerasData;

	};
}