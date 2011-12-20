#include <U2_3D/src/U23DLibPCH.h>
#include "U2Dx9IndexBufferMgr.h"


U2Dx9IndexBufferMgr::U2Dx9IndexBufferMgr(LPDIRECT3DDEVICE9 pD3DDev)
{
	m_pD3DDev = pD3DDev;
	m_pD3DDev->AddRef();

	m_pTempPointConnIB = 0;
	m_uTempPointConnLen = 0	;
	m_pTempLineConnIB = 0;
	m_uTempLineConnLen = 0;
}


U2Dx9IndexBufferMgr::~U2Dx9IndexBufferMgr()
{
	SAFE_RELEASE(m_pTempPointConnIB);
	SAFE_RELEASE(m_pTempLineConnIB);
	SAFE_RELEASE(m_pD3DDev);
}

LPDIRECT3DINDEXBUFFER9 U2Dx9IndexBufferMgr::PackIB(const unsigned short* pusTriList, 
							  unsigned int uiLength, unsigned int uiMaxLength, 
							  LPDIRECT3DINDEXBUFFER9 pkOldIB, unsigned int& uiIBSize, 
							  D3DPOOL ePool, unsigned int uiUsage)
{

	if(!m_pD3DDev)
		return NULL;

	if(!pusTriList)
		return NULL;

	LPDIRECT3DINDEXBUFFER9 pD3DIB = NULL;
	HRESULT hr;

	uint32 uCreateSize = uiMaxLength * sizeof(unsigned short);
	uint32 uCopySize = uiLength * sizeof(unsigned short);
	U2ASSERT(uCreateSize >= uCopySize);

	if(pkOldIB)
	{
		D3DINDEXBUFFER_DESC IBDesc;

		memset((void*)&IBDesc, 0, sizeof(IBDesc));

		hr = pkOldIB->GetDesc(&IBDesc);
		if(SUCCEEDED(hr))
		{
			if (   (IBDesc.Format == D3DFMT_INDEX16) 
				&& (IBDesc.Type == D3DRTYPE_INDEXBUFFER) 
				&& (IBDesc.Usage == uiUsage) 
				&& (IBDesc.Pool == ePool) 
				&& (IBDesc.Size >= uCreateSize)
				)
			{
				pD3DIB = pkOldIB;
			}
		}

	}

	if(!pD3DIB)
	{
		pD3DIB = CreateIndexBuffer(uCreateSize, uiUsage, D3DFMT_INDEX16, ePool);
		if(!pD3DIB)
		{
			U2ASSERT(_T("Create Failed to Index Buffer"));
			return NULL;
		}

		uiIBSize = uCreateSize;
	}

	if(pD3DIB && uCopySize > 0)
	{
		void* pvData = NULL;

		hr = pD3DIB->Lock(0, uCopySize, &pvData, 0);
		if(SUCCEEDED(hr))
		{
			U2ASSERT(pvData);
			memcpy_s(pvData, uCopySize, (const void*)pusTriList, uCopySize);
			pD3DIB->Unlock();
		}
		else 
		{
			U2ASSERT(_T("Failed to Lock Index Buffer"));
			pD3DIB->Unlock();
			SAFE_RELEASE(pD3DIB);

		}
	}
	return pD3DIB;
}


LPDIRECT3DINDEXBUFFER9 U2Dx9IndexBufferMgr::ResizeTempPointConnect(unsigned int uiPoints, 
											  bool bRefill, unsigned int uiUsage, 
											  D3DPOOL ePool )
{
	if(m_pD3DDev == 0)
		return NULL;

	if(uiPoints == 0)
		return NULL;

	LPDIRECT3DINDEXBUFFER9 pD3DIB = NULL;
	LPDIRECT3DINDEXBUFFER9 pOldIB = NULL;
	HRESULT hr;

	unsigned int uiSize = uiPoints * sizeof(unsigned short) * 6;

	pOldIB = m_pTempPointConnIB;
	if(pOldIB)
	{
		D3DINDEXBUFFER_DESC IBDesc;

		memset((void*)&IBDesc, 0, sizeof(IBDesc));
		hr = pOldIB->GetDesc(&IBDesc);
		if(SUCCEEDED(hr))
		{
			if (   (IBDesc.Format == D3DFMT_INDEX16) 
				&& (IBDesc.Type == D3DRTYPE_INDEXBUFFER) 
				&& (IBDesc.Usage == uiUsage) 
				&& (IBDesc.Pool == ePool) 
				&& (IBDesc.Size >= uiSize)
				)
			{
				pD3DIB = pOldIB;
				//  No need to repack...
				if (!bRefill)
					return pD3DIB;
			}
			else
			{
				//  Assume it's no good
				SAFE_RELEASE(pOldIB);
			}
		}
	}

	if(!pOldIB)
	{
		pD3DIB = CreateIndexBuffer(uiSize, uiUsage, D3DFMT_INDEX16, ePool);
		if(!pD3DIB)
		{
			U2ASSERT(_T("Failed to Create Index Buffer"));
			return NULL;
		}

		bRefill = true;
	}

	if(pD3DIB && bRefill)
	{
		void* pvData = NULL;

		hr = pD3DIB->Lock(0, uiSize, &pvData, 0);
		if(SUCCEEDED(hr))
		{
			uint16* pusTemp = (uint16*)pvData;
			uint32 uiBase = 0;
			uint32 u;
			for(u=0; u < uiPoints; u++)
			{
				*(pusTemp++) = uiBase + 0;
				*(pusTemp++) = uiBase + 1;
				*(pusTemp++) = uiBase + 2;
				*(pusTemp++) = uiBase + 0;
				*(pusTemp++) = uiBase + 2;
				*(pusTemp++) = uiBase + 3;

				uiBase += 4;
			}
			pD3DIB->Unlock();
		}
		else 
		{
			U2ASSERT(_T("Failed to Lock Index Buffer"));
			pD3DIB->Unlock();
			SAFE_RELEASE(pD3DIB);
			pD3DIB = 0;
		}				
	}

	m_pTempPointConnIB = pD3DIB;
	m_uTempPointConnLen = uiSize;
	return pD3DIB;
}

