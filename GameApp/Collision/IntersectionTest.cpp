#include "stdafx.h"
#include "GameApp/GameApp.h"
#include "lineintersect_utils.h"

void InitParallelLines();
void InitNonparallelLines();
void InitLines();

float A[6];
float B[6];
float IA[6] = {0.001f, -0.01f, 0.001f, -0.01f, 0.01f, -0.003f};
float IB[6] = {0.005f, 0.0075f, 0.002f, 0.002f, 0.005f, -0.01f};
float Intersection[] = {0.0, 0.0, 0.0};
float Vector[] = {0.0, 0.0, 0.0};
bool true_intersection = false;;
int count = 0, frame = 0;
float elevation = 0.f;
float azimuth = 0.f;
float g = 32.2;
float ftemp[6];

bool trueintersect = false;
bool temp = false;
bool infinite_lines = false;

void InitLines()
{
	A[0] = -0.5f;
	A[1] = -0.5f;
	A[2] = 0.f;
	A[3] = 0.f;
	A[4] = -0.5f;
	A[5] = 0.f;
	B[0] = 0.25f;
	B[1] = 0.f;
	B[2] = 0.f;
	B[3] = 0.5f;
	B[4] = 0.f;
	B[5] = 0.f;
}

void InitNonparallelLines()
{
	IA[0] = 0.001f;
	IA[1] = -0.01f;
	IA[2] = 0.001f;
	IA[3] = -0.01f;
	IA[4] = 0.01f;
	IA[5] = -0.003f;
	IB[0] = 0.005f;
	IB[1] = 0.0075f;
	IB[2] = 0.002f;
	IB[3] = 0.002f;
	IB[4] = 0.005f;
	IB[5] = -0.01f;
}