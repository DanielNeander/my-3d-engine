#include "stdafx.h"
#include "M2_Manager.h"

TextureManager texturemanager;

unsigned int TextureManager::add( wxString name )
{
	unsigned int id = 0;

	// if the item already exists, return the existing ID
	if (names.find(name) != names.end()) {
		id = names[name];
		items[id]->addref();
		return id;
	}

	// Else, create the texture

	Texture *tex = new Texture(name);
	if (tex) {
		// Clear Old Texture
		tex->id = id;
		//LoadBLP(id, tex);

		do_add(name, id, tex);
		return id;
	}

	return 0;

}

void TextureManager::doDelete( unsigned int id )
{

}
