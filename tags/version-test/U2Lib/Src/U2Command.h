#pragma once 
#ifndef U2_Command_H
#define U2_Command_H

#include "U2LibType.h"

class U2LIB U2Command : public U2MemObj
{
public:
	U2Command (int iQuantity, TCHAR** apcArgument);
	U2Command (TCHAR* acCmdline);
	~U2Command ();

	// return value is index of first excess argument
	int ExcessArguments ();

	// Set bounds for numerical arguments.  If bounds are required, they must
	// be set for each argument.
	U2Command& Min (double dValue);
	U2Command& Max (double dValue);
	U2Command& Inf (double dValue);
	U2Command& Sup (double dValue);

	// The return value of the following methods is the option index within
	// the argument array.

	// Use the boolean methods for options which take no argument, for
	// example in
	//           myprogram -debug -x 10 -y 20 filename
	// the option -debug has no argument.

	int Boolean (TCHAR* acName);  // returns existence of option
	int Boolean (TCHAR* acName, bool& rbValue);
	int Integer (TCHAR* acName, int& riValue);
	int Float (TCHAR* acName, float& rfValue);
	int Double (TCHAR* acName, double& rdValue);
	int String (TCHAR* acName, TCHAR*& racValue);
	int Filename (TCHAR*& racName);

	// last error reporting
	const TCHAR* GetLastError ();

protected:
	// constructor support
	void Initialize ();

	// U2Command line information
	int m_iQuantity;       // number of arguments
	TCHAR** m_apcArgument;  // argument list (array of strings)
	TCHAR* m_acCmdline;     // argument list (single string)
	bool* m_abUsed;        // keeps track of arguments already processed

	// parameters for bounds checking
	double m_dSmall;   // lower bound for numerical argument (min or inf)
	double m_dLarge;   // upper bound for numerical argument (max or sup)
	bool m_bMinSet;    // if true, compare:  small <= arg
	bool m_bMaxSet;    // if true, compare:  arg <= large
	bool m_bInfSet;    // if true, compare:  small < arg
	bool m_bSupSet;    // if true, compare:  arg < large

	// last error strings
	const TCHAR* m_acLastError;
	static TCHAR ms_acOptionNotFound[];
	static TCHAR ms_acArgumentRequired[];
	static TCHAR ms_acArgumentOutOfRange[];
	static TCHAR ms_acFilenameNotFound[];
};



#endif