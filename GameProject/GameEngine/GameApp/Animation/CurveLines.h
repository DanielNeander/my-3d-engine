#pragma once 

#include "GameApp/FSM.h"

class CurveLines : public FSMObject {
public:
	~CurveLines();

	void Init(class NavDebugDraw* drawtool);
	void Update(float deltaTime);
	void Draw(float deltaTime);
	void CalculateLine();
	void ClearQueue();
	void SetCoordQueue(std::queue<noVec3>*	q);
	std::queue<noVec3>*	GetLastQueue();

	void AddPoint(noVec3 p);
	void CleanUp();	
	
	int	Index_;
	int m_NumVerts;
private:
	std::queue<noVec3>	CoordQueue_;
	std::queue<noVec3>	LastQueue_;
	class NavDebugDraw* Draw_;
};