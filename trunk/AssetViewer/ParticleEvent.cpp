#include "stdafx.h"
#include "particleevent.h"

void CParticleEvent::ProcessPropEqualsValue(CMinMax<float> &prop,
											std::vector<CParticleEmitterToken>::iterator &TokenIter, 
											std::vector<CParticleEmitterToken>::iterator &EndIter)
{
	// next token should be =
	CParticleEmitterTokenizer::NextToken(TokenIter, EndIter);
	if (TokenIter->m_Type != Equals) throw("Expecting = after property.");

	// next token should be a number
	CParticleEmitterTokenizer::NextToken(TokenIter, EndIter);
	CParticleEmitterTokenizer::ProcessNumber(prop, TokenIter, EndIter);
}

void CParticleEvent::ProcessPropEqualsValue(CMinMax<D3DXVECTOR3> &prop,
											std::vector<CParticleEmitterToken>::iterator &TokenIter, 
											std::vector<CParticleEmitterToken>::iterator &EndIter)
{
	// next token should be =
	CParticleEmitterTokenizer::NextToken(TokenIter, EndIter);
	if (TokenIter->m_Type != Equals) throw("Expecting = after property.");

	// next token should be a number
	CParticleEmitterTokenizer::NextToken(TokenIter, EndIter);
	CParticleEmitterTokenizer::ProcessVector(prop, TokenIter, EndIter);
}

void CParticleEvent::ProcessPropEqualsValue(CMinMax<D3DXCOLOR> &prop,
											std::vector<CParticleEmitterToken>::iterator &TokenIter, 
											std::vector<CParticleEmitterToken>::iterator &EndIter)
{
	// next token should be =
	CParticleEmitterTokenizer::NextToken(TokenIter, EndIter);
	if (TokenIter->m_Type != Equals) throw("Expecting = after property.");

	// next token should be a number
	CParticleEmitterTokenizer::NextToken(TokenIter, EndIter);
	CParticleEmitterTokenizer::ProcessColor(prop, TokenIter, EndIter);
}

bool CParticleEventSize::ProcessTokenStream(std::vector<CParticleEmitterToken>::iterator &TokenIter, 
											 std::vector<CParticleEmitterToken>::iterator &EndIter)
{
	if (TokenIter->m_strValue.find("SIZE") == std::string::npos) throw("Expecting Size!");
	ProcessPropEqualsValue(m_Size, TokenIter, EndIter);
	return(true);
}