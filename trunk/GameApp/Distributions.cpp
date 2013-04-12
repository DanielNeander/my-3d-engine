#include "stdafx.h"
#include "Distributions.h"

#define RESTRICT __restrict


INT GSRandSeed		= 0;
UBOOL					GIsEditor						= FALSE;					/* Whether engine was launched for editing */
UBOOL					GIsGame							= FALSE;

DWORD GDistributionType = 1;
#define GLookupTableMaxFrames 100
#define GLookupTableFrameRate 20.0f

/*-----------------------------------------------------------------------------
	FDistribution
-----------------------------------------------------------------------------*/

void noRawDistribution::GetValue1(FLOAT Time, FLOAT* Value, INT Extreme)
{
	switch (Op)
	{
	case RDO_None:
		GetValue1None(Time,Value);
		break;
	case RDO_Extreme:
		GetValue1Extreme(Time,Value,Extreme);
		break;
	case RDO_Random:
		GetValue1Random(Time,Value);
		break;
	default: // compiler complains		
		assert(0);
		*Value = 0.0f;
		break;
	}
}

void noRawDistribution::GetValue3(FLOAT Time, FLOAT* Value, INT Extreme)
{
	switch (Op)
	{
	case RDO_None:
		GetValue3None(Time,Value);
		break;
	case RDO_Extreme:
		GetValue3Extreme(Time,Value,Extreme);
		break;
	case RDO_Random:
		GetValue3Random(Time,Value);
		break;
	}
}

void noRawDistribution::GetValue1Extreme(FLOAT Time, FLOAT* InValue, INT Extreme)
{
	FLOAT* RESTRICT Value = InValue;
	const LOOKUPVALUE* Entry1;
	const LOOKUPVALUE* Entry2;
	FLOAT LerpAlpha = 0.0f;
	GetEntry( Time, Entry1, Entry2, LerpAlpha );
	const LOOKUPVALUE* RESTRICT NewEntry1 = Entry1;
	const LOOKUPVALUE* RESTRICT NewEntry2 = Entry2;
	INT InitialElement = (Extreme > 0 || (Extreme == 0 && appSRand() > 0.5f));
	Value[0] = Lerp(NewEntry1[InitialElement + 0], NewEntry2[InitialElement + 0], LerpAlpha);
}

void noRawDistribution::GetValue3Extreme(FLOAT Time, FLOAT* InValue, INT Extreme)
{
	FLOAT* RESTRICT Value = InValue;
	const LOOKUPVALUE* Entry1;
	const LOOKUPVALUE* Entry2;
	FLOAT LerpAlpha = 0.0f;
	GetEntry( Time, Entry1, Entry2, LerpAlpha );
	const LOOKUPVALUE* RESTRICT NewEntry1 = Entry1;
	const LOOKUPVALUE* RESTRICT NewEntry2 = Entry2;
	INT InitialElement = (Extreme > 0 || (Extreme == 0 && appSRand() > 0.5f));
	InitialElement *= 3;
	FLOAT T0 = Lerp(NewEntry1[InitialElement + 0], NewEntry2[InitialElement + 0], LerpAlpha);
	FLOAT T1 = Lerp(NewEntry1[InitialElement + 1], NewEntry2[InitialElement + 1], LerpAlpha);
	FLOAT T2 = Lerp(NewEntry1[InitialElement + 2], NewEntry2[InitialElement + 2], LerpAlpha);
	Value[0] = T0;
	Value[1] = T1;
	Value[2] = T2;
}

void noRawDistribution::GetValue1Random(FLOAT Time, FLOAT* InValue)
{
	FLOAT* RESTRICT Value = InValue;
	const LOOKUPVALUE* Entry1;
	const LOOKUPVALUE* Entry2;
	FLOAT LerpAlpha = 0.0f;
	GetEntry( Time, Entry1, Entry2, LerpAlpha );
	const LOOKUPVALUE* RESTRICT NewEntry1 = Entry1;
	const LOOKUPVALUE* RESTRICT NewEntry2 = Entry2;
	LOOKUPVALUE Value1,Value2;
	Value1 = Lerp(NewEntry1[0], NewEntry2[0], LerpAlpha);
	Value2 = Lerp(NewEntry1[1 + 0], NewEntry2[1 + 0], LerpAlpha);
	Value[0] = Value1 + (Value2 - Value1) * appSRand();
}

