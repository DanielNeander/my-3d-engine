//--------------------------------------------------------------------------------
// TArray
//
// TArray represents a partially filled array of objects.  The objects are stored
// in the first m_iQuantity indices, and any additional indices are not
// guaranteed to have any particular value.
//
// This data structure does not preserve the index of an item after it is added to
// the array.  For instance, if an item is removed before this element, all 
// elements are shifted forward by one to preserve continuity in the array.  Thus
// if the indices are required to remain constant, you must ensure that no 
// elements are removed from array.
//
// Copyright (C) 2003-2009 Jason Zink
//--------------------------------------------------------------------------------

//--------------------------------------------------------------------------------
#ifndef TArray_h
#define TArray_h
//--------------------------------------------------------------------------------
template <class T>
class T_Array
{
public:
	T_Array();
	T_Array( const T_Array& item );
	~T_Array();

	T_Array& operator=( const T_Array& item );

	// Append an element to the list.  If the list is currently empty, then 
	// the element is added in the first location.  Otherwise it is added in
	// the location 'count()' which is quantity+1.  If there isn't enough
	// room in the array, it is expanded as necesary.

	void add( const T& element );

	// The remove operation will remove the element at the given index, and
	// shift all elements after index forward one position.  This ensures
	// that the user can iterate over the elements from 0 to count-1 without
	// hitting an empty index.

	void remove( int index );

	// Sets the internal quantity of elements to zero, effectively 
	// invalidating all elements that had been added previously.

	void empty( );
	
	// Returns the number of elements currently in the array.

	int count();
	
	// Returns true if the input element has been previously added to the
	// array, and false otherwise.

	bool contains( const T& element );
	
	// Returns the index of the input element if it has been previously 
	// added to the array, and -1 otherwise.

	int find( const T& element );

	// Array style access
    T& operator[] (int i);
	const T& operator[] (int i) const;

private:
	int m_iQuantity, m_iCapacity;
	T* m_aData;
};

#include "TArray.inl"
//--------------------------------------------------------------------------------
#endif // TArray_h
