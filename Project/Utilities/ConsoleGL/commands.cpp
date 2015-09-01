//-----------------------------------------------------------------------------
// Commands
//-----------------------------------------------------------------------------

#include "cl.h"

#ifdef WIN32
	#include <tchar.h>					// for _vsntprintf
#else
	#include <stdlib.h>
	#include <strings.h>
	#include <stdio.h>
	#include <stdarg.h>
	#include <sys/va_list.h>
	#define _vsntprintf vsnprintf
	#define stricmp strcasecmp
	#define strnicmp strncasecmp
#endif

// For function completion
typedef struct _charlist
{
	commline name;
	commline beginning;
	struct _charlist *next;
	struct _charlist *prev;
} charlist;

charlist *llist = NULL, *curr = NULL;

int cmd_argc;
char** cmd_argv;

char last_command[COMMAND_LINELENGTH] = { '\0' };

char* Script;					/**< Console script */
bool WriteScript = false;		/**< Is currently editing a script ? */
bool allowedtowrite = true;		/**< Is allowed to write in script ? */

#define ADD_PARTIAL		0		/**< Specify if partial command is added to solutions list. */
#define ADD_PREFIX		0		/**< Specify if common prefix of completed commands is added to solutions list. */
#define OUTPUTTABLEBLOC	8		/**< Size of table allocation. */

void cmd_clear(int argc, char *argv[])
{
	gConsole->Clear();
}

void cmd_stats(int argc, char *argv[])
{
	gConsole->GetStats();
}

void cmd_help(int argc, char *argv[])
{
	/**
	 * @todo Support alias here
	 */
	if (argc > 1)
	{
		cmd_function_t* cmd = gCommands->GetFunction(argv[1]);
		if (cmd) gConsole->Insert("^5%s^0 : %s", argv[1], cmd->description);
		else gConsole->Insert("^5WARNING: %s -> Command not found", argv[1]);
	}
	else gConsole->Insert("usage: %s <string>", argv[0]);
}

void cmd_commands(int argc, char *argv[])
{
	gCommands->DisplayCommands(argc>1?1:0);
}

void cmd_echo(int argc, char *argv[])
{
	if (argc > 1)
	{
		if (!stricmp(argv[1],"on") || !stricmp(argv[1],"1")) gCommands->AddToConsole = true;
		else if (!stricmp(argv[1],"off") || !stricmp(argv[1],"0")) gCommands->AddToConsole = false;
		else
		{
			gConsole->Insert("^5Argument not valid");
			allowedtowrite = false;
		}
	}
	else gConsole->Insert("echo : %s", gCommands->AddToConsole?"on":"off");
}

void cmd_rem(int argc, char *argv[])
{
	if (argc > 1)
	{
		char command[1024] = { '\0' };
		for (int i = 0; i < argc; ++i)
		{
			strcat(command, argv[i]);
			if (i != argc-1) strcat(command, " ");
		}
		if (!stricmp(argv[0],"rem")) gConsole->Insert(command+4);
		else if (!stricmp(argv[0],"//")) gConsole->Insert(command+3);
		else if (!stricmp(argv[0],"#")) gConsole->Insert(command+2);
	}
	else gConsole->Insert("usage: %s <string>", argv[0]);
}

void cmd_togglehistory(int argc, char *argv[])
{
	gCommands->AddToHistory = !gCommands->AddToHistory;
}

void cmd_newscript(int argc, char *argv[])
{
	allowedtowrite = false;
	Script = new char[MAX_SCRIPT_LINES*CONSOLE_LINELENGTH];
	memset(Script, '\0', MAX_SCRIPT_LINES*CONSOLE_LINELENGTH);
	WriteScript = true;
}

void cmd_savescript(int argc, char *argv[])
{
	allowedtowrite = false;
	if (argc > 1)
	{
		FILE *f = fopen(argv[1], "w");
		fwrite(Script, sizeof(char), strlen(Script), f);
		fclose(f);
	}
	else gConsole->Insert("usage: %s <string>", argv[0]);
}

