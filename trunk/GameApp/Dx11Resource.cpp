#include "stdafx.h"
#include "Dx11Resource.h"

using namespace SpeedTree;


///////////////////////////////////////////////////////////////////////
//  Static member variables

ID3D11Device* g_pDx11 = NULL;
ID3D11DeviceContext* g_pDx11Context = NULL;
ID3D11RenderTargetView* g_pMainRenderTargetView = NULL;
ID3D11DepthStencilView* g_pMainDepthStencilView = NULL;


///////////////////////////////////////////////////////////////////////  
//  DX10::Device

ID3D11Device* DX11::Device(void)
{ 
	return g_pDx11;
}

ID3D11DeviceContext*	DX11::Context( void )
{
	return g_pDx11Context;
}

///////////////////////////////////////////////////////////////////////  
//  DX11::SetDevice

void DX11::SetDevice(ID3D11Device* pDevice)
{ 
	g_pDx11 = pDevice; 
}

void DX11::SetContext(ID3D11DeviceContext* pContext)
{ 
	g_pDx11Context = pContext; 
}


///////////////////////////////////////////////////////////////////////  
//  DX10::MainRenderTargetView

ID3D11RenderTargetView* DX11::MainRenderTargetView(void)
{
	return g_pMainRenderTargetView;
}


///////////////////////////////////////////////////////////////////////  
//  DX10::SetMainRenderTargetView

void DX11::SetMainRenderTargetView(ID3D11RenderTargetView* pTarget)
{
	g_pMainRenderTargetView = pTarget;
}


///////////////////////////////////////////////////////////////////////  
//  DX10::MainDepthStencilView

ID3D11DepthStencilView* DX11::MainDepthStencilView(void)
{
	return g_pMainDepthStencilView;
}


///////////////////////////////////////////////////////////////////////  
//  DX10::SetMainDepthStencilView

void DX11::SetMainDepthStencilView(ID3D11DepthStencilView* pTarget)
{
	g_pMainDepthStencilView = pTarget;
}

