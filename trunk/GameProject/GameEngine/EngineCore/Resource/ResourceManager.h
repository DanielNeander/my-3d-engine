#pragma once 
#ifndef __RESOURCEMANAGER_H__
#define __RESOURCEMANAGER_H__


#include <EngineCore/RefCount.h>

// disable warnings about extra chars in debug builds for templates
#pragma warning( disable : 4786 )
// Disable warning messages about needs to have dll-interface to be used by clients	
#pragma warning( disable : 4251 )	

#include <ctime>
#include <map>
#include <stack>

#ifndef UINT
#define UINT unsigned int
#endif

// A resource handle is define as an unsigned integer
#define ResourceId UINT

#define INVALID_RESID	0XFFFFFFFF

#define IS_INVALID_RESID(id)	((id == INVALID_RESID) ? true : false)
#define IS_VALID_RESID(id)		((id == INVALID_RESID) ? false : true)


class BaseResource : public RefCounter
{
public:
	enum Priority
	{
		RES_LOW_PRIORITY = 0,
		RES_MED_PRIORITY,
		RES_HIGH_PRIORITY
	};

	BaseResource()				{	Clear(); }
	virtual	~BaseResource()		{ Destroy(); }

	virtual void Clear();


	virtual bool Create()		{ return false; }
	virtual void Destroy()		{}

	virtual bool Recreate() = 0;
	virtual void Dispose() = 0;

	virtual size_t GetSize() = 0;
	virtual bool IsDisposed() = 0;

	inline void SetPriority(Priority priority)		{ priority_ = priority; }
	inline Priority GetPriority()					{ return priority_; }

	inline bool IsLocked()							{ return (m_iRefCount > 0) ? true : false; }

	inline void SetLastAccess(time_t lastAccess)	{ last_access_ = lastAccess; }
	inline time_t GetLastAccess()					{ return last_access_; }

	virtual bool operator< (BaseResource& container);

protected:
	Priority	priority_;
	time_t		last_access_;
};

template<class T>
class ptr_less
{
public:
	inline bool operator ()(T left, T right)
	{
		return ((*left) < (*right)); 
	}
};

template<class T>
class ptr_greater
{
public:
	inline bool operator ()(T left, T right)
	{
		return !((*left) < (*right));
	}
};

typedef std::map<ResourceId, BaseResource*>	ResMap;
typedef ResMap::iterator	ResMapItor;
typedef ResMap::value_type ResMapPair;

class ResManager
{
public:
	ResManager()			{	Clear(); }
	virtual ~ResManager()	{	Destory(); }

	void Clear();

	bool Create(UINT nMaxSize);
	void Destory();

	bool SetMaximumMemory(size_t nMem);
	size_t GetMaximumMemory()		{ }

	inline void Begin()
	{
		current_resource_ = resource_map_.begin();
	}
	inline BaseResource* GetCurrentRes()
	{
		return (*current_resource_).second;
	}
	inline bool Next()
	{
		current_resource_++;	return IsValid(); 
	}
	inline bool IsValid()
	{
		return (current_resource_ != resource_map_.end()) ? true : false;
	}

	bool ReserveMemory(size_t nMem);

	bool InsertResource(ResourceId* unique_id, BaseResource * pRes);
	bool InsertResource(ResourceId unique_id, BaseResource* pRes);

	bool RemoveResource(BaseResource* pRes);
	bool RemoveResource(ResourceId unique_id);

	bool	DestoryResource(BaseResource* pRes);
	bool	DestoryResource(ResourceId unique_id);

	BaseResource* GetResource(ResourceId unique_id);
	
	BaseResource* Lock(ResourceId unique_id);

	int Unlock(ResourceId unique_id);
	int Unlock(BaseResource* pRes);

	ResourceId	FindResourceId(BaseResource* pRes);

protected:
	inline void AddMemory(size_t nMem)		{ current_used_memory_ += nMem; }
	inline void RemoveMemory(size_t nMem)	{ current_used_memory_ -= nMem; }	
	UINT	GetNextResourceId()				{ return --next_resource_id_; }

	bool CheckForOverallocation();	

protected:
	ResourceId		next_resource_id_;
	size_t			current_used_memory_;
	size_t			maximum_memory_;
	bool			resource_reserved_;
	ResMapItor		current_resource_;
	ResMap			resource_map_;
};


#endif