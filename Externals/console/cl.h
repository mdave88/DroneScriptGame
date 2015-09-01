#ifndef __CL_H__
#define __CL_H__

#define _LIBRARY_VERSION_	"0.4.2"		/**< Current version. */

/**
 * @mainpage Documentation for "Console library for OpenGL"
 * @version 0.4.2
 * @date may 2004
 * @author <a href="mailto:m0rbac@bluewin.ch">morbac</a> (icq 74468810)
 *
 * This library offers functionnalities for interractive configuration on run
 * time in an OpenGL application. It features
 * - a movable, resizable and skinnable console (with two display modes: flying or static)
 * - a log file system
 * - a command system
 * - a variables system
 * - an alias system
 *
 * Go to the <a href="http://www.calodox.org/morbac/console">homepage</a> for
 * more details and new versions.
 *
 * Thanks to all people who give feedback and participate for debugging and
 * improving the library.
 * Special greetings to :
 * - <a href="http://www.calodox.org">calodox</a> team for hosting
 * - <a href="mailto:nicolasweber@gmx.de">Nicolas Weber</a> for feedback and advices
 *
 * @warning The library is still in development. If you find bugs, please help
 *          to debug by reporting it :)
 * @warning The console has only been tested with applications using double buffer.
 * @warning The console output requires a good graphic card (with 3D acceleration)
 * @warning The library architecture will certainly change in future versions.
 */

#include "console.h"
#include "vars.h"
#include "commands.h"
#include "alias.h"
#include "logfile.h"
#include "glutils.h"

extern Console*			gConsole;		/**< Console support. */
extern LogFile*			gLogFile;		/**< Logfile support. */
extern Vars*			gVars;			/**< Variables support. */
extern Alias*			gAlias;			/**< Aliases support. */
extern Commands*		gCommands;		/**< Commands support. */

/**
 * Console library main class.
 * The console library class is the kernel of the library. The class is responsible
 * for creation and destruction of library tools. The tools are accessible
 * via the global variables:
 * - gLogfile is the log file tool
 * - gConsole is the console tool
 * - gVars is the variable tool
 * - gAlias is the alias tool
 * - gCommands is the alias tool
 * @todo Change the ConsoleLibrary constructor to avoid having to give the width
 *       and height parameters.
 * @todo Modify architecture so there is no more public variables (gConsole,
 *       gCommands, etc.). Try to make an OpenGL like library.
 * @todo Change all integer variables that have a boolean function to bool variables.
 * @todo Port the library to Linux and other platforms.
 * @todo Make possible to have a console (and commands, variables, alias, etc.)
 *       with no graphical output. The graphical part should be placed appart,
 *       or could be defined in main program.
 * @todo Add namespaces.
 */
class ConsoleLibrary
{
	public:
		/**
		 * Default constructor.
		 * @param wwidth The main window width.
		 * @param wheight The main window height.
		 */
		ConsoleLibrary(int wwidth, int wheight);

		/**
		 * Default destructor.
		 */
		~ConsoleLibrary(void);
};

#endif	/* __CL_H__ */