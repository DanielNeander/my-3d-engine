/******************************************************************************
"Practical Methods for Precomputed Radiance Transfer Using Spherical Harmonics"

Jerome Ko - submatrix@gmail.com
Manchor Ko - man961.great@gmail.com
Matthias Zwicker - mzwicker@cs.ucsd.edu

******************************************************************************/

#ifndef VECMAP_H
#define VECMAP_H

#include <malloc.h>		//_alloca
#include <map>

namespace GEOM 
{
	/*!
	-- VecMap: maintains a map of unique objects
	*/
	template <class T>
	class VecMap {
	public:
		VecMap() : m_next(0) {}
		~VecMap() {}

		void	Reset() { m_map.clear(); m_next = 0; }
		size_t	Size() const { return m_map.size(); }
		/// inserts if 'v' is new. Return index for the entry.
		int		Add( const T& v ) {
			int index;
			Map::const_iterator ubegin = m_map.find( v );
			if (ubegin != m_map.end()) {
				index = ubegin->second;
			}
			else {
				index = Insert(v);
			}
			return index;
		}
		/// insert a new entry:
		int		Insert( const T& v ) {
			int index = m_next++;
			m_map.insert( std::make_pair(v, index) );
			return index;
		}
		/// locates 
		int		FindIndex( const T& v ) const {
			Map::const_iterator ubegin = m_map.find( v );
			if (ubegin != m_map.end()) {
				return ubegin->second;
			}
			return -1;
		}
	public:
		typedef std::map<T,int>	Map;

		Map		m_map;
		int		m_next;
	};	//end VecMap

};

#endif
