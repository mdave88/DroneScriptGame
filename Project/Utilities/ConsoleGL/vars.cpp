//-----------------------------------------------------------------------------
// Variables
//-----------------------------------------------------------------------------

#include "cl.h"
#include "parser.h"
#include <math.h>		// for floating related stuff

#ifndef WIN32
	#include <strings.h>
#endif

// Display the variables list
void cmd_varlist (int argc, char *argv[])
{
	gVars->printList();
}

//-----------------------------------------------------------------------------
// Var - a variable
//-----------------------------------------------------------------------------

// Integer var
Var::Var(const char *name, int ivalue, int flags)
{
	this->name = new char[strlen(name)+1];
	strcpy(this->name, name);

	this->svalue = NULL;
	SetInteger(ivalue);
	this->flags = flags;

	// default value
	char str[64];
	sprintf(str, "%i", ivalue);
	default_value = new char[strlen(str)+1];
	strcpy(default_value, str);
}

// Float var
Var::Var(const char *name, float fvalue, int flags)
{
	this->name = new char[strlen(name)+1];
	strcpy(this->name, name);

	this->svalue = NULL;
	SetFloat(fvalue);
	this->flags = flags;

	// default value
	char str[64];
	sprintf(str, "%f", fvalue);
	default_value = new char[strlen(str)+1];
	strcpy(default_value, str);
}

// String var
Var::Var(const char *name, const char *svalue, int flags)
{
	this->name = new char[strlen(name)+1];
	strcpy(this->name, name);

	this->svalue = NULL;
	SetString(svalue);
	this->flags = flags;

	// default value
	default_value = new char[strlen(svalue)+1];
	strcpy(default_value, svalue);
}

// Destructor
Var::~Var(void)
{
	if (name) delete [] name; name = NULL;
	if (svalue) delete [] svalue; svalue = NULL;
	if (default_value) delete [] default_value; default_value = NULL;
}

void Var::SetString(const char *svalue)
{
	delete [] this->svalue;
	this->svalue = new char[strlen(svalue)+1];
	strcpy(this->svalue, svalue);

	this->fvalue = (float) atof(svalue);
	this->ivalue = atoi(svalue);
}

void Var::SetInteger(int ivalue)
{
	char str[64];
	delete [] this->svalue;
	sprintf(str, "%d", ivalue);
	svalue = new char[strlen(str)+1];
	strcpy(svalue, str);
		
	this->fvalue = (float) ivalue;
	this->ivalue = ivalue;
}

void Var::SetFloat(float fvalue)
{
	char str[64];
	delete [] this->svalue;
	if (ceil(fvalue)!=fvalue) sprintf(str, "%f", fvalue);
	else sprintf(str, "%d", (int) fvalue);
	svalue = new char[strlen(str)+1];
	strcpy(svalue, str);
		
	this->fvalue = fvalue;
	this->ivalue = (int) fvalue;
}

void Var::SetDefault(const char *svalue)
{
	delete [] this->default_value;
	default_value = new char[strlen(svalue)+1];
	strcpy(default_value, svalue);
}

int Var::operator= (const int ivalue)
{
	SetInteger(ivalue);
	return ivalue;
}

float Var::operator= (const float fvalue)
{
	SetFloat(fvalue);
	return fvalue;
}

void Var::operator= (const char *svalue)
{
	SetString(svalue);
}

//-----------------------------------------------------------------------------
// Vars - variables list
//-----------------------------------------------------------------------------

Vars::Vars(void)
{
	list = NULL;
	gCommands->AddCommand("varlist", cmd_varlist, "displays the variables list");
}

Vars::~Vars(void)
{	
	Var *v, *vnext;
	for (v = list; v; v = vnext)
	{
		vnext = v->next;
		if (v->flags & VF_TEMP) 
			delete v;	// NOTE: the coherency of the list is broken,
						// but that doesn't mind in the destructor
	}
}

// Create a temporal variable.
// Set the new value if the variable already exists
void Vars::CreateVar(const char *name, const char *string, int flags)
{
	if (!SetKeyValue(name, string))
	{
		flags |= VF_TEMP;
		Var *v = new Var(name, string, flags);
		RegisterVar(*v);
	}
}

void Vars::CreateVar(const char *name, float fvalue, int flags)
{
	if (!SetKeyValue(name, fvalue))
	{
		flags |= VF_TEMP;
		Var *v = new Var(name, fvalue, flags);
		RegisterVar(*v);
	}
}

void Vars::CreateVar(const char *name, int ivalue, int flags)
{
	if (!SetKeyValue(name, ivalue))
	{
		flags |= VF_TEMP;
		Var *v = new Var(name, ivalue, flags);
		RegisterVar(*v);
	}
}

// Set key value
bool Vars::SetKeyValue(const char *name, const char *string)
{
	// search key, if found setstring
	for (Var *v = list; v; v = v->next)
	{
		if (v->name && !strcmp(name, v->name))
		{
			v->SetString(string);
			return true;
		}
	}
	return false;
}

bool Vars::SetKeyValue(const char *name, const float value)
{
	for (Var *v = list; v; v = v->next)
	{
		if (v->name && !strcmp(name, v->name))
		{
			v->SetFloat(value);
			return true;
		}
	}
	return false;
}

bool Vars::SetKeyValue(const char *name, const int value)
{
	for (Var *v = list; v; v = v->next)
	{
		if (v->name && !strcmp(name, v->name))
		{
			v->SetInteger(value);
			return true;
		}
	}
	return false;
}

