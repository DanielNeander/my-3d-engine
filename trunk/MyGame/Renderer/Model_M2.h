#ifndef MODEL_M2_H
#define MODEL_M2_H


#include "Model.h"
#include "Model_M2Format.h"

class noAnimation;
class noSkeleton;
class noAnimBinding;
class noMeshBinding;
class noAnimatedSkeleton;
class noLoader;

class noRenderModelM2 : public noRenderModel {
public:

	noAnimation* m_anim;
	noSkeleton* m_skeleton;
	noAnimBinding* m_binding;
	noMeshBinding** m_skinBindings;
	int32	m_numSkinBindings;

	noAnimatedSkeleton*	m_skeletonInst;
	noLoader*	m_loader;		

	
	//Bone* bones;			

	

};

#endif