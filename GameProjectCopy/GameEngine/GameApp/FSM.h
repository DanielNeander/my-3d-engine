#pragma once 

class IFSMState 
{
public:
	virtual ~IFSMState() {}
	virtual bool Update(void *data)=0;
	virtual bool UpdateInput(void *data)=0;
	virtual const char *GetDescription()=0;
};

template <class T>
class FSMState : public IFSMState
{
public:
	typedef bool (T::*Function)();

	FSMState(const char *description)
	{
		m_updateFunction	= NULL;
		m_inputFunction     = NULL;
		m_description		= description;
	}

	FSMState(Function updateFunction, Function inputFunction, const char *description)
	{
		m_updateFunction	= updateFunction;
		m_inputFunction     = inputFunction;
		m_description		= description;
	}

	virtual ~FSMState()
	{
	}

	bool Update(void *self)
	{
		if( m_updateFunction )
		{
			T *cast = (T*)(self);
			return (cast->*m_updateFunction)();
		}
		return false;
	}

	bool UpdateInput(void *self)
	{
		if( m_inputFunction )
		{
			T *cast = (T*)(self);
			return (cast->*m_inputFunction)();
		}
		return false;
	}

	virtual const char *GetDescription()
	{
		if( m_description )
			return m_description;

		return "";
	}

private:
	Function	m_updateFunction;
	Function    m_inputFunction;
	const char	*m_description;
};


class FSMObject
{
public:
	FSMObject();
	virtual ~FSMObject()    {}

protected:
	IFSMState				*m_state;
	float                   m_stateTime;
	float                   m_stateStartTime;

	virtual bool TickState();
	virtual bool TickInput();
	virtual void TransitionState(IFSMState *pkState, float time=0);
	virtual float GetTime();
	virtual float GetRandom(float min, float max);
};

// use this to simplify definition of states....
#define DEFINE_SIMPLE_STATE(state,Class,description) static FSMState<Class>	state(description);
#define DEFINE_STATE(state,Class,updateFunction,description) static FSMState<Class>	state(&Class::updateFunction,NULL,description);
#define DEFINE_STATE2(state,Class,updateFunction,inputFunction,description) static FSMState<Class>	state(&Class::updateFunction,&Class::inputFunction,description);

extern FSMState<FSMObject>		        STATE_NONE;

