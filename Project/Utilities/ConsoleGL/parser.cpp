//-----------------------------------------------------------------------------
// File parser
// version 0.2
//-----------------------------------------------------------------------------

#include "parser.h"
#include "cl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TOKEN_ALLOCATION_BLOC 1024		// Current token allocation size

Parser::Parser(void)
{
	buffer = buf_start = buf_end = mem = NULL;
	tokensize = TOKEN_ALLOCATION_BLOC;
	token = (char*) malloc(tokensize*sizeof(char));
	memset(token, 0, tokensize*sizeof(char));
	line = 0;
}

Parser::~Parser(void)
{
	StopParse();
	if (token) free(token);
	token = NULL;
	tokensize = 0;
}

int Parser::StartParseFile(const char *name)
{
	StopParse();

	FILE *fp;

	if ((fp = fopen(name, "rb")) == NULL) return 0;

	// Get the file size
	fseek(fp, 0L, SEEK_END);
	fsize = ftell(fp);
	rewind(fp);

	// Copy the file content in memory
	mem = new unsigned char[fsize];
	if (mem == NULL)
	{
		fclose(fp);
		return 0;
	}
	fread(mem, fsize, sizeof(unsigned char), fp);
	fclose(fp);

	// Define the local variables
	strncpy(filename, name, FILENAME_LEN);
	buf_start = buffer = mem;
	buf_end = mem + fsize;
	line = 1;

	return 1;
}

void Parser::StartParseString(const char *string)
{
	StopParse();

	buf_start = buffer = (unsigned char*) string;
	buf_end = (unsigned char*) string + strlen(string);
	line = 1;
}

void Parser::StartParseBuffer(const unsigned char *buff, const int size)
{
	StopParse();

	buf_start = buffer = buff;
	buf_end = buff + size;
	line = 1;
}

void Parser::StopParse(void)
{
	buffer = buf_start = buf_end = NULL;
	if (mem) delete [] mem;
	mem = NULL;
	memset(filename, 0, FILENAME_LEN);
}

bool Parser::GetToken(bool crossline)
{
	char *token_p;

	if (!buffer) return 0;
	if (buffer == buf_end) return 0;

	*token = 0;		// Re-initialize the current token

	// Skip the spaces
skipspace:
	while (*buffer <= 32 || *buffer == '\n' || *buffer == ';')
	{
		if (*buffer == '\n') ++line;
		if (buffer >= buf_end) { if(!crossline) return 0; }
		if (!crossline)
		{
			if (*buffer == '\n' || *buffer == ';')
			{
				--line;
				return 0;
			}
		}
		++buffer;
	}

	if (buffer >= buf_end) return 0;

	// Skip the comments with have the following form : // or #
	if (*buffer == '#' || (buffer[0] == '/' && buffer[1] == '/'))
	{
		while (*buffer++ != '\n')
		{
			if (buffer >= buf_end) return 0;
		}
		++line;

		if (!crossline) return 0;
		goto skipspace;
	}

	// Skip the comments with have the following form : /* */
	if (buffer[0] == '/' && buffer[1] == '*')
	{
		buffer += 2;
		while (buffer[0] != '*' && buffer[1] != '/')
		{
			++buffer;
			if (buffer >= buf_end) return 0;
			if (*buffer == '\n') ++line;
		}
		buffer += 2;
		goto skipspace;
	}

	// Copy the token
	token_p = token;

	if (*buffer == '"')
	{
		// quoted token
		++buffer;
		while (*buffer != '"')
		{
			*token_p++ = *buffer++;
			if (buffer == buf_end) break;
			if (token_p == &token[tokensize])
			{
				// Create a bigger token buffer (the current token is bigger than allocated memory)
				tokensize += TOKEN_ALLOCATION_BLOC;
				token = (char*) realloc(token, tokensize*sizeof(char));
				memset(&token[tokensize-TOKEN_ALLOCATION_BLOC], 0, TOKEN_ALLOCATION_BLOC*sizeof(char));
			}
		}
		++buffer;
	}
	else
	{
		// regular token
		if (*buffer == '{' || *buffer == '}')
		{
			*token_p++ = *buffer++;
			*token_p = 0;
			return 1;
		}
		while (*buffer > 32 &&
			   *buffer != '\n' &&
			   *buffer != ';' &&
			   *buffer !='{' &&
			   *buffer !='}')
		{
			*token_p++ = *buffer++;
			if (buffer == buf_end) break;
			if (token_p == &token[tokensize])
			{
				// Create a bigger token buffer (the current token is bigger than allocated memory)
				tokensize += TOKEN_ALLOCATION_BLOC;
				token = (char*) realloc(token, tokensize*sizeof(char));
				memset(&token[tokensize-TOKEN_ALLOCATION_BLOC], 0, TOKEN_ALLOCATION_BLOC*sizeof(char));
			}
		}
	}

	*token_p = 0;

	return 1;
}

int Parser::CompToken(const char *word)
{
	return strcmp(token, word);
}

int Parser::Assert(const char *expected_token)
{
	if (CompToken(expected_token))
	{
		if (strlen(filename)) gConsole->Insert("%s (line %d) : Expected \"%s\" but found \"%s\".\n", filename, line, expected_token, token);
		else gConsole->Insert("Line %d : Expected \"%s\" but found \"%s\".\n", line, expected_token, token);
		gCommands->DoNotAddToScript();
		return 0;
	}
	return 1;
}

void Parser::GoTo(int offset)
{
	buffer = buf_start + offset;
}

int Parser::GetOffset(void)
{
	return (int) (buffer - buf_start);
}