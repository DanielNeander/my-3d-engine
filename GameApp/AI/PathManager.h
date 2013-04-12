#pragma once 

class PathNode 
{
public:
	uint32    m_pauseChance;
	noVec3    m_position;

	PathNode()
	{
		m_pauseChance = 0;
	}

	PathNode(const noVec3 &position, uint32 pauseChance=0)
	{
		m_position = position;
		m_pauseChance = pauseChance;
	}

	void SetPauseChance(uint32 chance)        { m_pauseChance = chance; }
	uint32 GetPauseChance(uint32 chance)    { return m_pauseChance; }

	noVec3 GetPosition()                      { return m_position; }

protected:
};

class Path : public std::vector<PathNode> 
{
public:
	Path(const std::string &name);
	~Path();

	std::string GetName() { return m_name; }

	void SetHeight(float height);

	void SetPauseChance(uint32 index, uint32 chance);

protected:
	std::string               m_name;

};

// --------------------------------------------------------------------
// Manages all the paths in the world
// --------------------------------------------------------------------
class PathManager
{
public:


	Path *GetPath(const std::string &name);
	Path *GetPath(uint32 index);

	uint32 GetPathCount() ;

	void AddPaths();

	static PathManager *Create();
	static void Destroy();

	static PathManager *Get();

	// will grab the name from this entity and
	// create a path from entities with that name pattern.
	// In the process, these entities are removed from the system.	
	void AddPath(Path *pkPath);

protected:
	static PathManager *m_pkPathManager;

	PathManager();
	~PathManager();

	Path *AddPath(const std::string &prefix);

private:
	std::vector<Path*>          m_paths;
};
