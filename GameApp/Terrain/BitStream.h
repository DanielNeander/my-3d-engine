// Copyright 2011 Intel Corporation
// All Rights Reserved
//
// Permission is granted to use, copy, distribute and prepare derivative works of this
// software for any purpose and without fee, provided, that the above copyright notice
// and this statement appear in all copies.  Intel makes no representations about the
// suitability of this software for any purpose.  THIS SOFTWARE IS PROVIDED "AS IS."
// INTEL SPECIFICALLY DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED, AND ALL LIABILITY,
// INCLUDING CONSEQUENTIAL AND OTHER INDIRECT DAMAGES, FOR THE USE OF THIS SOFTWARE,
// INCLUDING LIABILITY FOR INFRINGEMENT OF ANY PROPRIETARY RIGHTS, AND INCLUDING THE
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  Intel does not
// assume any responsibility for any errors which may appear in this software nor any
// responsibility to update it.

#pragma once

#include <vector>

#ifndef BYTE
    typedef unsigned char       BYTE;
#endif

// Class implements simple bit stream
class CBitStream
{
public:
    CBitStream(void);
    ~CBitStream(void);

    // Prepares the bit stream for reading the data
    void StartReading();
    // Prepares the bit stream for writing the data
    void FinishReading();

    void StartWriting();
    void FinishWriting();

    // Gets one bit from the stream
    inline int ReadBit();
    // Puts one bit to the stream
    inline void WriteBit(int bit);

    int GetTotalBits()const{return total_bits;}
    int GetMaxBits()const{return MaxBits;}
    int GetBitStreamSizeInBits()const{return (m_AccessMode == BIT_STREAM_ACCESS_MODE_WRITING) ? GetTotalBits() : GetMaxBits();}
    inline bool CBitStream::IsEmpty()const{return GetTotalBits()<=0;}

    const CBitStream &operator = (const CBitStream &BS);
    
    // Saves the bit stream to the file stream
    void SaveToFile(FILE *pOutputFile)const;
    // Loads the bit stream from the file stream
    void LoadFromFile(FILE *pInputFile);

private:
    std::vector<BYTE> m_BitSequence;
    std::vector<BYTE>::const_iterator m_CurrentByteIter;

    BYTE m_CurrentByte;
    int m_iBitsLeftInCurrByte;
    
    int total_bits;
    int MaxBits;
    enum 
    {
        BIT_STREAM_ACCESS_MODE_UNDEFINED,
        BIT_STREAM_ACCESS_MODE_READING,
        BIT_STREAM_ACCESS_MODE_WRITING
    }m_AccessMode;
};


inline int CBitStream::ReadBit()
{
    assert(m_AccessMode == BIT_STREAM_ACCESS_MODE_READING);

    if (m_iBitsLeftInCurrByte==0)  
    {
        // Read the current byte if no bits are left in buffer
        m_CurrentByte = *(m_CurrentByteIter++);
        m_iBitsLeftInCurrByte = 8;
    }

    int bit = m_CurrentByte&1; // Return the next bit from the bottom of the byte
    m_CurrentByte >>= 1;
    m_iBitsLeftInCurrByte--;
    
    total_bits--;

    return bit;
}

inline void CBitStream::WriteBit(int bit)
{
    assert(m_AccessMode == BIT_STREAM_ACCESS_MODE_WRITING);

    m_CurrentByte >>= 1; // Put bit In top of buffer
    if (bit)
        m_CurrentByte |= 0x80;
    m_iBitsLeftInCurrByte--;    // Decrease number of bits to output to buffer
    
    if (m_iBitsLeftInCurrByte==0)  // Output current byte if it is now full
    {
        m_BitSequence.push_back( m_CurrentByte );
        m_iBitsLeftInCurrByte = 8;
    }

    total_bits++;
}