void noRawDistribution::GetValue3Random(FLOAT Time, FLOAT* InValue)
{
	FLOAT* RESTRICT Value = InValue;
	const LOOKUPVALUE* Entry1;
	const LOOKUPVALUE* Entry2;
	FLOAT LerpAlpha = 0.0f;
	GetEntry( Time, Entry1, Entry2, LerpAlpha );
	const LOOKUPVALUE* RESTRICT NewEntry1 = Entry1;
	const LOOKUPVALUE* RESTRICT NewEntry2 = Entry2;
	LOOKUPVALUE Value1,Value2;
	FLOAT T0;
	FLOAT T1;
	FLOAT T2;

	if (DIST_IS_UNIFORMCURVE(Type))
	{
		FLOAT X0, Y0, Z0;
		FLOAT X1, Y1, Z1;

		switch (DIST_GET_LOCKFLAG_0(Type))
		{
		case RDL_XY:
			X0 = Lerp(NewEntry1[0], NewEntry2[0], LerpAlpha);
			Y0 = X0;
			Z0 = Lerp(NewEntry1[2], NewEntry2[2], LerpAlpha);
			break;
		case RDL_XZ:
			X0 = Lerp(NewEntry1[0], NewEntry2[0], LerpAlpha);
			Y0 = Lerp(NewEntry1[1], NewEntry2[1], LerpAlpha);
			Z0 = X0;
			break;
		case RDL_YZ:
			X0 = Lerp(NewEntry1[0], NewEntry2[0], LerpAlpha);
			Y0 = Lerp(NewEntry1[1], NewEntry2[1], LerpAlpha);
			Z0 = Y0;
			break;
		case RDL_XYZ:
			X0 = Lerp(NewEntry1[0], NewEntry2[0], LerpAlpha);
			Y0 = X0;
			Z0 = X0;
			break;
		case RDL_None:
		default:
			X0 = Lerp(NewEntry1[0], NewEntry2[0], LerpAlpha);
			Y0 = Lerp(NewEntry1[1], NewEntry2[1], LerpAlpha);
			Z0 = Lerp(NewEntry1[2], NewEntry2[2], LerpAlpha);
			break;
		}

		switch (DIST_GET_LOCKFLAG_1(Type))
		{
		case RDL_XY:
			X1 = Lerp(NewEntry1[3 + 0], NewEntry2[3 + 0], LerpAlpha);
			Y1 = X1;
			Z1 = Lerp(NewEntry1[3 + 2], NewEntry2[3 + 2], LerpAlpha);
			break;
		case RDL_XZ:
			X1 = Lerp(NewEntry1[3 + 0], NewEntry2[3 + 0], LerpAlpha);
			Y1 = Lerp(NewEntry1[3 + 1], NewEntry2[3 + 1], LerpAlpha);
			Z1 = X1;
			break;
		case RDL_YZ:
			X1 = Lerp(NewEntry1[3 + 0], NewEntry2[3 + 0], LerpAlpha);
			Y1 = Lerp(NewEntry1[3 + 1], NewEntry2[3 + 1], LerpAlpha);
			Z1 = Y1;
			break;
		case RDL_XYZ:
			X1 = Lerp(NewEntry1[3 + 0], NewEntry2[3 + 0], LerpAlpha);
			Y1 = X1;
			Z1 = X1;
			break;
		case RDL_None:
		default:
			X1 = Lerp(NewEntry1[3 + 0], NewEntry2[3 + 0], LerpAlpha);
			Y1 = Lerp(NewEntry1[3 + 1], NewEntry2[3 + 1], LerpAlpha);
			Z1 = Lerp(NewEntry1[3 + 2], NewEntry2[3 + 2], LerpAlpha);
			break;
		}

		Value[0] = X0 + (X1 - X0) * appSRand();
		Value[1] = Y0 + (Y1 - Y0) * appSRand();
		Value[2] = Z0 + (Z1 - Z0) * appSRand();
	}
	else
	{
/***
		Value1 = Lerp(NewEntry1[0], NewEntry2[0], LerpAlpha);
		Value2 = Lerp(NewEntry1[3 + 0], NewEntry2[3 + 0], LerpAlpha);
		T0 = Value1 + (Value2 - Value1) * appSRand();
		Value1 = Lerp(NewEntry1[1], NewEntry2[1], LerpAlpha);
		Value2 = Lerp(NewEntry1[3 + 1], NewEntry2[3 + 1], LerpAlpha);
		T1 = Value1 + (Value2 - Value1) * appSRand();
		Value1 = Lerp(NewEntry1[2], NewEntry2[2], LerpAlpha);
		Value2 = Lerp(NewEntry1[3 + 2], NewEntry2[3 + 2], LerpAlpha);
		T2 = Value1 + (Value2 - Value1) * appSRand();
***/
		switch (DIST_GET_LOCKFLAG_0(Type))
		{
		case RDL_XY:
			Value1 = Lerp(NewEntry1[0], NewEntry2[0], LerpAlpha);
			Value2 = Lerp(NewEntry1[3 + 0], NewEntry2[3 + 0], LerpAlpha);
			T0 = Value1 + (Value2 - Value1) * appSRand();
			Value1 = Lerp(NewEntry1[2], NewEntry2[2], LerpAlpha);
			Value2 = Lerp(NewEntry1[3 + 2], NewEntry2[3 + 2], LerpAlpha);
			T2 = Value1 + (Value2 - Value1) * appSRand();
			Value[0] = T0;
			Value[1] = T0;
			Value[2] = T2;
			break;
		case RDL_XZ:
			Value1 = Lerp(NewEntry1[0], NewEntry2[0], LerpAlpha);
			Value2 = Lerp(NewEntry1[3 + 0], NewEntry2[3 + 0], LerpAlpha);
			T0 = Value1 + (Value2 - Value1) * appSRand();
			Value1 = Lerp(NewEntry1[1], NewEntry2[1], LerpAlpha);
			Value2 = Lerp(NewEntry1[3 + 1], NewEntry2[3 + 1], LerpAlpha);
			T1 = Value1 + (Value2 - Value1) * appSRand();
			Value[0] = T0;
			Value[1] = T1;
			Value[2] = T0;
			break;
		case RDL_YZ:
			Value1 = Lerp(NewEntry1[0], NewEntry2[0], LerpAlpha);
			Value2 = Lerp(NewEntry1[3 + 0], NewEntry2[3 + 0], LerpAlpha);
			T0 = Value1 + (Value2 - Value1) * appSRand();
			Value1 = Lerp(NewEntry1[1], NewEntry2[1], LerpAlpha);
			Value2 = Lerp(NewEntry1[3 + 1], NewEntry2[3 + 1], LerpAlpha);
			T1 = Value1 + (Value2 - Value1) * appSRand();
			Value[0] = T0;
			Value[1] = T1;
			Value[2] = T1;
			break;
		case RDL_XYZ:
			Value1 = Lerp(NewEntry1[0], NewEntry2[0], LerpAlpha);
			Value2 = Lerp(NewEntry1[3 + 0], NewEntry2[3 + 0], LerpAlpha);
			T0 = Value1 + (Value2 - Value1) * appSRand();
			Value[0] = T0;
			Value[1] = T0;
			Value[2] = T0;
			break;
		case RDL_None:
		default:
			Value1 = Lerp(NewEntry1[0], NewEntry2[0], LerpAlpha);
			Value2 = Lerp(NewEntry1[3 + 0], NewEntry2[3 + 0], LerpAlpha);
			T0 = Value1 + (Value2 - Value1) * appSRand();
			Value1 = Lerp(NewEntry1[1], NewEntry2[1], LerpAlpha);
			Value2 = Lerp(NewEntry1[3 + 1], NewEntry2[3 + 1], LerpAlpha);
			T1 = Value1 + (Value2 - Value1) * appSRand();
			Value1 = Lerp(NewEntry1[2], NewEntry2[2], LerpAlpha);
			Value2 = Lerp(NewEntry1[3 + 2], NewEntry2[3 + 2], LerpAlpha);
			T2 = Value1 + (Value2 - Value1) * appSRand();
			Value[0] = T0;
			Value[1] = T1;
			Value[2] = T2;
			break;
		}

	}
}

