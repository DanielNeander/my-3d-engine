#pragma once 

class  noRTTI 
{
public:
	noRTTI (const char* pcName, const noRTTI* pkBaseRTTI);

	inline const char* GetName() const {return m_pcName;}
	inline const noRTTI* GetBaseRTTI() const {return m_pkBaseRTTI;}

	bool CopyName(char* acNameBuffer, unsigned int uiMaxSize) const;

protected:
	const char* m_pcName;
	const noRTTI* m_pkBaseRTTI;
};

// insert in root class declaration
// insert in root class declaration
#define noDeclareRootRTTI(classname) \
	public: \
	static const noRTTI ms_RTTI; \
	virtual const noRTTI* GetRTTI() const {return &ms_RTTI;} \
	static bool IsExactKindOf(const noRTTI* pkRTTI, \
	const classname* pkObject) \
{ \
	if (!pkObject) \
{ \
	return false; \
} \
	return pkObject->IsExactKindOf(pkRTTI); \
} \
	bool IsExactKindOf(const noRTTI* pkRTTI) const \
{ \
	return (GetRTTI() == pkRTTI); \
} \
	static bool IsKindOf(const noRTTI* pkRTTI, \
	const classname* pkObject) \
{ \
	if (!pkObject) \
{ \
	return false; \
} \
	return pkObject->IsKindOf(pkRTTI); \
} \
	bool IsKindOf(const noRTTI* pkRTTI) const \
{ \
	const noRTTI* pkTmp = GetRTTI(); \
	while (pkTmp) \
{ \
	if (pkTmp == pkRTTI) \
{ \
	return true; \
} \
	pkTmp = pkTmp->GetBaseRTTI(); \
} \
	return false; \
} \
	static classname* VerifyStaticCastDebug(const noRTTI* pkRTTI, \
	const classname* pkObject) \
{ \
	if (!pkObject) \
{ \
	return NULL; \
} \
	classname* pkDynamicCast = DynamicCast(pkRTTI, pkObject); \
	assert("noVerifyStaticCast() caught an invalid type cast." \
	"Check callstack for invalid object typecast assumption." \
	&& pkDynamicCast); \
	return pkDynamicCast; \
} \
	static classname* DynamicCast(const noRTTI* pkRTTI, \
	const classname* pkObject) \
{ \
	if (!pkObject) \
{ \
	return NULL; \
} \
	return pkObject->DynamicCast(pkRTTI); \
} \
	classname* DynamicCast(const noRTTI* pkRTTI) const \
{ \
	return (IsKindOf(pkRTTI) ? (classname*) this : 0 ); \
}

// insert in class declaration
#define noDeclareRTTI \
	public: \
	static const noRTTI ms_RTTI; \
	virtual const noRTTI* GetRTTI() const {return &ms_RTTI;}

// insert in root class source file
#define noImplementRootRTTI(rootclassname) \
	const noRTTI rootclassname::ms_RTTI(#rootclassname, 0)

// insert in class source file
#define noImplementRTTI(classname, baseclassname) \
	const noRTTI classname::ms_RTTI(#classname, &baseclassname::ms_RTTI)

// macros for run-time type testing
#define noIsExactKindOf(classname, pkObject) \
	classname::IsExactKindOf(&classname::ms_RTTI, pkObject)

#define noIsKindOf(classname, pkObject) \
	classname::IsKindOf(&classname::ms_RTTI, pkObject)

// macro for compile time type casting
#define noStaticCast(classname, pkObject) \
	((classname*) pkObject)

// macro for compile time type casting, with debug run-time assert
#ifdef DEBUG
#define noVerifyStaticCast(classname, pkObject) \
	((classname*) classname::VerifyStaticCastDebug( \
	&classname::ms_RTTI, pkObject))
#else
#define noVerifyStaticCast(classname, pkObject) ((classname*) (pkObject))
#endif

// macro for run-time type casting, returns NULL if invalid cast
#define noDynamicCast(classname, pkObject) \
	((classname*) classname::DynamicCast(&classname::ms_RTTI, pkObject))


