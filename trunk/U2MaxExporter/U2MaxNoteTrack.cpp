#include "U2MaxNoteTrack.h"


U2MaxNoteTrack::U2MaxNoteTrack(int numFrame)
:m_iNumFrames(numFrame)
{

}

U2MaxNoteTrack::~U2MaxNoteTrack()
{

}

//---------------------------------------------------------------------------
/**
Collect note key from the given node and creates animation states.
A note key is used for representing animation state.

The following presents a note key format:
@verbatim
loopName: loop-type 
duration: frame-count 
fadeintime: seconds-as-float 
@endverbatim
Note that ':' colon is used to separate a name and its value.

-30-Aug-06  kims  Changed the current format to support fade-in-time.
Thank you bruce for the patch.

-20-Feb-06  kims  Fixed to get correct index of ExtractRange().
Thanks Ivan Tivonenko for the patch.

@param inode A node which we collect animation states from.
The node should be bone.
*/
void U2MaxNoteTrack::GetAnimState(INode *inode)
{
	U2ASSERT(inode);

	for (int i=0; i<inode->NumNoteTracks(); i++)
	{
		DefNoteTrack* noteTrack = static_cast<DefNoteTrack*>(inode->GetNoteTrack(i));

		for (int j=0; j<noteTrack->keys.Count(); j++)
		{
			NoteKey* noteKey = noteTrack->keys[j];

			int frame = noteKey->time / GetTicksPerFrame();
			int nextFrame = GetNextFrame(j, noteTrack);

			U2DynString szNoteData = noteKey->note.data();
			if(!szNoteData.IsEmpty())
			{
				int duration = -1;
				float fFadeInTime = 0.0f;
				U2DynString szNote, szNotename, szDuration, szLoopType, szFadeInTime;

				U2ObjVec<U2DynString> noteLines;
				szNoteData.Tokenize(TSTR("\n"), noteLines);
				for(unsigned int i=0; i < noteLines.FilledSize(); ++i)
				{
					U2DynString szLine = noteLines[i];
					szLine.Trim(TSTR(" \r"));
					U2ObjVec<U2DynString> tokens;
					szLine.Tokenize(TSTR(";"), tokens);
					if(tokens.Size() > 1)
					{
						U2DynString szKey = tokens[0];
						szKey.Trim(TSTR(" "));
						U2DynString szVal = tokens[1];
						szVal.Trim(" ");
						szKey.ToLower();
						szVal.ToLower();

						if(i == 0)
						{
							// First line, will be the state name and loop type.
							szNotename = tokens[0]; // Get the original token to maintain case.
							szLoopType = szVal;
						}
						else if(szKey == TSTR("duration"))
						{
							szDuration = szVal;
							duration = szVal.ToInt();
						}
						else if( szKey == TSTR("fadeintime"))
						{
							szFadeInTime = szVal;
							fFadeInTime = szVal.ToFloat();
						}
					}
					else if(tokens.FilledSize() ==1)
					{
						 // No value, just a key
						if(i == 0)
						{
						   // First line, it'll be the name.
							szNotename = szLine;
						}
						else 
						{
							// Is it an int? Maybe it is the duration, but warn
							// about deprecated usage and suggest that they update
							// their data format.
							szDuration = szLine;
							duration= szLine.ToInt();


						}

					}
				}

				if(duration == -1)
				{
					duration = nextFrame - frame;
				}

				U2MaxAnimState* pNewAnimState = U2_NEW U2MaxAnimState;
				pNewAnimState->m_szName = szNotename;
				pNewAnimState->m_iFirstFrame = frame;
				pNewAnimState->m_iDuration = duration;
				pNewAnimState->m_fFadeInTime = fFadeInTime;

				szLoopType.ToLower();
				if(szLoopType == TSTR("repeat"))
				{
					pNewAnimState->m_iLoopType = U2AnimBuilder::Group::REPEAT;
				}
				else if(szLoopType == TSTR("clamp"))
				{
					pNewAnimState->m_iLoopType = U2AnimBuilder::Group::CLAMP;
				}
				else 
				{
					 // unknown loop type. just specify it to repeat type.
					pNewAnimState->m_iLoopType = U2AnimBuilder::Group::REPEAT;

				}

				m_states.AddElem(pNewAnimState);
			}
		}
	}	
}