void cmd_loadscript(int argc, char *argv[])
{
	allowedtowrite = false;
	if (argc > 1)
	{
		FILE *f;
		if ((f = fopen(argv[1], "r")) == NULL)
		{
			gConsole->Insert("^1Unable to open file %s", argv[1]);
			return;
		}
		char* s = new char[CONSOLE_LINELENGTH];
		memset(s, '\0', sizeof(s));
		for (int i = 0; feof(f) == 0 ; fgets(s, CONSOLE_LINELENGTH, f), i++)
		{
			if (i == 0) continue;			// The first character is empty
			s[strlen(s)-1] = '\0';			// Remove the return char

			gCommands->ExecuteCommand(s);
		}
		delete s;
		fclose(f);
	}
	else gConsole->Insert("usage: %s <string>", argv[0]);
}

void cmd_grep(int argc, char *argv[])
{
	if (argc > 1)
	{
		// Concatenate all arguments
		char args[COMMAND_LINELENGTH] = { '\0' };
		for (int i = 1; i < argc; ++i)
		{
			strcat(args, argv[i]);
			if (i+1<argc) strcat(args, " ");
		}
		gCommands->CallGrep(args);
	}
	else gConsole->Insert("usage: %s <string> ...", argv[0]);
}

void cmd_sort(int argc, char *argv[])
{
	if (argc > 1) gCommands->CallSort((int) atoi(argv[1]));
	else gCommands->CallSort(1);
}

///////////////////////////////////////////////////////////////////////////////

Commands::Commands(void)
{
	AddToHistory			= true;
	AddToConsole			= true;
	cmd_functions			= NULL;

	nbrAllocatedHistLines	= HISTBLOC;
	nbrCommHistLines		= 0;

	commandsHistory = (commline*) malloc(nbrAllocatedHistLines*sizeof(commline));
	for (int i = 0; i < nbrAllocatedHistLines; ++i) memset(commandsHistory[i], '\0', CONSOLE_LINELENGTH);

	dataTables				= NULL;
	nbrTables				= 0;
	currTable				= -1;

	isMakingRecursion		= false;
	isWaitingForOutput		= false;

	AddCommand("clear", cmd_clear, "clears the console content");
	AddCommand("stats", cmd_stats, "displays console statistics");
	AddCommand("help", cmd_help, "displays help about a command");
	AddCommand("cmdlist", cmd_commands, "display all available commands in the console (if an argumentis present, the functions descriptions are given)");
	AddCommand("echo", cmd_echo, "defines the echo value (if echo is on, the function callsare added to the console, if not, only the function results are written in the console)");
	AddCommand("rem", cmd_rem, "writes a remark in the console");
	AddCommand("//", cmd_rem, "writes a remark in the console");
	AddCommand("#", cmd_rem, "writes a remark in the console");
	AddCommand("togglehistory", cmd_togglehistory, "enables or disables the history for console commands");
	AddCommand("newscript", cmd_newscript, "begins a new script (all commands executed in the console will now be added to the script)");
	AddCommand("savescript", cmd_savescript, "saves the current script (needs to have been created before)");
	AddCommand("loadscript", cmd_loadscript, "loads an existing script");
	AddCommand("grep", cmd_grep, "search a substring in a string");
	AddCommand("sort", cmd_sort, "sort the current output data (if sort function has negative argument, the sorting is inverted)");
}

