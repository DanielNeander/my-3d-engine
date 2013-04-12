#ifndef MATH_DISTRIBUTION_H
#define MATH_DISTRIBUTION_H

#include "EngineCore/Unreal3/UnName.h"
#include "UnMath.h"

class UObject;

enum EDistributionVectorMirrorFlags
{
	EDVMF_Same              =0,
	EDVMF_Different         =1,
	EDVMF_Mirror            =2,
	EDVMF_MAX               =3,
};
enum EDistributionVectorLockFlags
{
	EDVLF_None              =0,
	EDVLF_XY                =1,
	EDVLF_XZ                =2,
	EDVLF_YZ                =3,
	EDVLF_XYZ               =4,
	EDVLF_MAX               =5,
};

enum DistributionParamMode
{
	DPM_Normal              =0,
	DPM_Abs                 =1,
	DPM_Direct              =2,
	DPM_MAX                 =3,
};

class noDistributionFloat : public FCurveEdInterface
{
public:
    //## BEGIN PROPS DistributionFloat
    BITFIELD bCanBeBaked:1;
    BITFIELD bIsDirty:1;
    //## END PROPS DistributionFloat

    //DECLARE_ABSTRACT_CLASS(UDistributionFloat,UComponent,0,Core)

#if !CONSOLE
	/**
	 * Return the operation used at runtime to calculate the final value
	 */
	virtual ERawDistributionOperation GetOperation() { return RDO_None; }

	/**
	 * Return the lock flags used at runtime to calculate the final value
	 */
	virtual ERawDistributionLockFlags GetLockFlags(INT InIndex) { return RDL_None; }

	/**
	 * Fill out an array of floats and return the number of elements in the entry
	 *
	 * @param Time The time to evaluate the distribution
	 * @param Values An array of values to be filled out, guaranteed to be big enough for 4 values
	 * @return The number of elements (values) set in the array
	 */
	virtual DWORD InitializeRawEntry(FLOAT Time, FLOAT* Values);
#endif
	virtual FLOAT	GetValue( FLOAT F = 0.f, UObject* Data = NULL );

	virtual void	GetInRange(FLOAT& MinIn, FLOAT& MaxIn);
	virtual void	GetOutRange(FLOAT& MinOut, FLOAT& MaxOut);
	
	/**
	 * Return whether or not this distribution can be baked into a FRawDistribution lookup table
	 */
	virtual UBOOL CanBeBaked() const 
	{
		return bCanBeBaked; 
	}

	/** UObject interface */
	virtual void Serialize(FArchive& Ar);

	//virtual void PostEditChange(UProperty* PropertyThatChanged);
	
	/**
	 * If the distribution can be baked, then we don't need it on the client or server
	 */
	//virtual UBOOL NeedsLoadForClient() const;
	//virtual UBOOL NeedsLoadForServer() const;
};


struct noRawDistributionFloat : public noRawDistribution
{
    noDistributionFloat* Distribution;

#if !CONSOLE
	/**
	 * Initialize a raw distribution from the original Unreal distribution
	 */
	void Initialize();
#endif
	 	
	/**
	 * Gets a pointer to the raw distribution if you can just call FRawDistribution::GetValue1 on it, otherwise NULL 
	 */
	const noRawDistribution* GetFastRawDistribution();

	/**
	 * Get the value at the specified F
	 */
	float GetValue(float F=0.0f, UObject* Data=NULL);

	/**
	 * Get the min and max values
	 */
	void GetOutRange(float& MinOut, float& MaxOut);

	/**
	 * Is this distribution a uniform type? (ie, does it have two values per entry?)
	 */
	inline BOOL IsUniform() { return LookupTableNumElements == 2; }

};



struct noRawDistributionVec : public noRawDistribution {
	class noDistributionVec* Distribution;

	void Initialize();

	/**
	 * Get the value at the specified F
	 */
	noVec3 GetValue(float F=0.0f, UObject* Data=NULL, INT LastExtreme=0);

	/**
	 * Get the min and max values
	 */
	void GetOutRange(float& MinOut, float& MaxOut);

	/**
	 * Is this distribution a uniform type? (ie, does it have two values per entry?)
	 */
	inline BOOL IsUniform() { return LookupTableNumElements == 2; }
	const noRawDistribution *GetFastRawDistribution();
};


class noDistributionVec : public FCurveEdInterface
{
public:
	UINT32 bCanbeBaked:1;
	UINT32 bIsDirty:1;

	virtual ERawDistributionOperation GetOperation() { return RDO_None; }

		/**
	 * Return the lock flags used at runtime to calculate the final value
	 */
	virtual ERawDistributionLockFlags GetLockFlags(INT InIndex) { return RDL_None; }

	/**
	 * Return true if the distribution is a uniform curve
	 */
	virtual BOOL IsUniformCurve() { return FALSE; }

		/**
	 * Fill out an array of vectors and return the number of elements in the entry
	 *
	 * @param Time The time to evaluate the distribution
	 * @param Values An array of values to be filled out, guaranteed to be big enough for 2 vectors
	 * @return The number of elements (values) set in the array
	 */
	virtual DWORD InitializeRawEntry(float Time, noVec3* Values);

	virtual FVector	GetValue( FLOAT F = 0.f, UObject* Data = NULL, INT LastExtreme = 0 );

	virtual void	GetInRange(FLOAT& MinIn, FLOAT& MaxIn);
	virtual void	GetOutRange(FLOAT& MinOut, FLOAT& MaxOut);
	virtual	void	GetRange(FVector& OutMin, FVector& OutMax);

	/**
	 * Return whether or not this distribution can be baked into a FRawDistribution lookup table
	 */
	virtual UBOOL CanBeBaked() const 
	{
		return bCanbeBaked; 
	}

};


