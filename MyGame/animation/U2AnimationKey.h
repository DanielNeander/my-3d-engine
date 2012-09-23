#ifndef	U2_ANIMATIONKEY_H
#define	U2_ANIMATIONKEY_H


#include <d3dx9.h>

class Key 
{
public:
	enum KeyType
	{
		NOINTERP,
		LINKEY,
		BEZKEY,
		TCBKEY,
		EULERKEY,
		STEPKEY,
		NUMKEYTYPES
	};

protected:
	Key()	{}
public:
	float	startTime, endTime;	
};

class ScaleKey : public Key
{
public:
	float				*pScaleKey;		
};

class PosKey	: public Key
{
public:
	D3DXVECTOR3		*pPosKey;		
};

class RotKey	: public Key
{	
	D3DXQUATERNION	*pRotKey;
}


#endif