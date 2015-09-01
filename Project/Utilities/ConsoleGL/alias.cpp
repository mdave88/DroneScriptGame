//-----------------------------------------------------------------------------
// Alias
//-----------------------------------------------------------------------------

#include "cl.h"

#ifdef WIN32
	#include <tchar.h>					// for _vsntprintf
#else
	#include <stdlib.h>					// for free
	#include <strings.h>
	#include <stdio.h>					// for vsnprintf
	#include <stdarg.h>					// idem
	#include <sys/va_list.h>			// va_list
	#define _vsntprintf vsnprintf
#endif

void cmd_alias(int argc, char *argv[])
{
	if (argc > 2)
	{
		char arg[COMMAND_LINELENGTH] = { '\0' };
		for (int i = 2; i < argc; ++i)
		{
			strcat(arg, argv[i]);
			if (i != argc-1) strcat(arg, " ");
		}
		strcat(arg, "\0");
		gAlias->SetAlias(argv[1], arg);
	}
	else if (argc > 1)
	{
		// Check for alias value
		char arg[COMMAND_LINELENGTH] = { '\0' };
		strcpy(arg, gAlias->GetAlias(argv[1]));
		if (strlen(arg))
			gConsole->Insert("\"%s\" is \"%s\"", argv[1], gAlias->GetAlias(argv[1]));
		else
			gConsole->Insert("^5WARNING: Alias \"%s\" not defined.", argv[1]);
	}
	else gConsole->Insert("usage: %s <string> <value>", argv[0]);
}

void cmd_unalias(int argc, char *argv[])
{
	if (argc > 1)
	{
		gAlias->RemoveAlias(argv[1]);
	}
	else gConsole->Insert("usage: %s <string>", argv[0]);
}

void cmd_aliaslist(int argc, char *argv[])
{
	gAlias->printList();
}

Alias::Alias(void)
{
	alias					= NULL;
	depth					= 0;

	gCommands->AddCommand("alias", cmd_alias, "sets a value to an alias or creates an alias and sets a valueto it");
	gCommands->AddCommand("unalias", cmd_unalias, "destroy an existing alias");
	gCommands->AddCommand("aliaslist", cmd_aliaslist, "displays a list of all existing aliases");
}

Alias::~Alias(void)
{
	gCommands->RemoveCommand("alias");
	gCommands->RemoveCommand("unalias");
	gCommands->RemoveCommand("aliaslist");

	// Destroy remaining aliases
	alias_t *next, *a = alias;
	while (a)
	{
		next = a->next;
		a->next = NULL;
		if (a->value) free(a->value);
		free(a);
		a = next;
		next = NULL;
	}
}

alias_t* Alias::FindAlias(char *a_name)
{
	alias_t* a;
	
	for (a = alias; a; a = a->next)
		if (!stricmp(a_name, a->name)) return a;
		
	return NULL;
}

void Alias::SetAlias(char *a_name, char *value, ...)
{
	va_list	msg;
	char buffer[COMMAND_LINELENGTH] = { '\0' };
	
	va_start (msg, value);
	_vsntprintf (buffer, COMMAND_LINELENGTH - 1, value, msg);
	va_end (msg);
	
	alias_t* a;

	if (!stricmp(a_name, buffer))
	{
		gConsole->Insert("definition not valid: alias has same name as its value - recursion fault");
		return;
	}

	if (gCommands->GetFunction(a_name))
	{
		gConsole->Insert("already defined as command");
		return;
	}
	if (gVars->GetVar(a_name))
	{
		gConsole->Insert("already defined as variable");
		return;
	}

	a = FindAlias(a_name);

	if (a)
	{
		strcpy(a->value, buffer);
	}
	else
	{
		// Create a new alias
		a = (alias_t*) malloc(sizeof(*a));
		strncpy(a->name, a_name, MAX_ALIAS_NAME);
		a->value = (char*) malloc((strlen(buffer)+1)*sizeof(char));
		strcpy(a->value, buffer);

		a->next = alias;
		alias = a;
	}
}

void Alias::RemoveAlias(char *a_name)
{
	alias_t* a, **back;

	back = &alias;
	while (1)
	{
		a = *back;
		if (!a) return;

		if (!stricmp(a_name, a->name))
		{
			*back = a->next;
			if (a->value) free(a->value);
			free(a);
			return;
		}
		back = &a->next;
	}
}

char* Alias::GetAlias(char *a_name)
{
	alias_t* a = FindAlias(a_name);
	if (!a) return "";
	return a->value;
}

char* Alias::CompleteAlias(char *partial)
{
	alias_t* a;
	int len;

	if (!(len = (int) strlen(partial))) return NULL;

	// Check exact match
	for (a = alias; a; a = a->next)
		if (!stricmp(partial, a->name)) return a->name;

	// Check partial match
	for (a = alias; a; a = a->next)
		if (!strncmp(partial, a->name, len)) return a->name;

	return NULL;
}

alias_t* Alias::GetAliasList(void)
{
	return alias;
}

void Alias::printList(void)
{
	for (alias_t* a = gAlias->GetAliasList(); a; a = a->next)
		gConsole->Insert("\t\"%s\" is \"%s\"", a->name, a->value);
}

int Alias::GetNumber(void)
{
	int res = 0;
	for (alias_t *a = alias; a; a = a->next) ++res;
	return res;
}

char* Alias::GetName(int i)
{
	int n = i;
	alias_t *a;
	for (a = alias; n; n--, a = a->next)
		if (!n) return "";
	return a->name;
}