/**
 * Calcuate the float or vector value at the given time 
 * @param Time The time to evaluate
 * @param Value An array of (1 or 3) FLOATs to receivet the values
 * @param NumCoords The number of floats in the Value array
 * @param Extreme For distributions that use one of the extremes, this is which extreme to use
 */
void noRawDistribution::GetValue(FLOAT Time, FLOAT* Value, INT NumCoords, INT Extreme)
{
	assert(NumCoords == 3 || NumCoords == 1);
	switch (Op)
	{
	case RDO_None:
		if (NumCoords == 1)
		{
			GetValue1None(Time,Value);
		}
		else
		{
			GetValue3None(Time,Value);
		}
		break;
	case RDO_Extreme:
		if (NumCoords == 1)
		{
			GetValue1Extreme(Time,Value,Extreme);
		}
		else
		{
			GetValue3Extreme(Time,Value,Extreme);
		}
		break;
	case RDO_Random:
		if (NumCoords == 1)
		{
			GetValue1Random(Time,Value);
		}
		else
		{
			GetValue3Random(Time,Value);
		}
		break;
	}
}


/*-----------------------------------------------------------------------------
	FDistributionFloat
-----------------------------------------------------------------------------*/

#if !CONSOLE
/** 
 * Fill out the RawDistrubutions with data appropriate to this distribution 
 */
