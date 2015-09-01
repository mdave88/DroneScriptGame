//-----------------------------------------------------------------------------
// Console
//-----------------------------------------------------------------------------

#include "cl.h"
#include <math.h>

#ifdef WIN32
	#include <tchar.h>				// for _vsntprintf
	#define beep()	Beep(700, 1)	// Console beep.
#else
	#include <stdlib.h>
	#include <strings.h>
	#include <stdio.h>
	#include <stdarg.h>
	#include <sys/va_list.h>
	#define _vsntprintf vsnprintf
	#if 0
		#include <curses.h>
	#else
		#define beep() ;
	#endif
#endif

#include <assert.h>

#include <GL/glut.h>

// Some variables used frequently in rendering
int cursorxpos, prompt_width, len_to_cursor, width_to_cursor, font_size;

Var cons_consoleKey("cons_consoleKey", 167, VF_PERSISTENT);
Var cons_enablelog("cons_enablelog", 1);

///////////////////////////////////////////////////////////////////////////////

// Constructor
Console::Console(void)
{
	gVars->RegisterVar(cons_consoleKey);
	gVars->RegisterVar(cons_enablelog);
}

void Console::Init(int wwidth, int wheight)
{
	console_move = false, console_resize = false, cons_scroll = false;
	cons_resizefrom = 0;
	lButton = false, mButton = false, rButton = false;
	clic_delay = 0;							// Double-clic time counter

	skin					= new Skin;

	screen_Width			= wwidth;
	screen_Height			= wheight;

	initvScrollY			= 0;

	displayFunc				= NULL;

	font					= NULL;
	minifont				= NULL;

	timer					= new Timer;

	cursorSymbol			= '|';
	isCursorCentered		= false;

	mouse.x					= 0;
	mouse.y					= 0;
	mouse.button[0] = mouse.button[1] = mouse.button[2] = BUTTON_UP;

	SetPrompt("]");

	type					= C_FLYING;

	searchMode				= SEQUENCED;

	nbrAllocatedLines		= LINESBLOC;
	nbrUsedLines			= 1;

	currently_used_message_lines = 0;

	state					= CLOSED;

	nbrTrueLines			= 1;

	enableOpeningClosingAnimations = true;
	opening_or_closing_progress = 1.f;

	cursorPos				= 0;
	scrollVal				= 0;

	consoleIsMaximized		= false;
	consoleIsRolledUp		= false;

	showMessages			= true;
	addToMessages			= true;
	msgLifeLen				= 2.5f;
	openSpeed				= 4.f;
	closeSpeed				= 4.f;
	cursorSpeed				= 2.f;
	doubleclicSpeed			= 200.f;
	mouseRepeatSpeed		= 50.f;
	insert					= false;

	isActive				= false;

	mouse_repeat_start		= 0;
	isRepeating				= false;

	int i;

	consoleLines = (ConsLine*) malloc(nbrAllocatedLines*sizeof(ConsLine));
	for (i = 0; i < nbrAllocatedLines; ++i) consoleLines[i].Init(font);

	for (i = 0; i < C_NMESSAGELINES; ++i)
	{
		messageLines[i].Init(minifont);
		messageLife[i] = 0.f;
	}

	if (type == C_FLYING)
	{
		Resize(screen_Width - 60, screen_Height/2);
		SetTopPos(30);
		SetLeftPos(30);
	}
	else if (type == C_STATIC)
	{
		Resize(screen_Width, screen_Height/2);
		SetTopPos(0);
		SetLeftPos(0);		
	}

	Recalculate_NLines();
}

void Console::GetStats(void)
{
	Insert("^6Console stats :");
	Insert("   Allocated lines : %d", this->nbrAllocatedLines);
	Insert("   Used lines      : %d", this->nbrUsedLines);
	Insert("   Nbr true lines  : %d", this->nbrTrueLines);
	Insert("   Max Lines       : %d", this->nbrMaxLines);
}

// Destructeur
Console::~Console (void)
{
	gVars->UnregisterVar(cons_enablelog);
	gVars->UnregisterVar(cons_consoleKey);
	Destroy();
}

void Console::Destroy (void)
{
	for (int i = 0; i < C_NMESSAGELINES; ++i) messageLines[i].Shut();
	for (int i = 0; i < nbrAllocatedLines; ++i) consoleLines[i].Shut();
	free(consoleLines); consoleLines = NULL;

	if (font) delete font; font = NULL;
	if (minifont) delete minifont; minifont = NULL;
	if (timer) delete timer; timer = NULL;
	if (skin) delete skin; skin = NULL;
}

void Console::SetWindowSettings(int wwidth, 
								int wheight,
								int wleft,
								int wtop)
{
	if (wwidth > 0) screen_Width = wwidth;
	if (wheight > 0) screen_Height = wheight;
	if (wleft >= 0) screen_xPosition = wleft;
	if (wtop >= 0) screen_yPosition = wtop;

	if (type == C_STATIC)
	{
		Resize(screen_Width-skin->vScrollBar->coords.width, screen_Height/2);
	}
	else if (IsMaximized())
	{
		if (skin->titleBar->border == TB_TOP)
			Resize(screen_Width-skin->vScrollBar->coords.width, screen_Height-skin->border->topBorder.coords.height);
		else if (skin->titleBar->border == TB_BOTTOM)
			Resize(screen_Width-skin->vScrollBar->coords.width, screen_Height-skin->border->bottomBorder.coords.height);
		else if (skin->titleBar->border == TB_LEFT)
			Resize(screen_Width-skin->vScrollBar->coords.width-skin->border->leftBorder.coords.width, screen_Height);
		else if (skin->titleBar->border == TB_RIGHT)
			Resize(screen_Width-skin->vScrollBar->coords.width-skin->border->rightBorder.coords.width, screen_Height);
		else
			Resize(screen_Width-skin->vScrollBar->coords.width, screen_Height);
	}
	else
	{
		int new_width = screen_Width - leftPos - /*skin->border->leftBorder.coords.width - */skin->border->rightBorder.coords.width - skin->vScrollBar->coords.width;
		int new_height = screen_Height - topPos - /*skin->border->topBorder.coords.height - */skin->border->bottomBorder.coords.height;
		if (new_width < width) Resize(new_width, -1);
		if (new_height < height) Resize(-1, new_height);
	}
}

int Console::LoadSkin(char *filename)
{
	int res = skin->LoadSkin(filename, this->width, this->height, this->font);
	if (!res) Insert("^1ERROR: An error occured while loading the skin.");
	return res;
}