LPDIRECT3DINDEXBUFFER9 U2Dx9IndexBufferMgr::ResizeTempLineConnect(unsigned int uiPoints, 
											 unsigned char* pbFlags, unsigned int& uiConn, unsigned int uiUsage, 
											 D3DPOOL ePool)
{
	if(m_pD3DDev == 0)
		return NULL;

	if(uiPoints == 0)
		return NULL;
	
	if(pbFlags == 0)
		return NULL;

	LPDIRECT3DINDEXBUFFER9 pD3DIB = NULL;
	LPDIRECT3DINDEXBUFFER9 pOldIB = NULL;
	HRESULT hr;

	uint32 uiSize = (uiPoints * 2 + 2) * sizeof(uint16);
	pOldIB = m_pTempLineConnIB;

	if(pOldIB)
	{
		D3DINDEXBUFFER_DESC IBDesc;

		memset((void*)&IBDesc, 0, sizeof(IBDesc));
		
		hr = pOldIB->GetDesc(&IBDesc);
		if(SUCCEEDED(hr))
		{
			if (   (IBDesc.Format == D3DFMT_INDEX16) 
				&& (IBDesc.Type == D3DRTYPE_INDEXBUFFER) 
				&& (IBDesc.Usage == uiUsage) 
				&& (IBDesc.Pool == ePool) 
				&& (IBDesc.Size >= uiSize)
				)
			{
				pD3DIB = pOldIB;
				
			}
			else
			{
				//  Assume it's no good
				SAFE_RELEASE(pOldIB);
			}
		}		
	}

	if(!pOldIB)
	{
		pD3DIB = CreateIndexBuffer(uiSize, uiUsage, D3DFMT_INDEX16, ePool);
		if(!pD3DIB)
		{
			U2ASSERT(_T("Failed to Create Index Buffer"));
			return NULL;
		}		
	}

	if(pD3DIB)
	{
		void* pvData = NULL;
		uiConn = 0;

		hr = pD3DIB->Lock(0, uiSize, &pvData, 0);
		if(SUCCEEDED(hr))
		{
			uint16* pusConnTmp = (uint16*)pvData;
			uint32 i;
			uint32 uMax = uiPoints - 1;
			for(i = 0; i < uMax; ++i)
			{
				if(*pbFlags)
				{
					pusConnTmp[0] = (uint16)i;
					pusConnTmp[1] = (uint16)(i+1);
					pusConnTmp += 2;
					uiConn += 2;					
				}
				pbFlags++;
			}

			if(*pbFlags)
			{
				pusConnTmp[0] = (uint16)uMax;
				pusConnTmp[1] = (uint16)0;
				pusConnTmp += 2;
				uiConn += 2;								
			}

			

			pD3DIB->Unlock();
		}
		else 
		{
			pD3DIB->Unlock();
			SAFE_RELEASE(pD3DIB);
			pD3DIB = 0;
		}
	}
	m_pTempLineConnIB = pD3DIB;
	m_uTempLineConnLen = uiSize;
	return pD3DIB;
}

void U2Dx9IndexBufferMgr::DestoryBuffers()
{
	SAFE_RELEASE(m_pTempPointConnIB);
	SAFE_RELEASE(m_pTempLineConnIB);
}


LPDIRECT3DINDEXBUFFER9 U2Dx9IndexBufferMgr::CreateIndexBuffer(
	unsigned int uiLength, unsigned int uiUsage, D3DFORMAT eFormat, 
	D3DPOOL ePool, D3DINDEXBUFFER_DESC* pkDesc)
{
	LPDIRECT3DINDEXBUFFER9 pIB = NULL;
	U2ASSERT(m_pD3DDev);

	HRESULT hr = m_pD3DDev->CreateIndexBuffer(uiLength, uiUsage, eFormat, ePool, &pIB, NULL);
	if(SUCCEEDED(hr))
	{
		if(pkDesc)
		{
			pIB->GetDesc(pkDesc);
		}
	}
	else 
	{
		pIB = NULL;
	}

	return pIB;
}

unsigned short* U2Dx9IndexBufferMgr::LockIB(LPDIRECT3DINDEXBUFFER9 pIB,
	unsigned int uiOffset, unsigned int uiSize, 
					   unsigned int uiLockFlags)
{
	unsigned short* retVal = 0;
	void* pvData = NULL;
	if(FAILED(pIB->Lock(uiOffset, uiSize, &pvData, uiLockFlags)))
	{
		return NULL;
	}		
	
	retVal = (unsigned short*)pvData;

	return retVal;
}

bool  U2Dx9IndexBufferMgr::UnlockIB(LPDIRECT3DINDEXBUFFER9 pIB)
{
	return SUCCEEDED(pIB->Unlock());	
}