void noRawDistributionFloat::Initialize()
{
	// Nothing to do if we don't have a distribution.
	if( Distribution == NULL )
	{
		return;
	}

	// does this noRawDist need updating? (if UDist is dirty or somehow the distribution wasn't dirty, but we have no data)
	UBOOL bNeedsUpdating = FALSE;
	if (Distribution->bIsDirty || (LookupTable.Num() == 0 && Distribution->CanBeBaked()))
	{
		if (!Distribution->bIsDirty)
		{
			assert(false);//(TEXT("Somehow Distribution %s wasn't dirty, but its noRawDistribution wasn't ever initialized!"), *Distribution->GetFullName());
			
		}
		bNeedsUpdating = TRUE;
	}

	// only initialize if we need to
	if (!bNeedsUpdating)
	{
		return;
	}

	// always empty out the lookup table
	LookupTable.Empty();

	// distribution is no longer dirty (if it was)
	// template objects aren't marked as dirty, because any UDists that uses this as an archetype, 
	// aren't the default values, and has already been saved, needs to know to build the FDist
	//if (!Distribution->IsTemplate())
	//{
	//	Distribution->bIsDirty = FALSE;
	//}

	// if the distribution can't be baked out, then we do nothing here
	if (!Distribution->CanBeBaked())
	{
		return;
	}

	FLOAT MinIn, MaxIn;
	FLOAT MinOut, MaxOut;
	// fill out our min/max
	Distribution->GetInRange(MinIn, MaxIn);
	Distribution->GetOutRange(MinOut, MaxOut);

	DWORD NumPoints;
	FLOAT TimeScale;

	// check for some special cases

	// first check for linear line between two points - this there's no need for intermediate values at all
	if (Distribution->GetNumKeys() == 2 && Distribution->GetKeyInterpMode(0) == CIM_Linear)
	{
		NumPoints = 2;
		TimeScale = MaxIn - MinIn;
	}
	// next check for entire distribution fitting inside one time slice
	else if (MaxIn - MinIn < 1.0f / GLookupTableFrameRate)
	{
		NumPoints = 2;
		TimeScale = MaxIn - MinIn;
	}
	// otherwise, normal
	else
	{
		// we sample at GLookupTableFrameRate fps, or with a max of GLookupTableMaxFrames sample points, making sure there's always 1
		NumPoints = Min<DWORD>((DWORD)((MaxIn - MinIn) * GLookupTableFrameRate), (GLookupTableMaxFrames - 1)) + 1;

		// calculate time scale (1 point doesn't need a scale)
		TimeScale = (NumPoints > 1) ? (MaxIn - MinIn) / (FLOAT)(NumPoints - 1) : 0;
	}


	// get the operation to use, and calculate the number of elements needed for that operation
	Op = Distribution->GetOperation();
	LookupTableNumElements = (Op == RDO_None) ? 1 : 2;

	// Need to ensure this is 0 now as we use it for lock flags and such...
	Type = 0;

	// store our min/max as the first 2 entries in the table
	// note that there is only one element per entry here
	// @GEMINI_TODO: For (non-uniform) constants, we don't need these at 
	LookupTable.AddItem(MinOut);
	LookupTable.AddItem(MaxOut);

	// bake out all the points
	for (DWORD Sample = 0; Sample < NumPoints; Sample++)
	{
		// time for this point
		FLOAT Time = MinIn + Sample * TimeScale;
		// get all elements at this time point
		FLOAT Values[4];
		// get values, and remember how many were used
		Distribution->InitializeRawEntry(Time, Values);

		// add each one
		for (DWORD Element = 0; Element < LookupTableNumElements; Element++)
		{
			LookupTable.AddItem(Values[Element]);
		}
	}

	// fill out the raw distrib structure
	LookupTableChunkSize = LookupTableNumElements; 
	LookupTableTimeScale = TimeScale;
	if (TimeScale != 0.0f)
	{
		LookupTableTimeScale = 1.0f / TimeScale;
	}
	LookupTableStartTime = MinIn;
}
#endif

FLOAT noRawDistributionFloat::GetValue(FLOAT F, UObject* Data)
{
#if !CONSOLE
	// make sure it's up to date
	if( GIsEditor || (Distribution && Distribution->bIsDirty) )
	{
		Initialize();
	}
#endif

	// if this distribution is in memory, that means the package wasn't saved, or we
	// want to use it (or its the editor, and we are in "Use Original Distribution" mode
	if (Distribution 
#if !CONSOLE
		&& (GIsGame || GDistributionType == 0 || LookupTable.Num() == 0)
#endif
		)
	{
		return Distribution->GetValue(F, Data);
	}

	// if we get here, we better have been initialized!
	assert(LookupTable.Num());

	FLOAT Value;
	noRawDistribution::GetValue1(F, &Value);
	return Value;
}

const noRawDistribution *noRawDistributionFloat::GetFastRawDistribution()
{
#if !CONSOLE
	// make sure it's up to date
	if( GIsEditor )
	{
		Initialize();
	}
#endif

	if (!IsSimple())
	{
		return 0;
	}

	// if this distribution is in memory, that means the package wasn't saved, or we
	// want to use it (or its the editor, and we are in "Use Original Distribution" mode
	if (Distribution 
#if !CONSOLE
		&& (GIsGame || GDistributionType == 0 || LookupTable.Num() == 0)
#endif
		)
	{
		return 0;
	}

	// if we get here, we better have been initialized!
	assert(LookupTable.Num());

	return this;
}

/**
 * Get the min and max values
 */
void noRawDistributionFloat::GetOutRange(FLOAT& MinOut, FLOAT& MaxOut)
{
	if (LookupTable.Num() == 0 || GDistributionType == 0)
	{
		assert(Distribution);
		Distribution->GetOutRange(MinOut, MaxOut);
	}
	else
	{
		MinOut = LookupTable(0);
		MaxOut = LookupTable(1);
	}
}

FLOAT noDistributionFloat::GetValue( FLOAT F, UObject* Data )
{
	return 0.0;
}

