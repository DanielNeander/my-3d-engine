#include "DXUT.h"
#include <Core/GameLog.h>
#include "noRenderSystemDX9.h"

noRenderSystemDX9* g_renderSystem = NULL;

HRESULT noD3DResource::OnDeviceLoss( void )
{
	return S_OK;
}

HRESULT noD3DResource::OnDeviceRelease( void )
{
	return S_OK;
}

HRESULT noD3DResource::OnDeviceRestore( void )
{
	return S_OK;
}

noD3DResource::noD3DResource()
	:m_owner(NULL)
	//m_itResource(NULL)
{

}

noD3DResource::~noD3DResource()
{
			
}

HRESULT noRenderSystemDX9::AddResource( noD3DResource *pResource )
{
	if (!pResource)
	{
		SXTRACE("SXProbe ERROR: Invalid resource pointer passed to AddResource()\n");
		return E_POINTER;
	}

	if (pResource->m_owner)
	{
		if (pResource->m_owner != this)
		{
			SXTRACE("SXProbe ERROR: Attempting to add a resource managed by another SXProbe's control\n");
			return E_INVALIDARG;	// Baaad boy, trying to host a resource owned by another probe...
		}
		return S_OK;	// Already added to our list
	}

	pResource->m_owner = this;
	m_lstResources.push_back(pResource);
	pResource->m_itResource = --m_lstResources.end();
	return S_OK;

}

HRESULT noRenderSystemDX9::RemoveResource( noD3DResource *pResource )
{
	if (!pResource)
	{
		SXTRACE("SXProbe ERROR: Invalid resource pointer passed to RemoveResource()\n");
		return E_POINTER;
	}

	if (pResource->m_owner != this)
	{
		SXTRACE("SXProbe ERROR: Attempting to remove a resource not managed by this SXProbe\n");
		return E_INVALIDARG;	// We don't own that resource baby...
	}

	m_lstResources.erase(pResource->m_itResource);
	pResource->m_owner = NULL;
	//pResource->m_itResource = NULL;
	return S_OK;
}

HRESULT noRenderSystemDX9::NotifyLoss( void )
{
	if (m_bNotifiedAboutLastLoss)
		return S_OK;	// Skip if resources are already notified...

	SXTRACE("SXProbe INFO: Notifying resources about device loss\n");

	// Iterate registered resources
	HRESULT hRetval = S_OK;
	noD3DResourcesList::iterator itResources = m_lstResources.begin();
	while (itResources != m_lstResources.end())
	{
		noD3DResource *pResource = *itResources;
		hRetval = pResource->OnDeviceLoss();

		// You can handle notification failure here if you're interested...
		//if (FAILED(hRetval))
		//{
		// A warning sent to the debug output maybe?
		//}
		++itResources;
	}

	return hRetval;
}

HRESULT noRenderSystemDX9::NotifyRelease( void )
{
	SXTRACE("SXProbe INFO: Notifying resources about device release\n");

	// Iterate registered resources
	HRESULT hRetval = S_OK;
	noD3DResourcesList::iterator itResources = m_lstResources.begin();
	while (itResources != m_lstResources.end())
	{
		noD3DResource *pResource = *itResources;
		hRetval = pResource->OnDeviceRelease();

		// You can handle notification failure here if you're interested...
		//if (FAILED(hRetval))
		//{
		// A warning sent to the debug output maybe?
		//}
		++itResources;
	}

	return hRetval;
}

HRESULT noRenderSystemDX9::NotifyRestore( void )
{
	SXTRACE("SXProbe INFO: Notifying resources about device restore\n");

	// Iterate registered resources
	HRESULT hRetval = S_OK;
	noD3DResourcesList::iterator itResources = m_lstResources.begin();
	while (itResources != m_lstResources.end())
	{
		noD3DResource *pResource = *itResources;
		hRetval = pResource->OnDeviceRestore();

		// You can handle notification failure here if you're interested...
		//if (FAILED(hRetval))
		//{
		// A warning sent to the debug output maybe?
		//}
		++itResources;
	}

	return hRetval;
}

