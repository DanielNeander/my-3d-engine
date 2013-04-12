
//
// Spherical Harmonic Rotation
//
// Implementation Reference:
//    Rotation Matrices for Real Spherical Harmonics. Direct Determination by Recursion
//    Joseph Ivanic and Klaus Ruedenberg
//    J. Phys. Chem. 1996, 100, 6342-5347
//

#include "shtest/Matrix.h"
#include "prt.h"
#include <cmath>
#include <cstring>

static const int numbands = 3;

inline float delta(const int m, const int n)
{
	// Kronecker Delta
	return (m == n ? 1 : 0);
}


void uvw(const int l, const int m, const int n, float& u, float& v, float& w)
{
	// Pre-calculate simple reusable terms
	float d = delta(m, 0);
	int abs_m = abs(m);

	// Only calculate the required denominator once
	float denom;
	if (abs(n) == l)
		denom = (2 * l) * (2 * l - 1);

	else
		denom = (l + n) * (l - n);

	// Now just calculate the scalars
	u = sqrt((l + m) * (l - m) / denom);
	v = 0.5f * sqrt((1 + d) * (l + abs_m - 1) * (l + abs_m) / denom) * (1 - 2 * d);
	w = -0.5f * sqrt((l - abs_m - 1) * (l - abs_m) / denom) * (1 - d);
}


struct PermutedMatrix
{
	PermutedMatrix(const cMatrix& m) : matrix(m) { }

	static int permute(const int v)
	{
		if (v == 1) return (0);
		if (v == -1) return (1);
		if (v == 0) return (2);
		//ASSERT(false);
		return (0);
	}

	// m=row, n=column
	float operator () (const int m, const int n) const
	{
		return (matrix.e[permute(n)][permute(m)]);
	}

	const cMatrix& matrix;
};

struct SHRotateMatrix
{
	// The size of the matrix is the square of the number of bands
	static const int DIM = numbands * numbands;

	// Matrix array
	float e[DIM][DIM];

	// SH rotation  matrices are block diagonal sparse.
	// Given a band index this will offset the lookup into the block within the matrix
	// that is responsible for rotating that band and shift it by (a,b).
	float& operator () (const int l, const int a, const int b)
	{
		int centre = (l + 1) * l;
		return (e[centre + a][centre + b]);
	}
	float operator () (const int l, const int a, const int b) const
	{
		int centre = (l + 1) * l;
		return (e[centre + a][centre + b]);
	}

	void Transform(const RGBCoeff* source, RGBCoeff* dest) const
	{
		// Loop through each band
		for (int l = 0; l < numbands; l++)
		{
			for (int mo = -l; mo <= l; mo++)
			{
				RGBCoeff& rgb = dest[l * (l + 1) + mo];
				rgb.r = rgb.g = rgb.b = 0;

				for (int mi = -l; mi <= l; mi++)
					rgb += source[l * (l + 1) + mi] * (*this)(l, mo, mi);
			}

		}

		/*for (int i = 0; i < SampleSphere::NB_BASES; i++)
		{
			dest.e[i].r = 0;
			dest.e[i].g = 0;
			dest.e[i].b = 0;

			for (int j = 0; j < DIM; j++)
				dest.e[i] += source.e[j] * e[j][i];
		}*/
	}
};

float P(const int i, const int l, const int a, const int b, const PermutedMatrix& R, const SHRotateMatrix& M)
{
	if (b == -l)
		return (R(i, 1) * M(l - 1, a, -l + 1) + R(i, -1) * M(l - 1, a, l - 1));

	else if (b == l)
		return (R(i, 1) * M(l - 1, a, l - 1) - R(i, -1) * M(l - 1, a, -l + 1));

	else // |b|<l
		return (R(i, 0) * M(l - 1, a, b));
}


float U(const int l, const int m, const int n, const PermutedMatrix& R, const SHRotateMatrix& M)
{
	if (m == 0)
		return (P(0, l, 0, n, R, M));

	// For both m>0, m<0
	return (P(0, l, m, n, R, M));
}


float V(const int l, const int m, const int n, const PermutedMatrix& R, const SHRotateMatrix& M)
{
	if (m == 0)
	{
		float p0 = P(1, l, 1, n, R, M);
		float p1 = P(-1, l, -1, n, R, M);
		return (p0 + p1);
	}

	else if (m > 0)
	{
		float d = delta(m, 1);
		float p0 = P(1, l, m - 1, n, R, M);
		float p1 = P(-1, l, -m + 1, n, R, M);
		return (p0 * sqrt(1 + d) - p1 * (1 - d));
	}

	else // m < 0
	{
		float d = delta(m, -1);
		float p0 = P(1, l, m + 1, n, R, M);
		float p1 = P(-1, l, -m - 1, n, R, M);
		return (p0 * (1 - d) + p1 * sqrt(1 - d));
	}
}


float W(const int l, const int m, const int n, const PermutedMatrix& R, const SHRotateMatrix& M)
{
	if (m == 0)
	{
		// Never gets called as kd=0
		//ASSERT(false);
		return (0);
	}

	else if (m > 0)
	{
		float p0 = P(1, l, m + 1, n, R, M);
		float p1 = P(-1, l, -m - 1, n, R, M);
		return (p0 + p1);
	}

	else // m < 0
	{
		float p0 = P(1, l, m - 1, n, R, M);
		float p1 = P(-1, l, -m + 1, n, R, M);
		return (p0 - p1);
	}
}


float M(const int l, const int m, const int n, const PermutedMatrix& R, const SHRotateMatrix& M)
{
	// First get the scalars
	float u, v, w;
	uvw(l, m, n, u, v, w);

	// Scale by their functions
	if (u)
		u *= U(l, m, n, R, M);
	if (v)
		v *= V(l, m, n, R, M);
	if (w)
		w *= W(l, m, n, R, M);

	return (u + v + w);
}

void SHRotate(SHRotateMatrix& shrm, const cMatrix& rotation)
{
	// Start with already known 1x1 rotation matrix for band zero
	memset(shrm.e, 0, shrm.DIM * shrm.DIM * sizeof(float));
	shrm.e[0][0] = 1;

	// Create matrix index modifiers
	PermutedMatrix pm(rotation);

	for (int m = -1; m <= 1; m++)
		for (int n = -1; n <= 1; n++)
			shrm(1, m, n) = pm(m, n);

	// Calculate each block of the rotation matrix for each band
	for (int band = 2; band < numbands; band++)
	{
		for (int m = -band; m <= band; m++)
			for (int n = -band; n <= band; n++)
				shrm(band, m, n) = M(band, m, n, pm, shrm);
	}
}