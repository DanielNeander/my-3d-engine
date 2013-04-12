#ifndef __DYNARRAYCL_H__
#define __DYNARRAYCL_H__

#include <stdlib.h>

template <class TA> class DynArray_cl
{
  public:
  /// \brief
  ///   Constructor of the dynamic array class. The size for dynamic arrays is 0.
  DynArray_cl();

  DynArray_cl(unsigned int initSize, const TA &defaultElementValue);


  /// \brief
  ///   Contructor of the dynamic array class. The size of the array is given as an parameter
  /// 
  /// \param initSize
  ///   size of the dynamic array at initialisation
  DynArray_cl(unsigned int initSize);


  /// \brief
  ///   Destructor of the dynamic array; releases the allocated memory
  virtual ~DynArray_cl();


  /// \brief
  ///   Sets the default value which will be used for empty elements and initializes the array
  /// 
  /// \param defaultElementValue
  ///   default value for array elements.
  inline void Init(const TA &defaultElementValue);


  /// \brief
  ///   Sets the default value for empty elements
  /// 
  /// \param defaultElementValue
  ///   default value for array elements. 
  inline void SetDefaultValue(const TA &defaultElementValue);


  /// \brief
  ///   Returns the default value for empty elements
  /// 
  /// \return
  ///   TA&: default value for array elements.
  inline TA& GetDefaultValue();


  /// \brief
  ///   The assignment operator for the dynamic array class
  /// 
  /// \param copyArray
  ///   array where to copy the data from
  inline void operator=(const DynArray_cl &copyArray);


  /// \brief
  ///   Returns the element value with the given index
  /// 
  /// The array is resized automatically if necessary.
  /// 
  /// \param index
  ///   number of array element (0 to size-1)
  /// 
  /// \return
  ///   TA &: element value
  inline TA &operator[](unsigned int index);
  

  /// \brief
  ///   Resize the dynamic array to the new size
  /// 
  /// The new data will be kept as long as it does fit in the new array. New and empty elements
  /// will be initialises with the default value.
  /// 
  /// \param newSize
  ///   new size of array
  /// 
  /// \note
  ///   If you resize the array with an smaller size than the current size, than data will get
  ///   lost.
  inline void Resize(unsigned int newSize);

  /// \brief
  ///   Ensures that the array has at least minSize elements
  inline void EnsureSize(unsigned int minSize);


  /// \brief
  ///   Frees the array
  inline void Reset();

  /// \brief
  ///   Adjust the size of the dynamic array to the currently used size
  inline void AdjustSize();

  /// \brief
  ///   Returns the number of allocated memory in bytes
  /// 
  /// That is: array size * sizeof(element type)
  /// 
  /// \return
  ///   Number of bytes allocated
  inline unsigned int GetMemSize() const;

  /// \brief
  ///   Copies a given number of elements within the array
  /// 
  /// \param destElement
  ///   index to the element where the data will be copied to
  /// 
  /// \param sourceElement
  ///   index to the first element which will be copied
  /// 
  /// \param number
  ///   number of elements to copy
  inline void CopyElements(unsigned int destElement, unsigned int sourceElement, unsigned int number);


  /// \brief
  ///   Get the first available element in the array which is not yet used or currently empty
  /// 
  /// If no element within the allocated size is currently empty, then the array will be resized
  /// and the new element index will be returned.
  /// 
  /// \return
  ///   unsigned int: index of free element 
  inline unsigned int GetFreePos() const;


  /// \brief
  ///   Search for an element in the array
  /// 
  /// \param searchElement
  ///   element value to search for
  /// 
  /// \return
  ///   int index: index of the element if it has been found; will return -1 if the element has not
  ///   been found
  inline int GetElementPos(const TA &searchElement) const ;

  /// \brief
  ///   Search for an element in the array in the range from 0 to iNumEntries-1. Inputs TA
  ///
  /// \param searchElement
  ///   element value to search for
  ///
  /// \param iNumEntries
  ///   Number of array indices to search
  /// 
  /// \return
  ///   int index: index of the element if it has been found; will return -1 if the element has not
  ///   been found
  inline int GetElementPos(const TA &searchElement, int iNumEntries) const ;

  /// \brief
  ///   Get the current size of the array
  /// 
  /// \return
  ///   int size: current physical size of the array
  /// 
  /// \note
  ///   This function returns the physical size of the array, this means that also not yet defined
  ///   elements are considered for that size. The physical size is in most cases larger than the
  ///   currently used size, since the resize function does allocate an additional buffer (in order
  ///   to avoid resizes whenever one single element will be added). To get the size of the used
  ///   elements you can use the DynArray_cl::GetValidSize function.
  inline unsigned int GetSize() const;


  /// \brief
  ///   Get the currently used size of the array
  /// 
  /// \return
  ///   int size: currently used size of the array
  /// 
  /// \note
  ///   This function returns the logical size of the array. The logcial size means the currently
  ///   used size of the  array. The physical size might be larger then the logical size, since
  ///   empty elements might exist at the end of the array.
  inline unsigned int GetValidSize() const;

  /// \brief
  ///   Returns a pointer to the data array
  /// 
  /// For instance, if you have a dynarray of type DynArray_cl<char *>, this function will return a
  /// char** pointer
  /// 
  /// This function is useful if you want to step through the array without the [] opertator which
  /// performs  a range check (which is not necessary if you only access the range you got with
  /// GetSize()
  /// 
  /// \return
  ///   TA*: A pointer to the array of elements. Can be NULL if there are no elements 
  /// 
  /// \example
  ///   \code
  ///   int count = nameList.GetValidSize();
  ///   char **pNames = nameList.GetDataPtr();
  ///   for (int i=0;i<count;i++) 
  ///     if (pNames[i]) 
  ///     {
  ///       free(pNames[i]); 
  ///       pNames[i]=NULL;
  ///     }
  ///   \endcode
  inline TA* GetDataPtr() const;

  /// \brief
  ///   Returns the n-th element
  /// 
  /// Does not check the ranges
  /// 
  /// \param i
  ///   index of the element to be retrieved
  /// 
  /// \return
  ///   The n-th element
  inline TA& Get(int i) const;

  /// \brief
  ///   Remove sets the element with the given index to the default value
  /// 
  /// This element is then free again and can be returned by the function DynArray_cl::GetFreePos.
  /// 
  /// \param index
  ///   index of the element to be removed
  inline void Remove(unsigned int index);


  /// \brief
  ///   Return whether an element is valid or not
  /// 
  /// An element is valid as long as its value is different than the default value of the dynamic
  /// array Inputs.
  ///
  /// \param index
  ///   index of the element to be checked
  /// 
  /// \return
  ///   BOOL valid: if TRUE then the element is valid, otherwise the element is empty
  inline BOOL IsValid(unsigned int index) const;


  /// \brief
  ///   Returns whether the dynamic array is currently initialized or not
  /// 
  /// \return
  ///   BOOL initialized: if TRUE then the array is initialized
  inline BOOL IsInitialized() const;

  /// \brief
  ///   Rearranges the elements of the array in a random order
  /// 
  /// An adapted MinMaxSort algorithm is used.
  /// 
  /// \param startPos
  ///   entry position to start from
  /// 
  /// \param numEntries
  ///   number of entries to randomize, -1 means all remaining from startPos on
  void Randomize(unsigned int startPos=0, unsigned int numEntries=(unsigned int)-1);

protected:
  static unsigned int GetNewAllocSize(unsigned int iCurrentSize, unsigned int iRequiredSize);

  unsigned int size;   ///< physical size of the array
  TA *data;   ///< pointer to the array data
  TA defaultValue;    ///< default value of the array elements
};