Commands::~Commands(void)
{
	RemoveCommand("help");
	RemoveCommand("cmdlist");
	RemoveCommand("echo");
	RemoveCommand("rem");
	RemoveCommand("//");
	RemoveCommand("#");
	RemoveCommand("togglehistory");
	RemoveCommand("newscript");
	RemoveCommand("savescript");
	RemoveCommand("loadscript");
	RemoveCommand("grep");
	RemoveCommand("sort");

	free(commandsHistory); commandsHistory = NULL;

	if (cmd_argv)
	{
		for (int i = 0; i < cmd_argc; ++i) free(cmd_argv[i]);
		free(cmd_argv);
		cmd_argv = NULL;
	}

	// Unallocation of completion list
	for (; llist;)
	{
		charlist *back = llist->next;
		llist->next = NULL;
		llist->prev = NULL;
		free(llist);
		llist = back;
		back = NULL;
	}
	llist = NULL;
	curr = NULL;

	// Clear functions table
	cmd_function_t *next, *cmd = cmd_functions;
	while (cmd)
	{
		if (cmd->description) free(cmd->description);
		next = cmd->next;
		cmd->next = NULL;
		free(cmd);
		cmd = next;
		next = NULL;
	}
}

void Commands::AddCommand(char *cmd_name, xcommand_t function, const char *description_message)
{
	cmd_function_t *cmd;

	// Error if the command is an alias name
	if (gAlias && gAlias->GetAlias(cmd_name)[0])
	{
		gConsole->Insert("AddCommand: %s already defined as alias.", cmd_name);
		return;
	}

	// Error if command is a variable name
	if (gVars && gVars->isKey(cmd_name))
	{
		gConsole->Insert("AddCommand: %s already defined as variable.", cmd_name);
		return;
	}

	// Error if command already exists
	for (cmd = cmd_functions; cmd; cmd = cmd->next)
	{
		if (!stricmp(cmd_name, cmd->name))
		{
			gConsole->Insert("AddCommand: %s already defined.", cmd_name);
			return;
		}
	}

	cmd = (cmd_function_t*) malloc(sizeof(cmd_function_t));
	cmd->name = cmd_name;
	cmd->function = function;
	cmd->next = cmd_functions;
	if (description_message)
	{
		cmd->description = (char*) malloc(strlen(description_message)+1);
		memset(cmd->description, '\0', strlen(description_message)+1);
		strcpy(cmd->description, description_message);
	}
	else cmd->description = NULL;
	cmd_functions = cmd;
}

void Commands::SetDescription(char *cmd_name, const char *description)
{
	// Check functions
	cmd_function_t *cmd;
	for (cmd = cmd_functions; cmd; cmd = cmd->next)
	{
		if (!stricmp(cmd_name, cmd->name))
		{
			if (cmd->description) free(cmd->description);
			cmd->description = (char*) malloc(strlen(description)+1);
			memset(cmd->description, '\0', strlen(description)+1);
			strcpy(cmd->description, description);
			return;
		}
	}
}

void Commands::RemoveCommand(char *cmd_name)
{
	cmd_function_t* cmd, **back;
	
	back = &cmd_functions;
	while (1)
	{
		cmd = *back;
		if (!cmd)
		{
			// The command cannot be found in ithe list
			gConsole->Insert("RemoveCommand: %s not in command list.", cmd_name);
			return;
		}
		if (!stricmp(cmd_name, cmd->name))
		{
			*back = cmd->next;
			if (cmd->description) free(cmd->description);
			free(cmd);
			return;
		}
		back = &cmd->next;
	}
}

cmd_function_t* Commands::GetFunction(char *cmd_name)
{
	// Check functions
	for (cmd_function_t *cmd = cmd_functions; cmd; cmd = cmd->next)
	{
		if (!stricmp(cmd_name, cmd->name))
		{
			return cmd;
		}
	}
	return NULL;
}

bool Commands::Exists(char *cmd_name)
{
	for (cmd_function_t *cmd = cmd_functions; cmd; cmd = cmd->next)
	{
		if (!stricmp(cmd_name, cmd->name)) return true;
	}
	return false;
}

