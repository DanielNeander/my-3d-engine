#pragma once 
#include "GameObjectComponent.h"

class EventParam 
{
public:
	EventParam();
	EventParam(const char* pcString);
	EventParam(const std::string &kString);
	EventParam(uint32 ui);
	EventParam(int32 ui);
	EventParam(float fValue);
	EventParam(const noVec3 &kVector);
	EventParam(void* pkPointer);
	EventParam(GameObject* pkGameObject);

	float ToFloat();
	int32 ToInt();
	std::string& ToString();
	void* ToPointer();
	GameObject* ToGameObject();

	operator float();
	operator uint32();
	operator int32();
	operator GameObject*();
	operator void*();
	operator noVec3();

	EventParam& operator =(const EventParam &other);


private:
	std::string m_kString;
	noVec3		m_kVector;
	void*		m_pkPointer;
	GameObjectWPtr	m_pkGameObject;

};
