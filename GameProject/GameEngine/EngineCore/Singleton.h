#pragma once

#include <assert.h>


	//Singleton class as authored by Scott Bilas in the book Game Programming Gems

	template <typename T>
	class Singleton
	{
	public:
		Singleton( void )
		{
			assert( ms_Singleton == 0 && "Singleton constructor" );
			int offset = (int)(T*)1 - (int)(Singleton <T> *)(T*)1;
			ms_Singleton = (T*)((int)this + offset);
		}
		~Singleton( void )  {  assert( ms_Singleton != 0 && "Singleton destructor" );  ms_Singleton = 0;  }

		static T&   GetSingleton      ( void )  {  assert( ms_Singleton != 0 && "Singleton - GetSingleton" );  return ( *ms_Singleton );  }
		static T*   GetSingletonPtr   ( void )  {  return ( ms_Singleton );  }
		static bool NotNull ( void )  {  return ( ms_Singleton != 0 );  }

		static T& getInstance() {
			if( !isCreated() )
				ms_Singleton = T::createInstance();
			return *ms_Singleton;
		}

		static void finalize() {
			if( !isCreated() )
				return;
			T::deleteInstance( *ms_Singleton );
			ms_Singleton = 0;
		}
	protected:
		static bool isCreated() { return ( ms_Singleton != NULL ); }

	private:
		static T* ms_Singleton;

	};

template <typename T> T* Singleton <T>::ms_Singleton = 0;



#define IMPLEMENT_SIMPLE_SINGLETON(clazz) \
	static clazz* createInstance() { return new clazz(); } \
	static void deleteInstance( clazz& o ) { delete &o; } \
	friend class Singleton<clazz>