//*****************************************************************************************************
//NEW CLASS:                   TEMPLATE DYNAMIC ARRAY CLASS 
//*****************************************************************************************************


template<class TA> DynArray_cl<TA>::DynArray_cl(unsigned int initSize)
{
  size = initSize;
  if (size > 0)
    data = (TA*)malloc(sizeof(TA)*size);
  else
    data = NULL;
}


template<class TA> DynArray_cl<TA>::DynArray_cl(unsigned int initSize, const TA &defaultElementValue)
{
  size = initSize;
// cant be less then zero  if (size < 0) size = 0;
  if (size > 0)
    data = (TA*)malloc(sizeof(TA)*size);
  else
    data = NULL;
  Init(defaultElementValue);
}

template<class TA> DynArray_cl<TA>::DynArray_cl()
{
  size = 0;
  data = NULL;
}

template<class TA> DynArray_cl<TA>::~DynArray_cl()
{
  if (data)
    free(data);

  data = NULL;
}

template<class TA> void DynArray_cl<TA>::operator=(const DynArray_cl<TA> &copyArray)
{
  Resize(copyArray.size);
  for (unsigned int i = 0; i < copyArray.size; i++)
    data[i] = copyArray.data[i];
}


template<class TA> void DynArray_cl<TA>::Init(const TA &defaultElementValue)
{
  for (unsigned int i = 0; i < size; i++)
    data[i] = defaultElementValue;
  defaultValue = defaultElementValue;
}

template<class TA> void DynArray_cl<TA>::SetDefaultValue(const TA &defaultElementValue)
{
  defaultValue = defaultElementValue;
}

template<class TA> TA& DynArray_cl<TA>::GetDefaultValue()
{
  return defaultValue;
}

