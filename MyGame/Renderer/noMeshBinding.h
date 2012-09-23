#ifndef NO_MESHBINDING_H
#define NO_MESHBINDING_H

class noMeshBinding {

public:
	struct Mapping
	{
		//HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_ANIMATION, hkaMeshBinding::Mapping );
		//HK_DECLARE_REFLECTION();

		/// Mapping indices
		hkInt16* m_mapping;
		/// Size of m_mapping array.
		hkInt32 m_numMapping;
	};

	class noMesh* mesh;

	class noSkeleton* m_skeleton;

	struct Mapping* m_mappings;
	/// Size of m_mappings array.
	hkInt32 m_numMappings;

	hkTransform* m_boneFromSkinMeshTransforms;
	/// Size of m_boneFromSkinMeshTransforms array.
	hkInt32 m_numBoneFromSkinMeshTransforms;

};


#endif