#ifndef FRAMEWORK_RESOURCESYSTEM_H
#define FRAMEWORK_RESOURCESYSTEM_H

#include <list>

#include "RenderStats.h"
#include "EffectStateManager.h"

#define SXTRACE(text) LOG_DEBUG << (text)

class noD3DResource;
typedef std::list<noD3DResource*> noD3DResourcesList;

class noRenderSystemDX9 
{

public:
	noRenderSystemDX9();
	~noRenderSystemDX9();

	bool	Init();
	
	HRESULT AttachToDevice(PDIRECT3DDEVICE9 pDevice,D3DPRESENT_PARAMETERS *pPresentParams = NULL);
	HRESULT DetachFromCurrentDevice(void);
	HRESULT ProbeDevice(void);
	HRESULT ResetDevice(D3DPRESENT_PARAMETERS *pPresentParams = NULL);
	
	// Resource Management
	virtual HRESULT AddResource(noD3DResource *pResource);
	virtual HRESULT RemoveResource(noD3DResource *pResource);

	// Access
	LPDIRECT3DDEVICE9 GetD3DDevice(void);	// Fast Access
	HRESULT GetD3DDevice(LPDIRECT3DDEVICE9 *ppOutD3DDevice);
	D3DPRESENT_PARAMETERS& GetPresentParams(void);

	const CRenderStats& getStats() const { return mStats; }
	CRenderStats& getStats() { return mStats; }

	const CEffectStateManager& getStateManager() const { return mStateManager; }
	CEffectStateManager& getStateManager() { return mStateManager; }

	const ID3DXEffectPool* GetD3DEffectPool() const { return mD3DEffectPool; }
	ID3DXEffectPool* GetD3DEffectPool() { return mD3DEffectPool; }

private:
	virtual HRESULT NotifyLoss(void);
	virtual HRESULT NotifyRelease(void);
	virtual HRESULT NotifyRestore(void);

	// Internal Members
	PDIRECT3DDEVICE9 m_pD3DDevice;	// Reference Counted
	noD3DResourcesList m_lstResources;	// Registered Resources List
	D3DPRESENT_PARAMETERS m_paramsPresent;	// For restoration purposes
	bool m_bNotifiedAboutLastLoss;	// Did we notify our resources about device loss already?

	CEffectStateManager	mStateManager;	
	CRenderStats		mStats;
	ID3DXEffectPool*	mD3DEffectPool;
};

extern noRenderSystemDX9* g_renderSystem;

///////////////// SXProbe Inline Functions Implementation /////////////////
inline LPDIRECT3DDEVICE9 noRenderSystemDX9::GetD3DDevice(void)
{
	return m_pD3DDevice;
}

inline D3DPRESENT_PARAMETERS& noRenderSystemDX9::GetPresentParams(void)
{
	return m_paramsPresent;
}

class noD3DResource {

public:
	noD3DResource();
	virtual ~noD3DResource();

protected:
	friend class noRenderSystemDX9;	// Open access for the probe

	// Overrides
	virtual void createResource() { g_renderSystem->AddResource(this); }
	virtual void deleteResource() { g_renderSystem->RemoveResource(this); }
	virtual HRESULT OnDeviceLoss(void);
	virtual HRESULT OnDeviceRelease(void);
	virtual HRESULT OnDeviceRestore(void);

	// Internal Members
	noRenderSystemDX9* m_owner;

private:
	noD3DResourcesList::iterator m_itResource;	// For fast list access
};

class noReloadableResource : public noD3DResource {
public:

	virtual void Reload() {


	}
};

#endif