void noDistributionFloat::GetInRange(FLOAT& MinIn, FLOAT& MaxIn)
{
	MinIn	= 0.0f;
	MaxIn	= 0.0f;
}

void noDistributionFloat::GetOutRange(FLOAT& MinOut, FLOAT& MaxOut)
{
	MinOut	= 0.0f;
	MaxOut	= 0.0f;
}

/*-----------------------------------------------------------------------------
	noRawDistributionVector
-----------------------------------------------------------------------------*/
#if !CONSOLE
/** 
 * Fill out the RawDistrubutions with data appropriate to this distribution 
 */
void noRawDistributionVec::Initialize()
{
	// Nothing to do if we don't have a distribution.
	if( Distribution == NULL )
	{
		return;
	}

	// does this noRawDist need updating? (if UDist is dirty or somehow the distribution wasn't dirty, but we have no data)
	UBOOL bNeedsUpdating = FALSE;
	if (Distribution->bIsDirty || (LookupTable.Num() == 0 && Distribution->CanBeBaked()))
	{
		if (!Distribution->bIsDirty)
		{
			assert(false);
			//debugf(TEXT("Somehow Distribution %s wasn't dirty, but its noRawDistribution wasn't ever initialized!"), *Distribution->GetFullName());
		}
		bNeedsUpdating = TRUE;
	}

	// only initialize if we need to
	if (!bNeedsUpdating)
	{
		return;
	}

	// always empty out the lookup table
	LookupTable.Empty();

	// distribution is no longer dirty (if it was)
	// template objects aren't marked as dirty, because any UDists that uses this as an archetype, 
	// aren't the default values, and has already been saved, needs to know to build the FDist
	/*if (!Distribution->IsTemplate())
	{
		Distribution->bIsDirty = FALSE;	
	}*/

	// if the distribution can't be baked out, then we do nothing here
	if (!Distribution->CanBeBaked())
	{
		return;
	}

	FLOAT MinIn, MaxIn;
	FLOAT MinOut, MaxOut;
	// fill out our min/max
	Distribution->GetInRange(MinIn, MaxIn);
	Distribution->GetOutRange(MinOut, MaxOut);

	DWORD NumPoints;
	FLOAT TimeScale;

	// check for some special cases

	// first check for linear line between two points - this there's no need for intermediate values at all
	if (Distribution->GetNumKeys() == 2 && Distribution->GetKeyInterpMode(0) == CIM_Linear)
	{
		NumPoints = 2;
		TimeScale = MaxIn - MinIn;
	}
	// next check for entire distribution fitting inside one time slice
	else if (MaxIn - MinIn < 1.0f / GLookupTableFrameRate)
	{
		NumPoints = 2;
		TimeScale = MaxIn - MinIn;
	}
	// otherwise, normal
	else
	{
		// we sample at GLookupTableFrameRate fps, or with a max of GLookupTableMaxFrames sample points, making sure there's always 1
		NumPoints = Min<DWORD>((DWORD)((MaxIn - MinIn) * GLookupTableFrameRate), (GLookupTableMaxFrames - 1)) + 1;

		// calculate time scale (1 point doesn't need a scale)
		TimeScale = (NumPoints > 1) ? (MaxIn - MinIn) / (FLOAT)(NumPoints - 1) : 0;
	}

	// get the operation to use, and calculate the number of elements needed for that operation
	Op = Distribution->GetOperation();
	LookupTableNumElements = (Op == RDO_None) ? 1 : 2;

	Type = 0;
	DIST_SET_LOCKFLAG_0(Distribution->GetLockFlags(0), Type);
	DIST_SET_LOCKFLAG_1(Distribution->GetLockFlags(1), Type);
	DIST_SET_UNIFORMCURVE(Distribution->IsUniformCurve(), Type);

	// store our min/max as the first 2 values in the table
	// note that there is only one element per entry here
	// @GEMINI_TODO: For (non-uniform) constants, we don't need these at 
	LookupTable.AddItem(MinOut);
	LookupTable.AddItem(MaxOut);

	// bake out all the points
	for (DWORD Sample = 0; Sample < NumPoints; Sample++)
	{
		// time for this point
		FLOAT Time = MinIn + Sample * TimeScale;
		// get all elements at this time point
		FVector Values[2];
		// get values, and remember how many were used
		Distribution->InitializeRawEntry(Time, Values);

		// add each one
		for (DWORD Element = 0; Element < LookupTableNumElements; Element++)
		{
			LookupTable.AddItem(Values[Element].x);
			LookupTable.AddItem(Values[Element].y);
			LookupTable.AddItem(Values[Element].z);
		}
	}

	// fill out the raw distrib structure
	LookupTableChunkSize = LookupTableNumElements * 3;
	LookupTableTimeScale = TimeScale;
	if (TimeScale != 0.0f)
	{
		LookupTableTimeScale = 1.0f / TimeScale;
	}
	LookupTableStartTime = MinIn;
}
#endif

