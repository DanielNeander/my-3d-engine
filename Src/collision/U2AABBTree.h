/************************************************************************
module	:	U2AABBTree
Author	:	Yun sangyong
Desc	:	
************************************************************************/
#pragma once
#ifndef U2_AABBTREE_H
#define U2_AABBTREE_H

#define U2_NO_NEG_TREE

class U2AABBTreeBuilder;

#ifdef U2_NO_NEG_TREE
	#define IMPLEMENT_TREE(base_class,volume)										\
		public:																		\
				base_class();														\
				~base_class();														\
																					\
				__forceinlne const volume* Get##volume()const { return &m_bv; }		\
				__forceinline const base_class* GetPos() const						\
				{ return (const base_class*)(m_pos&~1); }							\
				__forceinline const base_class* GetNeg() const						\
				{ const base_class* p = GetPos(); return p ? p + 1 : 0; }			\
				__forceinline bool IsLeaf() const { return !GetPos(); }				\
				__forceinline unsigned int GetNodeSize() const						\
				{ return sizeof(*this); }											\
		protected:																	\
			volume			m_bv;													\
			unsigned int		m_pos													
#endif

				
	typedef void		(*CullingCallback) (unsigned int numPrimitives, 
		unsigned int* pPrimitives, bool m_bNeedClipping,void* pUsrData);

	class U2_3D U2AABBTreeNode : public U2RefObject
	{
		IMPLEMENT_TREE(U2AABBTreeNode, U2AABB);

	public:
		__forceinline const unsigned int* GetPrimitives() const { return m_pPrimitives; }
		__forceinline unsigned int GetNumPrimitives() const { return m_uiNumPrimitives; }

	protected:
		unsigned int* m_pPrimitives;
		unsigned int m_uiNumPrimitives;

		unsigned int Split(unsigned int uiAxis, U2AABBTreeBuilder* pBuilder);
		bool Subdivide(U2AABBTreeBuilder* pBuilder);
		void BuildHierarchy(U2AABBTreeBuilder* pBuilder);
		void Refit(U2AABBTreeBuilder* pBuilder);
	};

	typedef U2SmartPtr<U2AABBTreeNode> U2AABBTreeNodePtr;

	typedef bool		(*WalkingCallback) (const U2AABBTreeNode* pCurrNode, 
		unsigned int uiDepth, void* pUsrData);


	class U2_3D U2AABBTree : public U2AABBTreeNode
	{
	public:
		U2AABBTree();
		virtual ~U2AABBTree();

		bool BuildTree(U2AABBTreeBuilder* pBuilder);
		void Release();

		__forceinline const unsigned int* GetIndices() const { return m_puiIndices; }
		__forceinline unsigned int GetNumNodes() const { return m_uiTotalNodes; }

		bool IsComplete() const;
		unsigned int ComputeDepth() const;
		unsigned int GetUsedBytes() const;
		unsigned int Walk(WalkingCallback callback, void* pUsrData) const;
		bool Refit(U2AABBTreeBuilder* pBuilder);
		bool Refit2(U2AABBTreeBuilder* pBuilder);

	private:
		unsigned int*	m_puiIndices;
		U2AABBTreeNode* m_pPool;
		unsigned int m_uiTotalNodes;

	};


#endif