void Commands::CompleteCommand(const char *partial, bool display_solutions, bool auto_complete_command)
{
	// Unallocation of last completion list
	for (; llist;)
	{
		charlist *back = llist->next;
		llist->next = NULL;
		llist->prev = NULL;
		free(llist);
		llist = back;
		back = NULL;
	}
	llist = NULL;

	int len = (int) strlen(partial);
	if (!len) return;

	char begin[COMMAND_LINELENGTH] = { '\0' };
	const char *tmp_string;
	int n_partial_sol = 0, i;
	
	// Search the last word and eventual beginning
	i = len - 1;
	while (i >= 0 && partial[i] != COMMAND_SEPARATOR && partial[i] != COMMAND_PIPE && partial[i] != ' ') --i;
	tmp_string = &partial[++i];
	if (!(len = (int) strlen(tmp_string))) return;
	strncpy(begin, partial, i);		// Get beginning of command

	#if ADD_PARTIAL	// Add partial word to list
	{
		charlist *tmp = (charlist*) malloc(sizeof(charlist));
		memset(tmp->name, '\0', COMMAND_LINELENGTH);
		strcpy(tmp->name, tmp_string);
		tmp->next = llist;
		tmp->prev = NULL;
		memset(tmp->beginning, '\0', COMMAND_LINELENGTH);
		strcpy(tmp->beginning, begin);
		if (llist) llist->prev = tmp;
		llist = tmp;
	}
	#endif

	// Add all matching commands to list
	for (cmd_function_t *cmd = cmd_functions; cmd; cmd = cmd->next)
	{
		if (!strnicmp(tmp_string, cmd->name, len))
		{
			charlist *tmp = (charlist*) malloc(sizeof(charlist));
			memset(tmp->name, '\0', COMMAND_LINELENGTH);
			strcpy(tmp->name, cmd->name);
			strcat(tmp->name, " ");
			tmp->next = llist;
			tmp->prev = NULL;
			memset(tmp->beginning, '\0', COMMAND_LINELENGTH);
			strcpy(tmp->beginning, begin);
			if (llist) llist->prev = tmp;
			llist = tmp;

			++n_partial_sol;
		}
	}

	// Add all matching variables to list
	int nvars = gVars->GetNumber();
	char varname[128];
	for (i = 0; i < nvars; ++i)
	{
		strncpy(varname, gVars->GetName(i), 128);
		if (!strnicmp(tmp_string, varname, len))
		{
			charlist *tmp = (charlist*) malloc(sizeof(charlist));
			memset(tmp->name, '\0', COMMAND_LINELENGTH);
			strcpy(tmp->name, varname);
			strcat(tmp->name, " ");
			tmp->next = llist;
			tmp->prev = NULL;
			memset(tmp->beginning, '\0', COMMAND_LINELENGTH);
			strcpy(tmp->beginning, begin);
			if (llist) llist->prev = tmp;
			llist = tmp;

			++n_partial_sol;
		}
	}

	// Add all matching aliases to list
	int nalias = gAlias->GetNumber();
	char aliasname[128];
	for (i = 0; i < nalias; ++i)
	{
		strncpy(aliasname, gAlias->GetName(i), 128);
		if (!strnicmp(tmp_string, aliasname, len))
		{
			charlist *tmp = (charlist*) malloc(sizeof(charlist));
			memset(tmp->name, '\0', COMMAND_LINELENGTH);
			strcpy(tmp->name, aliasname);
			strcat(tmp->name, " ");
			tmp->next = llist;
			tmp->prev = NULL;
			memset(tmp->beginning, '\0', COMMAND_LINELENGTH);
			strcpy(tmp->beginning, begin);
			if (llist) llist->prev = tmp;
			llist = tmp;

			++n_partial_sol;
		}
	}

	// Sort list

	// Get common prefix of list
	commline prefix = { '\0' };
	char c;
	int n = 0;
	bool search_next = true;

	while (search_next && n_partial_sol)
	{
		if ((int) strlen(llist->name) > n) c = llist->name[n];
		else break;

		for (charlist *tmp = llist; tmp; tmp = tmp->next)
		{
			if ((int) strlen(tmp->name) > n)
			{
				if (tmp->name[n] != c)
				{
					search_next = false;
					break;
				}
			}
		}

		if (search_next) prefix[n++] = c;
	}

	#if ADD_PREFIX	// Add prefix to list
	{
		charlist *tmp = (charlist*) malloc(sizeof(charlist));
		memset(tmp->name, '\0', COMMAND_LINELENGTH);
		strcpy(tmp->name, prefix);
		tmp->next = llist;
		tmp->prev = NULL;
		memset(tmp->beginning, '\0', COMMAND_LINELENGTH);
		strcpy(tmp->beginning, begin);
		if (llist) llist->prev = tmp;
		llist = tmp;
	}
	#endif

	// Display the list if more than one solution
	if (display_solutions && n_partial_sol > 1)
	{
		gConsole->Insert("%s%s%s", gConsole->GetPrompt(), begin, prefix);
		charlist *tmp;
		#if ADD_PREFIX
			tmp = llist->next;
		#else
			tmp = llist;
		#endif

		#if ADD_PARTIAL
			for (; tmp && tmp->next; tmp = tmp->next)
				gConsole->Insert("     %s", tmp->name);
		#else
			for (; tmp; tmp = tmp->next)
				gConsole->Insert("     %s", tmp->name);
		#endif
	}
	else if (n_partial_sol == 1) prefix[n] = ' ';

	// Complete current console command with prefix
	if (auto_complete_command && n_partial_sol > 0)
	{
		strcat(begin, prefix);
		gConsole->SetCurrentCommand(begin);
	}
}

