#include "ResourceManager.h"
#include <queue>

void BaseResource::Clear()
{
	priority_ = RES_MED_PRIORITY;
	last_access_ = 0;
}

bool BaseResource::operator<( BaseResource& container )
{
	if(GetPriority() < container.GetPriority())
		return true;
	else if(GetPriority() > container.GetPriority())
		return false;
	else 
	{
		if(last_access_ < container.GetLastAccess())
			return true;
		else if(last_access_ > container.GetLastAccess())
			return false;
		else 
		{
			if(GetSize() < container.GetSize())
				return true;
			else 
				return false;
		}
	}
	return false;
}



void ResManager::Clear()
{
	resource_map_.clear();
	next_resource_id_ = INVALID_RESID;
	current_used_memory_ = 0;
	maximum_memory_ = 0;
	resource_reserved_ = false;
	current_resource_ = resource_map_.end();
}

bool ResManager::Create( UINT nMaxSize )
{
	Clear();
	SetMaximumMemory(nMaxSize);
	return true;
}

void ResManager::Destory()
{
	for (ResMapItor itor = resource_map_.begin(); itor != resource_map_.end(); ++itor)
	{
		if(!((*itor).second)->IsLocked())
		{
			delete ((*itor).second);
		}
	}
	resource_map_.clear();
	Clear();
}

bool ResManager::SetMaximumMemory( size_t nMem )
{
	maximum_memory_ = nMem;
	return CheckForOverallocation();
}

bool ResManager::ReserveMemory( size_t nMem )
{
	AddMemory(nMem);
	if(!CheckForOverallocation())
		return false;
	resource_reserved_ = true;
	return true;
}

bool ResManager::InsertResource( ResourceId* unique_id, BaseResource * pRes )
{
	*unique_id = GetNextResourceId();

	resource_map_.insert(ResMapPair(*unique_id, pRes));

	if (!resource_reserved_)
	{
		AddMemory(pRes->GetSize());
		if(CheckForOverallocation())
			return false;
	}
	else 
		resource_reserved_ = false;
	return true;
}

bool ResManager::InsertResource( ResourceId unique_id, BaseResource* pRes )
{
	ResMapItor itor = resource_map_.find(unique_id);
	if(itor != resource_map_.end())
		return false;

	resource_map_.insert(ResMapPair(unique_id, pRes));

	if(!resource_reserved_)
	{
		AddMemory(pRes->GetSize());
		if(CheckForOverallocation())
			return false;
	}
	else 
		resource_reserved_ = false;
	return true;
}

bool ResManager::RemoveResource( BaseResource* pRes )
{
	ResMapItor itor;

	for(itor = resource_map_.begin(); itor != resource_map_.end(); ++itor)
	{
		if(itor->second == pRes)
			break;
	}
	if(itor == resource_map_.end())
		return false;

	if(itor->second->IsLocked())
		return false;
	RemoveMemory(pRes->GetSize());
	resource_map_.erase(itor);
}

bool ResManager::RemoveResource( ResourceId unique_id )
{
	ResMapItor itor = resource_map_.find(unique_id);
	if(itor == resource_map_.end())
		return false;
	if(((*itor).second)->IsLocked())
		return false;
	RemoveMemory(((*itor).second)->GetSize());
	resource_map_.erase(itor);

	return true;
}

BaseResource* ResManager::GetResource( ResourceId unique_id )
{
	ResMapItor itor = resource_map_.find(unique_id);

	if(itor == resource_map_.end())
		return NULL;

	itor->second->SetLastAccess(time(0));

	if(itor->second->IsDisposed())
	{
		itor->second->Recreate();
		AddMemory(itor->second->GetSize());

		Lock(unique_id);
		CheckForOverallocation();
		Unlock(unique_id);
	}
	return itor->second;
}

BaseResource* ResManager::Lock( ResourceId unique_id )
{
	ResMapItor itor = resource_map_.find(unique_id);
	if (itor == resource_map_.end())
		return NULL;

	itor->second->AddRef();

	if(itor->second->IsDisposed())
	{
		itor->second->Recreate();
		AddMemory(itor->second->GetSize());
		CheckForOverallocation();
	}

	return itor->second;	
}

int ResManager::Unlock( ResourceId unique_id )
{
	ResMapItor itor = resource_map_.find(unique_id);
	if(itor == resource_map_.end())
		return -1;

	if(itor->second->GetRefCount() > 0)
		itor->second->Release();	// Decrement ref count	
	return itor->second->GetRefCount();
}

int ResManager::Unlock( BaseResource* pRes )
{
	ResourceId resid = FindResourceId(pRes);
	if (IS_INVALID_RESID(resid))
		return -1;
	return Unlock(resid);
}

ResourceId ResManager::FindResourceId( BaseResource* pRes )
{
	ResMapItor itor;
	for(itor = resource_map_.begin(); itor != resource_map_.end(); ++itor)
	{
		if(itor->second == pRes)
			break;
	}
	if(itor == resource_map_.end())
		return INVALID_RESID;
	return itor->first;
}

bool ResManager::CheckForOverallocation()
{
	if (current_used_memory_ > maximum_memory_)
	{
		int iMemToPurge = current_used_memory_ - maximum_memory_;

		std::priority_queue<BaseResource*, std::vector<BaseResource*>, ptr_greater<BaseResource*> > ResPriQueue;

		for(ResMapItor itor = resource_map_.begin(); itor != resource_map_.end(); ++itor)
		{
			if(!itor->second->IsDisposed() && !itor->second->IsLocked())
				ResPriQueue.push(itor->second);
		}

		while((!ResPriQueue.empty()) && (current_used_memory_ > maximum_memory_))
		{
			UINT nDisposalSize = ResPriQueue.top()->GetSize();
			ResPriQueue.top()->Dispose();
			if(ResPriQueue.top()->IsDisposed())
				RemoveMemory(nDisposalSize);
			ResPriQueue.pop();
		}

		if(ResPriQueue.empty() && (current_used_memory_ > maximum_memory_))
			return false;
	}
	return true;
}

bool ResManager::DestoryResource( BaseResource* pRes )
{
	if(!RemoveResource(pRes))
		return false;
	delete pRes;
	return true;
}

bool ResManager::DestoryResource( ResourceId unique_id )
{
	BaseResource* pRes = GetResource(unique_id);
	if(!RemoveResource(unique_id))
		return false;
	delete pRes;
	return true;
}
