///////////////////////////////////////////////////////////////////////
// File : SXEffect.h
// Description : D3DX Effects Parameter Manipulation Class
//				 (SXEffect) class declaration.
//				 (SXEffVariant) class declaration.
//				 (SXEffParam) class declaration.
// Author : Wessam Bahnassi, In|Framez
//
///////////////////////////////////////////////////////////////////////

#ifndef _SXEFFECT_H
#define _SXEFFECT_H

///////////////// #includes /////////////////
#include <hash_map>	// Using VS.NET 2003 here, change these to suit your compiler
#include <string>


//////////////////////////////// Forward References ////////////////////////////////
class noEffAnimation;
class noEffVariant;
class noEffParam;
class noFXEffect;


//////////////////////////////// Type Defines ////////////////////////////////
typedef stdext::hash_map<int,noEffVariant*> noEffVariantElemHash;
typedef stdext::hash_map<std::string,noEffVariant*> SXEffVariantsHash;
typedef stdext::hash_map<std::string,noEffParam*> noEffParamsHash;


//////////////////////////////// SXEffAnimation Class Declaration ////////////////////////////////
class noEffAnimation
{
protected:
	friend class noFXEffect;
	friend class noEffVariant;

	noEffAnimation(const noEffVariant& varValue,bool bColor);
	~noEffAnimation();
	static void LerpFloat1(noEffAnimation& animThis,noEffVariant& varOwner);
	static void LerpFloat2(noEffAnimation& animThis,noEffVariant& varOwner);
	static void LerpFloat3(noEffAnimation& animThis,noEffVariant& varOwner);
	static void LerpFloat4(noEffAnimation& animThis,noEffVariant& varOwner);
	static void LerpInteger(noEffAnimation& animThis,noEffVariant& varOwner);
	static void LerpColor(noEffAnimation& animThis,noEffVariant& varOwner);
	void Update(float fTime,noEffVariant& varOwner);

	typedef void (*LERPFUNC)(noEffAnimation&,noEffVariant&);
	LERPFUNC m_pfnLerper;
	float m_fWeight;
	D3DXVECTOR4 m_vec4Key1;
	D3DXVECTOR4 m_vec4Key2;
};


//////////////////////////////// SXEffVariant Class Declaration ////////////////////////////////
class noEffVariant
{
public:
	// Type Checking
	bool IsValid(void) const;
	bool IsA(D3DXPARAMETER_TYPE eParamType) const;
	bool IsA(D3DXPARAMETER_CLASS eParamClass) const;
	D3DXPARAMETER_TYPE GetType(void) const;
	D3DXPARAMETER_CLASS GetClass(void) const;

	inline void SetHandle(D3DXHANDLE handle) { m_hParam = handle; }

	// Matrix/Array Accessors
	float mat(int iCol,int iRow);
	noEffVariant& operator[](int iIndex);	// Arrays only
	UINT GetElemsCount(void) const;	// Arrays only
	UINT GetVectorSize(void) const;	// Vectors only

	// Value Get
	operator bool(void);
	operator int(void);
	operator float(void);
	operator D3DXVECTOR2(void);
	operator D3DXVECTOR3(void);
	operator D3DXVECTOR4(void);
	operator D3DXMATRIX(void);
	operator PCSTR(void);
	
	operator PDIRECT3DVERTEXSHADER9 (void);
	operator PDIRECT3DPIXELSHADER9 (void);
	operator PDIRECT3DBASETEXTURE9 (void);

	// Value Set
	noEffVariant& operator= (const noEffVariant& varVal);
	noEffVariant& operator= (bool bVal);
	noEffVariant& operator= (int iVal);
	noEffVariant& operator= (float fVal);
	noEffVariant& operator= (const D3DXVECTOR2& vec2Val);
	noEffVariant& operator= (const D3DXVECTOR3& vec3Val);
	noEffVariant& operator= (const D3DXVECTOR4& vec4Val);
	noEffVariant& operator= (const D3DXMATRIX& matVal);
	noEffVariant& operator= (PCSTR pszVal);
	noEffVariant& operator= (PDIRECT3DVERTEXSHADER9 pVal);
	noEffVariant& operator= (PDIRECT3DPIXELSHADER9 pVal);
	noEffVariant& operator= (PDIRECT3DBASETEXTURE9 pVal);

