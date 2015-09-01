#include "cl.h"
#include "parser.h"

#ifdef WIN32
	#include <windows.h>
	#include <tchar.h>
#else
	#define _vsntprintf vsnprintf
#endif

#include <GL/gl.h>
#include <GL/glu.h>

#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <string.h>

#define fontMaxLen 1024

static GLfloat white[4] = {1.0, 1.0, 1.0, 1.0};
static GLfloat gray[4] = {0.5, 0.5, 0.5, 1.0};

bool m3d_text_has_toggled_mode;

Font::Font(void)
{
	fontTabSpace = 4;
	fontITOF = 1.f/255.f;		// 1/255 -> used for color
	fontItalic = 8;

	ColorCopy(fgColor, white); 
	ColorCopy(gdColor, gray);
	ColorCopy(gdColor, gray);
	ysize = 12;
	shadow = 0;
	gradient = 0;
	italic = 0;
	bold = 0;
	texId = 0;

	tHeight = 1;
	height = 12;
	ratio = 1.f;
	SetScale(1.f);

	memset(tWidth, 0, 256*sizeof(float));
	memset(width, 0, 256*sizeof(int));
}

Font::~Font(void)
{
}

void Font::Reset(void)
{
	shadow = 0;
	gradient = 0;
	italic = 0;
	bold = 0;
	ForeColorReset();
	ShadowColorReset();
	GradientColorReset();
}

int Font::Load(char *name)
{
	// Get the font path
	int c, i, l;
	char path[PATH_LENGTH] = { '\0' };
	l = (int) strlen(name)-1;
	while (l >= 0 && name[l] != '\\' && name[l] != '/') --l;
	for (i = 0; i <= l && i < PATH_LENGTH; ++i) path[i] = name[i];

	// Parse the font descriptor file
	Parser p;
	p.StartParseFile(name);

	// read the mapname
	p.GetToken();
	char fullmapname[PATH_LENGTH] = { '\0' };
	sprintf(fullmapname, "%s%s", path, p.token);
	if (!charmap.LoadImage(fullmapname)) return 0;

	float w = (float) charmap.GetWidth();
	float h = (float) charmap.GetHeight();

	// read the font height
	p.GetToken();
	tHeight = (float) atof(p.token);
	height = (int) tHeight;
	tHeight /= h;
	SetRatio(1.f);
	SetScale(1.f);

	// read the characters parameters
	while (p.GetToken())
	{
		p.Assert("{");
		p.GetToken(); c = (int) atoi(p.token);						// read the character value
		p.GetToken(); tPoints[c][0] = ((float) atof(p.token))/w;	// character x position
		p.GetToken(); tPoints[c][1] = 1.f-((float) atof(p.token))/h-tHeight;	// character y position
		p.GetToken(); tWidth[c] = ((float) atof(p.token));			// character width
		width[c] = (int) tWidth[c];
		tWidth[c] /= w;
		p.GetToken(); p.Assert("}");
	}

	p.StopParse();

	return 1;
}

int Font::GetSize(void)
{
	return ysize;
}

float Font::GetRatio(void)
{
	return ratio;
}

void Font::SetRatio(float r)
{
	ratio = r*scale;
}

void Font::SetScale(float s)
{
	scale = s;
	ysize = (int)((float)height*scale);
}

float Font::GetScale(void)
{
	return scale;
}

float Font::GetWidth(char c)
{
	if (c < 0 || c > 255) return 0;
	return width[(int)c]*ratio*scale;
}

void Font::DrawChar(unsigned char c, int x, int y, int ysize, int shadow)
{
	if (!gradient && !shadow) glColor4fv(fgColor);
	else if (!gradient && shadow) glColor4fv(bgColor);
	else if (gradient && !shadow) glColor4fv(gdColor);

	glBegin(GL_QUADS);
		glTexCoord2f(tPoints[(int)c][0]+tWidth[(int)c], tPoints[(int)c][1]);
		glVertex2i(x+(int)((float)width[(int)c]*ratio*scale), y);

		glTexCoord2f(tPoints[(int)c][0], tPoints[(int)c][1]);
		glVertex2i(x, y);

		if (!shadow) glColor4fv(fgColor);
		else glColor4fv(bgColor);

		glTexCoord2f(tPoints[(int)c][0], tPoints[(int)c][1]+tHeight);
		glVertex2i(x+italic, y+(int)((float)ysize));

		glTexCoord2f(tPoints[(int)c][0]+tWidth[(int)c], tPoints[(int)c][1]+tHeight);
		glVertex2i(x+(int)((float)width[(int)c]*ratio*scale)+italic, y+(int)((float)ysize));
	glEnd();
}

