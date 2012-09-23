#ifndef MATH_DISTRIBUTION_H
#define MATH_DISTRIBUTION_H

#include <Math/Math.h>
#include <Math/Vector.h>

class noObject;

struct noRawDistributionVec : public noRawDistribution {
	class noDistributionVec* Distribution;

	void Initialize();

	/**
	 * Get the value at the specified F
	 */
	noVec3 GetValue(FLOAT F=0.0f, noObject* Data=NULL, INT LastExtreme=0);

	/**
	 * Get the min and max values
	 */
	void GetOutRange(FLOAT& MinOut, FLOAT& MaxOut);

	/**
	 * Is this distribution a uniform type? (ie, does it have two values per entry?)
	 */
	inline BOOL IsUniform() { return LookupTableNumElements == 2; }

};

struct noRawDistributionFloat : public noRawDistribution
{
    class noDistributionFloat* Distribution;

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
	//FLOAT GetValue(FLOAT F=0.0f, UObject* Data=NULL);

	/**
	 * Get the min and max values
	 */
	void GetOutRange(FLOAT& MinOut, FLOAT& MaxOut);

	/**
	 * Is this distribution a uniform type? (ie, does it have two values per entry?)
	 */
	inline BOOL IsUniform() { return LookupTableNumElements == 2; }

};

class noDistributionVec 
{
public:
	UINT32 m_canbeBaked:1;
	UINT32 m_isDirty:1;

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
	virtual DWORD InitializeRawEntry(FLOAT Time, noVec3* Values);
};


#endif