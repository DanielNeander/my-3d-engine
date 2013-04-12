/*
--------------------------------------------------------------------------------
This source file is part of SkyX.
Visit http://www.paradise-studios.net/products/skyx/

Copyright (C) 2009-2012 Xavier Vergu? Gonz?ez <xavyiy@gmail.com>

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
--------------------------------------------------------------------------------
*/

#ifndef _SkyX_MeshManager_H_
#define _SkyX_MeshManager_H_

class SceneNode;
class BaseCamera;

namespace VCloud
{
	class VCloudSkySystem;

    class SkyDomeMesh 
	{
	public:
		/** Vertex struct
		 */
		struct VERTEX
		{
			      // Position
			float x, y, z;
			      // Normalized position
			float nx, ny, nz;
				  // Texture coords
			float u, v;
				  // Opacity
			float o;
		};

		/** Constructor
		    @param s Parent SkyX pointer
		 */
		SkyDomeMesh(VCloudSkySystem *s);

		/** Destructor 
		 */
		~SkyDomeMesh();

		/** Create our water mesh, geometry, entity, etc...
            @remarks Call it after setMaterialName()
         */
        void create();

		/** Remove all resources
		 */
		void remove();

		/** Update geometry
		    @param cam BaseCamera
		 */
		void updateGeometry(BaseCamera* cam);

		/** Get mesh
            @return Mesh
         */
        
        /** Get material name
            @return Material name
      \
		/** Get the SceneNode pointer where Hydrax mesh is attached
		    @return SceneNode*
		 */
		inline SceneNode* getSceneNode()
		{
			return mSceneNode;
		}

		/** Is _createGeometry() called?
		    @return true if created() have been already called
		 */
		inline const bool& isCreated() const
		{
			return mCreated;
		}

		/** Set geometry parameters
		    @param Steps Sphere number of steps
			@param Circles Spehere number of circes
		 */
		void setGeometryParameters(const int& Steps, const int& Circles);

		/** Get number of steps
		    @return Number of steps
		 */
		inline const int& getSteps() const
		{
			return mSteps;
		}

		/** Get number of circles
		    @return Number of circles
		 */
		inline const int& getCircles() const
		{
			return mCircles;
		}

		/** Set under-horizon rendering params
		    @remarks In an ideal situation, you only must see the avobe horizon sky due to the fact that the terrain/water
			         must be 'infinite' and the under-horizont sky part is hide.
					 But, infinite terrain/water is not always implemented in games and 3d apps in general, so... in order to
					 get a good-looking sky, SkyX provides an approach to render realistic under-horizont sky.
			@param UnderHorizonCircles Number of circles of SkyX::MeshManager::mCircles reserved for the under-horizont geometry part,
				   0 means not under-horizon rendering
			@param UnderHorizonFading true/false to fade or not the under-horizon sky
			@param UnderHorizonFadingExponent Exponent of the fading, pow(vertex_angle, exp), 1=linear
			@param UnderHorizonFadingMultiplier Fading multiplier, opacity = saturate(pow(opacity,fading_exp)*fading_multiplier)
		 */
		void setUnderHorizonParams(const int& UnderHorizonCircles = 15, const bool& UnderHorizonFading = true, const float& UnderHorizonFadingExponent = 1, const float& UnderHorizonFadingMultiplier = 2);

		/** Get under-horizon circles
		    @return Under-horizon circles
		 */
		inline const int& getUnderHorizonCircles() const
		{
			return mUnderHorizonCircles;
		}

		/** Get under-horizon fading
		    @return Under-horizon fading
		 */
		inline const bool& getUnderHorizonFading() const
		{
			return mUnderHorizonFading;
		}

		/** Get under-horizon exponent fading
		    @return under-horizon exponent fading
	     */
		inline const float& getUnderHorizonFadingExponent() const
		{
			return mUnderHorizonFadingExponent;
		}

		/** Get under-horizon fading multiplier
		    @return Under-horizon fading multiplier
		 */
		inline const float& getUnderHorizonFadingMultiplier() const
		{
			return mUnderHorizonFadingMultiplier;
		}

		/** Set radius multiplier
		    @param RadiusMultiplier Radius multiplier
			@remarks Radius multiplier in [0,1] range
			         Radius = BaseCameraFarClipDistance * RadiusMultiplier
		 */
		inline void setRadiusMultiplier(const float& RadiusMultiplier)
		{
			mRadiusMultiplier = RadiusMultiplier;
		}

		/** Get skydome radius
		    @param c BaseCamera
		    @return Skydome radius relative to the given BaseCamera
		 */
		const float getSkydomeRadius(BaseCamera* c) const;

		void setMaterial(ShaderID shader);

		void Render(BaseCamera* camera);

		VertexBufferID	mVB;
		IndexBufferID mIB;
		VERTEX* mVertices;
		ShaderID	mShader;
		VertexFormatID mVF;
		DepthStateID mDepthWrite;

		int	mNumIndices;
		int mNumVertices;
	private:
		/** Create geometry
		 */
		void _createGeometry();
		/// Has been create() already called?
		bool mCreated;
			

		/// Vertices
		
		
		

		/// Circles
		int mCircles;
		/// Steps
		int mSteps;

		/// Under-horizon rendering
		int mUnderHorizonCircles;
		/// Under-horizon fading
		bool mUnderHorizonFading;
		/// Under-horizon exponent fading (1=linear fading)
		float mUnderHorizonFadingExponent;
		/// Under-horizon fading multiplier: opacity = saturate(pow(opacity,fading_exp)*fading_multiplier)
		float mUnderHorizonFadingMultiplier;

		/// Radius multiplier
		float mRadiusMultiplier;

		/// SceneNode pointer
		SceneNode* mSceneNode;
		         

		/// Main SkyX pointer
		VCloudSkySystem* mSkyX;
	};
}

#endif