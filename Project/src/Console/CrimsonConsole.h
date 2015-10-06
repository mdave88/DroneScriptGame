#pragma once

#include <string>

class GameConsole
{
public:
#ifdef CLIENT_SIDE
	GameConsole();
	~GameConsole();

	void init(float width, float height);
	void releaseKeys();

	void addKeyword(const std::string& keyword);

	void render();
	void print(const std::string& str);

	bool keyDown(uint8_t key, int x, int y);
	bool keyUp(uint8_t key, int x, int y);
	bool special(int key, int x, int y);
	bool specialUp(int key, int x, int y, bool isGamePaused);

	void mouseMove(int x, int y);
	//void mouseDrag(int x, int y);
	void mouseAction(int button, int state, int x, int y);

	bool isOpen() const;
	std::string getCommand() const;
#endif // CLIENT_SIDE

	static void addKeywordToConsole(const std::string& keyword);
};


// console and script operations
int getIndentNum();
void updateIndentNum(int inc);
void logToConsole(std::string str);


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