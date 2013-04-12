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
#include "stdafx.h"
#include "BasicController.h"

namespace VCloud
{
	BasicController::BasicController(const bool& deleteBySkyX)
		: Controller(deleteBySkyX)
		, mTime(noVec3(14.0f, 7.50f, 20.50f))
		, mSunDirection(noVec3(0,1,0))
		, mMoonDirection(noVec3(0,-1,0))
		, mEastDirection(noVec2(0,1))
		, mMoonPhase(0)
	{
	}

	void BasicController::update(const Real& simDeltaTime)
	{
		mTime.x += simDeltaTime;

		if (mTime.x > 24)
		{
			mTime.x -= 24;
		} 
		else if (mTime.x < 0)
		{
			mTime.x += 24;
		}

		// 24h day: 
		// 0______A(Sunrise)_______B(Sunset)______24
		//                     

		float y,
			X = mTime.x,
			A = mTime.y,
			B = mTime.z,
			AB  = A+24-B,
			AB_ = B-A,
			XB  = X+24-B;

		if (X<A || X>B)
		{
			if (X<A)
			{
                y = -XB / AB;
			}
			else
			{
				y = -(X-B) / AB;
			}
            
			if (y > -0.5f)
			{
				y *= 2;
			}
			else
			{
				y = -(1 + y)*2;
			}
		}
		else
		{
			y = (X-A)/(B-A);

			if (y < 0.5f)
			{
				y *= 2;
			}
			else
			{
				y = (1 - y)*2;
			}
		}

		noVec2 East = mEastDirection;

		if (X > A && X < B)
		{
			if (X > (A + AB_/2))
			{
				East = -East;
			}
		}
		else
		{
			if (X<=A)
			{
				if (XB < (24-AB_)/2)
				{
					East = -East;
				}
			}
			else
			{
				if ((X-B) < (24-AB_)/2)
				{
					East = -East;
				}
			}
		}

		float ydeg = (noMath::PI/2)*y,
		      sn = noMath::Sin(ydeg),
		      cs = noMath::Cos(ydeg);

		mSunDirection = noVec3(East.x*cs, sn, East.y*cs);
		mMoonDirection = -mSunDirection;
	}
}