void Commands::FirstSolution(const char *partial)
{
	if (partial != NULL) CompleteCommand(partial, 0, 0);

	if (llist == NULL) return;
	
	curr = llist;

	#if ADD_PREFIX
		curr = curr->next;
		if (curr == NULL) curr = llist;
	#endif

	gConsole->SetCurrentCommand("%s%s", curr->beginning, curr->name);
}

void Commands::LastSolution(const char *partial)
{
	if (partial != NULL) CompleteCommand(partial, 0, 0);

	if (llist == NULL) return;
	
	for (charlist* tmp = llist; tmp; tmp = tmp->next) curr = tmp;

	#if ADD_PARTIAL
		curr = curr->prev;
		if (curr == NULL) curr = tmp;
	#endif

	gConsole->SetCurrentCommand("%s%s", curr->beginning, curr->name);
}

void Commands::NextSolution(void)
{
	if (llist == NULL) return;

	if (curr) curr = curr->next;
	if (curr == NULL) curr = llist;

	gConsole->SetCurrentCommand("%s%s", curr->beginning, curr->name);
}

void Commands::PrevSolution(void)
{
	if (llist == NULL) return;

	if (curr) curr = curr->prev;
	if (curr == NULL)
		for (charlist* tmp = llist; tmp; tmp = tmp->next) curr = tmp;

	gConsole->SetCurrentCommand("%s%s", curr->beginning, curr->name);
}

void Commands::DisplayCommands(int display_descriptions)
{
	gLogFile->OpenFile();
	for (cmd_function_t *cmd = cmd_functions; cmd; cmd = cmd->next)
	{
		if (display_descriptions)
			gConsole->Insert("^5%s^0 : %s", cmd->name, cmd->description);
		else
			gConsole->Insert("\t%s", cmd->name);
	}
	gLogFile->CloseFile();
}