class UDistributionFloatConstant : public noDistributionFloat
{
public:
	//## BEGIN PROPS DistributionFloatConstant
	FLOAT Constant;
	//## END PROPS DistributionFloatConstant

	//DECLARE_CLASS(UDistributionFloatConstant,noDistributionFloat,0,Engine)
	virtual FLOAT GetValue( FLOAT F = 0.f, UObject* Data = NULL );

	// FCurveEdInterface interface
	virtual INT		GetNumKeys();
	virtual INT		GetNumSubCurves();
	virtual FLOAT	GetKeyIn(INT KeyIndex);
	virtual FLOAT	GetKeyOut(INT SubIndex, INT KeyIndex);
	virtual void	GetInRange(FLOAT& MinIn, FLOAT& MaxIn);
	virtual void	GetOutRange(FLOAT& MinOut, FLOAT& MaxOut);
	virtual BYTE	GetKeyInterpMode(INT KeyIndex);
	virtual void	GetTangents(INT SubIndex, INT KeyIndex, FLOAT& ArriveTangent, FLOAT& LeaveTangent);
	virtual FLOAT	EvalSub(INT SubIndex, FLOAT InVal);

	virtual INT		CreateNewKey(FLOAT KeyIn);
	virtual void	DeleteKey(INT KeyIndex);

	virtual INT		SetKeyIn(INT KeyIndex, FLOAT NewInVal);
	virtual void	SetKeyOut(INT SubIndex, INT KeyIndex, FLOAT NewOutVal);
	virtual void	SetKeyInterpMode(INT KeyIndex, EInterpCurveMode NewMode);
	virtual void	SetTangents(INT SubIndex, INT KeyIndex, FLOAT ArriveTangent, FLOAT LeaveTangent);
};

class UDistributionFloatParameterBase : public UDistributionFloatConstant
{
public:
    //## BEGIN PROPS DistributionFloatParameterBase
    FName ParameterName;
    FLOAT MinInput;
    FLOAT MaxInput;
    FLOAT MinOutput;
    FLOAT MaxOutput;
    BYTE ParamMode;
    //## END PROPS DistributionFloatParameterBase

    //DECLARE_ABSTRACT_CLASS(UDistributionFloatParameterBase,UDistributionFloatConstant,0,Engine)
	virtual FLOAT GetValue( FLOAT F = 0.f, UObject* Data = NULL );
	
	virtual UBOOL GetParamValue(UObject* Data, FName ParamName, FLOAT& OutFloat) { return false; }

	/**
	 * Return whether or not this distribution can be baked into a FRawDistribution lookup table
	 */
	virtual UBOOL CanBeBaked() const { return FALSE; }
};

class UDistributionVectorConstant : public noDistributionVec
{
public:
	//## BEGIN PROPS DistributionVectorConstant
	FVector Constant;
	BITFIELD bLockAxes:1;	
	BYTE LockedAxes ;
	//## END PROPS DistributionVectorConstant

	DECLARE_CLASS(UDistributionVectorConstant,UDistributionVector,0,Engine)
	virtual FVector GetValue(FLOAT F = 0.f, UObject* Data = NULL, INT Extreme = 0);

	// UObject interface
	//virtual void Serialize(FArchive& Ar);

	// FCurveEdInterface interface
	virtual INT		GetNumKeys();
	virtual INT		GetNumSubCurves();
	virtual FLOAT	GetKeyIn(INT KeyIndex);
	virtual FLOAT	GetKeyOut(INT SubIndex, INT KeyIndex);
	virtual FColor	GetKeyColor(INT SubIndex, INT KeyIndex, const FColor& CurveColor);
	virtual void	GetInRange(FLOAT& MinIn, FLOAT& MaxIn);
	virtual void	GetOutRange(FLOAT& MinOut, FLOAT& MaxOut);
	virtual BYTE	GetKeyInterpMode(INT KeyIndex);
	virtual void	GetTangents(INT SubIndex, INT KeyIndex, FLOAT& ArriveTangent, FLOAT& LeaveTangent);
	virtual FLOAT	EvalSub(INT SubIndex, FLOAT InVal);

	virtual INT		CreateNewKey(FLOAT KeyIn);
	virtual void	DeleteKey(INT KeyIndex);

	virtual INT		SetKeyIn(INT KeyIndex, FLOAT NewInVal);
	virtual void	SetKeyOut(INT SubIndex, INT KeyIndex, FLOAT NewOutVal);
	virtual void	SetKeyInterpMode(INT KeyIndex, EInterpCurveMode NewMode);
	virtual void	SetTangents(INT SubIndex, INT KeyIndex, FLOAT ArriveTangent, FLOAT LeaveTangent);

	// DistributionVector interface
	virtual	void	GetRange(FVector& OutMin, FVector& OutMax);
};


class UDistributionVectorParameterBase : public UDistributionVectorConstant
{
public:
    //## BEGIN PROPS DistributionVectorParameterBase
    FName ParameterName;
    FVector MinInput;
    FVector MaxInput;
    FVector MinOutput;
    FVector MaxOutput;
    BYTE ParamModes[3];
    //## END PROPS DistributionVectorParameterBase

    //DECLARE_ABSTRACT_CLASS(UDistributionVectorParameterBase,UDistributionVectorConstant,0,Engine)
	virtual FVector GetValue(FLOAT F = 0.f, UObject* Data = NULL, INT Extreme = 0);
	
	virtual UBOOL GetParamValue(UObject* Data, FName ParamName, FVector& OutVector) { return false; }

	/**
	 * Return whether or not this distribution can be baked into a FRawDistribution lookup table
	 */
	virtual UBOOL CanBeBaked() const { return FALSE; }
};

#endif