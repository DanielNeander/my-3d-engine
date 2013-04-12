
/* * * * * * * * * * * * * Author's note * * * * * * * * * * * *\
*   _       _   _       _   _       _   _       _     _ _ _ _   *
*  |_|     |_| |_|     |_| |_|_   _|_| |_|     |_|  _|_|_|_|_|  *
*  |_|_ _ _|_| |_|     |_| |_|_|_|_|_| |_|     |_| |_|_ _ _     *
*  |_|_|_|_|_| |_|     |_| |_| |_| |_| |_|     |_|   |_|_|_|_   *
*  |_|     |_| |_|_ _ _|_| |_|     |_| |_|_ _ _|_|  _ _ _ _|_|  *
*  |_|     |_|   |_|_|_|   |_|     |_|   |_|_|_|   |_|_|_|_|    *
*                                                               *
*                     http://www.humus.name                     *
*                                                                *
* This file is a part of the work done by Humus. You are free to   *
* use the code in any way you like, modified, unmodified or copied   *
* into your own work. However, I expect you to respect these points:  *
*  - If you use this file and its contents unmodified, or use a major *
*    part of this file, please credit the author and leave this note. *
*  - For use in anything commercial, please request my approval.     *
*  - Share your work and ideas too as much as you can.             *
*                                                                *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "MathTools.h"
#include <stdlib.h>

bool throwDarts(vec2 *samples, const int firstSample, const int nSamples, const float minDistSq, const int maxFailedTries){
	for (int i = firstSample; i < nSamples; i++){
		vec2 sample;
		bool failed;
		int nFailed = 0;
		do {
			do {
				sample = vec2(2 * float(rand()) / RAND_MAX - 1, 2 * float(rand()) / RAND_MAX - 1);
			} while (dot(sample, sample) > 1);

			failed = false;
			for (int k = 0; k < i; k++){
				vec2 d = samples[k] - sample;
				if (dot(d, d) < minDistSq){
					failed = true;

					nFailed++;
					if (nFailed >= maxFailedTries) return false;
					break;
				}
			}

		} while (failed);

		samples[i] = sample;
	}

	return true;
}

bool generatePoissonSamples(vec2 *samples, const int nSamples, const float minDist, const int maxFailedTries, const int nRetries, const bool includeCenter){
	if (includeCenter){
		samples[0] = vec2(0, 0);
	}

	for (int t = 0; t < nRetries; t++){
		if (throwDarts(samples, int(includeCenter), nSamples, minDist * minDist, maxFailedTries)) return true;
	}

	return false;
}

const mat4 ToMat4( const XMMATRIX& mat )
{
	mat4 dest;
	for (int i = 0; i < 4; ++i)
		dest.rows[i] = ToVec4(XMFLOAT4(mat.m[i][0], mat.m[i][1], mat.m[i][2], mat.m[i][3]));

	return dest;
}

const mat4 ToMat4( const noMat4& mat )
{
	mat4 dest;
	for (int i = 0; i < 4; ++i)
		dest.rows[i] = ToVec4(XMFLOAT4(mat.mat[i][0], mat.mat[i][1], mat.mat[i][2], mat.mat[i][3]));

	return dest;
}

const mat4 ToMat4( const Matrix& mat )
{
	mat4 dest;
	for (int i = 0; i < 4; ++i)
		dest.rows[i] = ToVec4(XMFLOAT4(mat.m[i][0], mat.m[i][1], mat.m[i][2], mat.m[i][3]));

	return dest;
}



const float2 ToVec2( const XMFLOAT2& src )
{
	float2 dest;
	dest.x = src.x;
	dest.y = src.y;	
	return dest;
}

const float3 ToVec3( const XMFLOAT3& src )
{
	float3 dest;
	dest.x = src.x;
	dest.y = src.y;
	dest.z = src.z;	
	return dest;
}

const float4 ToVec4( const XMFLOAT4& src )
{
	float4 dest;
	dest.x = src.x;
	dest.y = src.y;
	dest.z = src.z;
	dest.w = src.w;
	return dest;
}

void ComputeTangentVectors(const noVec3 &v0, const noVec3 &v1, const noVec3 &v2, const noVec2 &t0, const noVec2 &t1, const noVec2 &t2, noVec3 &sdir, noVec3 &tdir, noVec3 &normal){
	noVec3 dv0 = v1 - v0;
	noVec3 dv1 = v2 - v0;

	noVec2 dt0 = t1 - t0;
	noVec2 dt1 = t2 - t0;

	float r = 1.0f / (dt0.x * dt1.y - dt1.x * dt0.y);
	sdir = noVec3(dt1.y * dv0.x - dt0.y * dv1.x, dt1.y * dv0.y - dt0.y * dv1.y, dt1.y * dv0.z - dt0.y * dv1.z) * r;
	tdir = noVec3(dt0.x * dv1.x - dt1.x * dv0.x, dt0.x * dv1.y - dt1.x * dv0.y, dt0.x * dv1.z - dt1.x * dv0.z) * r;
	normal =dv0.Cross(dv1);
	normal.Normalize();
}