#pragma once 

#define IMPLEMENT_COMPARE_CONSTREF( Type, Filename, FunctionBody )		\
class Compare##Filename##Type##ConstRef									\
	{																	\
	public:																\
	static inline INT Compare( const Type& A, const Type& B	)		\
	FunctionBody												\
};

