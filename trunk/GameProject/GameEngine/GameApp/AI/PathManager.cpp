#include "stdafx.h"
#include "PathManager.h"


Path::Path( const std::string &name )
{
	m_name = name;
}

Path::~Path()
{

}

void Path::SetHeight( float height )
{	
	for (size_t i=0; i < this->size(); ++i)
	{
		PathNode &pathNode = this->at(i);
		pathNode.m_position.y = height;
	}
}

void Path::SetPauseChance( uint32 index, uint32 chance )
{
	if (index >= this->size())
		return;

	PathNode	&pathNode = this->at(index);
	pathNode.SetPauseChance(chance);
}

Path * PathManager::GetPath( const std::string &name )
{
	size_t count = m_paths.size();
	for(size_t i=0; i < count; ++i)
	{
		if(m_paths.at(i)->GetName() == name)
			return m_paths.at(i);
	}
	return NULL;
}

Path * PathManager::GetPath( uint32 index )
{
	return m_paths.at(index);
}

uint32 PathManager::GetPathCount()
{
	return m_paths.size();
}

void PathManager::AddPaths()
{

}

PathManager * PathManager::Create()
{
	m_pkPathManager = new PathManager;
	return m_pkPathManager;
}

void PathManager::Destroy()
{
	if( m_pkPathManager )
		delete m_pkPathManager;
	m_pkPathManager = NULL;
}

PathManager * PathManager::Get()
{
	return m_pkPathManager;
}

void PathManager::AddPath( Path *pkPath )
{
	m_paths.push_back( pkPath );
}

Path * PathManager::AddPath( const std::string &prefix )
{
	return NULL;
}

PathManager::PathManager()
{

}

PathManager::~PathManager()
{
	size_t count = GetPathCount();
	
	for(size_t i=0; i < count; ++i)
	{
		delete m_paths.at(i);
	}

	m_paths.clear();
	m_pkPathManager = NULL;
}

PathManager * PathManager::m_pkPathManager;