void Console::SetCharTable(char *fontname, int n)
{
	FILE *fp = fopen(fontname, "r");
	if (fp != NULL)
	{
		fclose(fp);

		if (n == 1 || n == 0)
		{
			if (font) delete font;
			font = new Font;
			if (!font->Load(fontname))
				Insert("%s : An error occured while loading %s", fontname);
			for (int i = 0; i < nbrAllocatedLines; ++i) consoleLines[i].SetFont(font);
			prompt.SetFont(font);
		}
		if (n == 2 || n == 0)
		{
			if (minifont) delete minifont;
			minifont = new Font;
			if (!minifont->Load(fontname))
				Insert("%s : An error occured while loading %s", fontname);
			for (int i = 0; i < C_NMESSAGELINES; ++i) messageLines[i].SetFont(minifont);
		}	
	}
	else printf("The file %s does not exist.\n", fontname);

	Recalculate_NLines();
}	

void Console::SetTitle(char *s)
{
//	memset(title, '\0', CONSOLE_LINELENGTH);
//	strncpy(title, s, CONSOLE_LINELENGTH);
}

void Console::SetPrompt(char *s)
{
	bool must_readapt_len = false;

	if (strlen(s) > MAXPROMPTLEN)
	{
		must_readapt_len = true;
		while (strlen(s) > MAXPROMPTLEN) ++s;
	}
	char tmp[MAXPROMPTLEN] = { '\0' };
	strncpy(tmp, s, MAXPROMPTLEN);

	if (must_readapt_len)
		tmp[0] = tmp[1] = tmp[2] = '.';

	prompt.SetFont(font);
	prompt.SetContent(tmp);
}

void Console::SetCursorSymbol(char c)
{
	cursorSymbol = c;
}

void Console::SetCursorSpeed(float f)
{
	if (f < 0) return;
	cursorSpeed = f;
}

void Console::Open(void)
{
	if (enableOpeningClosingAnimations && type == C_STATIC) state = OPENING;
	else state = OPEN;
	isActive = true;
}

void Console::Close(void)
{
	if (enableOpeningClosingAnimations && type == C_STATIC) state = CLOSING;
	else state = CLOSED;
	isActive = false;
}

void Console::ToggleType(void)
{
	if (type == C_FLYING) SetType(C_STATIC);
	else SetType(C_FLYING);
}
	
void Console::SetType(enum_ConsoleType console_type)
{
	if (type == C_FLYING && console_type == C_STATIC)
	{
		if (!consoleIsMaximized)
		{
			hBackup = height;
			wBackup = width;
			lBackup = leftPos;
			tBackup = topPos;
		}
		
		topPos = 0;
		leftPos = ((skin->vScrollBar->pos == VSP_RIGHT)?0:skin->vScrollBar->coords.width);
		type = console_type;

		Resize(screen_Width-skin->vScrollBar->coords.width, screen_Height/2);
	}
	else if (type == C_STATIC && console_type == C_FLYING)
	{
		type = console_type;

		if (consoleIsMaximized)
		{
			if (skin->titleBar->border == TB_TOP)
			{
				topPos = skin->border->topBorder.coords.height;
				leftPos = ((skin->vScrollBar->pos == VSP_RIGHT)?0:skin->vScrollBar->coords.width);
				Resize(screen_Width-skin->vScrollBar->coords.width, screen_Height-skin->border->topBorder.coords.height);
			}
			else if (skin->titleBar->border == TB_BOTTOM)
			{
				topPos = 0;
				leftPos = ((skin->vScrollBar->pos == VSP_RIGHT)?0:skin->vScrollBar->coords.width);
				Resize(screen_Width-skin->vScrollBar->coords.width, screen_Height-skin->border->bottomBorder.coords.height);
			}
			else if (skin->titleBar->border == TB_LEFT)
			{
				topPos = 0;
				leftPos = skin->border->leftBorder.coords.width + ((skin->vScrollBar->pos == VSP_RIGHT)?0:skin->vScrollBar->coords.width);
				Resize(screen_Width-skin->vScrollBar->coords.width-skin->border->leftBorder.coords.width, screen_Height);
			}
			else if (skin->titleBar->border == TB_RIGHT)
			{
				topPos = 0;
				leftPos = ((skin->vScrollBar->pos == VSP_RIGHT)?0:skin->vScrollBar->coords.width);
				Resize(screen_Width-skin->vScrollBar->coords.width-skin->border->rightBorder.coords.width, screen_Height);
			}
			else
			{
				topPos = 0;
				leftPos = ((skin->vScrollBar->pos == VSP_RIGHT)?0:skin->vScrollBar->coords.width);
				Resize(screen_Width-skin->vScrollBar->coords.width, screen_Height);
			}
		}
		else
		{
			topPos = tBackup;
			leftPos = lBackup;
			Resize(wBackup, hBackup);
		}
	}
}

int Console::Resize(int newwidth, int newheight)
{
	int result = 0;

	if (newwidth > 100)
	{
		width = newwidth;
		if (skin->background.sprite != NULL) skin->background.sprite->SetSize(width, -1);
	}
	else ++result;

	if (!font || newheight > 3*font->GetSize())
	{
		height = newheight;
		if (scrollVal > (nbrTrueLines - nbrMaxLines))
			scrollVal = (nbrTrueLines - nbrMaxLines);
		if (scrollVal < 0) scrollVal = 0;
		if (skin->background.sprite != NULL) skin->background.sprite->SetSize(-1, height);
	}
	else ++result;

	Recalculate_NLines();

	return result;
}

void Console::Maximize(void)
{
	if (consoleIsMaximized || type != C_FLYING) return; // only applicable if not already maximised and flying mode

	hBackup = height;
	wBackup = width;
	lBackup = leftPos;
	tBackup = topPos;

	if (skin->titleBar->border == TB_TOP)
	{
		topPos = skin->border->topBorder.coords.height;
		leftPos = ((skin->vScrollBar->pos == VSP_RIGHT)?0:skin->vScrollBar->coords.width);
		Resize(screen_Width-skin->vScrollBar->coords.width, screen_Height-skin->border->topBorder.coords.height);
	}
	else if (skin->titleBar->border == TB_BOTTOM)
	{
		topPos = 0;
		leftPos = ((skin->vScrollBar->pos == VSP_RIGHT)?0:skin->vScrollBar->coords.width);
		Resize(screen_Width-skin->vScrollBar->coords.width, screen_Height-skin->border->bottomBorder.coords.height);
	}
	else if (skin->titleBar->border == TB_LEFT)
	{
		topPos = 0;
		leftPos = skin->border->leftBorder.coords.width + ((skin->vScrollBar->pos == VSP_RIGHT)?0:skin->vScrollBar->coords.width);
		Resize(screen_Width-skin->vScrollBar->coords.width-skin->border->leftBorder.coords.width, screen_Height);
	}
	else if (skin->titleBar->border == TB_RIGHT)
	{
		topPos = 0;
		leftPos = ((skin->vScrollBar->pos == VSP_RIGHT)?0:skin->vScrollBar->coords.width);
		Resize(screen_Width-skin->vScrollBar->coords.width-skin->border->rightBorder.coords.width, screen_Height);
	}
	else
	{
		topPos = 0;
		leftPos = ((skin->vScrollBar->pos == VSP_RIGHT)?0:skin->vScrollBar->coords.width);
		Resize(screen_Width-skin->vScrollBar->coords.width, screen_Height);
	}

	consoleIsMaximized = skin->isMaximized = true;
	Recalculate_NLines();
}

