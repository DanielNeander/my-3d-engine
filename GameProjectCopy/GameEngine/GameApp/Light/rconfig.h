/******************************************************************************
"Practical Methods for Precomputed Radiance Transfer Using Spherical Harmonics"

Jerome Ko - submatrix@gmail.com
Manchor Ko - man961.great@gmail.com
Matthias Zwicker - mzwicker@cs.ucsd.edu

******************************************************************************/

#ifndef RCONFIG_H
#define RCONFIG_H
//rendering configuration controls:
#define USEMESHFORDISPLAY 1

#define USESHADER	1		//use vertex shader or CPU to compute lighting
#define USEPRT		1		//send PRT coefficients to shader or not

#define USENORMAL	(USEPRT == 0)	//send normal to shader or not
//#define USEVBO	1		//use VBO or not
#define USEEB0		1
#define USEBATCH	0		//use DrawRangeElements or not

//n^M^n config:
#define USEBATCHFORCOLOR 0	//use small batch buffer for color - only used if (USEPRT == 0)
#define SEPCOLOR		 1	//use separate VBO for colors
#define INTERLEAVEDCOLOR 0	//color interleaved with the rest of the attributes
#define UPDATELIGHTMETHOD 2 //1: Method A
                            //2: Method B
                            //3: Method C
//PRT config:
//#define USEPACK	1		//use packing
//#define PRTPACK	0		//0: 333b -> vec3[3], 1: 441b -> vec4[2]+float, 2: variable 48 bits
#define USESOA		0		//use SoA for Li

#endif
