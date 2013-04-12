#pragma once 

#include <iostream>
#include <fstream>
#include<math.h>

#define mapsize 64
float heightmap[mapsize][mapsize];
inline void savefractal(int x, int y)
{
	fstream file;
	file.open("test2.raw",ios::out);
	for(int a=0;a<y;a++)
		for(int b=0;b<x;b++)
		{
			file<<(heightmap[a][b])<<" ";
		}
		file<<endl;
}
inline double cosineinterpolation(double number1,double number2,double x)
{
	double ft;
	double f;
	double ret;
	ft = x * 3.1415927;
	f = (1 - cos(ft)) * .5;
	ret=number1*(1-f) + number2*f;
	return ret;
}

inline double randomnumber(int x,int y)
{
	int n=x+y*57;
	n=(n<<13)^n;
	double ret;
	ret= (1 - ( (n * (n * n * 15731 + 789221) + 1376312589) & 2147483647) / 1073741824.0);
	return ret;    
}

inline float smoothrandom(int x,int y)
{
	float corners=(randomnumber(x-1,y-1)+randomnumber(x+1,y-1)+randomnumber(x-1,y+1)+randomnumber(x+1,y+1))/16;
	float sides   = (randomnumber(x-1, y)+randomnumber(x+1, y)+randomnumber(x, y-1)+randomnumber(x, y+1) ) /  8;
	float center  =  randomnumber(x, y) / 4;   
	float ret=corners+sides+center;
	return ret;
}
inline float noise(float x, float y)
{
	int xinterger=x;
	float fractionx=x- xinterger;
	int yinteger=y;
	float fractiony=y- yinteger;
	float v1,v2,v3,v4,i1,i2;
	float ret;
	v1=smoothrandom(xinterger, yinteger);
	v2=smoothrandom(xinterger + 1, yinteger);
	v3=smoothrandom(xinterger, yinteger + 1);
	v4=smoothrandom(xinterger + 1, yinteger + 1);
	i1= cosineinterpolation (v1,v2,fractionx);
	i2= cosineinterpolation (v3,v4,fractionx);
	ret = cosineinterpolation (i1,i2,fractiony);
	return ret;
}

inline float FBM(float x, float y, float octaves, float amplitude, float frequency, float h)
{
	float ret=0;
	for(int i=0;i<(octaves-1);i++)
	{
		ret +=( noise (x* frequency, y* frequency)* amplitude);
		amplitude*=h;
	}
	return ret;	
}