FVector noRawDistributionVec::GetValue(FLOAT F, UObject* Data, INT Extreme)
{
#if !CONSOLE
	// make sure it's up to date
	if( GIsEditor )
	{
		Initialize();
	}
#endif

	// if this distribution is in memory, that means the package wasn't saved, or we
	// want to use it (or its the editor, and we are in "Use Original Distribution" mode
	if (Distribution 
#if !CONSOLE
		&& (GIsGame || GDistributionType == 0 || LookupTable.Num() == 0)
#endif
		)
	{
		return Distribution->GetValue(F, Data, Extreme);
	}

	// if we get here, we better have been initialized!
	assert(LookupTable.Num());

	FVector Value;
	noRawDistribution::GetValue3(F, &Value.x);
	return Value;
}

const noRawDistribution *noRawDistributionVec::GetFastRawDistribution()
{
#if !CONSOLE
	// make sure it's up to date
	if( GIsEditor || (Distribution && Distribution->bIsDirty) )
	{
		Initialize();
	}
#endif

	if (!IsSimple()) 
	{
		return 0;
	}

	// if this distribution is in memory, that means the package wasn't saved, or we
	// want to use it (or its the editor, and we are in "Use Original Distribution" mode
	if (Distribution
#if !CONSOLE
		&& (GIsGame || GDistributionType == 0 || LookupTable.Num() == 0)
#endif
		)
	{
		return 0;
	}

	// if we get here, we better have been initialized!
	assert(LookupTable.Num());

	return this;
}

/**
 * Get the min and max values
 */
void noRawDistributionVec::GetOutRange(FLOAT& MinOut, FLOAT& MaxOut)
{
	if (LookupTable.Num() == 0 || GDistributionType == 0)
	{
		assert(Distribution);
		Distribution->GetOutRange(MinOut, MaxOut);
	}
	else
	{
		MinOut = LookupTable(0);
		MaxOut = LookupTable(1);
	}
}

/*-----------------------------------------------------------------------------
	UDistributionFloatConstant implementation.
-----------------------------------------------------------------------------*/

FLOAT UDistributionFloatConstant::GetValue( FLOAT F, UObject* Data )
{
	return Constant;
}


//////////////////////// FCurveEdInterface ////////////////////////

INT UDistributionFloatConstant::GetNumKeys()
{
	return 1;
}

INT UDistributionFloatConstant::GetNumSubCurves()
{
	return 1;
}

FLOAT UDistributionFloatConstant::GetKeyIn(INT KeyIndex)
{
	assert( KeyIndex == 0 );
	return 0.f;
}

FLOAT UDistributionFloatConstant::GetKeyOut(INT SubIndex, INT KeyIndex)
{
	assert( SubIndex == 0 );
	assert( KeyIndex == 0 );
	return Constant;
}

void UDistributionFloatConstant::GetInRange(FLOAT& MinIn, FLOAT& MaxIn)
{
	MinIn = 0.f;
	MaxIn = 0.f;
}

void UDistributionFloatConstant::GetOutRange(FLOAT& MinOut, FLOAT& MaxOut)
{
	MinOut = Constant;
	MaxOut = Constant;
}

BYTE UDistributionFloatConstant::GetKeyInterpMode(INT KeyIndex)
{
	assert( KeyIndex == 0 );
	return CIM_Constant;
}

void UDistributionFloatConstant::GetTangents(INT SubIndex, INT KeyIndex, FLOAT& ArriveTangent, FLOAT& LeaveTangent)
{
	assert( SubIndex == 0 );
	assert( KeyIndex == 0 );
	ArriveTangent = 0.f;
	LeaveTangent = 0.f;
}

FLOAT UDistributionFloatConstant::EvalSub(INT SubIndex, FLOAT InVal)
{
	assert(SubIndex == 0);
	return Constant;
}

INT UDistributionFloatConstant::CreateNewKey(FLOAT KeyIn)
{	
	return 0;
}

void UDistributionFloatConstant::DeleteKey(INT KeyIndex)
{
	assert( KeyIndex == 0 );
}

INT UDistributionFloatConstant::SetKeyIn(INT KeyIndex, FLOAT NewInVal)
{
	assert( KeyIndex == 0 );
	return 0;
}

void UDistributionFloatConstant::SetKeyOut(INT SubIndex, INT KeyIndex, FLOAT NewOutVal) 
{
	assert( SubIndex == 0 );
	assert( KeyIndex == 0 );
	Constant = NewOutVal;

	bIsDirty = TRUE;
}

void UDistributionFloatConstant::SetKeyInterpMode(INT KeyIndex, EInterpCurveMode NewMode) 
{
	assert( KeyIndex == 0 );
}

void UDistributionFloatConstant::SetTangents(INT SubIndex, INT KeyIndex, FLOAT ArriveTangent, FLOAT LeaveTangent)
{
	assert( SubIndex == 0 );
	assert( KeyIndex == 0 );
}

