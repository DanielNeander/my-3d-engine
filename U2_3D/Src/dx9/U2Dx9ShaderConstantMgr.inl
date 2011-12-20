inline bool U2Dx9ShaderConstantMgr::SetVertexShaderConstantB(unsigned int uiStartRegister,
									 const BOOL* pbConstantData, unsigned int uiBoolCount, 
									 bool bSave)
{
	if(bSave)
	{
		if((uiStartRegister + uiBoolCount) > m_uNumBVSConstants)
			return false;

		uint32  uByteSize = uiBoolCount * sizeof(*pbConstantData);
		memcpy_s(m_pSavedBVSConstants + uiStartRegister, uByteSize, 
			m_pBVSConstants + uiStartRegister, uByteSize);
	}
	return SetVertexShaderConstantB(uiStartRegister, pbConstantData, uiBoolCount);

}
inline bool U2Dx9ShaderConstantMgr::SetVertexShaderConstantB(unsigned int uiStartRegister,
									 const BOOL* pbConstantData, unsigned int uiBoolCount)
{
	if((uiStartRegister + uiBoolCount) > m_uNumBVSConstants)
		return false;
	uint32  uByteSize = uiBoolCount * sizeof(*pbConstantData);
	memcpy_s(m_pBVSConstants + uiStartRegister, uByteSize, 
		pbConstantData, uByteSize);

	if(m_uiFirstCleanBVSReg > uiStartRegister)
		m_uiFirstDirtyBVSReg = uiStartRegister;
	if(m_uiFirstCleanBVSReg < uiStartRegister + uiBoolCount)
		m_uiFirstCleanBVSReg = uiStartRegister + uiBoolCount;

	return true;
}
inline bool U2Dx9ShaderConstantMgr::GetVertexShaderConstantB(unsigned int uiStartRegister,
									 BOOL* pbConstantData, unsigned int uiBoolCount)
{
	if((uiStartRegister + uiBoolCount) > m_uNumBVSConstants )
		return false;

	uint32 uByteSize = uiBoolCount * sizeof(*pbConstantData);
	memcpy_s(pbConstantData, uByteSize, m_pBVSConstants + uiStartRegister, uByteSize);
	return true;
}
inline bool U2Dx9ShaderConstantMgr::RestoreVertexShaderConstantB(unsigned int uiStartRegister, 
										 unsigned int uiBoolCount)
{
	if((uiStartRegister + uiBoolCount) > m_uNumBVSConstants)
		return false;

	uint32 uByteSize = uiBoolCount * sizeof(*m_pBVSConstants);
	memcpy_s(m_pBVSConstants + uiStartRegister, uByteSize, m_pSavedBVSConstants + uiStartRegister,
		uByteSize);

	return true;
}
inline bool U2Dx9ShaderConstantMgr::SetVertexShaderConstantF(unsigned int uiStartRegister,
									 const float* pfConstantData, unsigned int uiFloat4Count, 
									 bool bSave)
{
	if(bSave)
	{
		if((uiStartRegister + uiFloat4Count) > m_uNumFVSConstants)
			return false;

		uint32 uBase = uiStartRegister * 4;
		uint32  uByteSize = uiFloat4Count * 4 * sizeof(*pfConstantData);
		memcpy_s(m_pSavedFVSConstants + uBase, uByteSize, 
			m_pFVSConstants + uBase, uByteSize);
	}
	return SetVertexShaderConstantF(uiStartRegister, pfConstantData, uiFloat4Count);


}
inline bool U2Dx9ShaderConstantMgr::SetVertexShaderConstantF(unsigned int uiStartRegister,
									 const float* pfConstantData, unsigned int uiFloat4Count)
{
	if((uiStartRegister + uiFloat4Count) > m_uNumFVSConstants)
		return false;
	uint32 uBase = uiStartRegister * 4;
	uint32 uByteSize = uiFloat4Count * 4 * sizeof(*pfConstantData);

	memcpy_s(m_pFVSConstants + uBase, uByteSize, pfConstantData, uByteSize);
	if(m_uiFirstDirtyFVSReg > uiStartRegister)
		m_uiFirstDirtyFVSReg = uiStartRegister;
	if(m_uiFirstCleanFVSReg < uiStartRegister + uiFloat4Count)
		m_uiFirstCleanFVSReg = uiStartRegister + uiFloat4Count;

	return true;
}
inline bool U2Dx9ShaderConstantMgr::GetVertexShaderConstantF(unsigned int uiStartRegister,
									 float* pfConstantData, unsigned int uiFloat4Count)
{
	if((uiStartRegister + uiFloat4Count) > m_uNumFVSConstants)
		return false;
	uint32 uByteSize = uiFloat4Count * 4 * sizeof(*m_pFVSConstants);
	memcpy_s(pfConstantData, uByteSize, m_pFVSConstants + uiStartRegister * 4, uByteSize);
	
	return true;
}


