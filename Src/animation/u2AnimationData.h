#ifndef	U2_ANIMATIONDATA_H
#define	U2_ANIMATIONDATA_H


class PosKey;
class RotKey;
class ScaleKey;

class U2AnimationData 
{
public:

	unsigned int numPosKeys;
	unsigned int numRotKeys;
	unsigned int numScaleKeys;

	PosKey	*pPosKey;
	PosKey::KeyType ePosType;

	RotKey	*pRotKey;
	RotKey::KeyType eRotKey;

	ScaleKey	*pScaleKey;
	ScaleKey::KeyType eScaleType;
	
};




#endif