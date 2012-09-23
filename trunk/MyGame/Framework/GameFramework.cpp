#include <stdio.h>

#include "GameFramework.h"

#include <Common/Base/System/hkBaseSystem.h>
#include <Common/Base/System/Error/hkDefaultError.h>
#include <Common/Base/Memory/Memory/Debug/hkDebugMemory.h>
#include <Common/Base/Memory/hkDebugMemorySnapshot.h>
#include <Common/Base/Memory/Memory/Pool/hkPoolMemory.h>
#include <Common/Base/Memory/MemoryClasses/hkMemoryClassDefinitions.h>

#include <Common/Base/System/Io/IStream/hkIStream.h>
#include <Common/Base/System/Io/Reader/hkStreamReader.h>
#include <Common/Base/System/Io/Writer/hkStreamWriter.h>
#include <Common/Base/System/Io/Reader/Buffered/hkBufferedStreamReader.h>
#include <Common/Base/System/Stopwatch/hkStopwatch.h>

#include <Common/Base/Memory/Memory/FreeList/hkFreeListMemory.h>
#include <Common/Base/Memory/Memory/FreeList/SystemMemoryBlockServer/hkSystemMemoryBlockServer.h>


// Enable FORCE_STATS_GENERATION to run the stats bootstrapper with the renderer switched off.
// This works for all platforms but is for Wii mostly, as the renderer can't be disabled via 
// command args until file IO is supported.
// #define FORCE_STATS_GENERATION

// Utility class for persistent text display. Substitutes "demo" for "game" in the framework menus
// in the virtual function outputText.
static void HK_CALL errorReportFunction(const char* str, void*)
{
	printf("%s", str);
}

static hkThreadMemory* threadMemory;
static char* stackBuffer;

void Init() 
{
	hkMemoryBlockServer* server = HK_NULL;
	
	server = new hkSystemMemoryBlockServer(1*1024*1024);
	hkMemory* memoryManager = new hkFreeListMemory(server);
	threadMemory = new hkThreadMemory(memoryManager);

	hkBaseSystem::init( memoryManager, threadMemory, errorReportFunction);
	memoryManager->removeReference();

	// We now initialize the stack area to 2 mega bytes (fast temporary memory to be used by the engine).
	{
		int stackSize = 2*1024*1024; // 2MB stack

		stackBuffer = hkAllocate<char>( stackSize, HK_MEMORY_CLASS_BASE);
		threadMemory->setStackArea( stackBuffer, stackSize);
	}


}

void shutdown()
{
	threadMemory->setStackArea(0, 0);
	hkDeallocate(stackBuffer);
	threadMemory->removeReference();

}