int Commands::ExecuteCommand(const char *commandline, ...)
{
	va_list	msg;
	char buffer[COMMAND_LINELENGTH] = { '\0' };
	char partcommand[COMMAND_LINELENGTH] = { '\0' };
	char *separator_pos = NULL;
	int i, res = 0, len;

	va_start (msg, commandline);
	_vsntprintf (buffer, COMMAND_LINELENGTH - 1, commandline, msg);
	va_end (msg);

	char *command = buffer;

	if (strlen(command) <= 0) return 0;

	if (AddToHistory && !isMakingRecursion) AddToCommHistory(command);

	while (strlen(command) && command[0] == ' ') ++command;		// Eliminate spaces before command
	if ((len = (int) strlen(command)) <= 0) return 0;

	// Separate substrings (can be separated by pipes and semicolon)
	i = 0; while (i < len && command[i] != COMMAND_SEPARATOR) ++i;	// Find first occurence of ';'

	if (command[i] == COMMAND_SEPARATOR) { strncpy(partcommand, command, i); separator_pos = &command[i+1]; }
	else strcpy(partcommand, command);
	command = partcommand;
	if ((len = (int) strlen(partcommand)) <= 0) return 0;

	// Execute command

	// Displays the command with prompt
	if (AddToConsole && command[0] != '@')
	{
		bool waitingState = isWaitingForOutput;
		isWaitingForOutput = false;
		gConsole->Insert("%s%s", gConsole->GetPrompt(), command);
		isWaitingForOutput = waitingState;
	}

	// Check for pipes
	for (int p = 0; p < len; ++p)
	{
		res = 0;
		if (p > 0) isPiped = (partcommand[p-1] == COMMAND_PIPE);
		else isPiped = false;

		char pipepartcommand[COMMAND_LINELENGTH] = { '\0' };
		int startpos = p;
		while (p < len && partcommand[p] != COMMAND_PIPE) ++p;		// Find first occurence of '|'
		isWaitingForOutput = (p < len);
		strncpy(pipepartcommand, &partcommand[startpos], p-startpos);
		command = pipepartcommand;

		// Create the pipe output table
		if (!nbrTables)
		{
			dataTables = (DataTable*) malloc(sizeof(DataTable));
			currTable = 0;
			nbrTables = 1;
		}
		else
		{
			dataTables = (DataTable*) realloc(dataTables, ++nbrTables*sizeof(DataTable));
			++currTable;
		}
		dataTables[currTable].outputData = NULL;
		dataTables[currTable].outputDataSize = 0;
		dataTables[currTable].outputDataAllocatedSize = 0;

		// executing command
		
		while (strlen(command) && command[0] == ' ') ++command;		// eliminate spaces before command
		if (strlen(command) <= 0) return 0;

		strcpy(last_command, buffer);						// Backup last command
		allowedtowrite = true;
		gConsole->ReInitCurrentCommand();

		char c[COMMAND_LINELENGTH] = { '\0' };
		while (command[0] == '@') ++command;				// The @ don't appear at the line beginning
		strcpy(c, command);

		// (Re-)Initialization of last command arguments
		if (cmd_argc)
		{
			for (i = 0; i < cmd_argc; ++i) free(cmd_argv[i]);
			free(cmd_argv);
			cmd_argv = NULL;
		}
		cmd_argc = 0;

		// Separates the command arguments
		// Get the number of aguments
		i = 0;
		while (i < (int) strlen(c))
		{
			while (c[i] == ' ' && i < (int) strlen(c)) ++i;
			if (i < (int) strlen(c))
			{
				++cmd_argc;
				while (c[i] != ' ' && i < (int) strlen(c)) ++i;
			}
		}
		cmd_argv = (char**) malloc(cmd_argc*sizeof(char*));
		for (i = 0; i < cmd_argc; ++i)
		{
			cmd_argv[i] = (char*) malloc(COMMAND_LINELENGTH*sizeof(char));
			memset(cmd_argv[i], '\0', COMMAND_LINELENGTH - 1);
			if (!GetArg(c, i, cmd_argv[i]))
			{
				bool waitingState = isWaitingForOutput;
				isWaitingForOutput = false;
				gConsole->Insert("^1ERROR: An error occured in command manager. Please report bug to the author.");
				isWaitingForOutput = waitingState;
			}
		}

		if (!cmd_argc) return 0;

		// Check functions
		cmd_function_t *cmd;
		for (cmd = cmd_functions; cmd; cmd = cmd->next)
		{
			if (!stricmp(cmd_argv[0], cmd->name))
			{
				if (cmd->function)
				{
					if (cmd_argc > 1 && !stricmp(STR_ASK, cmd_argv[1]))
					{
						gConsole->Insert("%s: %s", cmd_argv[0], cmd->description);
						res = 1;
						break;
					}
					cmd->function(cmd_argc, cmd_argv);
					res = 1;
					break;
				}
			}
		}

		// Check variables
		if (!res && gVars)
		{
			Var* v = gVars->GetVar(cmd_argv[0]);
			if (v != NULL)
			{
				char* a = v->svalue;
				if (a[0])
				{
					if (cmd_argc > 1)
					{
						if (v->flags & VF_SYSTEM)
						{
							bool waitingState = isWaitingForOutput;
							isWaitingForOutput = false;
							gConsole->Insert("^4You are not allowed to modify \"%s\".", cmd_argv[0]);
							isWaitingForOutput = waitingState;
						}
						else
						{
							gVars->SetKeyValue(cmd_argv[0], cmd_argv[1]);
							bool waitingState = isWaitingForOutput;
							isWaitingForOutput = false;
							if (v->flags & VF_LATCH)
								gConsole->Insert("%s will be changed upon restarting.", cmd_argv[0]);
							if (v->flags & VF_DEBUG)
								gConsole->Insert("^1!! DEBUG MODE !! ^0Do enable this variable for debug only.");
							isWaitingForOutput = waitingState;
						}
					}
					else gConsole->Insert("\"%s\" : \"%s\" [default: \"%s\"]", cmd_argv[0], a, v->default_value);

					res = 1;
				}
			}
		}

		// Check alias
		if (!res && gAlias)
		{
			char* a = gAlias->GetAlias(cmd_argv[0]);
			if (a[0])
			{
				if (gAlias->depth++ < MAX_ALIAS_DEPTH)
				{
					bool waitingState = isWaitingForOutput;
					isWaitingForOutput = false;
					gConsole->Insert("alias redirection...");
					isWaitingForOutput = waitingState;

					char str[COMMAND_LINELENGTH];
					sprintf(str, "%s ", a);
					for (int i = 1; i < cmd_argc; ++i)
					{
						strcat(str, cmd_argv[i]);
						strcat(str, " ");
					}
					res = ExecuteCommand(str);
				}
				else
				{
					bool waitingState = isWaitingForOutput;
					isWaitingForOutput = false;
					gConsole->Insert("^5WARNING: potential infinite recursive loop detected !");
					isWaitingForOutput = waitingState;
				}
			}
		}
		gAlias->depth = 0;

		// Add the command to script
		if (res && WriteScript && allowedtowrite)
		{
			strcat(Script, command);
			strcat(Script, "\n");
		}

		if (!res)
		{
			bool waitingState = isWaitingForOutput;
			isWaitingForOutput = false;
			gConsole->Insert("\"%s\" : ^5Command not found", command);
			isWaitingForOutput = waitingState;
		}
	}

	// Destroy the output tables
	if (nbrTables)
	{
		for (int t = 0; t < nbrTables; ++t)
			if (dataTables[t].outputDataAllocatedSize)
				free(dataTables[t].outputData);
		free(dataTables);
		nbrTables = 0;
		currTable = -1;
	}
	isWaitingForOutput = false;	// no more pipes

	if (separator_pos)
	{
		isMakingRecursion = true;
		ExecuteCommand(separator_pos);
	}

	isMakingRecursion = false;

	return 1;
}

