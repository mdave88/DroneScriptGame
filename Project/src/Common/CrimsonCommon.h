#pragma once

#include <set>
#include <map>
#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include <time.h>

// rarely changed header files -> precompiled header
#include "Common/GameDefines.h"
#include "Common/Assert.h"
#include "Common/Singleton.h"

#ifdef CLIENT_SIDE
// Glew
#include <GL/glew.h>

// OpenGL
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif

#include "Common/Directory.h"

#include "Common/Utils.h"
#include "Common/ConstantManager.h"

#include "Math/CrimsonMath.h"

#include "GameLogic/SerializationDefs.h"


// short version of unsigned int
#ifndef uint
typedef uint32_t uint;
#endif

#ifndef ushort
typedef uint16_t ushort;
#endif

// converters operations
#define VEC3_TO_F3(v)		(float)v.x,(float)v.y,(float)v.z

// safely delete pointers
#define SAFEDEL(pointer)	if (pointer) { delete pointer; pointer = 0; }
#define SAFEDEL2(pointer)	if (pointer) { delete[] pointer; pointer = 0; }


// console and script operations
int getIndentNum();
void updateIndentNum(int inc);
void logToConsole(std::string str);

const std::string getDataDir();
const std::string getResourcesDir();

void CheckGLError();

// logger operations
#define LOGLEVEL_INFO			0
#define LOGLEVEL_WARNING		1
#define LOGLEVEL_ERROR			2
#define LOGLEVEL_HIGHLIGHT		3
#define LOGLEVEL_LUA			4
#define LOGLEVEL_TODO			6

#define QLOG(s, inc)				{ for (int indent = 0; indent < getIndentNum(); indent++)	std::cout << '\t'; std::cout << s << std::endl; updateIndentNum(inc); }
#define MQLOG(s, inc)				{ for (int indent = 0; indent < getIndentNum(); indent++)	std::cout << '\t'; std::cout << __FUNCTION__ << "<" << this << ">: " << s << std::endl; updateIndentNum(inc); }
#define MQLOG2(classname, s, inc)	{ for (int indent = 0; indent < getIndentNum(); indent++)	std::cout << '\t'; std::cout << __FUNCTION__ << "<" << this << ">: " << s << std::endl; updateIndentNum(inc); }


// font coloring
#define CONSOLE_COLOR_BLUE		0x1
#define CONSOLE_COLOR_GREEN		0x2
#define CONSOLE_COLOR_RED		0x4
#define CONSOLE_COLOR_BRIGHT	0x8

short	consoleGetColours();
void	consoleSetColours(short c);
void	consoleSetColours(short fg, short bg);

#define	TRACE_COLOR(color, msg, inc) {			\
			short colors = consoleGetColours();	\
			consoleSetColours(color, 0);		\
			QLOG(msg, inc);						\
			consoleSetColours(colors);			\
		}

// logging

#if defined(_DEBUG)
#define TRACE_INFO(msg, inc)		TRACE_COLOR(CONSOLE_COLOR_RED|CONSOLE_COLOR_GREEN|CONSOLE_COLOR_BLUE, msg, inc)
#define TRACE_WARNING(msg, inc)		TRACE_COLOR(CONSOLE_COLOR_RED|CONSOLE_COLOR_GREEN|CONSOLE_COLOR_BRIGHT, msg, inc)
#else
#define TRACE_INFO(msg, inc)
#define TRACE_WARNING(msg, inc)
#endif

#define TRACE_ERROR(msg, inc)		TRACE_COLOR(CONSOLE_COLOR_RED|CONSOLE_COLOR_BRIGHT, msg, inc)
#define TRACE_HIGHLIGHT(msg, inc)	TRACE_COLOR(CONSOLE_COLOR_GREEN|CONSOLE_COLOR_BRIGHT, msg, inc)
#define TRACE_TODO(msg, inc)		TRACE_COLOR(CONSOLE_COLOR_RED|CONSOLE_COLOR_BLUE|CONSOLE_COLOR_BRIGHT, msg, inc)
#define TRACE_LUA(msg, inc)			TRACE_COLOR(CONSOLE_COLOR_GREEN|CONSOLE_COLOR_BRIGHT, msg, inc)
#define TRACE_NETWORK(msg, inc)		TRACE_COLOR(CONSOLE_COLOR_BLUE|CONSOLE_COLOR_BRIGHT, msg, inc)

#define LOG(level, message, inc)	log(level, message, inc);

#if defined(_DEBUG)
#	define STRINGIZE(X) DO_STRINGIZE(X)
#	define DO_STRINGIZE(X) #X
#	define PTODO_MESSAGE_STRING(msg) __FILE__ "(" STRINGIZE(__LINE__)") : TODO - " msg " - [ "__FUNCTION__ " ]"
#	if defined(_MSC_VER) // Pragma messages are only supported on MSVC...
#		define PTODO(msg) __pragma( message( PTODO_MESSAGE_STRING(msg) ) )
#		define PDEPRECATED(msg) PTODO(msg)
#	elif defined(__GNUC__) && defined(GCC_VERSION) && GCC_VERSION >= 40400  // And GCC > 4.4.x 
#		define PTODO_MESSAGE_GNUC(msg) _Pragma(#msg)
#		define PTODO(msg) PTODO_MESSAGE_GNUC( message( PTODO_MESSAGE_STRING(msg) ) )
#		define PDEPRECATED(msg) PTODO(msg)
#	else
#		define PTODO(msg)
#		define PDEPRECATED(msg)
#	endif
#else
#	define PTODO(msg)
#	define PDEPRECATED(msg)
#endif

// error checking
//#define CHKHEAP()  (check_heap(__FILE__, __LINE__))
//void check_heap(char *file, int line);
