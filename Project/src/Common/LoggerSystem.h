#pragma once

#include "Common/Singleton.h"
#include <entityx/System.h>


enum class LogLevel
{
	INFO = 0,
	WARNING,
	ERR,
	HIGHLIGHT,
	LUA,
	TODO,
	NUM,
};

enum class ConsoleColor
{
	WHITE	= 0x0,
	BLUE	= 0x1,
	GREEN	= 0x2,
	RED		= 0x4,
	BRIGHT	= 0x8,
};

struct LogMessage
{
	LogLevel logLevel;
	std::string message;
};

class LoggerSystem : public entityx::System<LoggerSystem>, public entityx::Receiver<LoggerSystem>, public Singleton<LoggerSystem>
{
public:
	LoggerSystem(LogLevel logLevel = LogLevel::NUM);

	void configure(entityx::EventManager& eventManager);
	void update(entityx::EntityManager& entityManager, entityx::EventManager& eventManager, entityx::TimeDelta dt);
	void receive(const LogMessage& logMessage);

	void prepareLog(LogLevel messageLogLevel, int indent = 0, short color = 0);
	void log(LogLevel messageLogLevel, const std::string& message, int indent = 0, short color = 0);

	static void setConsoleColors(short fg, short bg);

private:
	static void setConsoleColor(short c);
	static short getConsoleColor();

private:
	LogLevel logLevel;
	int indentNum;
};

#define QLOG(msg)								std::cout << msg << std::endl;
#define QLOG_(logLevel, msg, indent, color)		{ LoggerSystem::getInstance()->prepareLog(logLevel, indent, color); QLOG(msg); }
#define MQLOG(logLevel, msg, indent, color)		QLOG_(logLevel, __FUNCTION__ << "<" << this << ">: " << msg, indent, color);

#define	TRACE_COLOR(level, msg, indent, color)	QLOG_(level, msg, indent, color);
#define TRACE_INFO(msg, indent)					TRACE_COLOR(LogLevel::INFO, msg, indent,		(short)ConsoleColor::RED	| (short)ConsoleColor::GREEN	| (short)ConsoleColor::BLUE)
#define TRACE_WARNING(msg, indent)				TRACE_COLOR(LogLevel::WARNING, msg, indent,		(short)ConsoleColor::RED	| (short)ConsoleColor::GREEN	| (short)ConsoleColor::BRIGHT)
#define TRACE_ERROR(msg, indent)				TRACE_COLOR(LogLevel::ERR, msg, indent,			(short)ConsoleColor::RED	| (short)ConsoleColor::BRIGHT)
#define TRACE_HIGHLIGHT(msg, indent)			TRACE_COLOR(LogLevel::HIGHLIGHT, msg, indent,	(short)ConsoleColor::GREEN	| (short)ConsoleColor::BRIGHT)
#define TRACE_TODO(msg, indent)					TRACE_COLOR(LogLevel::TODO, msg, indent,		(short)ConsoleColor::RED	| (short)ConsoleColor::BLUE		| (short)ConsoleColor::BRIGHT)
#define TRACE_LUA(msg, indent)					TRACE_COLOR(LogLevel::LUA, msg, indent,			(short)ConsoleColor::GREEN	| (short)ConsoleColor::BRIGHT)
#define TRACE_NETWORK(msg, indent)				TRACE_COLOR(LogLevel::INFO, msg, indent,		(short)ConsoleColor::BLUE	| (short)ConsoleColor::BRIGHT)
