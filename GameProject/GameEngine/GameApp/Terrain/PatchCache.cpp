// Copyright 2011 Intel Corporation
// All Rights Reserved
//
// Permission is granted to use, copy, distribute and prepare derivative works of this
// software for any purpose and without fee, provided, that the above copyright notice
// and this statement appear in all copies.  Intel makes no representations about the
// suitability of this software for any purpose.  THIS SOFTWARE IS PROVIDED "AS IS."
// INTEL SPECIFICALLY DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED, AND ALL LIABILITY,
// INCLUDING CONSEQUENTIAL AND OTHER INDIRECT DAMAGES, FOR THE USE OF THIS SOFTWARE,
// INCLUDING LIABILITY FOR INFRINGEMENT OF ANY PROPRIETARY RIGHTS, AND INCLUDING THE
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  Intel does not
// assume any responsibility for any errors which may appear in this software nor any
// responsibility to update it.
#include "stdafx.h"
#include "PatchCache.h"

CDX11PatchCache::CDX11PatchCache(ID3D11Device *pd3dDevice11)
  : m_device(pd3dDevice11)
{
	InitializeCriticalSection(&m_cs);
}

CDX11PatchCache::~CDX11PatchCache()
{
	DeleteCriticalSection(&m_cs);
}

void CDX11PatchCache::Clear()
{
	m_groups.clear();
}

void CDX11PatchCache::EnterCS() {
	EnterCriticalSection(&m_cs);
}

void CDX11PatchCache::LeaveCS() {
	LeaveCriticalSection(&m_cs);
}

// Gets texture from the cache. If appropriate resource has not been found, creates it
// Returning false means that resource doesn't have requested identity and have to be filled again
bool CDX11PatchCache::GetPatchTexture(const D3D11_TEXTURE2D_DESC &desc,
                                      ID3D11ShaderResourceView **ppSRView, ID3D11RenderTargetView **ppRTView)
{
	assert((desc.BindFlags & D3D11_BIND_RENDER_TARGET) || (desc.BindFlags & D3D11_BIND_SHADER_RESOURCE));
	assert(desc.MipLevels);
	assert(ppSRView || ppRTView);

	EnterCriticalSection(&m_cs);
    
    // Try to find group with matching description
	for( GroupListType::iterator itGroup = m_groups.begin(); itGroup != m_groups.end(); ++itGroup )
	{
		if( 0 == memcmp(&itGroup->desc, &desc, sizeof(desc)) )
		{
			if( !itGroup->unitializedList.empty() )
			{
                // If there is a resource in the group, return it
				if( ppSRView ) *ppSRView = itGroup->unitializedList.back().srv.Detach();
				if( ppRTView ) *ppRTView = itGroup->unitializedList.back().rtv.Detach();
				itGroup->unitializedList.pop_back();
			}
			else
			{
				break; // no compatible resource found
			}
			LeaveCriticalSection(&m_cs);
			assert(!(desc.BindFlags & D3D11_BIND_RENDER_TARGET) ^ (ppRTView && *ppRTView || (desc.MiscFlags & D3D11_RESOURCE_MISC_GENERATE_MIPS)));
			assert(!(desc.BindFlags & D3D11_BIND_SHADER_RESOURCE) ^ (ppSRView && *ppSRView));
			return false;
		}
	}
	LeaveCriticalSection(&m_cs);

	// no compatible resource found; create a new one
	CComPtr<ID3D11Texture2D> pTexture2D;
	HRESULT hr = m_device->CreateTexture2D(&desc, NULL, &pTexture2D);
	if( FAILED(hr) )
		throw std::runtime_error("CreateTexture2D failed");

	if( desc.BindFlags & D3D11_BIND_RENDER_TARGET )
	{
		assert(ppRTView || (desc.MiscFlags & D3D11_RESOURCE_MISC_GENERATE_MIPS));
		hr = m_device->CreateRenderTargetView(pTexture2D, NULL, ppRTView);
		if( FAILED(hr) )
			throw std::runtime_error("CreateRenderTargetView failed");
	}
	if( desc.BindFlags & D3D11_BIND_SHADER_RESOURCE )
	{
		assert(ppSRView);
		hr = m_device->CreateShaderResourceView(pTexture2D, NULL, ppSRView);
		if( FAILED(hr) )
			throw std::runtime_error("CreateShaderResourceView failed");
	}
	return false;
}

// Returns the resource back into the cache
void CDX11PatchCache::ReleasePatchTexture(ID3D11ShaderResourceView *pSRView,
                                          ID3D11RenderTargetView *pRTView)
{
	if( ID3D11View *any = pSRView ? pSRView : static_cast<ID3D11View*>(pRTView) )
	{
		EnterCriticalSection(&m_cs);

		// get texture desc
		D3D11_TEXTURE2D_DESC desc;
		CComPtr<ID3D11Resource> resource;
		any->GetResource(&resource);
		CComQIPtr<ID3D11Texture2D>(resource)->GetDesc(&desc);

		// validate bind flags
		assert(!(desc.BindFlags & D3D11_BIND_RENDER_TARGET) ^ (pRTView || (desc.MiscFlags & D3D11_RESOURCE_MISC_GENERATE_MIPS)));
		assert(!(desc.BindFlags & D3D11_BIND_SHADER_RESOURCE) ^ !!pSRView);

        // Find appropriate group
		GroupListType::iterator itGroup = m_groups.begin();
		for( ; itGroup != m_groups.end(); ++itGroup )
		{
			if( 0 == memcmp(&itGroup->desc, &desc, sizeof(desc)) )
			{
				break;
			}
		}

		// create a new resource group if none was found
		if( itGroup == m_groups.end() )
		{
			itGroup = m_groups.insert(m_groups.end(), ResourceGroup());
			itGroup->desc = desc;
		}

        // Add resource to the group's list
		itGroup->unitializedList.push_back(PooledResource());
		itGroup->unitializedList.back().srv.Attach(pSRView);
		itGroup->unitializedList.back().rtv.Attach(pRTView);

		LeaveCriticalSection(&m_cs);
	}
}
// end of file