// Register a variable if it doesnt exists. If it exists and is temporal replace it.
void Vars::RegisterVar(Var &var)
{
	Var *v, *lastv = NULL;
	
	for (v = list; v; v = v->next)		// search for name
	{
		if (!strcmp(var.name, v->name))	// skip already defined variables
		{
			if (v->flags & VF_TEMP)
			{
				var.next = list;
				list = &var;
				
				if (!(var.flags & VF_SYSTEM))
				{		
					var.svalue=new char[strlen(v->svalue)+1];
					strcpy(var.svalue, v->svalue);
					var.fvalue = v->fvalue;
					var.ivalue = v->ivalue;
					var.flags = v->flags ^ VF_TEMP;
				}
				
				lastv->next = v->next;	// delete temporal var from the list
				delete v;
				return;
			}
			break;
		}
		lastv = v;
	}
	
	if (!v)
	{									// if not found insert at the beginning
		var.next = list;
		list = &var;
	}
}

// Register a variable if it doesnt exists. If it exists and is temporal replace it.
void Vars::RegisterVar(Var *var)
{
	Var *v, *lastv = NULL;
	
	for (v = list; v; v = v->next)			// search for name
	{
		if (!strcmp(var->name, v->name))	// skip already defined variables
		{
			if (v->flags & VF_TEMP)
			{
				var->next = list;
				list = var;
				
				if (!(var->flags & VF_SYSTEM))
				{		
					var->svalue=new char[strlen(v->svalue)+1];
					strcpy(var->svalue, v->svalue);
					var->fvalue = v->fvalue;
					var->ivalue = v->ivalue;
					var->flags = v->flags ^ VF_TEMP;
				}
				
				lastv->next = v->next;		// delete temporal var from the list
				delete v;
				return;
			}
			break;
		}
		lastv = v;
	}
	
	if (!v)
	{									// if not found insert at the beginning
		var->next = list;
		list = var;
	}
}

void Vars::UnregisterVar(Var &var)
{
	if (list == &var)
	{
		//delete [] list->name; list->name = NULL;
		//delete [] list->svalue; list->svalue = NULL;
		list = var.next;
		return;
	}
	
	for (Var *v = list; v; v = v->next)
	{
		if (v->next == &var)
		{
			v->next = var.next;
			return;
		}
	}
}

void Vars::UnregisterVar(Var *var)
{
	if (list == var)
	{
		//delete [] list->name; list->name = NULL;
		//delete [] list->svalue; list->svalue = NULL;
		list = var->next;
		return;
	}
	
	for (Var *v = list; v; v = v->next)
	{
		if (v->next == var)
		{
			v->next = var->next;
			return;
		}
	}
}

// query methods:
char* Vars::StringForKey(const char * name)
{
	for (Var *v = list; v; v = v->next)
	{
		if (v->name && !strcmp(name, v->name))
		{
			return v->svalue;
		}
	}
	return "";
}

float Vars::ValueForKey(const char * name)
{
	for (Var *v = list; v; v = v->next)
	{
		if (v->name && !strcmp(name, v->name))
		{
			return v->fvalue;
		}
	}
	return 0.0f;
}

int Vars::IntForKey(const char * name)
{
	for (Var *v = list; v; v = v->next)
	{
		if (v->name && !strcmp(name, v->name))
		{
			return v->ivalue;
		}
	}
	return 0;
}

char * Vars::DefaultForKey(const char *name)
{
	for (Var *v = list; v; v = v->next)
	{
		if (v->name && !strcmp(name, v->name))
		{
			return v->default_value;
		}
	}
	return "";
}

bool Vars::isKey(const char *name)
{
	for (Var *v = list; v; v = v->next)
	{
		if (v->name && !strcmp(name, v->name))
		{
			return true;
		}
	}
	return false;
}

Var* Vars::GetVar(const char *name)
{
	for (Var *v = list; v; v = v->next)
	{
		if (v->name && !strcmp(name, v->name))
		{
			return v;
		}
	}
	return NULL;
}

void Vars::printList(void)
{
	gLogFile->OpenFile();
	for (Var *v = list; v; v = v->next)
	{
		gConsole->Insert("\t%s : %s", v->name, v->svalue);
	}
	gLogFile->CloseFile();
}

int Vars::GetNumber(void)
{
	int res = 0;
	for (Var *v = list; v; v = v->next) ++res;
	return res;
}

char* Vars::GetName(int i)
{
	int a = i;
	Var *v;
	for (v = list; a; a--, v = v->next)
	{
		if (!v) return "";
	}
	return v->name;
}

int Vars::LoadFromFile(char *filename)
{
	char buffer[256];
	Parser parser;

	if (!parser.StartParseFile(filename)) return 0;

	gCommands->AddToConsole = false;

	while (parser.GetToken())
	{
		// if the variable exist
		if (isKey(parser.token))
		{
			strncpy(buffer, parser.token, 256);
			parser.GetToken();
			SetKeyValue(buffer, parser.token);
		}
		while (parser.GetToken(false));	// get to next line
	}
	parser.StopParse();

	gCommands->AddToConsole = true;
	return 1;
}

void Vars::SaveToFile(char *filename, const char *mode)
{
	// Saving persistent variables into config file
	FILE *fp;
	if (!(fp = fopen(filename, mode)))
	{
		gConsole->Insert("^1ERROR: Unable to open config file for appending");
	}
	else
	{
		char str[512];
		sprintf(str, "// persistent variables\n");
		fwrite(str, sizeof(char), strlen(str), fp);

		for (Var *v = list; v; v = v->next)
		{
			if (v->flags & VF_PERSISTENT)
			{
				if (strstr(v->svalue, " "))
					sprintf(str, "%s \"%s\"\n", v->name, v->svalue);
				else
					sprintf(str, "%s %s\n", v->name, v->svalue);
				fwrite(str, sizeof(char), strlen(str), fp);
			}
		}
		fclose(fp);
	}
}

