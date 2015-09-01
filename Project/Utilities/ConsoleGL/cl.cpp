#include "cl.h"
#include <IL/ilut.h>					// DevIL

Console*			gConsole = NULL;	// Console
LogFile*			gLogFile = NULL;	// Log file
Vars*				gVars = NULL;		// Variables
Alias*				gAlias = NULL;		// Aliases
Commands*			gCommands = NULL;	// Commands

void cmd_version(int argc, char *argv[])
{
	gConsole->Insert("^2Console Library v%s of %s (%s)", _LIBRARY_VERSION_, __DATE__, __TIME__);
}

void cmd_credit(int argc, char *argv[])
{
	gConsole->Insert("^2morbac (m0rbac@bluewin.ch - icq 74468810)");
}

/*
 * @bug Find why the program crash when adding a command directly in ConsoleLibrary constructor.
 *      The variable constructors (ex. v_enablelog) are not called.
 */
ConsoleLibrary::ConsoleLibrary(int wwidth, int wheight)
{
	// Needed to initialize DevIL.
	ilInit();
	// GL cannot use palettes anyway, so convert early.
	ilEnable(IL_CONV_PAL);
	// Gets rid of dithering on some nVidia-based cards.
	ilutEnable(ILUT_OPENGL_CONV);
	// Lets ILUT know to use its OpenGL functions
	ilutRenderer(ILUT_OPENGL);

	gCommands	= new Commands;
	gVars		= new Vars;
	gAlias		= new Alias;
	gLogFile	= new LogFile;
	gConsole	= new Console;

	gConsole->Init(wwidth, wheight);

	gCommands->AddCommand("version", cmd_version);
	gAlias->SetAlias("ver", "version");
	gCommands->AddCommand("credit", cmd_credit);
}

ConsoleLibrary::~ConsoleLibrary(void)
{
	gCommands->RemoveCommand("version");
	gCommands->RemoveCommand("credit");

	delete gConsole; gConsole = NULL;
	delete gLogFile; gLogFile = NULL;
	delete gVars; gVars = NULL;
	delete gAlias; gAlias = NULL;
	delete gCommands; gCommands = NULL;
}