	// Animation
	void CancelAnimation(void);
	void Animate(float fKey1,float fKey2);
	void Animate(const D3DXVECTOR2& vec2Key1,const D3DXVECTOR2& vec2Key2);
	void Animate(const D3DXVECTOR3& vec3Key1,const D3DXVECTOR3& vec3Key2);
	void Animate(const D3DXVECTOR4& vec4Key1,const D3DXVECTOR4& vec4Key2);
	void Animate(int iKey1,int iKey2);
	void AnimateAsColor(D3DCOLOR clrKey1,D3DCOLOR clrKey2);

protected:
	friend class noFXEffect;
	friend class noEffParam;

	// Construction/Destruction
	noEffVariant(noFXEffect *pOwnerEffect,D3DXHANDLE hParam);
	~noEffVariant();

	// Internal Properties
	noFXEffect *m_pOwnerEffect;
	D3DXHANDLE m_hParam;
	noEffVariantElemHash m_hashElems;	// Array elements
	noEffAnimation *m_pAnim;
};


//////////////////////////////// SXEffParam Class Declaration ////////////////////////////////
class noEffParam : public noEffVariant
{
public:
	// Operations
	noEffParam& operator () (PCSTR pszParamName);
	noEffVariant& GetPassAnnotation(PCSTR pszPassName,PCSTR pszAnnName);
	noEffVariant& Annotation(PCSTR pszAnnName);

	// Value Set
	noEffVariant& operator= (const noEffVariant& varVal);
	noEffVariant& operator= (bool bVal);
	noEffVariant& operator= (int iVal);
	noEffVariant& operator= (float fVal);
	noEffVariant& operator= (const D3DXVECTOR2& vec2Val);
	noEffVariant& operator= (const D3DXVECTOR3& vec3Val);
	noEffVariant& operator= (const D3DXVECTOR4& vec4Val);
	noEffVariant& operator= (const D3DXMATRIX& matVal);
	noEffVariant& operator= (PCSTR pszVal);
	noEffVariant& operator= (PDIRECT3DVERTEXSHADER9 pVal);
	noEffVariant& operator= (PDIRECT3DPIXELSHADER9 pVal);
	noEffVariant& operator= (PDIRECT3DBASETEXTURE9 pVal);

protected:
	friend class noFXEffect;

	// Construction/Destruction
	noEffParam(noFXEffect *pOwnerEffect,D3DXHANDLE hParam);
	~noEffParam();

	// Internal Properties
	SXEffVariantsHash m_hashAnnotations;
};


//////////////////////////////// SXEffect Class Declaration ////////////////////////////////
class noFXEffect : public noD3DResource
{
public:
	// Construction/Destruction
	noFXEffect();
	~noFXEffect();

	int Begin(bool saveState);

	void BeginPass(int pass);

	void CommitChanges();

	void End();

	void EndPass();

	// Initialization
	HRESULT CreateFromFile(PDIRECT3DDEVICE9 pDevice,PCTSTR pszFileName);
	HRESULT AttachToExisting(LPD3DXEFFECT pEffect);
	void Clear(void);

	// Operations
	noEffParam& operator() (PCSTR pszParamName,D3DXHANDLE hParent = NULL);
	noEffParam& GetTechnique(PCSTR pszTechniqueName);
	void SetAnimWeight(float fTime);

	// Notifications
	HRESULT OnDeviceLoss();
	HRESULT OnDeviceRestore();

	// Access
	LPD3DXEFFECT GetD3DXEffect(void) const;

private:
	void ValidateEffect();
	void UpdateAttbHandles();
	bool SetTechnique(const char* szTechnique);

protected:
	friend noEffParam;
	friend noEffVariant;

	// Internal Properties
	LPD3DXEFFECT m_pEffect;
	noEffParamsHash m_hashParams;
	noEffParam m_paramInvalid;	// Error Code

	bool m_bValidated;
	bool m_bNotValidate;
	bool m_bBeginPass;
	bool m_bNeedSoftwareVertexProcessing;	// Curr technique need software processing mode ?

};

#include "noFXEffect.inl"

#endif	// _SXEFFECT_H

////////////////// End of File : SXEffect.h //////////////////