inline bool U2Dx9ShaderConstantMgr::RestoreVertexShaderConstantF(unsigned int uiStartRegister, 
										 unsigned int uiFloat4Count)
{
	if((uiStartRegister + uiFloat4Count) > m_uNumFVSConstants)
		return false;

	uint32 uByteSize = uiFloat4Count * 4 * sizeof(*m_pFVSConstants);
	memcpy_s(m_pFVSConstants + uiStartRegister * 4, uByteSize, 
		m_pSavedFVSConstants + uiStartRegister * 4, uByteSize);

	return true;
}


inline bool U2Dx9ShaderConstantMgr::SetVertexShaderConstantI(unsigned int uiStartRegister,
									 const int* piConstantData, unsigned int uiInt4Count, 
									 bool bSave)
{
	if(bSave)
	{
		if((uiStartRegister + uiInt4Count) > m_uNumIVSConstants)
			return false;

		uint32 uBase = uiStartRegister * 4;
		uint32  uByteSize = uiInt4Count * 4 * sizeof(*piConstantData);
		memcpy_s(m_pSavedIVSConstants + uBase, uByteSize, 
			m_pIVSConstants + uBase, uByteSize);
	}
	return SetVertexShaderConstantI(uiStartRegister, piConstantData, uiInt4Count);

}
inline bool U2Dx9ShaderConstantMgr::SetVertexShaderConstantI(unsigned int uiStartRegister,
									 const int* piConstantData, unsigned int uiInt4Count)
{
	if((uiStartRegister + uiInt4Count) > m_uNumIVSConstants)
		return false;
	uint32 uBase = uiStartRegister * 4;
	uint32 uByteSize = uiInt4Count * 4 * sizeof(*piConstantData);

	memcpy_s(m_pIVSConstants + uBase, uByteSize, piConstantData, uByteSize);
	if(m_uiFirstDirtyIVSReg > uiStartRegister)
		m_uiFirstDirtyIVSReg = uiStartRegister;
	if(m_uiFirstCleanIVSReg < uiStartRegister + uiInt4Count)
		m_uiFirstCleanIVSReg = uiStartRegister + uiInt4Count;

	return true;
}

inline bool U2Dx9ShaderConstantMgr::GetVertexShaderConstantI(unsigned int uiStartRegister,
									 int* piConstantData, unsigned int uiInt4Count)
{
	if((uiStartRegister + uiInt4Count) > m_uNumIVSConstants)
		return false;
	uint32 uByteSize = uiInt4Count * 4 * sizeof(*m_pIVSConstants);
	memcpy_s(piConstantData, uByteSize, m_pIVSConstants + uiStartRegister * 4, uByteSize);

	return true;
}

inline bool U2Dx9ShaderConstantMgr::RestoreVertexShaderConstantI(unsigned int uiStartRegister, 
										 unsigned int uiInt4Count)
{
	if((uiStartRegister + uiInt4Count) > m_uNumIVSConstants)
		return false;

	uint32 uByteSize = uiInt4Count * 4 * sizeof(*m_pIVSConstants);
	memcpy_s(m_pIVSConstants + uiStartRegister * 4, uByteSize, 
		m_pSavedIVSConstants + uiStartRegister * 4, uByteSize);

	return true;
}

