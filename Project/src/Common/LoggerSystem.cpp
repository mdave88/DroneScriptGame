#include "GameStdAfx.h"
#include "Common/LoggerSystem.h"

#define	TRACE_COLOR(color, msg, indent) LoggerSystem::getInstance()->log(msg, color, indent);


LoggerSystem::LoggerSystem(LogLevel logLevel) : logLevel(logLevel)
{

}

void LoggerSystem::configure(entityx::EventManager& eventManager)
{
	eventManager.subscribe<LogMessage>(*this);
}

void LoggerSystem::update(entityx::EntityManager& entityManager, entityx::EventManager& eventManager, entityx::TimeDelta dt)
{
}

void LoggerSystem::receive(const LogMessage& logMessage)
{
	log(logMessage.logLevel, logMessage.message);
}

void LoggerSystem::prepareLog(LogLevel messageLogLevel, int indent, short color)
{
	if (messageLogLevel > logLevel)
	{
		return;
	}

	setConsoleColors(color, 0);

	indentNum += indent;
	for (int i = 0; i < indentNum; ++i)
	{
		std::cout << '\t';
	}
}

void LoggerSystem::log(LogLevel messageLogLevel, const std::string& message, int indent, short color)
{
	prepareLog(messageLogLevel, indent, color);
	std::cout << message << std::endl;
}

short LoggerSystem::getConsoleColor()
{
#ifdef WIN32
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hConsole != INVALID_HANDLE_VALUE)
	{
		CONSOLE_SCREEN_BUFFER_INFO info;
		if (::GetConsoleScreenBufferInfo(hConsole, &info))
		{
			return info.wAttributes;
		}
	}
#endif
	return 0;
}

void LoggerSystem::setConsoleColor(short c)
{
#ifdef WIN32
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hConsole != INVALID_HANDLE_VALUE)
	{
		::SetConsoleTextAttribute(hConsole, c);
	}
#endif
}

void LoggerSystem::setConsoleColors(short fg, short bg)
{
	setConsoleColor((((short)bg & 0xF) << 4) | ((short)fg & 0xF));
}