void Font::RenderChar(unsigned char c, int x, int y, int ysize)
{
	if (shadow)
	{
		if (!bold) DrawChar(c, x + 1,  y + 1, ysize, 1);
		else DrawChar(c, x + 2,  y + 1, ysize, 1);
	}

	DrawChar(c, x, y, ysize, 0);

	if (bold) DrawChar(c, x + 1, y, ysize, 0);
}

int Font::SlashParser(char *buffPtr, int *x, int *y)
{
	if (!(*(++buffPtr))) return 0;

	switch (*buffPtr)
	{
		case 'a':
		case 'c':
			return SetColorFromToken(buffPtr);
			break;
		case 'i':
			return ItalicsMode(buffPtr);
			break;
		case 'b':
			return BoldMode(buffPtr);
			break;
		default:
			RenderChar(*(--buffPtr), *x, *y, ysize);
			return 0;
			break;
	}
}

int Font::HutParser(char *buffPtr, int *x, int *y)
{
	if (!(*(++buffPtr))) return 0;

	int color = *buffPtr - '0';
	fgColor[0] = (float) text_colors[color][0];
	fgColor[1] = (float) text_colors[color][1];
	fgColor[2] = (float) text_colors[color][2];
	fgColor[3] = (float) text_colors[color][3];

	return 1;
}

void Font::WalkString(char *buffPtr, int xpos, int ypos, int *vPort, int dir)
{
	int x;
	int y = ypos;
	int tabs = 0;
	int xMax = vPort[0] + vPort[2];
	int ret;

	tabs = GetCharHits(buffPtr, '\t');
	
	if (!tabs) tabs = 1;
	else tabs *= fontTabSpace;

	if (dir > 0) x = xpos;
	else x = xpos - ysize * tabs * GetTrueLen(buffPtr);

	for ( ; *buffPtr; ++buffPtr)
	{
		if (x > xMax) break;
  	
		switch (*buffPtr)
		{
			case '\n':
				y -= ysize;
				x = xpos;
				continue;
				break;
        
			case '\t':
				x += (ysize * fontTabSpace);
				continue;
				break;
     
			case TEXT_STYLE_KEYWORD:
				ret = SlashParser(buffPtr, &x, &y);
				buffPtr += ret;
				if (*buffPtr == '\n' || *buffPtr == '\t')
				{
					--buffPtr;
					continue;
				}
				if (ret) continue;
				break;

			case '^':
				ret = HutParser(buffPtr, &x, &y);
				buffPtr += ret;
				if (ret) continue;
				break;
		
			default:
				RenderChar((unsigned char) *buffPtr, x, y, ysize);
				break;
		}
		x += (int)((float)width[(int)(*buffPtr)]*ratio*scale);
	}
}

void Font::Draw(int xpos, int ypos, char *s, ...)
{
	va_list	msg;
	char buffer[fontMaxLen] = {'\0'};
	int vtPort[4];

	va_start(msg, s);
	_vsntprintf(buffer, fontMaxLen - 1, s, msg);	
	va_end(msg);

	/* setup various opengl things that we need */
	m3d_text_has_toggled_mode = SetModes(_2D_MODE_);

	charmap.BindTexture();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	/* draw the string */
	glGetIntegerv(GL_VIEWPORT, vtPort);	
	WalkString(buffer, xpos, ypos, vtPort, 1);

	if (m3d_text_has_toggled_mode) SetModes(_3D_MODE_);
	Reset();
}

void Font::BackDraw(int xpos, int ypos, char *s, ...)
{
	va_list	msg;
	char buffer[fontMaxLen] = {'\0'};
	int vtPort[4];
	
	va_start (msg, s);
	_vsntprintf (buffer, fontMaxLen - 1, s, msg);	
	va_end (msg);
	
	/* setup various opengl things that we need */
	m3d_text_has_toggled_mode = SetModes(_2D_MODE_);
	
	charmap.BindTexture();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	
	/* draw the string */
	glGetIntegerv(GL_VIEWPORT, vtPort);	
	WalkString(buffer, xpos, ypos, vtPort, -1);
	
	if (m3d_text_has_toggled_mode) SetModes(_3D_MODE_);
	Reset();
}

int Font::SetColorFromToken(char *s)
{
	int clr[4];
	int ret = 1;

	if (*s == 'c')
	{
		s += 1;
		if (sscanf(s, "{%d;%d;%d}", &clr[0], &clr[1], &clr[2]) != 3) return -1;
		Color3f(clr[0] * fontITOF, clr[1] * fontITOF, clr[2] * fontITOF);
	}
	else if (*s == 'a')
	{
		s += 1;
		if (sscanf(s, "{%d;%d;%d;%d}", &clr[0], &clr[1], &clr[2], &clr[3]) != 4) return -1;
		Color4f(clr[0] * fontITOF, clr[1] * fontITOF, clr[2] * fontITOF, clr[3] * fontITOF);
	}

	while (*s != '}' && ret ++) ++s;

	return ret + 1;
}

