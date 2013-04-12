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

#ifndef _SkyX_MoonManager_H_
#define _SkyX_MoonManager_H_

class BaseCamera;
class SceneNode;
class BillboardSet;

namespace VCloud
{
	class VCloudSkySystem;

    class MoonManager 
	{
	public:
	    /** Constructor
		    @param s Parent SkyX pointer
		 */
		MoonManager(VCloudSkySystem *s);

		/** Destructor 
		 */
		~MoonManager();

		/** Create all resources
		 */
		void create();

		/** Remove all resources
		 */
		void remove();

		/** Update moon phase
		    @param phase Moon phase in [-1,1] range, where -1 means fully covered Moon, 0 clear Moon and 1 fully covered Moon
		 */
		void updateMoonPhase(const float& phase);

		/** Update geometry
		    @param c Camera
		 */
		void updateGeometry(BaseCamera* c);
	
		/** Get moon billboard
		    @return Moon billboard
		 */
		inline BillboardSet* getMoonBillboard()
		{
			return mMoonBillboard;
		}

		/** Get moon scene node
		    @return Moon scene node
		 */
		inline SceneNode* getMoonSceneNode()
		{
			return mMoonSceneNode;
		}

		/** Set moon size
		    @param MoonSize Moon size
		 */
		inline void setMoonSize(const float& MoonSize)
		{
			mMoonSize = MoonSize;
		}

		/** Get moon size
		    @return Moon size
		 */
		inline const float& getMoonSize() const
		{
			return mMoonSize;
		}

		/** Set moon halo intensity
		    @param MoonHaloIntensity Moon halo intensity
		 */
		inline void setMoonHaloIntensity(const float& MoonHaloIntensity)
		{
			mMoonHaloIntensity = MoonHaloIntensity;
		}

		/** Get moon halo intensity
		    @return Moon halo intensity
		 */
		inline const float& getMoonHaloIntensity() const
		{
			return mMoonHaloIntensity;
		}

		/** Set moon halo strength
		    @param MoonHaloStrength Moon halo strength (linear/exponential fading)
		 */
		inline void setMoonHaloStrength(const float& MoonHaloStrength)
		{
			mMoonHaloStrength = MoonHaloStrength;
		}

		/** Get moon halo strength
		    @return Moon halo strength (linear/exponential fading)
		 */
		inline const float& getMoonHaloStrength() const
		{
			return mMoonHaloStrength;
		}

		/** Is moon manager created?
		    @return true if yes, false if not
		 */
		inline const bool& isCreated() const
		{
			return mCreated;
		}

		void MoonManager::Render(BaseCamera* c);

		/// Moon material
		ShaderID mMoonMaterial;
	private:
		/** Update moon bounds
		    @param c Camera
	     */
		void _updateMoonBounds(BaseCamera* c);
		/// Moon billboard
		BillboardSet* mMoonBillboard;
		/// Moon scene node
		SceneNode* mMoonSceneNode;
		TextureID	mMoonTex, mMoonHaloTex;

		/// Is moon manager created?
		bool mCreated;

		/// Moon size
		float mMoonSize;
		/// Moon halo intensity
		float mMoonHaloIntensity;
		/// Moon halo strength
		float mMoonHaloStrength;

		

		/// SkyX parent pointer
		VCloudSkySystem *mSkyX;
	};
}

#endif