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

#ifndef _SkyX_VClouds_GeometryManager_H_
#define _SkyX_VClouds_GeometryManager_H_

#include "GeometryBlock.h"

namespace VCloud { 

	class VClouds;

	class GeometryManager
	{
	public:
		/** Constructor
			@param vc VClouds pointer
		 */
		GeometryManager(VClouds* vc);

		/** Destructor
		 */
		~GeometryManager();

		/** Create
		    @param Height x = Field altitude, y: Field height (both in world coordinates)
			@param Radius Radius
			@param Alpha Alpha angle
			@param Beta Beta angle
			@param NumberOfBlocks Number of geometry blocks
			@param Na Number of slices in A zone
			@param Nb Number of slices in B zone
			@param Nc Number of slices in C zone
		 */
		void create(const noVec2& Height, const float& Radius,
		    const float& Alpha, const float& Beta, 
            const int& NumberOfBlocks, const int& Na, const int& Nb, const int& Nc);

		/** Remove
		 */
		void remove();

		/** Update, to be invoked per frame
		    @param timeSinceLastFrame Time since last frame
         */
        void update(const float& timeSinceLastFrame);

		/** Update geoemtry
		    @param c Camera
		    @param timeSinceLastCameraFrame Time since last CAMERA frame
         */
		void updateGeometry(BaseCamera* c, const float& timeSinceLastCameraFrame);

		/** Has been create() already called?
		    @return true if created() have been already called, false if not
		 */
		inline const bool& isCreated() const
		{
			return mCreated;
		}

		/** Get scene node
		    @return Scene node
		 */
		inline SceneNode* getSceneNode()
		{
			return mSceneNode;
		}

		/** Get height (x = Altitude over the camera, y: Field height (both in world coordinates))
		    @return Height (x = Altitude over the camera, y: Field height (both in world coordinates))
		 */
		inline const noVec2 getHeight() const
		{
			return mHeight;
		}

		/** Set material name
		    @param mn Material name
		    @remarks Only for internal use
		 */
		void _setMaterialName(const std::string& mn);

		/** Update render queue group
		    @param rqg Render queue group
		    @remarks Only for internal use. Use VClouds::setRenderQueueGroups(...) instead.
		 */
		void _updateRenderQueueGroup(const uint8& rqg);

		/** Get current camera to cloud field distance
		    @return Current camera to cloud field distance
			@remarks Only for internal use
		 */
		inline const noVec3 _getCurrentDistance() const
		{
			return mCurrentDistance;
		}

		void GeometryManager::_setMaterial(ShaderID shader);

		void GeometryManager::Render();
	private:
		/** Create geometry
		 */
		void _createGeometry();

		/** Update geometry
		    @param c Camera
		    @param timeSinceLastFrame Time since last frame
		 */
		void _updateGeometry(BaseCamera* c, const float& timeSinceLastFrame);
		/// Has been create() already called?
		bool mCreated;

		/// Height: x = Altitude over the camera, y: Field height (both in world coordinates)
		noVec2 mHeight;
		/// Angles
		float mAlpha, mBeta;
		/// Radius
		float mRadius;
		/// Azimutal angle per block
		float mPhi;
		/// Number of blocks
		int mNumberOfBlocks;
		/// Number of slices per geometry zone
		int mNa, mNb, mNc;
		/// A, B and C radius
		float mA, mB, mC;

		/// World coords offset
		noVec2 mWorldOffset;

		ShaderID	mCurrShader;

		/// Geometry blocks
		std::vector<GeometryBlock*> mGeometryBlocks;

		/// Scene node
		SceneNode *mSceneNode;

		/// Current camera-clouds distance
		noVec3 mCurrentDistance;

		/// VClouds pointer
		VClouds *mVClouds;
	};


}

#endif