void Console::Rollup(void)
{
	consoleIsRolledUp = skin->isRolledUp = true;
}

void Console::Unroll(void)
{
	consoleIsRolledUp = skin->isRolledUp = false;
}

void Console::Unmaximize(void)
{
	if (!consoleIsMaximized || type != C_FLYING) return;

	consoleIsMaximized = skin->isMaximized = false;
	
	topPos = tBackup;
	leftPos = lBackup;
	Resize(wBackup, hBackup);

	Recalculate_NLines();
}

bool Console::IsMaximized(void)
{
	return consoleIsMaximized;
}

bool Console::IsRolledUp(void)
{
	return consoleIsRolledUp;
}

void Console::ToggleMaximisation(void)
{
	if (consoleIsMaximized) Unmaximize();
	else Maximize();
}

void Console::ToggleRolling(void)
{
	if (consoleIsRolledUp) Unroll();
	else Rollup();
}

void Console::SetTopPos(int newtop, int test, int include_skin)
{
	if (test >= 1 && (consoleIsMaximized || type != C_FLYING)) return;

	topPos = newtop;

	if (include_skin) topPos += skin->border->topBorder.coords.height;

	if (test >= 0 && test <= 1)
	{
		if (topPos < 0) topPos = 0;
		if ((topPos + height) > screen_Height) topPos = screen_Height - height;
	}
}

void Console::SetLeftPos(int newleft, int test, int include_skin)
{
	if (test >= 1 && (consoleIsMaximized || type != C_FLYING)) return;

	leftPos = newleft;

	if (include_skin) leftPos += skin->border->leftBorder.coords.width;

	if (test >= 0 && test <= 1)
	{
		if (leftPos < 0) leftPos = 0;
		if ((leftPos + width) > screen_Width) leftPos = screen_Width - width;
	}
}

void Console::SetVScrollYPos(int y, int y1, int mos)
{
	if (mos > 0)
	{
		int t = screen_Height - skin->vScrollBar->topScroller.coords.ypos;
		int b = screen_Height - skin->vScrollBar->bottomScroller.coords.ypos;
		int h = skin->vScrollBar->bottomScroller.coords.height;
		if (y1 >= 0) initvScrollY = t + ((b + h) - t)/2 - y1;
		else initvScrollY = 0;
	}

	int yoff = y + initvScrollY;
	
	scrollVal = (int) ((((float)yoff-(float)(topPos+height-skin->vScrollBar->up.coords.height-skin->vScrollBar->height/2)))*
					   (float)(nbrMaxLines-nbrTrueLines)/(float)(height-skin->vScrollBar->up.coords.height-skin->vScrollBar->height-skin->vScrollBar->down.coords.height));
	if (scrollVal < 0) scrollVal = 0;
	if (scrollVal > nbrTrueLines - nbrMaxLines)
		scrollVal = nbrTrueLines - nbrMaxLines;
	if (nbrTrueLines < nbrMaxLines) scrollVal = 0;
}

void Console::Recalculate_NLines(void)
{
	if (!font) return;

	nbrMaxLines = height / font->GetSize();

	nbrTrueLines = 1;
	UpdateConsoleLinesWidth(width-font->GetSize());
}

void Console::UpdateConsoleLinesWidth(int w)
{
	for (int i = 0; i < nbrUsedLines - 1; ++i)
		nbrTrueLines += consoleLines[i].Update(w);
}

int Console::GetFontHeight(int n)
{
	if (n == 2) return font->GetSize();
	return minifont->GetSize();
}

void Console::SetFontRatio(float ratio, int n)
{
	if (ratio <= 0) return;

	if (n == 1)
	{
		font->SetRatio(ratio);
	}
	else if (n == 2)
	{
		minifont->SetRatio(ratio);
	}
	else 
	{
		font->SetRatio(ratio);
		minifont->SetRatio(ratio);
	}

	Recalculate_NLines();
}

float Console::GetFontRatio(int n)
{
	if (n == 2) return minifont->GetRatio();
	return font->GetRatio();
}

void Console::SetFontScale(float scale, int n)
{
	if (scale <= 0) return;

	if (n == 1)
	{
		font->SetScale(scale);
	}
	else if (n == 2)
	{
		minifont->SetScale(scale);
	}
	else 
	{
		font->SetScale(scale);
		minifont->SetScale(scale);
	}

	Recalculate_NLines();
}

float Console::GetFontScale(int n)
{
	if (n == 2) return minifont->GetScale();
	return font->GetScale();
}

void Console::ScrollConsole(enum_ConsoleScrollDir dir)
{
	Recalculate_NLines();
	if (nbrTrueLines < nbrMaxLines)
	{
		beep();
		return;
	}

	if (dir == UP)
	{
		if ((scrollVal + 1) > (nbrTrueLines - nbrMaxLines)) { beep(); return; }
		++scrollVal;		
	}
	else if (dir == DOWN)
	{
		if ((scrollVal - 1) < 0) { beep(); return; }
		--scrollVal;		
	}
	else if (dir == TOP)	scrollVal = nbrTrueLines - nbrMaxLines;
	else if (dir == BOTTOM)	scrollVal = 0;
}

void Console::AddLine(void)
{
	// Lines are allocated by blocs (LINESBLOC) to avoid frequent allocations
	if (nbrUsedLines == nbrAllocatedLines)
	{
		consoleLines = (ConsLine*) realloc(consoleLines, (nbrAllocatedLines+LINESBLOC)*sizeof(ConsLine));
		if (consoleLines == NULL)
		{
			free (consoleLines);
			Insert("Error (re)allocating memory");
			exit (1);
		}
		for (int i = nbrAllocatedLines; i < nbrAllocatedLines+LINESBLOC; ++i) consoleLines[i].Init(font);
		nbrAllocatedLines += LINESBLOC;
	}
	++nbrUsedLines;
	cursorPos = 0;

	Recalculate_NLines();
}

