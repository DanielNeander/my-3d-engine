struct SH9Color
{
	float3 c[9];
};

//-------------------------------------------------------------------------------------------------
// Projects a direction onto SH and convolves with a given kernel
//-------------------------------------------------------------------------------------------------
void ProjectOntoSH9(in float3 n, in float3 color, in float A0,
					in float A1, in float A2, out SH9Color sh)
{
    // Band 0
    sh.c[0] = 0.282095f * A0 * color;

    // Band 1
    sh.c[1] = 0.488603f * n.y * A1 * color;
    sh.c[2] = 0.488603f * n.z * A1 * color;
    sh.c[3] = 0.488603f * n.x * A1 * color;

    // Band 2
    sh.c[4] = 1.092548f * n.x * n.y * A2 * color;
    sh.c[5] = 1.092548f * n.y * n.z * A2 * color;
    sh.c[6] = 0.315392f * (3.0f * n.z * n.z - 1.0f) * A2 * color;
    sh.c[7] = 1.092548f * n.x * n.z * A2 * color;
    sh.c[8] = 0.546274f * (n.x * n.x - n.y * n.y) * A2 * color;
}

//-------------------------------------------------------------------------------------------------
// Projects a direction onto SH and convolves with a given kernel
//-------------------------------------------------------------------------------------------------
void ProjectOntoSH9(in float3 n, in float3 color, out SH9Color sh)
{
	ProjectOntoSH9(n, color, 1.0f, 1.0f, 1.0f, sh);
}

//-------------------------------------------------------------------------------------------------
// Projects a direction onto SH and convolves with a cosine kernel to compute irradiance
//-------------------------------------------------------------------------------------------------
void ProjectOntoSH9Cosine(in float3 n, in float3 color, out SH9Color sh)
{
	// Cosine kernel
	const float A0 = 3.141593f;
    const float A1 = 2.095395f;
    const float A2 = 0.785398f;

	ProjectOntoSH9(n, color, A0, A1, A2, sh);
}

//-------------------------------------------------------------------------------------------------
// Computes the dot project of two SH9 RGB vectors
//-------------------------------------------------------------------------------------------------
float3 SH9DotProduct(in SH9Color a, in SH9Color b)
{
	float3 result = 0.0f;

	[unroll]
	for(uint i = 0; i < 9; ++i)
		result += a.c[i] * b.c[i];

	return result;
}

//-------------------------------------------------------------------------------------------------
// Projects a direction onto SH9, convolves with a cosine kernel, and dots it with another
// SH9 vector
//-------------------------------------------------------------------------------------------------
float3 EvalSH9Cosine(in float3 dir, in SH9Color sh)
{
	SH9Color dirsh;

	// Negate y to compensate for D3DX cubemap projection
	ProjectOntoSH9Cosine(dir * float3(1, -1, 1), float3(1.0f, 1.0f, 1.0f), dirsh);
	return SH9DotProduct(dirsh, sh);
}

//-------------------------------------------------------------------------------------------------
// Converts from 3-band SH to 2-band H-Basis. See "Efficient Irradiance Normal Mapping" by
// Ralf Habel and Michael Wimmer for the derivations.
//-------------------------------------------------------------------------------------------------
void ConvertToHBasis(in SH9Color sh, out float3 hBasis[4])
{
	const float rt2 = sqrt(2.0f);
	const float rt32 = sqrt(3.0f / 2.0f);
	const float rt52 = sqrt(5.0f / 2.0f);
	const float rt152 = sqrt(15.0f / 2.0f);
	const float convMatrix[4][9] =
	{
		{ 1.0f / rt2, 0, 0.5f * rt32, 0, 0, 0, 0, 0, 0 },
		{ 0, 1.0f / rt2, 0, 0, 0, (3.0f / 8.0f) * rt52, 0, 0, 0 },
		{ 0, 0, 1.0f / (2.0f * rt2), 0, 0, 0, 0.25f * rt152, 0, 0 },
		{ 0, 0, 0, 1.0f / rt2, 0, 0, 0, (3.0f / 8.0f) * rt52, 0 }
	};

	[unroll(4)]
	for(uint row = 0; row < 4; ++row)
	{
		hBasis[row] = 0.0f;

		[unroll(9)]
		for(uint col = 0; col < 9; ++col)
			hBasis[row] += convMatrix[row][col] * sh.c[col];
	}
}

//-------------------------------------------------------------------------------------------------
// Evalutes the H-Basis coefficients in the given direction
//-------------------------------------------------------------------------------------------------
float3 EvalHBasis(in float3 n, in float3 H0, in float3 H1, in float3 H2, in float3 H3)
{
	float3 color = 0.0f;

    // Band 0
    color += H0 * (1.0f / sqrt(2.0f * 3.14159f));

    // Band 1
    color += H1 * -sqrt(1.5f / 3.14159f) * n.y;
    color += H2 * sqrt(1.5f / 3.14159f) * (2 * n.z - 1.0f);
    color += H3 * -sqrt(1.5f / 3.14159f) * n.x;

	return color;
}

//-------------------------------------------------------------------------------------------------
// Evalutes 2-band SH in the given direction
//-------------------------------------------------------------------------------------------------
float3 EvalSH4(in float3 n, in float3 SH0, in float3 SH1, in float3 SH2,
				in float3 SH3, in float3 A0, in float3 A1)
{
	float3 color = 0.0f;

	// Band 0
    color += 0.282095f * SH0 * A0;

    // Band 1
    color += 0.488603f * n.y * SH1 * A1;
    color += 0.488603f * n.z * SH2 * A1;
    color += 0.488603f * n.x * SH3 * A1;

	return color;
}

//-------------------------------------------------------------------------------------------------
// Evalutes 2-band SH in the given direction
//-------------------------------------------------------------------------------------------------
float3 EvalSH4(in float3 n, in float3 SH0, in float3 SH1, in float3 SH2, in float3 SH3)
{
	return EvalSH4(n, SH0, SH1, SH2, SH3, 1.0f, 1.0f);
}

//-------------------------------------------------------------------------------------------------
// Evalutes 2-band SH in the given direction, and convolves with the cosine kernel
//-------------------------------------------------------------------------------------------------
float3 EvalSH4Cosine(in float3 n, in float3 SH0, in float3 SH1, in float3 SH2, in float3 SH3)
{
	// Cosine kernel
	const float A0 = 3.141593f;
    const float A1 = 2.095395f;

	return EvalSH4(n, SH0, SH1, SH2, SH3, A0, A1);
}
