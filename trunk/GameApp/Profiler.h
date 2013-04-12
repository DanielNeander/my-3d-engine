//=================================================================================================
//
//	Query Profiling Sample
//  by MJP
//  http://mynameismjp.wordpress.com/
//
//  All code and content licensed under Microsoft Public License (Ms-PL)
//
//=================================================================================================

#pragma once

#include "InterfacePointers.h"
#include "Timer.h"


class SpriteRenderer;
class SpriteFont;

class Profiler
{

public:

    static Profiler GlobalProfiler;

    void Initialize(ID3D11Device* device, ID3D11DeviceContext* immContext);

    void StartProfile(const std::wstring& name);
    void EndProfile(const std::wstring& name);

    void EndFrame(SpriteRenderer& spriteRenderer, SpriteFont& spriteFont);

protected:

    // Constants
    static const UINT64 QueryLatency = 5;

    struct ProfileData
    {
        ID3D11QueryPtr DisjointQuery[QueryLatency];
        ID3D11QueryPtr TimestampStartQuery[QueryLatency];
        ID3D11QueryPtr TimestampEndQuery[QueryLatency];
        BOOL QueryStarted;
        BOOL QueryFinished;

        static const UINT FilterSize = 64;
        float TimeSamples[FilterSize];
        UINT CurrSample;

        ProfileData() : QueryStarted(FALSE), QueryFinished(FALSE), CurrSample(0)
        {
            for(UINT i = 0; i < FilterSize; ++i)
                TimeSamples[i] = 0.0f;
        }
    };

    typedef std::map<std::wstring, ProfileData> ProfileMap;

    ProfileMap profiles;
    UINT64 currFrame;

    ID3D11DevicePtr device;
    ID3D11DeviceContextPtr context;

    Timer timer;
};

class ProfileBlock
{
public:

    ProfileBlock(const std::wstring& name);
    ~ProfileBlock();

protected:

    std::wstring name;
};


