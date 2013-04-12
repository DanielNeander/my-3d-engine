#pragma once 


//
// Globally unique identifier.
//
class FGuid
{
public:
	DWORD A,B,C,D;
	FGuid()
	{}
	FGuid( DWORD InA, DWORD InB, DWORD InC, DWORD InD )
	: A(InA), B(InB), C(InC), D(InD)
	{}
	explicit FORCEINLINE FGuid(EEventParm)
	: A(0), B(0), C(0), D(0)
    {
    }

	/**
	 * Returns whether this GUID is valid or not. We reserve an all 0 GUID to represent "invalid".
	 *
	 * @return TRUE if valid, FALSE otherwise
	 */
	UBOOL IsValid() const
	{
		return (A | B | C | D) != 0;
	}

	/** Invalidates the GUID. */
	void Invalidate()
	{
		A = B = C = D = 0;
	}

	friend UBOOL operator==(const FGuid& X, const FGuid& Y)
	{
		return ((X.A ^ Y.A) | (X.B ^ Y.B) | (X.C ^ Y.C) | (X.D ^ Y.D)) == 0;
	}
	friend UBOOL operator!=(const FGuid& X, const FGuid& Y)
	{
		return ((X.A ^ Y.A) | (X.B ^ Y.B) | (X.C ^ Y.C) | (X.D ^ Y.D)) != 0;
	}
	DWORD& operator[]( INT Index )
	{
		assert(Index>=0);
		assert(Index<4);
		switch(Index)
		{
		case 0: return A;
		case 1: return B;
		case 2: return C;
		case 3: return D;
		}

		return A;
	}
	const DWORD& operator[]( INT Index ) const
	{
		assert(Index>=0);
		assert(Index<4);
		switch(Index)
		{
		case 0: return A;
		case 1: return B;
		case 2: return C;
		case 3: return D;
		}

		return A;
	}
	friend FArchive& operator<<( FArchive& Ar, FGuid& G )
	{
		return Ar << G.A << G.B << G.C << G.D;
	}
	FString String() const
	{
		return FString::Printf( TEXT("%08X%08X%08X%08X"), A, B, C, D );
	}
	friend DWORD GetTypeHash(const FGuid& Guid)
	{
		return appMemCrc(&Guid,sizeof(FGuid));
	}
};