//
//	UDistributionFloatParameterBase
//
FLOAT UDistributionFloatParameterBase::GetValue( FLOAT F, UObject* Data )
{
	FLOAT ParamFloat = 0.f;
	UBOOL bFoundParam = GetParamValue(Data, ParameterName, ParamFloat);
	if(!bFoundParam)
	{
		ParamFloat = Constant;
	}

	if(ParamMode == DPM_Direct)
	{
		return ParamFloat;
	}
	else if(ParamMode == DPM_Abs)
	{
		ParamFloat = Abs(ParamFloat);
	}

	FLOAT Gradient;
	if(MaxInput <= MinInput)
		Gradient = 0.f;
	else
		Gradient = (MaxOutput - MinOutput)/(MaxInput - MinInput);

	FLOAT ClampedParam = ::Clamp(ParamFloat, MinInput, MaxInput);
	FLOAT Output = MinOutput + ((ClampedParam - MinInput) * Gradient);

	return Output;
}

FVector UDistributionVectorParameterBase::GetValue( FLOAT F, UObject* Data, INT Extreme )
{
	FVector ParamVector(vec3_zero);
	UBOOL bFoundParam = GetParamValue(Data, ParameterName, ParamVector);
	if(!bFoundParam)
	{
		ParamVector = Constant;
	}

	if(ParamModes[0] == DPM_Abs)
	{
		ParamVector.x = Abs(ParamVector.x);
	}

	if(ParamModes[1] == DPM_Abs)
	{
		ParamVector.y = Abs(ParamVector.y);
	}

	if(ParamModes[2] == DPM_Abs)
	{
		ParamVector.z = Abs(ParamVector.z);
	}

	FVector Gradient;
	if(MaxInput.x <= MinInput.x)
		Gradient.x = 0.f;
	else
		Gradient.x = (MaxOutput.x - MinOutput.x)/(MaxInput.x - MinInput.x);

	if(MaxInput.y <= MinInput.y)
		Gradient.y = 0.f;
	else
		Gradient.y = (MaxOutput.y - MinOutput.y)/(MaxInput.y - MinInput.y);

	if(MaxInput.z <= MinInput.z)
		Gradient.z = 0.f;
	else
		Gradient.z = (MaxOutput.z - MinOutput.z)/(MaxInput.z - MinInput.z);

	FVector ClampedParam;
	ClampedParam.x = ::Clamp(ParamVector.x, MinInput.x, MaxInput.x);
	ClampedParam.y = ::Clamp(ParamVector.y, MinInput.y, MaxInput.y);
	ClampedParam.z = ::Clamp(ParamVector.z, MinInput.z, MaxInput.z);

	FVector Output = MinOutput + ((ClampedParam - MinInput).Multiply(Gradient));

	if(ParamModes[0] == DPM_Direct)
	{
		Output.x = ParamVector.x;
	}

	if(ParamModes[1] == DPM_Direct)
	{
		Output.y = ParamVector.y;
	}

	if(ParamModes[2] == DPM_Direct)
	{
		Output.z = ParamVector.z;
	}

	return Output;
}

/*-----------------------------------------------------------------------------
	UDistributionVectorConstant implementation.
-----------------------------------------------------------------------------*/

FVector UDistributionVectorConstant::GetValue(FLOAT F, UObject* Data, INT Extreme)
{
	switch (LockedAxes)
	{
    case EDVLF_XY:
		return FVector(Constant.x, Constant.x, Constant.z);
    case EDVLF_XZ:
		return FVector(Constant.x, Constant.y, Constant.x);
    case EDVLF_YZ:
		return FVector(Constant.x, Constant.y, Constant.y);
	case EDVLF_XYZ:
		return FVector(Constant.x, Constant.x, Constant.x);
    case EDVLF_None:
	default:
		return Constant;
	}
}

//void UDistributionVectorConstant::Serialize(FArchive& Ar)
//{
//	Super::Serialize(Ar);
//}


//////////////////////// FCurveEdInterface ////////////////////////

INT UDistributionVectorConstant::GetNumKeys()
{
	return 1;
}

INT UDistributionVectorConstant::GetNumSubCurves()
{
	switch (LockedAxes)
	{
    case EDVLF_XY:
    case EDVLF_XZ:
    case EDVLF_YZ:
		return 2;
	case EDVLF_XYZ:
		return 1;
	}
	return 3;
}

FLOAT UDistributionVectorConstant::GetKeyIn(INT KeyIndex)
{
	assert( KeyIndex == 0 );
	return 0.f;
}

FLOAT UDistributionVectorConstant::GetKeyOut(INT SubIndex, INT KeyIndex)
{
	assert( SubIndex >= 0 && SubIndex < 3);
	assert( KeyIndex == 0 );
	
	if (SubIndex == 0)
	{
		return Constant.x;
	}
	else 
	if (SubIndex == 1)
	{
		if ((LockedAxes == EDVLF_XY) || (LockedAxes == EDVLF_XYZ))
		{
			return Constant.x;
		}
		else
		{
			return Constant.y;
		}
	}
	else 
	{
		if ((LockedAxes == EDVLF_XZ) || (LockedAxes == EDVLF_XYZ))
		{
			return Constant.x;
		}
		else
		if (LockedAxes == EDVLF_YZ)
		{
			return Constant.y;
		}
		else
		{
			return Constant.z;
		}
	}
}

