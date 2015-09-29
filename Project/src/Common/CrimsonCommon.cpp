#include "GameStdAfx.h"
#include "Common/CrimsonCommon.h"

#include <malloc.h>


void CheckGLError()
{
	GLuint error = glGetError();

	if(error != GL_NO_ERROR)
	{
		switch(error)
		{
		case GL_INVALID_ENUM:
			TRACE_ERROR("GL_INVALID_ENUM\n", 0);
			break;

		case GL_INVALID_VALUE:
			TRACE_ERROR("GL_INVALID_VALUE\n", 0);
			break;

		case GL_INVALID_OPERATION:
			TRACE_ERROR("GL_INVALID_OPERATION\n", 0);
			break;

		case GL_OUT_OF_MEMORY:
			TRACE_ERROR("GL_OUT_OF_MEMORY\n", 0);
			break;
		};

		TRACE_ERROR("Unknown gl error: " << error, 0);
	}
}


// console indentation
int indentNum = 0;

int getIndentNum()
{
	return indentNum;
}

void updateIndentNum(int inc)
{
	if (inc < -100)
	{
		inc = 0;
	}
	else
	{
		indentNum += inc;
	}
}

// logging

#ifdef WIN32

short consoleGetColours()
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	if (hConsole != INVALID_HANDLE_VALUE)
	{
		CONSOLE_SCREEN_BUFFER_INFO info;

		if (::GetConsoleScreenBufferInfo(hConsole, &info))
		{
			return info.wAttributes;
		}
	}

	return 0;
}

void consoleSetColours(short c)
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	if (hConsole != INVALID_HANDLE_VALUE)
	{
		::SetConsoleTextAttribute(hConsole, c);
	}
}

void consoleSetColours(short fg, short bg)
{
	consoleSetColours(((bg & 0xF) << 4) | (fg & 0xF));
}

#else

short consoleGetColours()
{
	return 0;
}

void consoleSetColours(short c)
{
}

void consoleSetColours(short fg, short bg)
{
}


#endif


//void check_heap(char *file, int line) {
//	static char *lastOkFile = "here";
//	static int lastOkLine = 0;
//	static int heapOK = 1;
//
//	if (!heapOK) return;
//
//	if (_heapchk() == _HEAPOK) {
//		lastOkFile = file;
//		lastOkLine = line;
//		printf("Last OK at %s (%d)\n", lastOkFile, lastOkLine);
//
//		return;
//	}
//
//	heapOK = 0;
//	printf("Heap corruption detected at %s (%d)\n", file, line);
//	printf("Last OK at %s (%d)\n", lastOkFile, lastOkLine);
//}
