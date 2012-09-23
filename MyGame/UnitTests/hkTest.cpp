#include "hkTest.h"
#include <windows.h>
#include <Core/Types.h>
#include <Core/U2WinFile.h>
#include <Renderer/ModelPSK.h>
#include <Renderer/Model_M2.h>
#include <Framework/M2Loader.h>
#include <Renderer/noMesh.h>

void hkTest::loadAsset()
{
	const char* skinFile = "Resources/Animation/HavokGirl/hkLowResSkin18Bones.hkx";

	BYTE*		Buffer;
	BYTE*		BufferEnd;
	// Fill with data from buffer - contains the full .PSK file. 	
	FSkeletalMeshBinaryImport SkelMeshImporter;

	U2WinFile f;
	if (f.Open("J:\\Unreal3\\Terra_HumanF\\Human_F_H\\SkeletalMesh3\\Human_F_H00_Body_Skel.psk", "r"));
	{
		Buffer = new BYTE[f.GetSize()];		
		f.Read(Buffer, f.GetSize());		
		SkelMeshImporter.ImportFromFile( (BYTE*)Buffer, Buffer + f.GetSize() - 1 );
	}
	
	PSABinaryImport* psa = new PSABinaryImport;
	psa->ImportFromFile(TEXT("J:\\Unreal3\\Terra_HumanF\\Human_F_ANI\\AnimSet\\Human_F_Share_Anim.psa"));
	delete psa;

	{	
		M2Loader* loader = new M2Loader; 
		loader->Load("H:\\mpqediten32\\Work\\Character\\BloodElf\\Female\\BloodElfFemale.M2");
		delete loader;
	}
	// pseudo code 
	//noLoader loader;
	//hkString assetFile;
	//noAssetConverter conv;
	//rootlevelContainer container = loader->load( assetFile );
	//noScene scene = scene;
	//conv.Convert(scene);
	//{	
	//	animContainer ac = loader->load( assetFile ).find( ac );
	//	noSkeleton * skeleton = ac.m_skeletons[0];
	//}

	//{	
	//	animContainer ac = loader->load( assetFile ).find( ac );
	//	noAnimation* anim = ac.m_animations[0];
	//	noAnimBinding* m_binding = ac.m_bindings[0];
	//}

	//{
	//	rootlevelContainer container = loader->load( assetFile );
	//	noScene scene = scene;
	//	animContainer ac = loader->load( assetFile ).find( ac );
	//	int32 m_numSkinBindings = ac->m_numSkins;
	//	noMeshBinding** m_skinBindings = ac->m_skins;
	//	conv.Convert(scene);

	//	for (int ms=0; ms < m_numSkinBindings; ++ms)
	//	{
	//		noMeshBinding* skinBinding = m_skinBindings[ms];
	//		if ( m_env->m_sceneConverter->setupHardwareSkin( m_env->m_window->getContext(), skinBinding->m_mesh,
	//			reinterpret_cast<hkgAssetConverter::IndexMapping*>( skinBinding->m_mappings ),
	//			skinBinding->m_numMappings, (hkInt16)skinBinding->m_skeleton->m_numBones ) )
	//		{
	//			// based on the hkxMesh will find the hkg object and set the shader in the material(s)
	//			//setShader( skinBinding->m_mesh, platformShader );
	//		}
	//	}

	//	// Create the skeleton
	//	NOAnimatedSkeleton* m_skeletonInstance = new hkaAnimatedSkeleton( skeleton );
	//	{
	//		hkaDefaultAnimationControl* ac = new hkaDefaultAnimationControl( m_binding );

	//		ac->setPlaybackSpeed(1.0f);
	//		m_skeletonInstance->addAnimationControl( ac );
	//		ac->removeReference();
	//	}

	//	UPDATE 
	//	{
	//		m_skeletonInstance->StepDeltaTime( .016f );
	//		noPose pose (m_skeleton);
	//		m_skeletonInstance->sampleAndCombineAnimations( pose.accessUnsyncedPoseLocalSpace().begin(), pose.getFloatSlotValues().begin()  );

	//		// Grab the pose in model space
	//		const hkArray<hkQsTransform>& poseModelSpace = pose.getSyncedPoseModelSpace();

	//		const int numSkins = m_numSkinBindings;
	//		for (int i=0; i < numSkins; i++)
	//		{
	//			const hkxMesh* inputMesh = m_skinBindings[i]->m_mesh;
	//			int numBones = m_skinBindings[i]->m_numBoneFromSkinMeshTransforms;

	//			// Compute the skinning matrix palette
	//			hkLocalArray<hkTransform> compositeWorldInverse( numBones );
	//			compositeWorldInverse.setSize( numBones );

	//			// Multiply through by the bind pose inverse world inverse matrices, according to the skel to mesh bone mapping
	//			for (int p=0; p < numBones; p++)
	//			{
	//				compositeWorldInverse[p].setMul( poseModelSpace[ p ], m_skinBindings[i]->m_boneFromSkinMeshTransforms[ p ] );
	//			}

	//			// reflect the matrices in the palette used by the shader
	//			m_env->m_sceneConverter->updateSkin( inputMesh, compositeWorldInverse, hkTransform::getIdentity() );
	//		}
	//	}

	//}


	{	
		/*noMesh mesh;
		mesh.CreatevertexBuffer();
		mesh.CreateIndexBuffer();

		hkString fname;
		M2Loader loader;
		loader.Load(fname);*/
	}
}