void Console::Insert(const char* s, ...)
{
	if (!s || !strlen(s)) return;

	va_list	msg;
	char buffer[CONSOLE_LINELENGTH] = { '\0' };
	
	va_start(msg, s);
	_vsntprintf(buffer, CONSOLE_LINELENGTH, s, msg);
	va_end(msg);

	if (strlen(buffer) <= 0) return;

	// Check if gCommand is waiting for output
	if (gCommands->IsWaitingForOutput())
	{
		gCommands->GetOutput(buffer);
		return;
	}

	if (cons_enablelog.ivalue) gLogFile->Insert(buffer);

	char final[CONSOLE_LINELENGTH] = { '\0' };	
	scrollVal = 0;
	int l = (int) strlen(buffer);

	// Parse the input string and
	//  -> replace tabulations with spaces
	//  -> remove HTML syntax
	//  -> recopy other characters
	for (int i = 0, j = 0; i < l; ++i)
	{
		if (buffer[i] == '\t') for (int k = 0; k < TAB_LEN; ++k) final[j++] = ' ';
		else if (buffer[i] == '<')
		{
			// analyse string to find if it is a valid tag
			int backup_pos = i;
			while (i < l && buffer[i] != '>') ++i;
			if (i < l)
			{
				if (buffer[i-1] != '/' && buffer[backup_pos+1] != '/')
				{
					// search for closing tag
					// get tag
					char tag[CONSOLE_LINELENGTH] = { '\0' };
					strncpy(tag, &buffer[backup_pos+1], i-backup_pos-1);
					int taglen = (int) strlen(tag);
					while (++i < l)
					{
						if (i < l-taglen-2 && buffer[i] == '<' && buffer[i+1] == '/' &&
							!strncmp(&buffer[i+2], tag, taglen) && buffer[i+2+taglen] == '>')
						{
							// closing tag found
							i = backup_pos+taglen+1;	// skip '/', '>' and tag length
							break;
						}
					}
					if (i == l)
					{
						// no closing tag found, recopy the full string
						i = backup_pos;
						final[j++] = buffer[i];
					}
				}
			}
			else
			{
				// no closing tag (it is a single "smaller" character)
				i = backup_pos;
				final[j++] = buffer[i];
			}
		}
		else final[j++] = buffer[i];	// default char, recopy it
	}

	// Add the line in messages (miniconsole) lines
	if (addToMessages) AddMessageLine(final);

	if (consoleLines[nbrUsedLines-1].length)
	{
		char backupCommand[CONSOLE_LINELENGTH] = { '\0' };	//Save the current command
		strncpy(backupCommand, consoleLines[nbrUsedLines-1].content, CONSOLE_LINELENGTH);
		consoleLines[nbrUsedLines-1].SetContent(final);

		AddLine();
		consoleLines[nbrUsedLines-1].SetContent(backupCommand);
	}
	else
	{
		consoleLines[nbrUsedLines-1].SetContent(final);
		AddLine();
	}

	Recalculate_NLines();

	// Call the display function (if defined) so the console can directly
	// update itself. It makes things smoother if a lot of lines are
	// inserted together.
	timer->Refresh();
	if (timer->frametime < 0.03) UpdateDisplay();
}

void Console::AddMessageLine(char* s)
{
	if (currently_used_message_lines < C_NMESSAGELINES) ++currently_used_message_lines;
	else
		for (int i = 0; i < currently_used_message_lines; ++i)
		{
			messageLines[i].ReInit();
			messageLines[i].SetContent(messageLines[i+1].content);
			messageLife[i] = messageLife[i+1];
		}

	messageLines[currently_used_message_lines-1].SetContent(s);
	messageLife[currently_used_message_lines-1] = 0.f;
}

void Console::UpdateMessageLines(void)
{
	int i;

	// Update the messages life times
	for (i = 0; i < currently_used_message_lines; ++i)
		messageLife[i] += (float) timer->frametime;

	for (i = 0; i < currently_used_message_lines; ++i)
	{
		// Kill too old messages
		if (messageLife[i] >= msgLifeLen)
		{
			for (int j = i; j < currently_used_message_lines; ++j)
			{
				messageLines[j].ReInit();
				messageLines[j].SetContent(messageLines[j+1].content);
				messageLife[j] = messageLife[j+1];
			}
			messageLines[--currently_used_message_lines].ReInit();
			--i;
		}
	}
}

