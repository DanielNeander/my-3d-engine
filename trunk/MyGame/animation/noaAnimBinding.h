#ifndef NO_ANIMBINDING_H
#define NO_ANIMBINDING_H

class noaAnimBinding 
{
public:
	noaAnimBinding();

	class NOAnimation* m_anim;

	hkInt16* m_transformTrackToBoneIndices;

	hkInt32 m_numTransformTrackToBoneIndices;

	hkInt16* m_floatTrackToFloatSlotIndices;
	hkInt32 m_numfloatTrackToFloatSlotIndices;

	enum BlendHint 
	{
		NORMAL = 0,
		ADDITIVE = 1
	};

	/// The blend hint.
	hkEnum<BlendHint, hkInt8> m_blendHint; //+default(0/*hkaAnimationBinding::NORMAL*/)



};


#endif