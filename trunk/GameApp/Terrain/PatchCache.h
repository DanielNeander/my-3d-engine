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
#pragma once

#include <deque>
#include "DynamicQuadTreeNode.h"

// Type of the resource in a cache
enum DX11TextureType
{
	TT_UNINITIALIZED = 0,
	TT_ELEVATION,
	TT_NORMAL,
	TT_DIFFUSE,
};

// Class implementing patch cache functionality
class CDX11PatchCache
{
public:
	CDX11PatchCache(ID3D11Device *pd3dDevice11);
	~CDX11PatchCache();

    // Clears the cache
	void Clear();
	ID3D11Device* GetDevice() const { return m_device; }

	// Gets texture from the cache. If appropriate resource has not been found, creates it
    // Returning false means that resource doesn't have requested identity and have to be filled again
	bool GetPatchTexture(const D3D11_TEXTURE2D_DESC &desc, ID3D11ShaderResourceView **ppSRView, ID3D11RenderTargetView **ppRTView);
    
    // Returns the resource back into the cache
	void ReleasePatchTexture(ID3D11ShaderResourceView *pSRView, ID3D11RenderTargetView *pRTView);
	
	void EnterCS();
	void LeaveCS();
private:
	struct PooledResource
	{
		CComPtr<ID3D11ShaderResourceView> srv;
		CComPtr<ID3D11RenderTargetView> rtv;
	};
	typedef std::deque<PooledResource> ListType;

	struct ResourceGroup
	{
		D3D11_TEXTURE2D_DESC desc;
		ListType unitializedList; // List of available resources of that type
	};
	typedef std::vector<ResourceGroup> GroupListType; // Array of free resources lists for each resource type

	GroupListType m_groups;
	CComPtr<ID3D11Device> m_device;
	CRITICAL_SECTION m_cs;

	CDX11PatchCache(CDX11PatchCache&); // no copy
	CDX11PatchCache& operator = (CDX11PatchCache&);
	
};



// end of file
