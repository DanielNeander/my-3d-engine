#ifndef _MAPGRIDNODE_H_
#define _MAPGRIDNODE_H_

#include "CellSpacePartition.h"

class MapGridNode
{
public:
	// constructors
	MapGridNode() { m_cost = m_x = m_y = 0; m_parent = NULL; m_visited = false; }
	MapGridNode(const int x, const int y, MapGridNode *parent, const bool visited, const int cost) { m_x = x; m_y = y; m_parent = parent; m_visited = visited; m_cost = cost; }
	MapGridNode(const MapGridNode &copy);

	// destructor
	virtual ~MapGridNode() { m_parent = NULL; }

	virtual MapGridNode &operator=(const MapGridNode &rhs);
	virtual bool operator==(const MapGridNode &rhs);
	virtual bool operator<(const MapGridNode &rhs);
	virtual bool operator>(const MapGridNode &rhs);

	// accessors
	void setParent(MapGridNode* parent) { m_parent = parent;}
	void setVisited(const bool visited) { m_visited = visited; }
	bool getVisited() const { return m_visited; }
	virtual void setCost(const int cost);
	virtual int getCost() const;

	// helpers
	bool equals(const MapGridNode &rhs) const { return ((m_x == rhs.m_x) && (m_y == rhs.m_y)); }

	// members
	int m_x, m_y; // the coord of the grid cell
	int m_cost;
	bool m_visited;
	const static int BLOCKED;
	MapGridNode *m_parent;
};


class AStarMapGridNode : public MapGridNode
{
public:
	// constructors
	AStarMapGridNode() : MapGridNode() { m_g = m_f = m_h = 0; }
	AStarMapGridNode(const AStarMapGridNode &copy);

	// destructor
	virtual ~AStarMapGridNode() { m_parent = NULL; }

	virtual AStarMapGridNode &operator=(const AStarMapGridNode &rhs);
	virtual void setCost(const int cost);
	virtual int getCost() const;

	// members
	int m_g;
	int m_f;
	int m_h;
};

class MapGridPriorityQueue
{
public:
	MapGridPriorityQueue();
	~MapGridPriorityQueue() { makeEmpty(); delete m_head->m_node; delete m_tail->m_node; delete m_head; delete m_tail; }

	void makeEmpty();
	bool isEmpty() { return m_size == 0; }
	void enqueue( MapGridNode *node );
	MapGridNode* dequeue();
	void remove(MapGridNode *node);
	bool contains(MapGridNode *node) const;

private:
	class QueueNode
	{
	public:
		QueueNode() { m_node = NULL; m_next = m_back = NULL; }
		QueueNode(MapGridNode *node) { m_node = node; m_next = m_back = NULL; }

		MapGridNode *m_node;
		QueueNode *m_next;
		QueueNode *m_back;
	};

	unsigned int m_size;
	QueueNode *m_head;
	QueueNode *m_tail;
};

#endif