template<class TA> void DynArray_cl<TA>::Resize(unsigned int newSize)
{
  if (newSize==size) return;
//cant be less then zero  if (newSize<0) newSize=0;

  TA *tempData;
  int copySize;

  if (newSize > size)
    copySize = size;
  else
    copySize = newSize;
  tempData = data;
  if (newSize > 0)
    data = (TA*)malloc(sizeof(TA)*newSize);
  else
    data = NULL;
  size = newSize;
  Init(defaultValue);
  if ( (size > 0) && (tempData != NULL) )
  {
    for (int i = 0; i < copySize; i++)
      data[i] = tempData[i];
  }
  if (tempData)
    free(tempData);
}


template<class TA> void DynArray_cl<TA>::EnsureSize(unsigned int minSize)
{
  if (minSize>size) 
    Resize(minSize);
}


template<class TA> void DynArray_cl<TA>::Reset()
{
  if (data) free(data);
  data = NULL;
  size = 0;
}


template<class TA> void DynArray_cl<TA>::AdjustSize()
{
  Resize(GetValidSize());
}

template<class TA> unsigned int DynArray_cl<TA>::GetMemSize() const
{
  return size * sizeof(TA);
}


template<class TA> void DynArray_cl<TA>::CopyElements(unsigned int destElement, unsigned int sourceElement, unsigned int number)
{
  unsigned int copynum = number;
  if (sourceElement+number > size)
    copynum = size-sourceElement;
  if (destElement+number > size)
    Resize(destElement+number);

  if (copynum>0)
    memmove(&data[destElement], &data[sourceElement], sizeof(TA)*copynum);

  // fill up the rest with default-value
  while (copynum<number)
    data[copynum++] = defaultValue;
}

template<class TA> unsigned int DynArray_cl<TA>::GetFreePos() const
{
  for (unsigned int i = 0; i < size; i++)
  {
    if ( defaultValue == data[i])
      return i;
  }
  return size;
}

template<class TA> int DynArray_cl<TA>::GetElementPos(const TA &searchElement) const
{
  for (unsigned int i = 0; i < size; i++)
  {
    if (searchElement == data[i])
      return i;
  }
  return -1;
}

template<class TA> int DynArray_cl<TA>::GetElementPos(const TA &searchElement, int iNumEntries) const
{
  unsigned int iSize = iNumEntries;
  if (size < iSize)
    iSize = size;
  for (unsigned int i = 0; i < iSize; i++)
  {
    if (searchElement == data[i])
      return i;
  }
  return -1;
}

template<class TA> unsigned int DynArray_cl<TA>::GetSize() const
{
  return(size);
}


template<class TA> unsigned int DynArray_cl<TA>::GetValidSize() const
{
  unsigned int validSize = 0;
  for (unsigned int i = 0; i < size; i++)
  {
    if (data[i] != defaultValue)
      validSize = i + 1;
  }
  return(validSize);
}

template<class TA> TA* DynArray_cl<TA>::GetDataPtr() const
{
  return data;
}

template<class TA> TA& DynArray_cl<TA>::Get(int i) const
{
  return data[i];
}


// increase array size by doubling; start with at least 16 bytes
template<class TA> unsigned int DynArray_cl<TA>::GetNewAllocSize(unsigned int iCurrentSize, unsigned int iRequiredSize)
{
  if (iCurrentSize<16)
    iCurrentSize = 16;

  while (iCurrentSize<iRequiredSize)
    iCurrentSize*=2;

  return iCurrentSize;
}

template<class TA> TA& DynArray_cl<TA>::operator[] (unsigned int index)
{
  if (index >= size) {
    Resize(GetNewAllocSize(size,index+1));
  }    
  return data[index];
}

template<class TA> void DynArray_cl<TA>::Remove(unsigned int index)
{
  data[index] = defaultValue;
}

template<class TA> BOOL DynArray_cl<TA>::IsValid(unsigned int index) const
{
  return (data[index] != defaultValue);
}

template<class TA> BOOL DynArray_cl<TA>::IsInitialized() const
{
  return (data != NULL);
}

template<class TA> void DynArray_cl<TA>::Randomize(unsigned int startPos, unsigned int numEntries)
{
  if (numEntries==(unsigned int)-1)
    numEntries = size;

  if (startPos+numEntries>size)
    numEntries = size - startPos;

  if (startPos >= size)
    return;

  if (numEntries < 2)
    return;

  // for position N in the array, pick a random one from 0 to N
  
  for (unsigned int n = numEntries; n>0; n--)
  {
    // pick one item from the remaining ordered ones:
    int newpos = rand() % n;
      
    // swap the two items:
    TA temp = data[startPos + n-1];
    data[startPos + n-1] = data[startPos + newpos];
    data[startPos + newpos] = temp;    
  }
}

#endif