int Font::ItalicsMode(char *s)
{
	if (!(*(++s))) return 0;

	if (*s == '+') italic = fontItalic;
	else if (*s == '-') italic = 0;
	else return 0;

	return 2;
}

int Font::BoldMode(char *s)
{
	if (!(*(++s))) return 0;

	if (*s == '+') bold = 1;
	else if (*s == '-') bold = 0;
	else return 0;

	return 2;
}

int Font::GetCharHits(char *s, char f)
{
	int hits;
	for (hits = 0; *s; ++s)
		if (*s == f) ++hits;

	return hits;
}

int Font::GetTrueLen(char *s)
{
	int result = 0;

	for (; *s; ++s, ++result)
	{	
		if (*s == TEXT_STYLE_KEYWORD)
		{
			if (!(*(++s))) return result;

			switch (*s)
			{
				case 'a':
				case 'c':
					while (*s && *s != '}') ++s;
					++s;
					break;
				case 'b':
				case 'i':
					++s;
					break;
				default:
					break;
			}
		}
		else if (*s == '^')
		{
			++s; if (!s) return result;
			++s; if (!s) return result;
		}
	}
	return result;
}

void Font::RemoveKeyCodes(char *dest, char *source)
{
	if (!dest) return;

	for (; *source; ++source)
	{	
		if (*source == TEXT_STYLE_KEYWORD)
		{
			if (!(*(++source))) return;

			switch (*source)
			{
				case 'a':
				case 'c':
					while (*source && *source != '}') ++source;
					++source;
					break;
				case 'b':
				case 'i':
					++source;
					break;
				default:
					break;
			}
		}
		else if (*source == '^')
		{
			if (!(*(++source))) return;	// skip '^'
			if (!(*(++source))) return;	// skip value
		}
		else
		{
			*dest = *source;
			++dest;
		}
	}
}

void Font::ColorCopy(float *dest, float *src)
{
	dest[0] = src[0]; dest[1] = src[1]; dest[2] = src[2]; dest[3] = src[3];
}

void Font::ForeColorReset(void)
{
	ColorCopy(fgColor, white);
}

void Font::ShadowColorReset(void)
{
	ColorCopy(bgColor, gray);
}

void Font::GradientColorReset(void)
{
	ColorCopy(gdColor, gray);
}

void Font::Shadow(void)
{
	shadow = 1;
}

void Font::Gradient(void)
{
	gradient = 1;
}

void Font::Color3f(float r, float g, float b)
{
	fgColor[0] = r; fgColor[1] = g; fgColor[2] = b; fgColor[3] = 1.0;
}

void Font::Color4f(float r, float g, float b, float a)
{
	fgColor[0] = r; fgColor[1] = g; fgColor[2] = b; fgColor[3] = a;
}

void Font::Color3fv(float *clr)
{
	fgColor[0] = clr[0]; fgColor[1] = clr[1]; fgColor[2] = clr[2]; fgColor[3] = 1.0;
}

void Font::Color4fv(float *clr)
{
	fgColor[0] = clr[0]; fgColor[1] = clr[1]; fgColor[2] = clr[2]; fgColor[3] = clr[3];
}

void Font::ShadowColor3f(float r, float g, float b)
{
	bgColor[0] = r; bgColor[1] = g; bgColor[2] = b; bgColor[3] = 1.0;
}

void Font::ShadowColor4f(float r, float g, float b, float a)
{
	bgColor[0] = r; bgColor[1] = g; bgColor[2] = b; bgColor[3] = a;
}

void Font::ShadowColor3fv(float *clr)
{
	bgColor[0] = clr[0]; bgColor[1] = clr[1]; bgColor[2] = clr[2]; bgColor[3] = 1.0;
}

void Font::ShadowColor4fv(float *clr)
{
	bgColor[0] = clr[0]; bgColor[1] = clr[1]; bgColor[2] = clr[2]; bgColor[3] = clr[3];
}

void Font::GradientColor3f(float r, float g, float b)
{
	gdColor[0] = r; gdColor[1] = g; gdColor[2] = b; gdColor[3] = 1.0;
}

void Font::GradientColor4f(float r, float g, float b, float a)
{
	gdColor[0] = r; gdColor[1] = g; gdColor[2] = b; gdColor[3] = a;
}

void Font::GradientColor3fv(float *clr)
{
	gdColor[0] = clr[0]; gdColor[1] = clr[1]; gdColor[2] = clr[2]; gdColor[3] = 1.0;
}

void Font::GradientColor4fv(float *clr)
{
	gdColor[0] = clr[0]; gdColor[1] = clr[1]; gdColor[2] = clr[2]; gdColor[3] = clr[3];
}
