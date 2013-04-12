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

#ifndef _SkyX_VClouds_Lightning_H_
#define _SkyX_VClouds_Lightning_H_

#include "gameApp/SceneNode.h"
#include "GameApp/BillboardSet.h"


namespace VCloud { 
	class Lightning
	{
	public:
		/** Segment struct
		 */
		struct Segment
		{
		public:
			/** Default constructor
			 */
			Segment()
				: a(noVec3())
				, b(noVec3())
			{
			}

			/** Constructor
			    @param a_ First segment point (Start)
				@param b_ Second segment point (End)
			 */
			Segment(const noVec3& a_, const noVec3& b_)
				: a(a_)
				, b(b_)
			{
			}

			/// Segment start
			noVec3 a;
			/// Segment end
			noVec3 b;
		};

		/** Constructor
		    @param sm Scene manager
			@param sn Scene node
			@param orig Lighting origin
			@param dir Lighting direction
			@param l Lighting lenth
			@param d Divisions
			@param rec Recursivity level
			@param tm Time multiplier
			@param wm Width multiplier
			@param b Bounds
		 */
		Lightning(SceneNode* sn, const noVec3& orig, const noVec3& dir, const float& l, 
			const uint32& d, const uint32& rec, const float& tm,  const float& wm, const noVec2& b = noVec2(0,1));

		/** Destructor
		 */
		~Lightning();

		/** Create
		 */
		void create();

		/** Remove
		 */
		void remove();

		/** Update
		    @param timeSinceLastFrame Time since last frame
         */
        void update(float timeSinceLastFrame);

		/** Get ray direction
		    @return Ray direction
		 */
		inline const noVec3& getDirection() const
		{
			return mDirection;
		}

		/** Get ray length
		    @return Ray length
		 */
		inline const float& getLength() const
		{
			return mLength;
		}

		/** Get lightning intensity
		    @return Lightning intensity
		 */
		inline const float& getIntensity() const
		{
			return mIntensity;
		}
				
		/** Get scene node
		    @return Scene node
		 */
		inline SceneNode* getSceneNode() const
		{
			return mSceneNode;
		}

		/** Has the ray finished?
		    @return true if the ray has finished, false otherwise
		 */
		inline const bool& isFinished() const
		{
			return mFinished;
		}

		/** Update render queue group
		    @param rqg Render queue group
		    @remarks Only for internal use. Use VClouds::setRenderQueueGroups(...) instead.
		 */
		//void _updateRenderQueueGroup(const uint8& rqg);

		inline BillboardSet* getBillboardSet() const
		{
			return mBillboardSet;
		}
				

	private:
		/** Update data
		    @param alpha Alpha
			@param currentPos Current position
			@param parentTime Parent time
		 */
		void _updateData(const float& alpha, const float& currentPos, const float& parentTime);

		/// Ray origin
		noVec3 mOrigin;
		/// Ray direction
		noVec3 mDirection;
		/// Ray length
		float mLength;

		/// Real ray length (total segments length amount)
		float mRealLength;
		/// Number of divisions
		uint32 mDivisions;
		/// Recursivity level
		uint32 mRecursivity;
		/// Width multiplier
		float mWidthMultiplier;
		/// Ray bounds (for internal visual calculations)
		noVec2 mBounds;
		/// Angle range (Little values -> Less derivations, bigger values -> More derivations)
		noVec2 mAngleRange;

		/// Current elapsed time
		float mTime;
		/// Global time multiplier
		float mTimeMultiplier;
		/// Per step time multipliers
		noVec3 mTimeMultipliers;

		/// Lightning intensity
		float mIntensity;

		/// Segments
		std::vector<Segment> mSegments;
		/// Children lightnings
		std::vector<Lightning*> mChildren;

		/// Billboard set
		BillboardSet* mBillboardSet;
		/// Scene manager
		
		/// Scene node
		SceneNode* mSceneNode;

		/// Has been create() already called?
		bool mCreated;
		/// Has the ray finished?
		bool mFinished;
	};

}

#endif