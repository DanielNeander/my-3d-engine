#include <U2_3D/Src/U23DLibPCH.h>
#include "U2PoolAlloc.h"



template class U2_3D U2PoolAlloc<size_t>;

U2AllocDeclareStatics(size_t, 2048);

//template<> U2PoolAlloc<size_t>::Block* U2PoolAlloc<size_t>::ms_pBlockHeader = NULL;
//template<> U2PoolAlloc<size_t>::Block* U2PoolAlloc<size_t>::ms_pMemPool = NULL;
//template<> uint32 U2PoolAlloc<size_t>::ms_uNumSize = 1024;


