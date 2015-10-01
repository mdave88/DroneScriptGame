#include "GameStdAfx.h"
#include "Console/CrimsonConsole.h"

#include <console/cl.h>


ConsoleLibrary* cl = 0;
std::set<Var*> commands;


GameConsole::GameConsole()
{
}

GameConsole::~GameConsole()
{
	releaseKeys();
	if (cl)
	{
		delete cl;
	}
}

void GameConsole::init(float width, float height)
{
	cl = new ConsoleLibrary(width, height);

	const char* dataDir = CONST_STR("dataDir").c_str();
	const std::string consoleSkin	= utils::conversion::formatStr("%s/consoleSkins/noborder.skin", dataDir);
	const std::string verdana14Font	= utils::conversion::formatStr("%s/consoleSkins/verdana-14.font", dataDir);
	const std::string verdana12Font	= utils::conversion::formatStr("%s/consoleSkins/verdana-12.font", dataDir);

	gConsole->SetTitle("Survive");
	gConsole->SetCharTable((char*) verdana14Font.c_str(), 1);
	gConsole->SetCharTable((char*) verdana12Font.c_str(), 2);
	gConsole->SetState(OPEN);

	if (!gConsole->LoadSkin((char*) consoleSkin.c_str()))
	{
		printf("Couldn't load skin.\n");
		exit(EXIT_FAILURE);
	}

	gConsole->Insert("Console library %s", _LIBRARY_VERSION_);

	gConsole->SetWindowSettings(width, height);
	gConsole->Resize(gConsole->GetWidth(), 100);

	std::ifstream file(utils::conversion::formatStr("%s/settings/consoleCommands", dataDir), std::ifstream::in);
	if (file.is_open())
	{
		while (!file.eof())
		{
			std::string command;
			getline(file, command);

			command = utils::tokenize(command, "\t").front();

			if (!command.empty())
			{
				addKeyword(command);
			}
		}
	}

	gConsole->Close();
}

void GameConsole::releaseKeys()
{
	std::set<Var*>::iterator it = commands.begin();
	while (it != commands.end())
	{
		gVars->UnregisterVar(*it);
		delete (*it);
		++it;
	}
	TRACE_INFO("Console commands released.", 0);
}

void GameConsole::addKeyword(const std::string& keyword)
{
	Var* var = new Var(keyword.c_str());
	commands.insert(var);
	gVars->RegisterVar(var);
}

void GameConsole::render()
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	gConsole->Update();
	glPopAttrib();
}

void GameConsole::print(const std::string& str)
{
	if (gConsole)
	{
		gConsole->Insert(str.c_str());
	}
}

bool GameConsole::keyDown(uint8_t key, int x, int y)
{
	if (gConsole->GetState() == OPEN)
	{
		//std::string command(gConsole->GetCurrentCommand());

		gConsole->Keyboard(key, x, y);
		if (key == 13)
		{
			gConsole->Clear();
		}

		return false;
	}

	return true;
}

bool GameConsole::keyUp(uint8_t key, int x, int y)
{
	if (gConsole->GetState() == OPEN)
	{
		return false;
	}

	return true;
}

bool GameConsole::special(int key, int x, int y)
{

	if (gConsole->GetState() == OPEN)
	{
		gConsole->KeyboardSpec(key, x, y);
	}

	return true;
}

bool GameConsole::specialUp(int key, int x, int y, bool isGamePaused)
{
	if (!isGamePaused && gConsole->GetState() == OPEN)
	{
		gConsole->Close();
	}
	else if (!isGamePaused && gConsole->GetState() == CLOSED)
	{
		gConsole->Open();
	}

	return true;
}

void GameConsole::mouseMove(int x, int y)
{
	gConsole->MouseMove(x, y);
}

void GameConsole::mouseAction(int button, int state, int x, int y)
{
	gConsole->MouseClic(button, state, x, y);
}

//void GameConsole::mouseDrag(int x, int y) {
//	gConsole->MouseMove(x, y);
//}

bool GameConsole::isOpen() const
{
	return (gConsole->GetState() == OPEN);
}

std::string GameConsole::getCommand() const
{
	return gConsole->GetCurrentCommand();
}

void GameConsole::addKeywordToConsole(const std::string& keyword)
{
	if (gVars)
	{
		Var* var = new Var(keyword.c_str());
		commands.insert(var);
		gVars->RegisterVar(var);
	}
}
