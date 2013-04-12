#include "stdafx.h"

#include "MapGridNode.h"


const int MapGridNode::BLOCKED = 3000;

MapGridNode::MapGridNode(const MapGridNode &copy) 
{ 
	m_x = copy.m_x; 
	m_y = copy.m_y; 
	m_parent = copy.m_parent; 
	m_visited = copy.m_visited; 
	m_cost = copy.m_cost; 
}

MapGridNode & MapGridNode::operator=(const MapGridNode &rhs) 
{ 
	m_x = rhs.m_x; 
	m_y = rhs.m_y; 
	m_parent = rhs.m_parent; 
	m_visited = rhs.m_visited; 
	m_cost = rhs.m_cost; 
	
	return *this; 
}

bool MapGridNode::operator==(const MapGridNode &rhs)
{
	return this->equals(rhs);
}

bool MapGridNode::operator<(const MapGridNode &rhs)
{
	return m_cost < rhs.m_cost;
}

bool MapGridNode::operator>(const MapGridNode &rhs)
{
	return m_cost > rhs.m_cost;
}

void MapGridNode::setCost(const int cost)
{ 
	m_cost = cost; 
}

int MapGridNode::getCost() const 
{ 
	return m_cost; 
}

MapGridPriorityQueue::MapGridPriorityQueue()
{
	m_size = 0;
	m_head = new QueueNode(new MapGridNode(-1, -1, NULL, true, -1));
	m_tail = new QueueNode(new MapGridNode(-1, -1, NULL, true, 10000));
	m_head->m_next = m_tail;
	m_head->m_back = NULL;
	m_tail->m_back = m_head;
	m_tail->m_next = NULL;
}

void MapGridPriorityQueue::enqueue( MapGridNode * node )
{
	QueueNode *insertnode = new QueueNode(node);

	// find the place where it fits.
	QueueNode *c = m_head;
	while(node->getCost() > c->m_node->getCost())
	{
		c = c->m_next;
	}

	// insert before
	insertnode->m_back = c->m_back;
	insertnode->m_next = c;
	c->m_back = insertnode;
	insertnode->m_back->m_next = insertnode;

	m_size++;
}

MapGridNode* MapGridPriorityQueue::dequeue()
{
	// always remove from the front...
	MapGridNode *item = NULL;
	QueueNode *removednode;

	if(m_size > 0)
	{
		m_head->m_next->m_next->m_back = m_head;
		removednode = m_head->m_next;		
		m_head->m_next = m_head->m_next->m_next;

		item = removednode->m_node;
		delete removednode;
	}

	m_size--;

	return item;
}

void MapGridPriorityQueue::remove(MapGridNode* node)
{
	QueueNode *c = m_head;
	while(!node->equals(*(c->m_node)))
	{
		c = c->m_next;
	}

	c->m_next->m_back = c->m_back;
	c->m_back->m_next = c->m_next;

	m_size--;

	delete c;
}

bool MapGridPriorityQueue::contains(MapGridNode* node) const
{
	QueueNode *c = m_head;

	while(c != m_tail)
	{
		if(c->m_node->equals(*node))
			return true;
		c = c->m_next;
	}
	return false;
}

void MapGridPriorityQueue::makeEmpty()
{
	while(!isEmpty())
	{
		dequeue();
	}
}

AStarMapGridNode::AStarMapGridNode(const AStarMapGridNode &copy)
{
	m_x = copy.m_x; 
	m_y = copy.m_y; 
	m_parent = copy.m_parent; 
	m_visited = copy.m_visited; 
	m_cost = copy.m_cost; 
	m_g = copy.m_g;
	m_h = copy.m_h;
	m_f = copy.m_f;
}

AStarMapGridNode & AStarMapGridNode::operator=(const AStarMapGridNode &rhs)
{
	m_x = rhs.m_x; 
	m_y = rhs.m_y; 
	m_parent = rhs.m_parent; 
	m_visited = rhs.m_visited; 
	m_cost = rhs.m_cost; 
	m_g = rhs.m_g;
	m_h = rhs.m_h;
	m_f = rhs.m_f;

	return *this;
}

void AStarMapGridNode::setCost(const int cost) 
{ 
	m_f = cost; 
}

int AStarMapGridNode::getCost() const 
{ 
	return m_f; 
}
