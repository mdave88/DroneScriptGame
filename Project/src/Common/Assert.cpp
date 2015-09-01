#include "GameStdAfx.h"
#include "Common/Assert.h"

#include <sstream>

// windows and VS specific stuff
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

void gameAssert(const char* error, const char* file , int line, const char* function, ...)
{
	char destination[1024];

	va_list argp;
	va_start(argp, function);
	vsprintf(destination, error, argp);
	va_end(argp);

	std::stringstream ss;
	ss << "Error : ";
	ss << destination;
	ss << "\nFile : ";
	ss << file;
	ss << "\nLine : ";
	ss << line;
	ss << "\nFunction : ";
	ss << function;

#ifdef WIN32
	int ret = MessageBox( GetActiveWindow() , ss.str().c_str() , "Game Error" , MB_OKCANCEL | MB_ICONERROR );
	GX_DEBUG_BREAK_IF(ret > 1);
#endif
}
