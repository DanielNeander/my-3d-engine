/******************************************************************************
"Practical Methods for Precomputed Radiance Transfer Using Spherical Harmonics"

Jerome Ko - submatrix@gmail.com
Manchor Ko - man961.great@gmail.com
Matthias Zwicker - mzwicker@cs.ucsd.edu

******************************************************************************/
#include "stdafx.h"
#include <assert.h>
#include <malloc.h>  // _alloca
#include <conio.h>

#include "rconfig.h"
#include "shlight.h"
#include "prefilter.h"
//#include "shader.h"
#include "soa.h"
#include "utils/pfmreader.h"

SHLight::SHLight( const float* coeffs, int bands ) : m_Lcoeffs(0), m_bands(bands)
{
	Init( coeffs, bands );
}

SHLight::~SHLight()
{
	delete [] m_Lcoeffs;	m_Lcoeffs = 0;
}

void SHLight::Init(const float* coeffs, int bands)
{
	assert( bands == 9 );
	float Lcoeffs[9][3];

	m_Lcoeffs = new float[bands*3];
	float* Lc = m_Lcoeffs;
	const float* input = coeffs;

	for (int i = 0; i < bands; ++i) { //copy the coeffs into a 27 float array
		for (int j = 0; j < 3; ++j) {
			Lc[j] = input[j];
			Lcoeffs[i][j] = input[j];
		}
		Lc += 3;
		input += 3;
	}

	float ccoeffs[5];
	//compute 'M' matrix:
	ccoeffs[0] = 0.429043;
	ccoeffs[1] = 0.511664;
	ccoeffs[2] = 0.743125;
	ccoeffs[3] = 0.886227;
	ccoeffs[4] = 0.247708;
	
	noMat4& redMatrix = m_redMatrix, &greenMatrix = m_greenMatrix, &blueMatrix = m_blueMatrix;

	redMatrix[0] = noVec4(ccoeffs[0]*Lcoeffs[8][0], ccoeffs[0]*Lcoeffs[4][0], ccoeffs[0]*Lcoeffs[7][0], ccoeffs[1]*Lcoeffs[3][0]);
	redMatrix[1] = noVec4(ccoeffs[0]*Lcoeffs[4][0], -ccoeffs[0]*Lcoeffs[7][0], ccoeffs[0]*Lcoeffs[5][0], ccoeffs[1]*Lcoeffs[1][0]);
	redMatrix[2] = noVec4(ccoeffs[0]*Lcoeffs[8][0], ccoeffs[0]*Lcoeffs[4][0], ccoeffs[2]*Lcoeffs[7][0], ccoeffs[1]*Lcoeffs[3][0]);
	redMatrix[3] = noVec4(ccoeffs[1]*Lcoeffs[3][0], ccoeffs[1]*Lcoeffs[1][0], ccoeffs[1]*Lcoeffs[2][0], ccoeffs[3]*Lcoeffs[0][0] - ccoeffs[4]*Lcoeffs[6][0]);
	redMatrix.TransposeSelf();
	greenMatrix[0] = noVec4(ccoeffs[0]*Lcoeffs[8][1], ccoeffs[0]*Lcoeffs[4][1], ccoeffs[0]*Lcoeffs[7][1], ccoeffs[1]*Lcoeffs[3][1]);
	greenMatrix[1] = noVec4(ccoeffs[0]*Lcoeffs[4][1], -ccoeffs[0]*Lcoeffs[7][1], ccoeffs[0]*Lcoeffs[5][1], ccoeffs[1]*Lcoeffs[1][1]);
	greenMatrix[2] = noVec4(ccoeffs[0]*Lcoeffs[8][1], ccoeffs[0]*Lcoeffs[4][1], ccoeffs[2]*Lcoeffs[7][1], ccoeffs[1]*Lcoeffs[3][1]);
	greenMatrix[3] = noVec4(ccoeffs[1]*Lcoeffs[3][1], ccoeffs[1]*Lcoeffs[1][1], ccoeffs[1]*Lcoeffs[2][1], ccoeffs[3]*Lcoeffs[0][1] - ccoeffs[4]*Lcoeffs[6][1]);
	greenMatrix.TransposeSelf();
	blueMatrix[0] = noVec4(ccoeffs[0]*Lcoeffs[8][2], ccoeffs[0]*Lcoeffs[4][2], ccoeffs[0]*Lcoeffs[7][2], ccoeffs[1]*Lcoeffs[3][2]);
	blueMatrix[1] = noVec4(ccoeffs[0]*Lcoeffs[4][2], -ccoeffs[0]*Lcoeffs[7][2], ccoeffs[0]*Lcoeffs[5][2], ccoeffs[1]*Lcoeffs[1][2]);
	blueMatrix[2] = noVec4(ccoeffs[0]*Lcoeffs[8][2], ccoeffs[0]*Lcoeffs[4][2], ccoeffs[2]*Lcoeffs[7][2], ccoeffs[1]*Lcoeffs[3][2]);
	blueMatrix[3] = noVec4(ccoeffs[1]*Lcoeffs[3][2], ccoeffs[1]*Lcoeffs[1][2], ccoeffs[1]*Lcoeffs[2][2], ccoeffs[3]*Lcoeffs[0][2] - ccoeffs[4]*Lcoeffs[6][2]);
	blueMatrix.TransposeSelf();
}