void Console::Update(void)
{
	timer->Refresh();

	if (!font) return;

	CheckScrollWithMouse();
	UpdateMessageLines();

	// Display messages if the console is closed, closing or opening
	if (state == CLOSED && showMessages && currently_used_message_lines)
	{
		for (int i = 0; i < currently_used_message_lines; ++i)
		{
			minifont->Draw(0, screen_Height-(i+1)*minifont->GetSize(), "^0%s", messageLines[i].content);
		}
	}
	if (state == CLOSED) return;

	// Opening/Closing animation
	if (type == C_STATIC && enableOpeningClosingAnimations)
	{
		if (state == OPENING) opening_or_closing_progress += (float)timer->frametime*openSpeed;
		else if (state == CLOSING) opening_or_closing_progress -= (float)timer->frametime*closeSpeed;
		if (opening_or_closing_progress >= 1.f && state == OPENING) { state = OPEN; opening_or_closing_progress = 1.f; } 
		if (opening_or_closing_progress <= 0.f && state == CLOSING) { state = CLOSED; opening_or_closing_progress = 0.f;}
	}
	else
	{
		if (state == OPENING)
		{
			state = OPEN;
			opening_or_closing_progress = 1.f;
		} 
		else if (state == CLOSING)
		{ 
			state = CLOSED;
			opening_or_closing_progress = 0.f;
		}		
	}
	
	DrawBackground();

	if (consoleIsRolledUp) return;

	// Display the console lines
	int cons_i, cons_j;
	if (nbrTrueLines < nbrMaxLines) cons_j = nbrTrueLines - 1;
	else cons_j = nbrMaxLines - 1;
	int pos_j = screen_Height-topPos+(int)((height+topPos)*(1.f-opening_or_closing_progress))-(cons_j+1)*font->GetSize();

	// Command line
	cons_i = nbrUsedLines - 1;

	cursorxpos;
	prompt_width = prompt.GetWidth();
	len_to_cursor = consoleLines[cons_i].length-cursorPos;
	width_to_cursor = consoleLines[cons_i].GetWidth(len_to_cursor);
	font_size = font->GetSize();
	if (consoleLines[cons_i].GetWidth() + prompt_width < width - font_size)
	{	// Normal case : the command line can be fully displayed in the console
		font->Draw(leftPos, pos_j, "^0%s%s", prompt.content, consoleLines[cons_i].content);
		cursorxpos = leftPos+prompt_width+width_to_cursor;
	}
	else
	{	// The command line is larger than the console width
		char buffer[CONSOLE_LINELENGTH] = { '\0' };

		if (width_to_cursor + prompt_width > width - font_size)
		{	// If the cursor is out of the console width
			// The command line needs to be scrolled and clamped to suit with console width
			int s = consoleLines[cons_i].FindChar(-width+prompt_width+font_size, len_to_cursor-1);
			int l = len_to_cursor-s;
			strncpy(buffer, &(consoleLines[cons_i].content[s]), l);
			font->Draw(leftPos, pos_j, "^0%s%s", prompt.content, buffer);
			cursorxpos = leftPos+prompt_width+consoleLines[cons_i].GetWidth(l, s);
		}
		else
		{	// The cursor is in the console width
			// The command line needs to be clamped to suit with console width
			int l = consoleLines[cons_i].GetLen(width-prompt_width-font_size);
			strncpy(buffer, consoleLines[cons_i].content, l);
			font->Draw(leftPos, pos_j, "^0%s%s", prompt.content, buffer);
			cursorxpos = leftPos+prompt_width+width_to_cursor;
		}
	}

	// Cursor
	if ((long)(timer->fTime*cursorSpeed)%2)	// cursor blinking effect
	{
		int d = 0;
		if (isCursorCentered) d = 0;
		else d = (int) (font->GetWidth(cursorSymbol)/2.f);
		font->Draw(cursorxpos-d, pos_j, "^%d%c", insert?1:0, cursorSymbol);
	}

	pos_j += font_size;
	--cons_j;
	--cons_i;

	// Other lines
	int t = scrollVal;
	while (cons_j >= 0 && cons_i >= 0)
	{
		for (int k = consoleLines[cons_i].nlines - 1; k >= 0; --k)
		{
			if (t <= 0)
			{
				font->Draw(leftPos, pos_j, consoleLines[cons_i].GetContent(k));

				pos_j += font_size;
				--cons_j;
			}
			else --t;
			if (cons_j < 0) break;
		}
		--cons_i;
	}
}

void Console::DrawBackground(void)
{
	SetModes(_2D_MODE_);
	glDisable(GL_CULL_FACE);

	this->skin->UpdateSkin(leftPos, screen_Height - (int)(topPos - (height + topPos)*(1.f-opening_or_closing_progress)), width, height);
	this->skin->UpdateScroller(1.f - (float)scrollVal/(float)(nbrTrueLines-nbrMaxLines), (float)nbrMaxLines/(float)nbrTrueLines);
	this->skin->DrawSkin(width, height);

	// Resize zones
	if (type == C_FLYING && !consoleIsMaximized)
	{
		int imors;
		if ((imors = IsMouseOnResizeSquare()) > 0)
		{
			glColor4f(0.75f, 0.75f, 0.75f, 0.5f);
			glBegin(GL_QUADS);
				if (imors == 1)
				{
					glVertex2i(leftPos, screen_Height - topPos);
					glVertex2i(leftPos + RESIZE_SQUARE, screen_Height - topPos);
					glVertex2i(leftPos + RESIZE_SQUARE, screen_Height - topPos - RESIZE_SQUARE);
					glVertex2i(leftPos, screen_Height - topPos - RESIZE_SQUARE);
					glVertex2i(leftPos, screen_Height - topPos);
				}
				if (imors == 2)
				{
					glVertex2i(leftPos + width - RESIZE_SQUARE, screen_Height - topPos);
					glVertex2i(leftPos + width, screen_Height - topPos);
					glVertex2i(leftPos + width, screen_Height - topPos - RESIZE_SQUARE);
					glVertex2i(leftPos + width - RESIZE_SQUARE, screen_Height - topPos - RESIZE_SQUARE);
					glVertex2i(leftPos + width - RESIZE_SQUARE, screen_Height - topPos);
				}
				if (imors == 3)
				{
					glVertex2i(leftPos, screen_Height - topPos - height + RESIZE_SQUARE);
					glVertex2i(leftPos + RESIZE_SQUARE, screen_Height - topPos - height + RESIZE_SQUARE);
					glVertex2i(leftPos + RESIZE_SQUARE, screen_Height - topPos - height);
					glVertex2i(leftPos, screen_Height - topPos - height);
					glVertex2i(leftPos, screen_Height - topPos - height + RESIZE_SQUARE);
				}
				if (imors == 4)
				{
					glVertex2i(leftPos + width - RESIZE_SQUARE, screen_Height - topPos - height + RESIZE_SQUARE);
					glVertex2i(leftPos + width, screen_Height - topPos - height + RESIZE_SQUARE);
					glVertex2i(leftPos + width, screen_Height - topPos - height);
					glVertex2i(leftPos + width - RESIZE_SQUARE, screen_Height - topPos - height);
					glVertex2i(leftPos + width - RESIZE_SQUARE, screen_Height - topPos - height + RESIZE_SQUARE);
				}
			glEnd();
		}
	}
	
	SetModes(_3D_MODE_);
}

void Console::AddChar(char c)
{
	if (consoleLines[nbrUsedLines-1].length > CONSOLE_LINELENGTH)
	{
		beep();
		return;
	}

//	if (c == TEXT_STYLE_KEYWORD) return;

	if (c != DELETE_CHAR)
	{
		if (cursorPos > 0)
		{
			if (!insert)
			{
				char begin[CONSOLE_LINELENGTH] = { '\0' };
				char end[CONSOLE_LINELENGTH] = { '\0' };

				strncpy(begin, consoleLines[nbrUsedLines-1].content, consoleLines[nbrUsedLines-1].length-cursorPos);
				strncpy(end, &consoleLines[nbrUsedLines-1].content[consoleLines[nbrUsedLines-1].length-cursorPos], CONSOLE_LINELENGTH);
				
				strcat(begin, " ");
				
				begin[strlen(begin)-1] = c;
				
				strncat(begin, end, CONSOLE_LINELENGTH - 1);
				consoleLines[nbrUsedLines-1].SetContent(begin);
			}
			else consoleLines[nbrUsedLines-1].content[consoleLines[nbrUsedLines-1].length-cursorPos--] = c;
		}
		else
		{
			strcat(consoleLines[nbrUsedLines-1].content, " ");
			consoleLines[nbrUsedLines-1].content[consoleLines[nbrUsedLines-1].length++] = c;
		}
	}
	else	// Delete a char with 'delete' command
	{
		if (cursorPos > 0)
		{
			char begin[CONSOLE_LINELENGTH] = { '\0' };
			char end[CONSOLE_LINELENGTH] = { '\0' };

			strncpy(begin, consoleLines[nbrUsedLines-1].content, consoleLines[nbrUsedLines-1].length-cursorPos);
			strncpy(end, &consoleLines[nbrUsedLines-1].content[consoleLines[nbrUsedLines-1].length-cursorPos+1], CONSOLE_LINELENGTH);

			strncat(begin, end, CONSOLE_LINELENGTH - 1);
			consoleLines[nbrUsedLines-1].SetContent(begin);

			--cursorPos;
		}
		else beep();
	}
}

