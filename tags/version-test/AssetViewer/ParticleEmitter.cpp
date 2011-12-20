#include "stdafx.h"
#include "ParticleEmitter.h"
#include "ParticleEmitterTokenizer.h"

bool CParticleEmitter::Compile(const char* szScript)
{
	m_szLastError = "0 error(s), you're good to go!";

	try {
		CParticleEmitterTokenizer tokenizer;
		// 1. √ ±‚»≠
		InvalidateDeviceObjects();
		Init();	
		// 2. Parse
		tokenizer.Tokenize(szScript);

		std::vector<CParticleEmitterToken>::iterator TokenIter = tokenizer.m_TokenVector.begin();
		std::vector<CParticleEmitterToken>::iterator EndIter = tokenizer.m_TokenVector.end();
		if (tokenizer.m_TokenVector.size() < 2) throw("This script is too small to be valid.");
		// The first three tokens out of the gate should be
		// ParticleSystem, followed by a name and version number, then 
		// an open brace.
		if (TokenIter->m_Type != KeywordParticleSystem) { throw("First word must be ParticleSystem"); }
		CParticleEmitterTokenizer::NextToken(TokenIter, EndIter);
		if (TokenIter->m_Type != Quote) { throw("Must name particle system"); }
		m_szName = AppMisc::RemoveQuotes(TokenIter->m_strValue);
		CParticleEmitterTokenizer::NextToken(TokenIter, EndIter);

		if (TokenIter->m_Type != RealNumber) { throw("Must have version number"); }
		CParticleEmitterTokenizer::NextToken(TokenIter, EndIter);
		if (TokenIter->m_Type != OpenBrace) { throw("Missing opening brace for ParticleSystem block"); }

		ProcessParticleSystemBlock(TokenIter, EndIter);
 
	} catch(char *e) {
		m_szLastError = e; Init(); OutputDebugString(e); return(false); 
	} catch(const char* e) {
		m_szLastError = e; Init(); OutputDebugString(e); return(false);
	} catch (...) {
		m_szLastError = "Unhandled exception in CParticleEmitter::Compile.";
		OutputDebugString(m_szLastError.c_str());
		Init();
		return(false);
	}

	// do misc. processing and calcuations
	{
		m_vPos = m_vPosRange.GetRandomNumInRange();
	}

	RestoreDeviceObjects();
	return(true);
}

bool CParticleEmitter::ProcessParticleSystemBlock(
								std::vector<CParticleEmitterToken>::iterator &TokenIter,
								std::vector<CParticleEmitterToken>::iterator &EndIter)
{
	bool StartedProcessingSequences = false;
	// move past the opening brace...
	CParticleEmitterTokenizer::NextToken(TokenIter, EndIter);

	while (TokenIter->m_Type != CloseBrace) {

		CParticleEmitterToken savedtoken;
		savedtoken = *TokenIter;

		// the first token here can be a SysNumberProperty, SysAlphaBlendModeProperty, SysVectorProperty,
		// or an EventSequence.
		switch (TokenIter->m_Type) {

			case SysVectorProp:
				{
					if (StartedProcessingSequences)
						throw("Cannot specify any particle system properties after"
						"specifying sequences");
					CMinMax< D3DXVECTOR3 > v;
					CParticleEmitterTokenizer::NextToken(TokenIter, EndIter);
					if (TokenIter->m_Type != Equals) throw("Expeced equals sign");
					CParticleEmitterTokenizer::NextToken(TokenIter, EndIter);
					CParticleEmitterTokenizer::ProcessVector(v, TokenIter, EndIter);
					if (savedtoken.IsPosition()) {
						m_vPosRange = v;
					}
					else {
						throw("Unknown particle system property!");
					}

				}
				break;

			case KeywordEventSequence:
				break;

			default:
				{
					char buf[256];
					_snprintf(buf, sizeof(buf), "Unexpeced\"%s\" in Particle System Block!", 
						TokenIter->m_strValue.c_str());
					throw(&buf[0]);
				}
				break;
		}
	}
	CParticleEmitterTokenizer::NextToken(TokenIter, EndIter);
	return(true);
}

bool CParticleEmitter::ProcessEventSequenceBlock(CParticleEventSequence &seq,
							   std::vector<CParticleEmitterToken>::iterator &TokenIter,
							   std::vector<CParticleEmitterToken>::iterator &EndIter)
{

}

void CParticleEmitter::Update(float fElapsedTime, float fDeltaTime)
{
	if (IsRunning()) {
		for (uint32 i = 0; i < m_vecSeq.FilledSize(); i++)
		{
			m_vecSeq.GetElem(i)->Update(fElapsedTime, fDeltaTime, m_vPos);
		}
	}
}