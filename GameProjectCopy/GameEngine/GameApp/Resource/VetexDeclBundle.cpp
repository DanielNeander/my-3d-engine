#include "DXUT.h"
#include <vector>
#include <map>
#include <Renderer/noRenderSystemDX9.h>
#include "VertexDeclBundle.h"


IDirect3DVertexDeclaration9* CVertexDeclBundle::createDecl( const CVertexDesc& d ) const
{
	int i;
	int n = d.getStreams().size();

	// count elements
	int nelems = 0;
	for( i = 0; i < n; ++i )
		nelems += d.getStreams()[i].getFormat().calcComponentCount();

	// construct elements
	D3DVERTEXELEMENT9* els = new D3DVERTEXELEMENT9[nelems+1];
	ASSERT( els );
	D3DVERTEXELEMENT9* e = els;
	for( i = 0; i < n; ++i ) {
		const CVertexStreamDesc& s = d.getStreams()[i];
		s.getFormat().calcVertexDecl( e, i, s.getUVIndex() );
		e += s.getFormat().calcComponentCount();
	}
	D3DVERTEXELEMENT9 elEnd = D3DDECL_END();
	*e = elEnd;

	// create vertex declaration
	HRESULT hres;
	IDirect3DVertexDeclaration9* decl = NULL;

	hres = g_renderSystem->GetD3DDevice()->CreateVertexDeclaration( els, &decl );
	if( FAILED( hres ) ) {
		std::string msg = "failed to create vertex decl";
		LOG_ERR << msg.c_str();
		//CConsole::CON_ERROR.write(msg);
		//THROW_DXERROR( hres, msg );
	}
	ASSERT( decl );

	delete[] els;

	return decl;
}


CD3DVertexDecl* CVertexDeclBundle::loadResourceById( const CVertexDesc& id )
{
	IDirect3DVertexDeclaration9* decl = createDecl( id );
	return new CD3DVertexDecl( decl );
}

void CVertexDeclBundle::createResource()
{
	// recreate all objects
	TResourceMap::iterator it;
	for( it = mResourceMap.begin(); it != mResourceMap.end(); ++it ) {
		const CVertexDesc& fmt = it->first;
		CD3DVertexDecl& res = *it->second;
		res.setObject( createDecl( fmt ) );
		ASSERT( !res.isNull() );
	}

	noD3DResource::createResource();
}

void CVertexDeclBundle::deleteResource()
{
	// unload all objects
	TResourceMap::iterator it;
	for( it = mResourceMap.begin(); it != mResourceMap.end(); ++it ) {
		CD3DVertexDecl& res = *it->second;
		ASSERT( !res.isNull() );
		res.getObject()->Release();
		res.setObject( NULL );
	}

	noD3DResource::deleteResource();
}
