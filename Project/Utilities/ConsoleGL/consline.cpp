#include "consline.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifdef WIN32
	#include <stdio.h>				// for _vsntprintf
	#include <tchar.h>
#else
	#include <stdlib.h>
	#include <strings.h>
	#include <stdio.h>
	#include <stdarg.h>
	#include <sys/va_list.h>
	#define _vsntprintf vsnprintf
#endif

void ConsLine::Init(Font *f)
{
	font = f;

	memset(content, 0, sizeof(ConsLineContent));
	length = 0;
	nlines = 1;
	lines = (ConsLineContent*) malloc(sizeof(ConsLineContent));
	memset(lines[0], '\0', CONSOLE_LINELENGTH*sizeof(char));
	breakPos = (int*) malloc(sizeof(int));
	breakPosSize = 1;
	breakPos[0] = 0;
}

void ConsLine::Shut(void)
{
	if (breakPos) free(breakPos);
	if (lines) free(lines);

	breakPos = NULL;
	lines = NULL;
}


void ConsLine::ReInit(void)
{
	if (breakPos) free(breakPos);
	if (lines) free(lines);

	memset(content, 0, sizeof(ConsLineContent));
	length = 0;
	nlines = 1;
	lines = (ConsLineContent*) malloc(sizeof(ConsLineContent));
	memset(lines[0], '\0', CONSOLE_LINELENGTH*sizeof(char));
	breakPos = (int*) malloc(sizeof(int));
	breakPosSize = 1;
	breakPos[0] = 0;
}

void ConsLine::RecopyFormatting(int n)
{
	ConsLineContent tmp;
	memset(tmp, '\0', CONSOLE_LINELENGTH*sizeof(char));
	int t = 0, k = 0;
	while (k < breakPos[n])
	{
		if (content[k] == TEXT_STYLE_KEYWORD)
		{
			tmp[t++] = content[k++];	// TEXT_STYLE_KEYWORD
			if (k >= length) break;
			if (content[k] == 'c' || content[k] == 'a')
			{
				while (k < length && content[k] != '}') tmp[t++] = content[k++];
				if (k < length) tmp[t++] = content[k++];
			}
			else if (content[k] == 'i' || content[k] == 'b')
			{
				tmp[t++] = content[k++];	// i || b
				if (k >= length) break;
				tmp[t++] = content[k++];	// + || -
			}
		}
		else if (content[k] == '^')
		{
			tmp[t++] = content[k++];	// '^'
			if (k >= length) break;
			tmp[t++] = content[k++];	// value
		}
		else ++k;
	}

	strcat(tmp, lines[n]);
	memset(lines[n], '\0', CONSOLE_LINELENGTH*sizeof(char));
	strcpy(lines[n], tmp);
}

int ConsLine::Update(int linewidth)
{
	assert(font);

	nlines = 1;

	int llen = 0, i = 0, s, z;
	while (i < length)
	{
		// invisible formatting character
		if (content[i] == TEXT_STYLE_KEYWORD)
		{
			++i;
			if (content[i] == 'a' || content[i] == 'c')
				while (i < length && content[i] != '}') ++i;
			else if (content[i] == 'i' || content[i] == 'b') ++i;
			continue;
		}
		else if (content[i] == '^')
		{
			if (i < length-1) i += 2;
			continue;
		}
		
		if (llen >= linewidth)	// line break
		{
			if (++nlines > breakPosSize)
			{
				breakPos = (int*) realloc(breakPos, nlines*sizeof(int));
				lines = (ConsLineContent*) realloc(lines, nlines*sizeof(ConsLineContent));
				breakPosSize = nlines;
				if (!breakPos || !lines) exit(1);				// TODO: Generate an error
			}

			// try to find a better place for cutting
			z = i-1;

			// rem: nlines is always > 1 here
			bool cuttingposfound = false;
			while (z > breakPos[nlines-2] && !cuttingposfound)
			{
				// find a matching char for line cutting
				s = (int) strlen(CUTTING_CHARS);
				while (s--)
				{
					if (content[z] == CUTTING_CHARS[s])
					{
						cuttingposfound = true;					// a potential good separation place was found
						break;
					}
				}
				if (cuttingposfound) break;
				--z;
			}

			if (!cuttingposfound) z = i;						// cannot find a good place
			else ++z;

			breakPos[nlines-1] = z;
			i = z;

			// Store line content
			memset(lines[nlines-2], '\0', CONSOLE_LINELENGTH*sizeof(char));
			memcpy(lines[nlines-2], &content[breakPos[nlines-2]], (breakPos[nlines-1]-breakPos[nlines-2])*sizeof(char));

			if (nlines > 2)	RecopyFormatting(nlines-2);			// no need to recopy font formatting for first line

			llen = (int) font->GetWidth(content[i]);
		}
		else
		{
			llen += (int) font->GetWidth(content[i]);
		}
		++i;
	}

	memset(lines[nlines-1], '\0', CONSOLE_LINELENGTH*sizeof(char));
	if (nlines > 1)
	{
		memcpy(lines[nlines-1], &content[breakPos[nlines-1]], (length-breakPos[nlines-1])*sizeof(char));
		RecopyFormatting(nlines-1);
	}
	else memcpy(lines[nlines-1], content, length*sizeof(char));

#if 0
	printf("------------------------------------\n");
	for (int a = 0; a < nlines; ++a) printf("%d : %d\t%s\n", a, breakPos[a], lines[a]);
#endif

	return nlines;
}

void ConsLine::SetContent(const char *c, ...)
{
	va_list	msg;
	ConsLineContent buffer;

	va_start(msg, c);
	_vsntprintf(buffer, CONSOLE_LINELENGTH, c, msg);
	va_end(msg);

	memset(content, 0, CONSOLE_LINELENGTH*sizeof(char));
	strcpy(content, buffer);
	length = (int) strlen(content);
}

char* ConsLine::GetContent(int p)
{
	if (p < 0 || p >= nlines) return content;
	else return lines[p];
}

void ConsLine::SetFont(Font *f)
{
	font = f;
}

int ConsLine::GetWidth(int len, int start)
{
	assert(font);

	if (start < 0 || start >= length || len+start > length) return 0;

	int res = 0, l;
	if (len >= 0 && len <= length) l = len;
	else l = length;

	l += start;

	char buffer[CONSOLE_LINELENGTH];
	font->RemoveKeyCodes(buffer, content);

	for (int i = start; i < l; ++i)
		res += (int) font->GetWidth(buffer[i]);
	return res;
}

int ConsLine::GetLen(int width, int start)
{
	assert(font);

	if (width < 0 || start < 0 || start >= length) return 0;

	int w = 0, l = start;
	while (l < length && w <= width)
	{
		w += (int) font->GetWidth(content[l]);
		++l;
	}
	return l;
}

int ConsLine::FindChar(int dist, int start)
{
	assert(font);

	int s = start, d = 0;
	if (s > length || s < 0) return 0;
	
	if (dist > 0)
	{
		while (s < length - 1 && d < dist)
		{
			++s;
			d += (int) font->GetWidth(content[s]);
		}
		return s;
	}
	else if (dist < 0)
	{
		while (s > 0 && d > dist)
		{
			--s;
			d -= (int) font->GetWidth(content[s]);
		}
		return s;
	}
	else return start;
}