inline bool U2Dx9ShaderConstantMgr::SetPixelShaderConstantB(unsigned int uiStartRegister,
									const BOOL* pbConstantData, unsigned int uiBoolCount, 
									bool bSave)
{
	if(bSave)
	{
		if((uiStartRegister + uiBoolCount) > m_uNumBPSConstants)
			return false;

		uint32  uByteSize = uiBoolCount * sizeof(*pbConstantData);
		memcpy_s(m_pSavedBPSConstants + uiStartRegister, uByteSize, 
			m_pBPSConstants + uiStartRegister, uByteSize);
	}
	return SetPixelShaderConstantB(uiStartRegister, pbConstantData, uiBoolCount);


}

inline bool U2Dx9ShaderConstantMgr::SetPixelShaderConstantB(unsigned int uiStartRegister,
									const BOOL* pbConstantData, unsigned int uiBoolCount)
{
	if((uiStartRegister + uiBoolCount) > m_uNumBPSConstants)
		return false;
	uint32  uByteSize = uiBoolCount * sizeof(*pbConstantData);
	memcpy_s(m_pBPSConstants + uiStartRegister, uByteSize, 
		pbConstantData, uByteSize);

	if(m_uiFirstCleanBPSReg > uiStartRegister)
		m_uiFirstDirtyBPSReg = uiStartRegister;
	if(m_uiFirstCleanBPSReg < uiStartRegister + uiBoolCount)
		m_uiFirstCleanBPSReg = uiStartRegister + uiBoolCount;
	
	return true;
}

inline bool U2Dx9ShaderConstantMgr::GetPixelShaderConstantB(unsigned int uiStartRegister,
									BOOL* pbConstantData, unsigned int uiBoolCount)
{
	if((uiStartRegister + uiBoolCount) > m_uNumBPSConstants )
		return false;

	uint32 uByteSize = uiBoolCount * sizeof(*pbConstantData);
	memcpy_s(pbConstantData, uByteSize, m_pBPSConstants + uiStartRegister, uByteSize);
	return true;

}

inline bool U2Dx9ShaderConstantMgr::RestorePixelShaderConstantB(unsigned int uiStartRegister, 
										unsigned int uiBoolCount)
{
	if((uiStartRegister + uiBoolCount) > m_uNumBPSConstants)
		return false;

	uint32 uByteSize = uiBoolCount * sizeof(*m_pBPSConstants);
	memcpy_s(m_pBPSConstants + uiStartRegister, uByteSize, m_pSavedBPSConstants + uiStartRegister,
		uByteSize);

	return true;
}

inline bool U2Dx9ShaderConstantMgr::SetPixelShaderConstantF(unsigned int uiStartRegister,
									const float* pfConstantData, unsigned int uiFloat4Count, 
									bool bSave)
{
	if(bSave)
	{
		if((uiStartRegister + uiFloat4Count) > m_uNumFPSConstants)
			return false;

		uint32 uBase = uiStartRegister * 4;
		uint32 uByteSize = uiFloat4Count * 4 * sizeof(*pfConstantData);

		memcpy_s(m_pSavedFPSConstants + uBase, uByteSize, 
			m_pFPSConstants + uBase, uByteSize);
	}
	return SetPixelShaderConstantF(uiStartRegister, pfConstantData, uiFloat4Count);
}

inline bool U2Dx9ShaderConstantMgr::SetPixelShaderConstantF(unsigned int uiStartRegister,
									const float* pfConstantData, unsigned int uiFloat4Count)
{
	if((uiStartRegister + uiFloat4Count) > m_uNumFPSConstants)
		return false;
	uint32 uBase = uiStartRegister * 4;
	uint32 uByteSize = uiFloat4Count * 4 * sizeof(*pfConstantData);

	memcpy_s(m_pFPSConstants + uBase, uByteSize, pfConstantData, uByteSize);
	if(m_uiFirstDirtyFPSReg > uiStartRegister)
		m_uiFirstDirtyFPSReg = uiStartRegister;
	if(m_uiFirstCleanFPSReg < uiStartRegister + uiFloat4Count)
		m_uiFirstCleanFPSReg = uiStartRegister + uiFloat4Count;

	return true;
}