void Console::DelChar (void)
{
	if (consoleLines[nbrUsedLines-1].length == 0 || consoleLines[nbrUsedLines-1].length == cursorPos)
	{
		beep();
		return;
	}

	if (cursorPos > 0)
	{
		if (cursorPos < consoleLines[nbrUsedLines-1].length)
		{
			char begin[CONSOLE_LINELENGTH] = { '\0' };
			char end[CONSOLE_LINELENGTH] = { '\0' };
			
			strncpy(begin, consoleLines[nbrUsedLines-1].content, consoleLines[nbrUsedLines-1].length-cursorPos-1);
			strncpy(end, &consoleLines[nbrUsedLines-1].content[consoleLines[nbrUsedLines-1].length-cursorPos], CONSOLE_LINELENGTH);
			
			strncat(begin, end, CONSOLE_LINELENGTH);
			consoleLines[nbrUsedLines-1].SetContent(begin);
		}
	}
	else
	{
		if (consoleLines[nbrUsedLines-1].length)
		{
			consoleLines[nbrUsedLines-1].content[consoleLines[nbrUsedLines-1].length-1] = '\0';
			--consoleLines[nbrUsedLines-1].length;
		}
	}
}

void Console::Clear(void)
{
	int i;
	for (i = 0; i < nbrAllocatedLines; ++i) consoleLines[i].Shut();
	free(consoleLines);
	nbrUsedLines			= 1;
	cursorPos				= 0;
	scrollVal				= 0;
	nbrAllocatedLines		= LINESBLOC;
	consoleLines			= (ConsLine*) malloc(nbrAllocatedLines*sizeof(ConsLine));
	for (i = 0; i < nbrAllocatedLines; ++i) consoleLines[i].Init(font);
	Recalculate_NLines();
}

void Console::ReInitCurrentCommand(void)
{
	cursorPos = 0;
	consoleLines[nbrUsedLines-1].ReInit();
}

char* Console::GetCurrentCommand (void)
{
	return consoleLines[nbrUsedLines-1].content;
}

void Console::SetCurrentCommand (char* s, ...)
{
	va_list	msg;
	char buffer[CONSOLE_LINELENGTH] = { '\0' };
	
	va_start (msg, s);
	_vsntprintf (buffer, CONSOLE_LINELENGTH, s, msg);
	va_end (msg);

	consoleLines[nbrUsedLines-1].SetContent(buffer);
}

int Console::GetNbrUsedLines (void)
{
	return nbrUsedLines;
}

void Console::SetState(enum_ConsoleStates cs)
{
	state = cs;

	if (!enableOpeningClosingAnimations)
	{
		if (state == OPENING) state = OPEN;
		if (state == CLOSING) state = CLOSED;
	}

	if (state == OPEN)
	{
		opening_or_closing_progress = 1.f;
		isActive = true;
	}
	else if (state == CLOSED)
	{
		opening_or_closing_progress = 0.f;
		isActive = false;
	}
}

enum_ConsoleStates Console::GetState(void)
{
	return state;
}

enum_ConsoleStates Console::ToggleState(void)
{
	if (state == CLOSING || state == CLOSED) Open();
	else Close();
	return state;
}

void Console::MoveCursor(enum_ConsoleCursorScroll d)
{
	cursorPos -= d;
	if (cursorPos < 0)
	{
		cursorPos = 0;
		if (d != C_EXTREM_RIGHT) beep();
	}
	if (cursorPos > consoleLines[nbrUsedLines-1].length)
	{
		cursorPos = consoleLines[nbrUsedLines-1].length;
		if (d != C_EXTREM_LEFT) beep();
	}
}

int Console::GetLeft(void)
{ 
	return leftPos;
}

int Console::GetTop(void)
{
	return topPos;
}

int Console::GetWidth(void)
{
	return width;
}

int Console::GetHeight(void)
{
	return height;
}

enum_ConsoleType Console::GetType(void)
{ 
	return type;
}

char* Console::GetPrompt(void)
{
	return prompt.content;
}

void Console::SetTabMode(enum_ConsoleCommandSearchMode mode)
{
	searchMode = mode;
}

// The following functions return informations on mouse position. It informs if a the
// mouse is on an element of the console. To return a valid value, the GetMouse* values
// have to updated.

bool Console::IsMouseOnConsole(void)
{	
	return skin->IsMouseOnSkin();
}

int Console::MouseIsInsideWindow(int x, int y, int xoffset, int yoffset)
{
	if (x >= yoffset && (x - xoffset) <= screen_Width &&
		y >= yoffset && (y - yoffset) <= screen_Height) return 1;
	else if (y >= yoffset && (y - yoffset) <= screen_Height) return 2;
	else if (x >= yoffset && (x - xoffset) <= screen_Width) return 3;
	else return 0;
}

int Console::IsMouseOnResizeSquare(void)
{
	if (state != OPEN || consoleIsRolledUp) return 0;
	if (mouse.x >= leftPos && mouse.x <= leftPos+RESIZE_SQUARE &&
		mouse.y >= topPos  && mouse.y <= topPos +RESIZE_SQUARE) return 1;					// Upper left corner
	else if (mouse.x >= leftPos+width-RESIZE_SQUARE && mouse.x <= leftPos+width &&
		mouse.y >= topPos && mouse.y <= topPos+RESIZE_SQUARE) return 2;				// Upper right corner
	else if (mouse.x >= leftPos && mouse.x <= leftPos+RESIZE_SQUARE &&
		mouse.y >= topPos+height-RESIZE_SQUARE && mouse.y <= topPos+height) return 3;	// Lower left corner
	else if (mouse.x >= leftPos+width-RESIZE_SQUARE && mouse.x <= leftPos+width &&
		mouse.y >= topPos+height-RESIZE_SQUARE && mouse.y <= topPos+height) return 4;	// Lower right corner
	else return 0;
}

bool Console::IsMouseOnVScrollSquare(void)
{
	return (state == OPEN && skin->IsMouseOnVScroller());
}