FColor UDistributionVectorConstant::GetKeyColor(INT SubIndex, INT KeyIndex, const FColor& CurveColor)
{
	assert( SubIndex >= 0 && SubIndex < 3);
	assert( KeyIndex == 0 );

	if(SubIndex == 0)
		return FColor(255,0,0);
	else if(SubIndex == 1)
		return FColor(0,255,0);
	else
		return FColor(0,0,255);
}

void UDistributionVectorConstant::GetInRange(FLOAT& MinIn, FLOAT& MaxIn)
{
	MinIn = 0.f;
	MaxIn = 0.f;
}

void UDistributionVectorConstant::GetOutRange(FLOAT& MinOut, FLOAT& MaxOut)
{
	FVector Local;

	switch (LockedAxes)
	{
    case EDVLF_XY:
		Local = FVector(Constant.x, Constant.x, Constant.z);
		break;
    case EDVLF_XZ:
		Local = FVector(Constant.x, Constant.y, Constant.x);
		break;
    case EDVLF_YZ:
		Local = FVector(Constant.x, Constant.y, Constant.y);
		break;
    case EDVLF_XYZ:
		Local = FVector(Constant.x, Constant.x, Constant.x);
		break;
	case EDVLF_None:
	default:
		Local = FVector(Constant);
		break;
	}

	MinOut = Local.GetMin();
	MaxOut = Local.GetMax();
}

BYTE UDistributionVectorConstant::GetKeyInterpMode(INT KeyIndex)
{
	assert( KeyIndex == 0 );
	return CIM_Constant;
}

void UDistributionVectorConstant::GetTangents(INT SubIndex, INT KeyIndex, FLOAT& ArriveTangent, FLOAT& LeaveTangent)
{
	assert( SubIndex >= 0 && SubIndex < 3);
	assert( KeyIndex == 0 );
	ArriveTangent = 0.f;
	LeaveTangent = 0.f;
}

FLOAT UDistributionVectorConstant::EvalSub(INT SubIndex, FLOAT InVal)
{
	assert( SubIndex >= 0 && SubIndex < 3);
	return GetKeyOut(SubIndex, 0);
}

INT UDistributionVectorConstant::CreateNewKey(FLOAT KeyIn)
{	
	return 0;
}

void UDistributionVectorConstant::DeleteKey(INT KeyIndex)
{
	assert( KeyIndex == 0 );
}

INT UDistributionVectorConstant::SetKeyIn(INT KeyIndex, FLOAT NewInVal)
{
	assert( KeyIndex == 0 );
	return 0;
}

void UDistributionVectorConstant::SetKeyOut(INT SubIndex, INT KeyIndex, FLOAT NewOutVal) 
{
	assert( SubIndex >= 0 && SubIndex < 3);
	assert( KeyIndex == 0 );

	if(SubIndex == 0)
		Constant.x = NewOutVal;
	else if(SubIndex == 1)
		Constant.y = NewOutVal;
	else if(SubIndex == 2)
		Constant.z = NewOutVal;

	bIsDirty = TRUE;
}

void UDistributionVectorConstant::SetKeyInterpMode(INT KeyIndex, EInterpCurveMode NewMode) 
{
	assert( KeyIndex == 0 );
}

void UDistributionVectorConstant::SetTangents(INT SubIndex, INT KeyIndex, FLOAT ArriveTangent, FLOAT LeaveTangent)
{
	assert( SubIndex >= 0 && SubIndex < 3);
	assert( KeyIndex == 0 );
}

// DistributionVector interface
/** GetRange - in the case of a constant curve, this will not be exact!				*/
void UDistributionVectorConstant::GetRange(FVector& OutMin, FVector& OutMax)
{
	OutMin	= Constant;
	OutMax	= Constant;
}

DWORD noDistributionVec::InitializeRawEntry( float Time, noVec3* Values )
{
	Values[0] = GetValue(Time);
	return 1;
}

FVector noDistributionVec::GetValue( FLOAT F /*= 0.f*/, UObject* Data /*= NULL*/, INT LastExtreme /*= 0 */ )
{
	return FVector(0,0,0);
}

void noDistributionVec::GetInRange( FLOAT& MinIn, FLOAT& MaxIn )
{
	MinIn	= 0.0f;
	MaxIn	= 0.0f;
}

void noDistributionVec::GetOutRange( FLOAT& MinOut, FLOAT& MaxOut )
{
	MinOut	= 0.0f;
	MaxOut	= 0.0f;
}

void noDistributionVec::GetRange( FVector& OutMin, FVector& OutMax )
{
	OutMin	= FVector(0.0f, 0.0f, 0.0f);
	OutMax	= FVector(0.0f, 0.0f, 0.0f);
}
