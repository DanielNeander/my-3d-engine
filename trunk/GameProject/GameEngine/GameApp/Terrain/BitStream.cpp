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

#include "stdafx.h"
#include "BitStream.h"
#include <assert.h>

CBitStream::CBitStream(void) : 
    m_AccessMode(BIT_STREAM_ACCESS_MODE_UNDEFINED),
    MaxBits(0)
{
}

CBitStream::~CBitStream(void)
{
}

// Prepares the bit stream for reading the data
void CBitStream::StartReading()
{
    assert(m_AccessMode == BIT_STREAM_ACCESS_MODE_UNDEFINED);
    total_bits = MaxBits;
    m_CurrentByteIter = m_BitSequence.begin();
    m_iBitsLeftInCurrByte = 0;
    m_AccessMode = BIT_STREAM_ACCESS_MODE_READING;
}

// Prepares the bit stream for writing the data
void CBitStream::StartWriting()
{
    assert(m_AccessMode == BIT_STREAM_ACCESS_MODE_UNDEFINED);
    m_BitSequence.clear();
    m_iBitsLeftInCurrByte = 8;
    m_CurrentByte = 0;
    total_bits = 0;
    MaxBits = 0;
    m_AccessMode = BIT_STREAM_ACCESS_MODE_WRITING;
}

void CBitStream::FinishWriting()
{
    m_AccessMode = BIT_STREAM_ACCESS_MODE_UNDEFINED;
    if( m_iBitsLeftInCurrByte < 8 )
    {
        m_BitSequence.push_back( (BYTE)(m_CurrentByte >> m_iBitsLeftInCurrByte) );
    }
    MaxBits = total_bits;
}

void CBitStream::FinishReading()
{
    m_AccessMode = BIT_STREAM_ACCESS_MODE_UNDEFINED;
}

const CBitStream& CBitStream::operator = (const CBitStream &BS)
{
    m_BitSequence = BS.m_BitSequence;
    m_CurrentByte = 0;
    m_iBitsLeftInCurrByte = 0;
    total_bits = 0;
    MaxBits = BS.MaxBits;

    return *this;
}

// Saves the bit stream to the file stream
void CBitStream::SaveToFile(FILE *pOutputFile)const
{
    if( 1 != fwrite( &MaxBits, sizeof(MaxBits), 1, pOutputFile) )
    {
        assert(false);
        throw std::runtime_error("CBitStream: failed to save MaxBits\n");
    }

    if( MaxBits > 0 && 1 != fwrite( &m_BitSequence[0],  sizeof(m_BitSequence[0])*((MaxBits+7)/8), 1, pOutputFile) )
    {
        assert(false);
        throw std::runtime_error("CBitStream: failed to save bit sequence\n");
    }
}

// Loads the bit stream from the file stream
void CBitStream::LoadFromFile(FILE *pInputFile)
{
    if( 1 != fread( &MaxBits, sizeof(MaxBits), 1, pInputFile) )
    {
        assert(false);
        throw std::runtime_error("CBitStream: failed to read MaxBits\n");
    }

    m_BitSequence.resize( (MaxBits+7) / 8 );
    if( MaxBits > 0 && 1 != fread( &m_BitSequence[0],  sizeof(m_BitSequence[0])*((MaxBits+7)/8), 1, pInputFile) )
    {
        assert(false);
        throw std::runtime_error("CBitStream: failed to load bit sequence\n");
    }
    
    total_bits = 0;
    m_CurrentByte = 0;
    m_iBitsLeftInCurrByte = 0;
}