bool Console::IsMouseOnScrollerUpButton(void)
{
	return (state == OPEN && skin->IsMouseOnScrollerUpButton());
}

bool Console::IsMouseOnScrollerDownButton(void)
{
	return (state == OPEN && skin->IsMouseOnScrollerDownButton());
}

bool Console::IsMouseOnVScrollZone(void)
{
	return (state == OPEN && skin->IsMouseOnVScrollZone());
}

bool Console::IsMouseOnTitleBar(void)
{
	return (state == OPEN && skin->IsMouseOnTitleBar());
}

bool Console::IsMouseOnCloseButton(void)
{
	return (state == OPEN && skin->IsMouseOnCloseButton());
}

bool Console::IsMouseOnMaximiseOrReduceButton(void)
{
	return (state == OPEN && skin->IsMouseOnMaximiseOrReduceButton());
}

bool Console::IsMouseOnRollUpOrUnrollButton(void)
{
	return (state == OPEN && skin->IsMouseOnRollUpOrUnrollButton());
}

// The following function are given as help for the main interface.

int Console::Keyboard(unsigned char key, int x, int y)
{
	if (key == cons_consoleKey.ivalue)
	{
		if (ToggleState() == OPENING) histLine = 0;
		lastkey = key;
		return 1;
	}

	// If console is active
	if (isActive && (GetState() == OPEN || GetState() == OPENING))
	{
		switch (key)
		{
			case 13:
				char ConsoleBufferCommand[CONSOLE_LINELENGTH];
				strncpy(ConsoleBufferCommand, consoleLines[nbrUsedLines-1].content, CONSOLE_LINELENGTH);
				if (strlen(ConsoleBufferCommand) > 0)
				{
					histLine = 0;
					gCommands->ExecuteCommand(ConsoleBufferCommand);
				}
				break;				
			case 27:
				if (strlen(consoleLines[nbrUsedLines-1].content))
					ReInitCurrentCommand();
				else
				{
					Close();
					ReInitCurrentCommand();
				}
				break;
			case 8:
				DelChar();
				break;
			case 9:
				if (searchMode)
				{
					if (lastkey == key)
					{
						if (glutGetModifiers() == GLUT_ACTIVE_SHIFT)
							gCommands->PrevSolution();
						else
							gCommands->NextSolution();
					}
					else
					{
						if (glutGetModifiers() == GLUT_ACTIVE_SHIFT)
							gCommands->LastSolution(consoleLines[nbrUsedLines-1].content);
						else
							gCommands->FirstSolution(consoleLines[nbrUsedLines-1].content);
					}
				}
				else
				{
					gCommands->CompleteCommand(consoleLines[nbrUsedLines-1].content);
				}
				break;
			default:
				AddChar((char) key);
				break;
		}
		lastkey = key;
		return 1;
	}
	lastkey = key;
	return 0;
}

int Console::KeyboardSpec(unsigned char key, int x, int y)
{
	// If console is active
	if (isActive && (state == OPEN || state == OPENING))
	{
		switch (key)
		{
			case GLUT_KEY_UP:
				if (++histLine > gCommands->GetNbrCommHistLines())
				{
					histLine = gCommands->GetNbrCommHistLines();
					beep();
				}
				cursorPos = 0;
				SetCurrentCommand(gCommands->GetHistoryLine(histLine));
				break;
			case GLUT_KEY_DOWN:
				if (--histLine < 0)
				{
					histLine = 0;
					beep();
				}
				cursorPos = 0;
				SetCurrentCommand(gCommands->GetHistoryLine(histLine));
				break;
			case GLUT_KEY_LEFT:
				MoveCursor(LEFT);
				break;
			case GLUT_KEY_RIGHT:
				MoveCursor(RIGHT);
				break;
			case GLUT_KEY_HOME:
				if (glutGetModifiers() == GLUT_ACTIVE_CTRL) ScrollConsole(TOP);
				else MoveCursor(C_EXTREM_LEFT);
				break;
			case GLUT_KEY_END:
				if (glutGetModifiers() == GLUT_ACTIVE_CTRL) ScrollConsole(BOTTOM);
				else MoveCursor(C_EXTREM_RIGHT);
				break;
			case GLUT_KEY_PAGE_UP:
				ScrollConsole(UP);
				break;
			case GLUT_KEY_PAGE_DOWN:
				ScrollConsole(DOWN);
				break;
			case GLUT_KEY_INSERT:
				insert = !insert;
				break;
		}
		lastkey = key;
		return 1;
	}

	histLine = 0;
	lastkey = key;

	return 0;
}

