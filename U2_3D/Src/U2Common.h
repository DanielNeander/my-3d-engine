#ifndef U2_COMMON_H
#define U2_COMMON_H


namespace Common {;


// swap variables of any type
template<typename Type>
inline void Swap(Type &A, Type &B)
{
	Type C=A;
	A=B;
	B=C;
}

// clamp variables of any type
template<typename Type>
inline Type Clamp(const Type &A, const Type &Min, const Type &Max)
{
	if (A < Min) return Min;
	if (A > Max) return Max;
	return A;
}

// return smaller of the given variables
template<typename Type>
inline Type Min(const Type &A, const Type &B)
{
	if (A < B) return A;
	return B;
}

// return larger of the given variables
template<typename Type>
inline Type Max(const Type &A, const Type &B)
{
	if (A > B) return A;
	return B;
}


}



#endif