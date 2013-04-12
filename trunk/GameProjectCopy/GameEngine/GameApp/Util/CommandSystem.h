#pragma once 


class CommandSystem : public Singleton<CommandSystem>
{
public:
	IMPLEMENT_SIMPLE_SINGLETON(CommandSystem);
	
private:
	CommandSystem();
	
public:

	void ProcessCommand(const std::string& cmd);
	
	void OnKeyDown(uint32 nChar);
	void OnChar(uint32 nChar);
	void Status(const char* str);
	void Draw();
	std::string mCommand;
	std::string mStatusLine;
	bool mCommandMode;
};