void Commands::AddToCommHistory(char* s)
{
	// History lines are allocated by blocs (HISTBLOC) to avoid frequent allocations
	if (nbrCommHistLines == nbrAllocatedHistLines)
	{
		commandsHistory = (commline*) realloc(commandsHistory, (nbrAllocatedHistLines+HISTBLOC)*sizeof(commline));
		for (int i = nbrAllocatedHistLines; i < nbrAllocatedHistLines+HISTBLOC; ++i)
			memset(commandsHistory[i], '\0', CONSOLE_LINELENGTH);
		nbrAllocatedHistLines += HISTBLOC;
	}
	++nbrCommHistLines;
	strncpy(commandsHistory[nbrCommHistLines-1], s, CONSOLE_LINELENGTH);
}

char* Commands::GetHistoryLine(int l)
{
	if (!nbrCommHistLines || l < 1) return "";
	else if (l > nbrCommHistLines) return commandsHistory[nbrCommHistLines-1];
	else return commandsHistory[nbrCommHistLines-l];
}

int Commands::GetNbrCommHistLines (void)
{
	return nbrCommHistLines;
}

void Commands::RepeatLastCommand(void)
{
	if (strlen(last_command)) ExecuteCommand(last_command);
}

void Commands::DoNotAddToScript(void)
{
	allowedtowrite = false;
}

