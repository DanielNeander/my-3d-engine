#include <U2_3D/Src/U23DLibPCH.h>
#include "U2Dx9FrameShader.h"


//-------------------------------------------------------------------------------------------------
U2FrameShader::U2FrameShader()
:m_uiBucketIdx((unsigned int)-1),
m_pEffectShader(0),
m_pCachedSahder(0)
{

}

//-------------------------------------------------------------------------------------------------
U2FrameShader::~U2FrameShader()
{
		
}

//-------------------------------------------------------------------------------------------------
void U2FrameShader::Validate()
{
	U2D3DXEffectShader* pShader = 0;
	if(!m_pEffectShader)
	{
		U2ASSERT(!m_szFilename.IsEmpty());

		//// Filename으로 체크..
		//U2D3DXEffectShader* pCachedShader = U2D3DXEffectShader::GetHead();
		//while(pCachedShader)
		//{
		//	if(pCachedShader->GetFilename() == this->GetFilename())
		//	{
		//		m_spEffectShader = pCachedShader;
		//		pCachedShader->IncRefCount();
		//		break;
		//	}
		//	pCachedShader = pCachedShader->GetNext();
		//}

		m_pEffectShader = FindEffectShader(this->GetFilename());

		pShader = m_pEffectShader;

		if(!m_pEffectShader)
		{
			pShader = U2_NEW U2D3DXEffectShader(U2Dx9Renderer::GetRenderer());			
		
			pShader->SetFilename(m_szFilename);

				if(!pShader->LoadResource())
			{
				
				U2ASSERT(FALSE);
				FDebug("Could not load shader file '%s'!\n", m_szFilename.Str());
				U2_DELETE pShader;
				pShader = NULL;
				return;
			}

			m_pEffectShader = pShader;

			pShader =m_pEffectShader;
		}
	}
	else 
		pShader = m_pEffectShader;

	U2ASSERT(pShader);
	
}

U2D3DXEffectShader* U2FrameShader::FindEffectShader(const U2DynString& szShderFilename)
{
	//// Filename으로 체크..
	U2D3DXEffectShader* pCachedShader = U2D3DXEffectShader::GetHead();

	U2D3DXEffectShader* pTargetShader = 0;
	while(pCachedShader)
	{
		if(pCachedShader->GetFilename() == szShderFilename)
		{
			pTargetShader = pCachedShader;			
			break;
		}
		pCachedShader = pCachedShader->GetNext();
	}

	return pTargetShader;
}

