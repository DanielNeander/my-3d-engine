// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#ifndef VERTEXDECL_BUNDLE_H
#define VERTEXDECL_BUNDLE_H

#include "ResourceBundle.h"
#include "Proxies.h"
#include <Renderer/VertexFormat.h>
#include <Core/Singleton.h>
#include <Renderer/noRenderSystemDX9.h>

// --------------------------------------------------------------------------

/**
 *  Vertex stream descriptor. Contains stream's CVertexFormat and starting UV
 *  index for a stream.
 */
class CVertexStreamDesc {
public:
	CVertexStreamDesc( const CVertexFormat& fmt, int uvIndex = 0 )
		: mFormat(fmt), mUVIndex(uvIndex) { }

	const CVertexFormat& getFormat() const { return mFormat; }
	int getUVIndex() const { return mUVIndex; }

	bool operator< ( const CVertexStreamDesc& rhs ) const {
		if( mFormat < rhs.mFormat )
			return true;
		if( rhs.mFormat < mFormat )
			return false;
		return mUVIndex < rhs.mUVIndex;
	}
	bool operator== ( const CVertexStreamDesc& rhs ) const {
		return mFormat==rhs.mFormat && mUVIndex==rhs.mUVIndex;
	}
	bool operator!= ( const CVertexStreamDesc& rhs ) const {
		return mFormat!=rhs.mFormat || mUVIndex!=rhs.mUVIndex;
	}
private:
	CVertexFormat	mFormat;
	int				mUVIndex;
};


// --------------------------------------------------------------------------

/**
 *  Complete vertex declatator. Essentially a vector of CVertexStreamDesc.
 */
class CVertexDesc {
public:
	typedef std::vector<CVertexStreamDesc> TStreamDescVector;

public:
	CVertexDesc() { }
	CVertexDesc( const CVertexFormat& fmt, int uvIndex = 0 ) { mStreams.push_back(CVertexStreamDesc(fmt,uvIndex)); }
	CVertexDesc( const TStreamDescVector& desc ) : mStreams(desc) { }
	CVertexDesc( const CVertexDesc& r ) : mStreams(r.mStreams) { }
	const CVertexDesc& operator=( const CVertexDesc& r ) { mStreams = r.mStreams; }

	bool operator< ( const CVertexDesc& r ) const { return mStreams < r.mStreams; }
	bool operator== ( const CVertexDesc& r ) const { return mStreams == r.mStreams; }
	bool operator!= ( const CVertexDesc& r ) const { return mStreams != r.mStreams; }

	const TStreamDescVector& getStreams() const { return mStreams; }
	TStreamDescVector& getStreams() { return mStreams; }
	
private:
	TStreamDescVector	mStreams;
};


// --------------------------------------------------------------------------

class CVertexDeclBundle :	public IResourceBundle,
							public CSingleton<CVertexDeclBundle>,
							public noD3DResource
							
{
private:
	typedef std::map<CVertexDesc, CD3DVertexDecl*> TResourceMap;
public:
	CD3DVertexDecl* getResourceById( const CVertexDesc& id ) {
		CD3DVertexDecl* resource = findResource( id );
		if( !resource ) {
			resource = loadResourceById( id );
			assert( resource );
			mResourceMap.insert( std::make_pair( id, resource ) );
		}
		return resource;
	}

	void clear() {
		for( TResourceMap::iterator it = mResourceMap.begin(); it != mResourceMap.end(); ) {
			assert( it->second );
			deleteResource( *it->second );
			it = mResourceMap.erase( it );
		}
	}

	void createResource();	
	void deleteResource();

protected:
	CD3DVertexDecl* loadResourceById( const CVertexDesc& id );

	CD3DVertexDecl* findResource( const CVertexDesc& id ) {
		TResourceMap::const_iterator it = mResourceMap.find( id );
		return ( it != mResourceMap.end() ) ? it->second : NULL;
	}

	void deleteResource( CD3DVertexDecl& resource ) {
		if( resource.getObject() )
			resource.getObject()->Release();
		delete &resource;
	}

private:
	IMPLEMENT_SIMPLE_SINGLETON(CVertexDeclBundle);
	CVertexDeclBundle() { };
	virtual ~CVertexDeclBundle() { clear(); };
	
	IDirect3DVertexDeclaration9* createDecl( const CVertexDesc& d ) const;

private:
	TResourceMap	mResourceMap;
};


/// Shortcut macro
#define RGET_VDECL(rid) CVertexDeclBundle::GetInstance().getResourceById(rid)



#endif