int Console::MouseClic(int button, int s, int x, int y)
{
	if ((state == OPEN || state == OPENING))
	{
		// Update the mouse state
		this->mouse.x = x;
		this->mouse.y = screen_Height - y;
		this->mouse.button[0] = lButton?BUTTON_DOWN:BUTTON_UP;
		this->mouse.button[1] = mButton?BUTTON_DOWN:BUTTON_UP;
		this->mouse.button[2] = rButton?BUTTON_DOWN:BUTTON_UP;
		switch (button)
		{
			case GLUT_LEFT_BUTTON:
				if (s == GLUT_DOWN) this->mouse.button[0] = BUTTON_DOWN;
				else if (s == GLUT_UP) this->mouse.button[0] = BUTTON_UP;
				break;
			case GLUT_MIDDLE_BUTTON:
				if (s == GLUT_DOWN) this->mouse.button[1] = BUTTON_DOWN;
				else if (s == GLUT_UP) this->mouse.button[1] = BUTTON_UP;
				break;
			case GLUT_RIGHT_BUTTON:
				if (s == GLUT_DOWN) this->mouse.button[2] = BUTTON_DOWN;
				else if (s == GLUT_UP) this->mouse.button[2] = BUTTON_UP;
				break;
		}
		this->skin->GiveMouseState(&this->mouse);
		this->mouse.y = y;

		if (s == GLUT_DOWN) isActive = IsMouseOnConsole();
		if (!isActive) return 0;

		// Manage mouse clics
		switch (button)
		{
			case GLUT_LEFT_BUTTON:
				if (s == GLUT_DOWN)
				{
					mouse_repeat_start = GetTime();
					isRepeating = false;

					lButton = true;
					mButton = false;
					rButton = false;
					dx = x;
					dy = y;
					if (IsMouseOnCloseButton() || IsMouseOnMaximiseOrReduceButton() || IsMouseOnRollUpOrUnrollButton()) return 1;
					if (IsMouseOnScrollerUpButton()) { ScrollConsole(UP); return 1; }
					if (IsMouseOnScrollerDownButton()) { ScrollConsole(DOWN); return 1; }
					cons_scroll = IsMouseOnVScrollSquare();
					cons_resizefrom = IsMaximized()?0:IsMouseOnResizeSquare();
					if (cons_resizefrom > 0) { console_move = false; console_resize = true; }
					else if (IsMouseOnTitleBar())
					{
						console_move = true;
						console_resize = false;
					}
					else if (IsMouseOnVScrollZone())
					{
						SetVScrollYPos(y, dy, cons_scroll);
						cons_scroll = true;
					}
					cons_left = leftPos;
					cons_top  = topPos;
					cons_width = width;
					cons_height = height;
				}
				else if (s == GLUT_UP)
				{
					initvScrollY = 0;	// no more scrolling
					if (IsMouseOnCloseButton()) Close();
					if (IsMouseOnMaximiseOrReduceButton()) ToggleMaximisation();
					if (IsMouseOnRollUpOrUnrollButton()) ToggleRolling();
					if ((GetTime()-clic_delay) < doubleclicSpeed)
					{
						isRepeating = false;
						//if (IsMouseOnScrollerUpButton() || IsMouseOnScrollerDownButton()) return 1;
						if (IsMouseOnTitleBar()) ToggleMaximisation();
						else if (IsMouseOnConsole()) ToggleType();
						lButton = false;
						mButton = false;
						rButton = false;
						cons_resizefrom = 0;
						console_move = false;
						console_resize = false;				
					}
					else if ((GetTime()-clic_delay) > doubleclicSpeed)
					{
						isRepeating = false;

						lButton = false;
						cons_resizefrom = 0;
						console_move = false;
						console_resize = false;
						clic_delay = GetTime();
					}
				}
				break;
			case GLUT_RIGHT_BUTTON:
				if (s == GLUT_DOWN)
				{
					lButton = false;
					mButton = false;
					rButton = true;
					dx = x;
					dy = y;
					cons_scroll = IsMouseOnVScrollSquare();
					if (IsMouseOnVScrollZone())
					{
						SetVScrollYPos(y, dy, cons_scroll);
						cons_scroll = true;
					}
					if (IsMouseOnConsole()) console_move = true;
					cons_left = leftPos;
					cons_top  = topPos;
					cons_width = width;
					cons_height = height;
				}
				else if (s == GLUT_UP)
				{
					initvScrollY = 0;	// no more scrolling
					rButton = false;
					console_move = false;
				}
				break;
			default:
				break;
		}
		return 1;
	}
	return 0;
}

int Console::MouseMove(int x, int y)
{
	this->mouse.x = x;
	this->mouse.y = screen_Height - y;
	this->skin->GiveMouseState(&this->mouse);
	this->mouse.y = y;

	if (/*isActive && */lButton)
	{
		if (cons_scroll) SetVScrollYPos(y);

		int res_4mouseisinsidewin = MouseIsInsideWindow(x, y);
		if (state == OPEN && res_4mouseisinsidewin && type != C_STATIC)
		{
			if (console_move)
			{
				if (res_4mouseisinsidewin == 1)
				{
					SetLeftPos((x - dx) + cons_left, 2);
					SetTopPos((y - dy) + cons_top, 2);
				}
				else if (res_4mouseisinsidewin == 2)
				{
					SetTopPos((y - dy) + cons_top, 2);
				}
				else if (res_4mouseisinsidewin == 3)
				{
					SetLeftPos((x - dx) + cons_left, 2);
				}
			}
			else if (console_resize)
			{
				if (cons_resizefrom == 1)
				{
					Resize((cons_width - (x - dx)), (cons_height - (y - dy)));
					SetLeftPos((x - dx) + cons_left, 2);
					SetTopPos((y - dy) + cons_top, 2);
				}
				else if (cons_resizefrom == 2)
				{
					Resize(((x - dx) + cons_width), (cons_height - (y - dy)));
					if (cons_height - (y - dy) > 30) SetTopPos((y - dy) + cons_top, 2);
				}
				else if (cons_resizefrom == 3)
				{
					if (!Resize((cons_width - (x - dx)), ((y - dy) + cons_height)))
					if ((cons_width - (x - dx)) > 20) SetLeftPos((x - dx) + cons_left, 2);
				}
				else if (cons_resizefrom == 4)
				{
					Resize(((x - dx) + cons_width), ((y - dy) + cons_height));
				}
			}
		}
		return 1;
	}
	else if (/*isActive && */rButton)
	{
		int res_4mouseisinsidewin = MouseIsInsideWindow(x, y);
		if (state == OPEN && res_4mouseisinsidewin && type != C_STATIC && console_move)
		{
			if (res_4mouseisinsidewin == 1)
			{
				SetLeftPos((x - dx) + cons_left, 2);
				SetTopPos((y - dy) + cons_top, 2);
			}
			else if (res_4mouseisinsidewin == 2)
			{
				SetTopPos((y - dy) + cons_top, 2);
			}
			else if (res_4mouseisinsidewin == 3)
			{
				SetLeftPos((x - dx) + cons_left, 2);
			}
		}
		return 1;
	}

	return 0;
}

void Console::CheckScrollWithMouse(void)
{
	if (isRepeating)
	{
		if (lButton && (GetTime()-mouse_repeat_start) > mouseRepeatSpeed)
		{
			mouse_repeat_start = GetTime();

			if (IsMouseOnScrollerUpButton()) ScrollConsole(UP);
			else if (IsMouseOnScrollerDownButton()) ScrollConsole(DOWN);
		}
	}
	else
	{
		if (lButton && (GetTime()-mouse_repeat_start) > mouseRepeatSpeed)
		{
			mouse_repeat_start = GetTime();
			isRepeating = true;

			if (IsMouseOnScrollerUpButton()) ScrollConsole(UP);
			else if (IsMouseOnScrollerDownButton()) ScrollConsole(DOWN);
		}
	}
}

void Console::SetDisplayFunc(FuncPointer func)
{
	displayFunc = func;
}

void Console::UpdateDisplay(void)
{
	if (displayFunc) (*displayFunc)();
}

void Console::GiveMousePos(int x, int y, int xoff, int yoff)
{
	this->mouse.x = x - xoff;
	this->mouse.y = y - yoff;
	
	if (state == OPEN || state == OPENING)
		skin->GiveMousePos(x - xoff, screen_Height - (y - yoff));
}

void Console::SetBackgroundStyle(enum_BackgroundStyle style)
{
	skin->SetBackgroundStyle(style);
}

void Console::SetBackgroundColor(float r, float g, float b, float a)
{
	skin->SetBackgroundColor(r, g, b, a);
}