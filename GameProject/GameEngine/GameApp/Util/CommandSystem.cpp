#include "stdafx.h"
#include "CommandSystem.h"
#include "GameApp/GameApp.h"
#include "../U2Light.h"
#include "GameApp/Font/Dx11TextHelper.h"

void CommandSystem::OnChar(uint32 nChar)
{
	if (nChar == 27)
	{
		mCommandMode = !mCommandMode;
	}
	else if (mCommandMode)
	{
		if (nChar == 8)
		{
			if (mCommand.size() >= 1)
			{
				mCommand = mCommand.substr(0, mCommand.size()-1);
			}
		}
		else if (nChar == 13)
		{
			ProcessCommand(mCommand);
			mCommand = "";
		}
		else if (isprint(nChar))
		{
			mCommand += nChar;
		}
		return;
	}
}

void CommandSystem::Status(const char* str)
{
	mStatusLine = str;
}

void CommandSystem::ProcessCommand(const std::string& _cmd )
{
	idStr cmd = _cmd.c_str();
	cmd.ToLower();
		
	if (cmd.Find("sun") != -1) {

		float x =0, y = 0, z=0;
		sscanf(cmd.c_str(), "sun %f %f %f", x, y, z);
		extern U2Light gSun;
		gSun.SetTrans(noVec3(x, y, z));			
		gSun.UpdateTM();
		Status("Sun Translated");
	}
}

void CommandSystem::Draw() {

	if (!mCommandMode)  return;

	int nHeight = GetApp()->getHeight() * 0.7f;
	GetApp()->text_->SetInsertionPos(50, nHeight);
	GetApp()->text_->SetForegroundColor(D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f));
	GetApp()->text_->DrawFormattedTextLine(mStatusLine.c_str());

	idStr displayString = "edit";
	GetApp()->text_->SetInsertionPos(50, nHeight+10);

	displayString += "> ";
	displayString += mCommand.c_str();	
	if (timeGetTime() & 0x200)
	{
		displayString += "_";
	}
	GetApp()->text_->DrawFormattedTextLine(displayString.c_str());

}

CommandSystem::CommandSystem()
{
	mCommandMode = true;
}