bool Commands::IsWaitingForOutput(void)
{
	return isWaitingForOutput;
}

void Commands::GetOutput(const char* data)
{
	if (!data || !isWaitingForOutput || currTable < 0) return;

	++dataTables[currTable].outputDataSize;

	if (!dataTables[currTable].outputDataAllocatedSize || !dataTables[currTable].outputData)
	{
		dataTables[currTable].outputData = (commline*) malloc((dataTables[currTable].outputDataAllocatedSize+OUTPUTTABLEBLOC)*sizeof(commline));
		for (int i = dataTables[currTable].outputDataAllocatedSize; i < dataTables[currTable].outputDataAllocatedSize+OUTPUTTABLEBLOC; ++i)
			memset(dataTables[currTable].outputData[i], '\0', COMMAND_LINELENGTH*sizeof(char));
		dataTables[currTable].outputDataAllocatedSize += OUTPUTTABLEBLOC;
	}
	else if (dataTables[currTable].outputDataSize == dataTables[currTable].outputDataAllocatedSize)
	{
		dataTables[currTable].outputData = (commline*) realloc(dataTables[currTable].outputData, (dataTables[currTable].outputDataAllocatedSize+OUTPUTTABLEBLOC)*sizeof(commline));
		for (int i = dataTables[currTable].outputDataAllocatedSize; i < dataTables[currTable].outputDataAllocatedSize+OUTPUTTABLEBLOC; ++i)
			memset(dataTables[currTable].outputData[i], '\0', COMMAND_LINELENGTH*sizeof(char));
		dataTables[currTable].outputDataAllocatedSize += OUTPUTTABLEBLOC;
	}

	strcpy(dataTables[currTable].outputData[dataTables[currTable].outputDataSize-1], data);
}

void Commands::CallGrep(const char *token)
{
	if (!token) return;

	if (isPiped)
	{
		for (int i = 0; i < dataTables[currTable-1].outputDataSize; ++i)
			if (strstr(dataTables[currTable-1].outputData[i], token))
				gConsole->Insert(dataTables[currTable-1].outputData[i]);
	}
	else gConsole->Insert("Unpiped grep call not supported.");	
}

int compare(const void *arg1, const void *arg2)
{
#ifdef WIN32
	return _stricmp((char*) arg1, (char*) arg2);
#else
	return stricmp((char*) arg1, (char*) arg2);
#endif
}

void Commands::CallSort(int order)
{
	if (isPiped)
	{
		qsort((void *)dataTables[currTable-1].outputData,
			(size_t)dataTables[currTable-1].outputDataSize,
			sizeof(commline), compare);

		// Display result
		if (order >= 0)
		{
			for (int i = 0; i < dataTables[currTable-1].outputDataSize; ++i)
				gConsole->Insert(dataTables[currTable-1].outputData[i]);
		}
		else
		{
			for (int i = dataTables[currTable-1].outputDataSize-1; i >= 0; --i)
				gConsole->Insert(dataTables[currTable-1].outputData[i]);
		}
	}
	else gConsole->Insert("Unpiped sort call not supported.");	

}
