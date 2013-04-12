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

#ifndef _SkyX_VClouds_GeometryBlock_H_
#define _SkyX_VClouds_GeometryBlock_H_

#include "GameApp/Mesh.h"
#include "GameApp/BaseCamera.h"


namespace VCloud { 

	class VClouds;

	class GeometryBlock : public Mesh
	{
	public:
		/** Vertex struct
		 */
		struct VERTEX
		{	      // Position
			float x,y,z,
				  // 3D Coords
				  xc, yc, zc,
				  // Noise coords
				  u, v,
				  // Opacity
				  o;
		};

		/** Constructor
			@param vc VClouds pointer
			@param Height Field height (in woorld coordinates)
			@param Alpha Alpha angle
			@param Beta Beta angle
			@param Radius Total radius
			@param Phi Actimutal angle
			@param Na Number of slices in A zone
			@param Nb Number of slices in B zone
			@param Nc Number of slices in C zone
			@param A A radius
			@param B B radius
			@param C C radius
		 */
		GeometryBlock(VClouds *vc,
			const float& Height, const float& Alpha, const float& Beta, 
			const float& Radius, const float& Phi, const int& Na, 
			const int& Nb, const int& Nc, const int& A, 
			const int& B, const int& C, const int& Position);

		/** Destructor
		 */
		~GeometryBlock();

		virtual bool CreateBuffers(void) { return true; };
		virtual void Draw(void);

		/** Create
		 */
		void create();

		/** Remove
		 */
		void remove();

		/** Update geometry
		    @param c Camera
		    @param displacement Current offset in world units per zone
			@param distance Current camera to cloud field distance
         */
		void updateGeometry(BaseCamera* c, const noVec3& displacement, const noVec3& distance);

		/** Has been create() already called?
		    @return true if created() have been already called, false if not
		 */
		inline const bool& isCreated() const
		{
			return mCreated;
		}

		/** Get mesh
            @return Mesh
         */        
        				
		/** Set world offset
		    @param WorldOffset World offset
		 */
		inline void setWorldOffset(const noVec2& WorldOffset)
		{
			mWorldOffset = WorldOffset;
		}

		/** Is the geometry block inside the camera frustum?
		    @param c Camera
			@return true if yes, false if not
		 */
		const bool isInFrustum(BaseCamera *c) const;

		ShaderID		mCurrShader;

		void Render();
	private:
		/** Build axis aligned box
		    @param fd Falling distance (Positive values for falling geometry, negative for reverse falling geometry)
		 */
		const BoundingBox _buildAABox(const float& fd) const;

		/** Calculate data size
		 */
		void _calculateDataSize();

		/** Create geometry
		 */
		void _createGeometry();

		/** Update geometry
		 */
		void _updateGeometry();

		/** Update zone C slice
		    @param n Number of slice
		 */
		void _updateZoneCSlice(const int& n);
		
		/** Update zone B slice
		    @param n Number of slice
		 */
		void _updateZoneBSlice(const int& n);

		/** Update zone A slice
		    @param n Number of slice
		 */
		void _updateZoneASlice(const int& n);

		/** Set vertex data
			@param index Vertex index
			@param o Slice opacity
			@param p Position
		 */
		void _setVertexData(const int& index, const noVec3& p, const float& o);

		/// Has been create() already called?
		bool mCreated;

		/// MeshPtr
        

		/// Vertices pointer
		VERTEX *mVertices;

		/// Current number of triangles
		int mNumberOfTriangles;
		/// Vertex count
		int mVertexCount;

		/// Height
		float mHeight;
		/// Angles
		float mAlpha, mBeta;
		/// Radius
		float mRadius;
		/// Acimutal angle
		float mPhi;
		/// Number of slices per geometry zone
		int mNa, mNb, mNc;
		/// A, B and C radius
		float mA, mB, mC;
		/// Number of block(Position)
		int mPosition;

		ShaderID		mShader;
		VertexFormatID mVF;
		VertexBufferID mVB;
		IndexBufferID	mIB;

		/// Precomputed Cos/Sin vectors
		noVec2 mV2Cos;
		noVec2 mV2Sin;
		/// PI - Beta, PI - Alpha Sin
		float mBetaSin;
		float mAlphaSin;

		/// Displacement
		noVec3 mDisplacement;
		/// World coords offset
		noVec2 mWorldOffset;

		/// VClouds pointer
		VClouds *mVClouds;
		/// Current rendering camera
		BaseCamera* mCamera;

		/// Current distance
		noVec3 mDistance;

		/// Last falling distance
		float mLastFallingDistance;
	};


}

#endif