//void SHLight::bindToShader( Shader*	vertexShader ) const
//{
//	//1: pass the red/green/blue matrices for n^M^n lighting:
//	if (vertexShader) {
//#if (USEPRT == 0)
//		const float* theredmatrix = GetRedMatrix();
//		GLint redMloc = vertexShader->GetUniformLocation("redM");
//		glUniformMatrix4fv(redMloc, 1, GL_TRUE, theredmatrix);
//		const float* thegreenmatrix = GetGrnMatrix();
//		GLint greenMloc = vertexShader->GetUniformLocation("greenM");
//		glUniformMatrix4fv(greenMloc, 1, GL_TRUE, thegreenmatrix);
//		const float* thebluematrix = GetBluMatrix();
//		GLint blueMloc = vertexShader->GetUniformLocation("blueM");
//		glUniformMatrix4fv(blueMloc, 1, GL_TRUE, thebluematrix);
//#endif
//		GLuint Liloc = vertexShader->GetUniformLocation( "Li" );
//		if (!Shader::IsValidLoc(Liloc)) {
//			static bool first = true;
//			if (first) {
//				printf("No uniform variable 'Li' in shader.\n");
//				first = false;
//			}
//		}
//		else {
//			int numcoeffs = 3*m_bands;
//#if USESOA
//			float* soa = (float*)_alloca( sizeof(float)*numcoeffs );
//			ToSoA( soa, m_Lcoeffs, 3, m_bands );
//#if (USES0A == 1)
//			vertexShader->SetUniformVec3Array( "Li", soa, numcoeffs );
//#else
//			int numsoa2 = 3*(4*3);		//1,4,4 x 3
//			float* soa2 = (float*)_alloca( sizeof(float)*numsoa2 );
//
//			ToSoa2( soa2, soa );
//
//			vertexShader->SetUniformVec4Array( "Li", soa2, numsoa2 );
//#endif
//#else //USESOA == 0
//			vertexShader->SetUniformVec3Array( "Li", m_Lcoeffs, numcoeffs );
//#endif
//		}
//	}
//}

SHLight* LoadAndProject( const char* filename, int bands )
{
	SHLight* shlight = 0;

	FILE* source = fopen(filename, "rb");	//grace_probe galileo_probe
	PfmReader* readpfm = new PfmReader();
	int width = readpfm->ReadHeader(source); //read in rgb data
	int height = readpfm->getHeight();

	if (width != 0) {
		float* envpixels = readpfm->ReadRgb(source); //now envpixels has the envmap data
		fclose(source);

		float* thecoeffs = (float*)_alloca( sizeof(float)*bands*3 );

		prefilterenvmap(envpixels, width, height, thecoeffs);
		for (int k = 0; k < bands*3; ++k) { //factor in the PI
			thecoeffs[k] /= noMath::PI;
		}
		shlight = new SHLight( thecoeffs, bands );

		delete [] envpixels;
		_cprintf("Environment map '%s' loaded.\n", filename);
	}
	else {	//file not found
		_cprintf("Cannot load envmap file %s.\n", filename);
	}
	return shlight;
}

void DumpLi( const float* thecoeffs, int bands )
{
	float coeffs[9][3]; //the Lcoeffs

	int counter = 0;
	for (int i = 0; i < 9; ++i) { //copy the coeffs into a 27 float array
		for (int j = 0; j < 3; ++j) {
			coeffs[i][j] = thecoeffs[counter];
			counter++;
		}
	}
	/* Output Results */

	printf("\n         Lighting Coefficients\n\n") ;
	printf("(l,m)       RED        GREEN     BLUE\n") ;

	printf("L_{0,0}   %9.6f %9.6f %9.6f\n",
		coeffs[0][0],coeffs[0][1],coeffs[0][2]) ;
	printf("L_{1,-1}  %9.6f %9.6f %9.6f\n",
		coeffs[1][0],coeffs[1][1],coeffs[1][2]) ;
	printf("L_{1,0}   %9.6f %9.6f %9.6f\n",
		coeffs[2][0],coeffs[2][1],coeffs[2][2]) ;
	printf("L_{1,1}   %9.6f %9.6f %9.6f\n",
		coeffs[3][0],coeffs[3][1],coeffs[3][2]) ;
	printf("L_{2,-2}  %9.6f %9.6f %9.6f\n",
		coeffs[4][0],coeffs[4][1],coeffs[4][2]) ;
	printf("L_{2,-1}  %9.6f %9.6f %9.6f\n",
		coeffs[5][0],coeffs[5][1],coeffs[5][2]) ;
	printf("L_{2,0}   %9.6f %9.6f %9.6f\n",
		coeffs[6][0],coeffs[6][1],coeffs[6][2]) ;
	printf("L_{2,1}   %9.6f %9.6f %9.6f\n",
		coeffs[7][0],coeffs[7][1],coeffs[7][2]) ;
	printf("L_{2,2}   %9.6f %9.6f %9.6f\n",
		coeffs[8][0],coeffs[8][1],coeffs[8][2]) ;
}
