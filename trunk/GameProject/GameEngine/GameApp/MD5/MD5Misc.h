/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company. 

This file is part of the Doom 3 GPL Source Code (?Doom 3 Source Code?).  

Doom 3 Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#ifndef __IDGAME_MISC_H__
#define __IDGAME_MISC_H__


/*
===============================================================================

idSpawnableEntity

A simple, spawnable entity with a model and no functionable ability of it's own.
For example, it can be used as a placeholder during development, for marking
locations on maps for script, or for simple placed models without any behavior
that can be bound to other entities.  Should not be subclassed.
===============================================================================
*/
class idSpawnableEntity : public idEntity {
public:
	CLASS_PROTOTYPE( idSpawnableEntity );

	void				Spawn( void );

private:
};



/*
===============================================================================

  Non-displayed entity used to activate triggers when it touches them.
  Bind to a mover to have the mover activate a trigger as it moves.
  When target by triggers, activating the trigger will toggle the
  activator on and off. Check "start_off" to have it spawn disabled.
	
===============================================================================
*/

class idActivator : public idEntity {
public:
	CLASS_PROTOTYPE( idActivator );

	void				Spawn( void );

	//void				Save( idSaveGame *savefile ) const;
	//void				Restore( idRestoreGame *savefile );

	//virtual void		Think( void );

private:
	bool				stay_on;

	void				Event_Activate( idEntity *activator );
};


class idWorldspawn : public idEntity {
public:
	CLASS_PROTOTYPE( idWorldspawn );

	~idWorldspawn();

	void			Spawn( void );

private:
	void			Event_Remove( void );
};


#endif