inline bool U2Dx9ShaderConstantMgr::GetPixelShaderConstantF(unsigned int uiStartRegister,
									float* pfConstantData, unsigned int uiFloat4Count)
{
	if((uiStartRegister + uiFloat4Count) > m_uNumFPSConstants)
		return false;
	uint32 uByteSize = uiFloat4Count * 4 * sizeof(*m_pFPSConstants);
	memcpy_s(pfConstantData, uByteSize, m_pFPSConstants + uiStartRegister * 4, uByteSize);

	return true;

}

inline bool U2Dx9ShaderConstantMgr::RestorePixelShaderConstantF(unsigned int uiStartRegister, 
										unsigned int uiFloat4Count)
{
	if((uiStartRegister + uiFloat4Count) > m_uNumFPSConstants)
		return false;

	uint32 uByteSize = uiFloat4Count * 4 * sizeof(*m_pFPSConstants);
	memcpy_s(m_pFPSConstants + uiStartRegister * 4, uByteSize, 
		m_pSavedFPSConstants + uiStartRegister * 4, uByteSize);

	return true;
}

inline bool U2Dx9ShaderConstantMgr::SetPixelShaderConstantI(unsigned int uiStartRegister,
									const int* piConstantData, unsigned int uiInt4Count, 
									bool bSave)
{
	if(bSave)
	{
		if((uiStartRegister + uiInt4Count) > m_uNumIPSConstants)
			return false;

		uint32 uBase = uiStartRegister * 4;
		uint32  uByteSize = uiInt4Count * 4 * sizeof(*piConstantData);
		memcpy_s(m_pSavedIPSConstants + uBase, uByteSize, 
			m_pIPSConstants + uBase, uByteSize);
	}
	return SetPixelShaderConstantI(uiStartRegister, piConstantData, uiInt4Count);

}
inline bool U2Dx9ShaderConstantMgr::SetPixelShaderConstantI(unsigned int uiStartRegister,
									const int* piConstantData, unsigned int uiInt4Count)
{
	if((uiStartRegister + uiInt4Count) > m_uNumIPSConstants)
		return false;
	uint32 uBase = uiStartRegister * 4;
	uint32 uByteSize = uiInt4Count * 4 * sizeof(*piConstantData);

	memcpy_s(m_pIPSConstants + uBase, uByteSize, piConstantData, uByteSize);
	if(m_uiFirstDirtyIPSReg > uiStartRegister)
		m_uiFirstDirtyIPSReg = uiStartRegister;
	if(m_uiFirstCleanIPSReg < uiStartRegister + uiInt4Count)
		m_uiFirstCleanIPSReg = uiStartRegister + uiInt4Count;

	return true;

}

inline bool U2Dx9ShaderConstantMgr::GetPixelShaderConstantI(unsigned int uiStartRegister,
									int* piConstantData, unsigned int uiInt4Count)
{
	  
	if((uiStartRegister + uiInt4Count) > m_uNumIPSConstants)
		return false;
	uint32 uByteSize = uiInt4Count * 4 * sizeof(*m_pIPSConstants);
	memcpy_s(piConstantData, uByteSize, m_pIPSConstants + uiStartRegister * 4, uByteSize);

	return true;

}

inline bool U2Dx9ShaderConstantMgr::RestorePixelShaderConstantI(unsigned int uiStartRegister, 
										unsigned int uiInt4Count)
{
	if((uiStartRegister + uiInt4Count) > m_uNumIPSConstants)
		return false;

	uint32 uByteSize = uiInt4Count * 4 * sizeof(*m_pIPSConstants);
	memcpy_s(m_pIPSConstants + uiStartRegister * 4, uByteSize, 
		m_pSavedIPSConstants + uiStartRegister * 4, uByteSize);

	return true;
}