HRESULT noRenderSystemDX9::AttachToDevice( PDIRECT3DDEVICE9 pDevice,D3DPRESENT_PARAMETERS *pPresentParams /*= NULL*/ )
{
	if (!pDevice)
		return E_POINTER;

	HRESULT hRetval;
	if (m_pD3DDevice)
	{
		hRetval = DetachFromCurrentDevice();
		if (FAILED(hRetval))
			return hRetval;
	}

	// Save present params for restoration purposes
	if (pPresentParams)
		m_paramsPresent = *pPresentParams;
	else
	{
		// Present parameters not provided, get them from the device. Note that this
		// works fine even thought the device is in lost state...
		PDIRECT3DSWAPCHAIN9 pSwapChain = NULL;
		hRetval = pDevice->GetSwapChain(0,&pSwapChain);
		if (FAILED(hRetval))
			return hRetval;
		if (!pSwapChain)
			return E_POINTER;
		hRetval = pSwapChain->GetPresentParameters(&m_paramsPresent);
		SAFE_RELEASE(pSwapChain);
		if (FAILED(hRetval))
			return hRetval;
	}

	// Own it
	m_pD3DDevice = pDevice;
	m_pD3DDevice->AddRef();

	// If it's not operational, then don't notify resources about it
	hRetval = m_pD3DDevice->TestCooperativeLevel();
	if (FAILED(hRetval))
		return S_OK;

	// Notify the resources about the new device
	return NotifyRestore();
}


HRESULT noRenderSystemDX9::DetachFromCurrentDevice( void )
{
	if (!m_pD3DDevice)
		return D3DERR_INVALIDCALL;

	NotifyRelease();
	SAFE_RELEASE(m_pD3DDevice);

	return S_OK;
}

HRESULT noRenderSystemDX9::ProbeDevice( void )
{
	if (!m_pD3DDevice)
		return D3DERR_INVALIDCALL;

	HRESULT hRetval = m_pD3DDevice->TestCooperativeLevel();
	if (SUCCEEDED(hRetval))
		return hRetval;

	// We can't do much if the device isn't available to us
	if (hRetval != D3DERR_DEVICENOTRESET)
		return hRetval;

	return ResetDevice(NULL);
}

HRESULT noRenderSystemDX9::ResetDevice( D3DPRESENT_PARAMETERS *pPresentParams /*= NULL*/ )
{
	if (!m_pD3DDevice)
		return D3DERR_INVALIDCALL;

	// We're about to reset, make sure all D3DPOOL_DEFAULT resources
	// are released first...
	NotifyLoss();

	// Attempt to restore device
	if (pPresentParams)
		m_paramsPresent = *pPresentParams;

	HRESULT hRetval = m_pD3DDevice->Reset(&m_paramsPresent);
	if (FAILED(hRetval))
		return hRetval;

	m_bNotifiedAboutLastLoss = false;

	hRetval = NotifyRestore();
	if (FAILED(hRetval))
	{
		// If we arrived here, then one (or more) resource must have failed
		// restoring its data. It's totally up to you to decide what to do
		// in such case... Here, we just fail the function with the error code
		// returned from NotifyRestore()...
		SXTRACE("SXProbe WARN: One or more resources failed to restore!\n");
		return hRetval;
	}

	SXTRACE("SXProbe INFO: Resources restored successfully...\n");
	return S_OK;
}

HRESULT noRenderSystemDX9::GetD3DDevice( LPDIRECT3DDEVICE9 *ppOutD3DDevice )
{
	if (!ppOutD3DDevice)
		return E_POINTER;
	if (!m_pD3DDevice)
		return D3DERR_INVALIDCALL;
	*ppOutD3DDevice = m_pD3DDevice;
	m_pD3DDevice->AddRef();
	return S_OK;
}

noRenderSystemDX9::noRenderSystemDX9()
{


}

noRenderSystemDX9::~noRenderSystemDX9()
{

}

bool noRenderSystemDX9::Init()
{
	HRESULT hr = D3DXCreateEffectPool(&mD3DEffectPool);
	if (FAILED(hr))
	{
		ASSERT(false && "Failed to Create Effect Pool");
		return false;
	}
	return true;
}