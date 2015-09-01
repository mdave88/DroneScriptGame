//-----------------------------------------------------------------------------
// Skin
//-----------------------------------------------------------------------------

#include "cl.h"
#include "const.h"
#include "skin.h"
#include "sprite.h"
#include "parser.h"
#include "font.h"
#include <IL/ilut.h>
#include <math.h>
#include <assert.h>
#include <string.h>

Var skin_gridmode("skin_gridmode", 0, VF_NONE);

// Skin Map -------------------------------------------------------------------

SkinMap::SkinMap(char *filename)
{
	// Generate the main image name to use
	ilGenImages(1, &ImgId);

	// Bind this image name
	ilBindImage(ImgId);

	// Loads the image specified by File into the ImgId image
	if (!ilLoadImage(filename))
	{
		ILenum err = ilGetError();
		printf("An error occured while loading %s: %d (%s)\n", filename, err, iluErrorString(err));
		return;
	}
	
	// Get image width and height
	map_width = (int) ilGetInteger(IL_IMAGE_WIDTH);
	map_height = (int) ilGetInteger(IL_IMAGE_HEIGHT);
	map_bpp = (int) ilGetInteger(IL_IMAGE_BPP);
}

SkinMap::~SkinMap(void)
{
	ilDeleteImages(1, &ImgId);
}

void SkinMap::GetRegion(Region *r)
{
	assert(r->coords.height > 0 && r->coords.width > 0);
	ILuint tmpID = 0;

#define GETREGION_METHOD 2
#if GETREGION_METHOD == 0
	// has problems with having transparency PNG files
	ilGenImages(1, &tmpID);
	ilBindImage(tmpID);
	ilTexImage(r->coords.width, r->coords.height, 1, map_bpp, map_bpp==3?IL_RGB:IL_RGBA, IL_UNSIGNED_BYTE, NULL);
	ilBlit(ImgId,
		   0, 0, 0,
		   r->coords.xpos, r->coords.ypos, 0,
		   r->coords.width, r->coords.height, 1);
#elif GETREGION_METHOD == 1
	ILubyte *itmp = new ILubyte[map_bpp*r->coords.width*r->coords.height];
	ILubyte* data = new ILubyte[map_bpp*r->coords.width*r->coords.height];
	
	// Get region from skin map
	ilBindImage(ImgId);

	ilCopyPixels(r->coords.xpos, r->coords.ypos, 0,
				 r->coords.width, r->coords.height, 1,
				 map_bpp==3?IL_RGB:IL_RGBA, IL_UNSIGNED_BYTE, itmp);

	// Copy region in skin region map
	ilGenImages(1, &tmpID);
	ilBindImage(tmpID);
	ilTexImage(r->coords.width, r->coords.height, 1, map_bpp, map_bpp==3?IL_RGB:IL_RGBA, IL_UNSIGNED_BYTE, NULL);

	// Flip the data (I don't know why it is necessary, but without that, all skin textures are inversed)
	memset(data, 0, map_bpp*r->coords.width*r->coords.height*sizeof(ILubyte));
	for (int i = 0; i < r->coords.height; ++i)
		memcpy(&data[map_bpp*r->coords.width*(r->coords.height-i-1)],
			   &itmp[map_bpp*r->coords.width*i],
			   map_bpp*r->coords.width);

	// Set data in map
	ilSetData(data);
	
	delete [] itmp;
	delete [] data;
#else
	ilBindImage(ImgId);
	ILubyte* itmp = ilGetData();											// source pixels
	ILubyte* data = new ILubyte[map_bpp*r->coords.width*r->coords.height];	// destination pixels

	// Copy the pixels in data
	for (int y = 0; y < r->coords.height; ++y)
	{
		memcpy(&data[map_bpp*r->coords.width*(r->coords.height-y-1)],
			   &itmp[map_bpp*map_width*(y+r->coords.ypos)+map_bpp*r->coords.xpos],
			   map_bpp*r->coords.width);
	}

	// Copy region in skin region map
	ilGenImages(1, &tmpID);
	ilBindImage(tmpID);
	ilTexImage(r->coords.width, r->coords.height, 1, map_bpp, map_bpp==3?IL_RGB:IL_RGBA, IL_UNSIGNED_BYTE, NULL);

	// Set data in map
	ilSetData(data);
	
	itmp = NULL;
	delete [] data;
#endif

	glEnable(GL_TEXTURE_2D);
	ilutRenderer(ILUT_OPENGL);
	r->TexName = ilutGLBindTexImage();
	ilDeleteImages(1, &tmpID);
}


// Skin -----------------------------------------------------------------------

Skin::Skin(void)
{
	Init();
	gVars->RegisterVar(skin_gridmode);
}

Skin::~Skin(void)
{
	gVars->UnregisterVar(skin_gridmode);
	Shut();
}

void Skin::Init(void)
{
	state = UNLOADED;
	map = NULL;
	background.sprite = NULL;
	background.color[0] = 0;
	background.color[1] = 0;
	background.color[2] = 0;
	background.color[3] = 0.5f;
	background.style = NOTIMAGED;
	closeButton = NULL;
	maximiseButton = NULL;
	reduceButton = NULL;
	rollupButton = NULL;
	unrollButton = NULL;
	titleBar = NULL;
	vScrollBar = NULL;
	border = NULL;

	isMaximized = false;
	isRolledUp = false;
	backup_x = backup_y = 0.f;	
}

void Skin::Shut(void)
{
	if (map) delete map; map = NULL;
	if (background.sprite) delete background.sprite; background.sprite = NULL;
	if (closeButton) delete closeButton; closeButton = NULL;
	if (maximiseButton) delete maximiseButton; maximiseButton = NULL;
	if (reduceButton) delete reduceButton; reduceButton = NULL;
	if (rollupButton) delete rollupButton; rollupButton = NULL;
	if (unrollButton) delete unrollButton; unrollButton = NULL;
	if (titleBar)
	{
		if (titleBar->title_str)
			delete [] titleBar->title_str;
		if (titleBar->title_map)
			delete titleBar->title_map;
		titleBar->title_str = NULL;
		titleBar->title_map = NULL;
		delete titleBar; titleBar = NULL;
	}
	if (vScrollBar) delete vScrollBar; vScrollBar = NULL;
	if (border) delete border; border = NULL;
	state = UNLOADED;
}

int Skin::LoadSkin(const char *filename, int width, int height, Font* font)
{
	Shut();
	state = LOADING;

	// Get the font path
	int i, l;
	char path[PATH_LENGTH] = { '\0' };
	l = (int) strlen(filename)-1;
	while (l >= 0 && filename[l] != '\\' && filename[l] != '/') --l;
	for (i = 0; i <= l && i < PATH_LENGTH; ++i) path[i] = filename[i];

	p = new Parser;
	if (!p->StartParseFile(filename)) return 0;
	
	char fullpathname[PATH_LENGTH];

	// Parsing file
	while (p->GetToken())
	{
		if (!p->CompToken(str_SKIN_MAP))
		{
			p->GetToken();
			memset(fullpathname, (int)PATH_LENGTH, sizeof(char));
			sprintf(fullpathname, "%s%s", path, p->token);
			map = new SkinMap(fullpathname);
		}
		else if (!p->CompToken(str_SKIN_BACKGROUND))
		{
			p->GetToken();
			memset(fullpathname, (int)PATH_LENGTH, sizeof(char));
			sprintf(fullpathname, "%s%s", path, p->token);
			background.sprite = new Sprite;
			background.style = ANIMATED;
			background.sprite->SetSize(width, height);
			background.sprite->LoadImage(fullpathname);
		}
		else if (!p->CompToken(str_SKIN_BACKCOLOR))
		{
			p->GetToken(); p->Assert(str_OPENBLOCK);
			p->GetToken(); background.color[0] = (float) atof(p->token);
			p->GetToken(); background.color[1] = (float) atof(p->token);
			p->GetToken(); background.color[2] = (float) atof(p->token);
			p->GetToken(); background.color[3] = (float) atof(p->token);
			p->GetToken(); p->Assert(str_CLOSEBLOCK);
		}
		else if (!p->CompToken(str_CLOSE_BUTTON))
		{
			closeButton = new Button;
			if (!ParseButton(closeButton)) return 0;
		}
		else if (!p->CompToken(str_MAXIMISE_BUTTON))
		{
			maximiseButton = new Button;
			if (!ParseButton(maximiseButton)) return 0;
		}
		else if (!p->CompToken(str_REDUCE_BUTTON))
		{
			reduceButton = new Button;
			if (!ParseButton(reduceButton)) return 0;
		}
		else if (!p->CompToken(str_ROLLUP_BUTTON))
		{
			rollupButton = new Button;
			if (!ParseButton(rollupButton)) return 0;
		}
		else if (!p->CompToken(str_UNROLL_BUTTON))
		{
			unrollButton = new Button;
			if (!ParseButton(unrollButton)) return 0;
		}
		else if (!p->CompToken(str_TITLE_BAR))
		{
			titleBar = new TitleBar;
			if (!ParseTitleBar(titleBar)) return 0;
		}
		else if (!p->CompToken(str_VSCROLL_BAR))
		{
			vScrollBar = new VScrollBar;
			if (!ParseVScrollBar(vScrollBar)) return 0;
		}
		else if (!p->CompToken(str_BORDER))
		{
			border = new Border;
			if (!ParseBorder(border)) return 0;
		}
	}
	
	p->StopParse();
	delete p;

	if (map && titleBar && border)	// minimum required
	{
		titleBar->font = font;
		state = LOADED;
		return 1;
	}

	state = UNLOADED;
	return 0;
}

int Skin::ParseButton(Button *b)
{
	if (!p || !b) return 0;

	b->state = BS_INACTIVE;
	b->reference.xoffset = 0;
	b->reference.yoffset = 0;

	p->GetToken(); if (!p->Assert(str_OPENBLOCK)) return 0;
	p->GetToken();
	while (p->CompToken(str_CLOSEBLOCK))
	{
		if (!p->CompToken(str_INACTIVE))
		{
			p->GetToken(); if (!p->Assert(str_OPENBLOCK)) return 0;
			p->GetToken(); b->region[BS_INACTIVE].coords.xpos = (int) atoi(p->token);
			p->GetToken(); b->region[BS_INACTIVE].coords.ypos = (int) atoi(p->token);
			p->GetToken(); if (!p->Assert(str_CLOSEBLOCK)) return 0;
		}
		else if (!p->CompToken(str_ACTIVE))
		{
			p->GetToken(); if (!p->Assert(str_OPENBLOCK)) return 0;
			p->GetToken(); b->region[BS_ACTIVE].coords.xpos = (int) atoi(p->token);
			p->GetToken(); b->region[BS_ACTIVE].coords.ypos = (int) atoi(p->token);
			p->GetToken(); if (!p->Assert(str_CLOSEBLOCK)) return 0;
		}
		else if (!p->CompToken(str_PRESSED))
		{
			p->GetToken(); if (!p->Assert(str_OPENBLOCK)) return 0;
			p->GetToken(); b->region[BS_PRESSED].coords.xpos = (int) atoi(p->token);
			p->GetToken(); b->region[BS_PRESSED].coords.ypos = (int) atoi(p->token);
			p->GetToken(); if (!p->Assert(str_CLOSEBLOCK)) return 0;
		}
		else if (!p->CompToken(str_DIMS))
		{
			p->GetToken(); if (!p->Assert(str_OPENBLOCK)) return 0;
			p->GetToken(); b->region[BS_INACTIVE].coords.width  = b->region[BS_ACTIVE].coords.width  = b->region[BS_PRESSED].coords.width  = b->coords.width =  (int) atoi(p->token);
			p->GetToken(); b->region[BS_INACTIVE].coords.height = b->region[BS_ACTIVE].coords.height = b->region[BS_PRESSED].coords.height = b->coords.height = (int) atoi(p->token);
			p->GetToken(); if (!p->Assert(str_CLOSEBLOCK)) return 0;
		}
		else if (!p->CompToken(str_POSITION))
		{
			p->GetToken(); if (!p->Assert(str_OPENBLOCK)) return 0;
			p->GetToken(); b->reference.xoffset =  (int) atoi(p->token);
			p->GetToken(); b->reference.yoffset = -(int) atoi(p->token);
			p->GetToken(); if (!p->Assert(str_CLOSEBLOCK)) return 0;
		}
		else if (!p->CompToken(str_RELATIVE))
		{
			p->GetToken();
			if (!p->CompToken(str_TL)) b->reference.refcorner = BR_TL;
			else if (!p->CompToken(str_TR)) b->reference.refcorner = BR_TR;
			else if (!p->CompToken(str_BL)) b->reference.refcorner = BR_BL;
			else if (!p->CompToken(str_BR)) b->reference.refcorner = BR_BR;
			else return 0;
		}
		p->GetToken();
	}

	map->GetRegion(&(b->region[BS_INACTIVE]));
	map->GetRegion(&(b->region[BS_ACTIVE]));
	map->GetRegion(&(b->region[BS_PRESSED]));

	return 1;
}

int Skin::ParseTitleBar(TitleBar *t)
{
	if (!p || !t) return 0;

	t->centered = false;
	t->fontsize[0] = t->fontsize[1] = 8;
	t->title_map = NULL;
	t->title_str = NULL;
	t->border = TB_NONE;
	t->reference.xoffset = 0;
	t->reference.yoffset = 0;
	t->reference.refcorner = BR_TL;

	p->GetToken(); if (!p->Assert(str_OPENBLOCK)) return 0;
	p->GetToken();
	while (p->CompToken(str_CLOSEBLOCK))
	{
		if (!p->CompToken(str_TITLE_BORDER))
		{
			p->GetToken();
			if (!p->CompToken(str_NONE)) t->border = TB_NONE;
			else if (!p->CompToken(str_TOP)) t->border = TB_TOP;
			else if (!p->CompToken(str_LEFT)) t->border = TB_LEFT;
			else if (!p->CompToken(str_RIGHT)) t->border = TB_RIGHT;
			else if (!p->CompToken(str_BOTTOM)) t->border = TB_BOTTOM;
			else return 0;
		}
		else if (!p->CompToken(str_TITLE_STRING))
		{
			p->GetToken();
			t->title_str = new char[strlen(p->token)+1];
			strcpy(t->title_str, p->token);
		}
		else if (!p->CompToken(str_TITLE_MAP))
		{
			t->title_map = new Region;
			p->GetToken(); if (!p->Assert(str_OPENBLOCK)) return 0;
			p->GetToken(); t->title_map->coords.xpos =   (int) atoi(p->token);
			p->GetToken(); t->title_map->coords.ypos =   (int) atoi(p->token);
			p->GetToken(); t->title_map->coords.width =  (int) atoi(p->token);
			p->GetToken(); t->title_map->coords.height = (int) atoi(p->token);
			map->GetRegion(t->title_map);
			p->GetToken(); if (!p->Assert(str_CLOSEBLOCK)) return 0;
		}
		else if (!p->CompToken(str_TITLE_FONTSIZE))
		{
			p->GetToken(); if (!p->Assert(str_OPENBLOCK)) return 0;
			p->GetToken(); int height = (int) atoi(p->token);
			p->GetToken(); int width  = (int) atoi(p->token);
			if (height > 0) t->fontsize[0] = height;
			if (width  > 0) t->fontsize[1] = width;
			p->GetToken(); if (!p->Assert(str_CLOSEBLOCK)) return 0;
		}
		else if (!p->CompToken(str_TITLE_MODE))
		{
			p->GetToken();
			if (!p->CompToken(str_CENTERED)) t->centered = true;
			else if (!p->CompToken(str_LEFT)) t->centered = false;
			else return 0;
		}
		else if (!p->CompToken(str_POSITION))
		{
			p->GetToken(); if (!p->Assert(str_OPENBLOCK)) return 0;
			p->GetToken(); t->reference.xoffset =  (int) atoi(p->token);
			p->GetToken(); t->reference.yoffset = -(int) atoi(p->token);
			p->GetToken(); if (!p->Assert(str_CLOSEBLOCK)) return 0;
		}
		else if (!p->CompToken(str_RELATIVE))
		{
			p->GetToken();
			if (!p->CompToken(str_TL)) t->reference.refcorner = BR_TL;
			else if (!p->CompToken(str_TR)) t->reference.refcorner = BR_TR;
			else if (!p->CompToken(str_BL)) t->reference.refcorner = BR_BL;
			else if (!p->CompToken(str_BR)) t->reference.refcorner = BR_BR;
			else return 0;
		}
		p->GetToken();
	}
	return 1;
}

int Skin::ParseVScrollBar(VScrollBar *v)
{
	if (!p || !v) return 0;

	v->height = 0;
	v->extendable = false;
	v->pos = VSP_RIGHT;

	p->GetToken(); if (!p->Assert(str_OPENBLOCK)) return 0;
	p->GetToken();
	while (p->CompToken(str_CLOSEBLOCK))
	{
		if (!p->CompToken(str_UP_BUTTON))
		{
			if (!ParseButton(&(v->up))) return 0;
		}
		else if (!p->CompToken(str_DOWN_BUTTON))
		{
 			if (!ParseButton(&(v->down))) return 0;
		}
		else if (!p->CompToken(str_SCROLLER))
		{
			p->GetToken(); if (!p->Assert(str_OPENBLOCK)) return 0;
			p->GetToken();
			while (p->CompToken(str_CLOSEBLOCK))
			{
				if (!p->CompToken(str_TOP))
				{
					if (!ParseButton(&(v->topScroller))) return 0;
					v->height += v->topScroller.coords.height;
				}
				else if (!p->CompToken(str_MIDDLE))
				{
					if (!ParseButton(&(v->middleScroller))) return 0;
					v->height += v->middleScroller.coords.height;
				}
				else if (!p->CompToken(str_BOTTOM))
				{
					if (!ParseButton(&(v->bottomScroller))) return 0;
					v->height += v->bottomScroller.coords.height;
				}
				else if (!p->CompToken(str_EXTENDABLE))
				{
					if (!ParseButton(&(v->extendableScroller))) return 0;
					v->extendable = true;
				}
				p->GetToken();
			}
		}
		else if (!p->CompToken(str_BACKGROUND))
		{
			p->GetToken(); if (!p->Assert(str_OPENBLOCK)) return 0;
			p->GetToken(); v->background.coords.xpos =   v->coords.xpos =   (int) atoi(p->token);
			p->GetToken(); v->background.coords.ypos =   v->coords.ypos =   (int) atoi(p->token);
			p->GetToken(); v->background.coords.width =  v->coords.width =  (int) atoi(p->token);
			p->GetToken(); v->background.coords.height = v->coords.height = (int) atoi(p->token);
			map->GetRegion(&(v->background));
			p->GetToken(); if (!p->Assert(str_CLOSEBLOCK)) return 0;
		}
		else if (!p->CompToken(str_FILLMODE))
		{
			p->GetToken();
			if (!p->CompToken(str_TILED)) v->tiled = true;
			else if (!p->CompToken(str_STRETCHED)) v->tiled = false;
			else return 0;
		}
		else if (!p->CompToken(str_POSITION))
		{
			p->GetToken();
			if (!p->CompToken(str_LEFT)) v->pos = VSP_LEFT;
			else if (!p->CompToken(str_RIGHT)) v->pos = VSP_RIGHT;
			else return 0;
		}
		p->GetToken();
	}
	v->coords.width =  v->middleScroller.coords.width;
	v->coords.height = v->height;
	return 1;
}

int Skin::ParseBorderElement(BorderElement *b)
{
	if (!p || !b) return 0;

	b->state = BES_INACTIVE;
	b->xoffset = 0;
	b->yoffset = 0;

	p->GetToken(); if (!p->Assert(str_OPENBLOCK)) return 0;
	p->GetToken();
	while (p->CompToken(str_CLOSEBLOCK))
	{
		if (!p->CompToken(str_INACTIVE))
		{
			p->GetToken(); if (!p->Assert(str_OPENBLOCK)) return 0;
			p->GetToken(); b->region[BES_INACTIVE].coords.xpos = (int) atoi(p->token);
			p->GetToken(); b->region[BES_INACTIVE].coords.ypos = (int) atoi(p->token);
			p->GetToken(); if (!p->Assert(str_CLOSEBLOCK)) return 0;
		}
		else if (!p->CompToken(str_ACTIVE))
		{
			p->GetToken(); if (!p->Assert(str_OPENBLOCK)) return 0;
			p->GetToken(); b->region[BES_ACTIVE].coords.xpos = (int) atoi(p->token);
			p->GetToken(); b->region[BES_ACTIVE].coords.ypos = (int) atoi(p->token);
			p->GetToken(); if (!p->Assert(str_CLOSEBLOCK)) return 0;
		}
		else if (!p->CompToken(str_DIMS))
		{
			p->GetToken(); if (!p->Assert(str_OPENBLOCK)) return 0;
			p->GetToken(); b->region[BES_INACTIVE].coords.width  =
						   b->region[BES_ACTIVE].coords.width  =
						   b->coords.width =  (int) atoi(p->token);
			p->GetToken(); b->region[BES_INACTIVE].coords.height =
						   b->region[BES_ACTIVE].coords.height =
						   b->coords.height = (int) atoi(p->token);
			p->GetToken(); if (!p->Assert(str_CLOSEBLOCK)) return 0;
		}
		else if (!p->CompToken(str_FILLMODE))
		{
			p->GetToken();
			if (!p->CompToken(str_TILED)) b->tiled = true;
			else if (!p->CompToken(str_STRETCHED)) b->tiled = false;
			else return 0;
		}
		else if (!p->CompToken(str_POSITION))
		{
			p->GetToken(); if (!p->Assert(str_OPENBLOCK)) return 0;
			p->GetToken(); b->xoffset =  (int) atoi(p->token);
			p->GetToken(); b->yoffset = -(int) atoi(p->token);
			p->GetToken(); if (!p->Assert(str_CLOSEBLOCK)) return 0;
		}
		p->GetToken();
	}
	map->GetRegion(&(b->region[BES_INACTIVE]));
	map->GetRegion(&(b->region[BES_ACTIVE]));
	return 1;
}

int Skin::ParseBorder(Border *b)
{
	if (!p || !b) return 0;

	p->GetToken(); if (!p->Assert(str_OPENBLOCK)) return 0;
	p->GetToken();
	while (p->CompToken(str_CLOSEBLOCK))
	{
		if (!p->CompToken(str_TOP_LEFT_CORNER))
		{
			if (!ParseBorderElement(&(b->topleftCorner))) return 0;
		}
		else if (!p->CompToken(str_TOP_RIGHT_CORNER))
		{
			if (!ParseBorderElement(&(b->toprightCorner))) return 0;
		}
		else if (!p->CompToken(str_BOTTOM_LEFT_CORNER))
		{
			if (!ParseBorderElement(&(b->bottomleftCorner))) return 0;
		}
		else if (!p->CompToken(str_BOTTOM_RIGHT_CORNER))
		{
			if (!ParseBorderElement(&(b->bottomrightCorner))) return 0;
		}
		else if (!p->CompToken(str_TOP_BORDER))
		{
			if (!ParseBorderElement(&(b->topBorder))) return 0;
		}
		else if (!p->CompToken(str_LEFT_BORDER))
		{
			if (!ParseBorderElement(&(b->leftBorder))) return 0;
		}
		else if (!p->CompToken(str_BOTTOM_BORDER))
		{
			if (!ParseBorderElement(&(b->bottomBorder))) return 0;
		}
		else if (!p->CompToken(str_RIGHT_BORDER))
		{
			if (!ParseBorderElement(&(b->rightBorder))) return 0;
		}
		p->GetToken();
	}
	return 1;
}

void Skin::UpdateSkin(int xpos, int ypos, int width, int height)
{
	if (state != LOADED) return;

	int xloffset, xroffset;	// Offset relative to vertical scrollbar position
	if (vScrollBar->pos == VSP_RIGHT)
	{
		xroffset = vScrollBar->coords.width;
		xloffset = 0;
	}
	else
	{
		xloffset = -vScrollBar->coords.width;
		xroffset = 0;
	}

	// Close button
	if (closeButton)
	{
		switch (closeButton->reference.refcorner)
		{
			case BR_TL:
				closeButton->coords.xpos = xpos + closeButton->reference.xoffset + xloffset;
				closeButton->coords.ypos = ypos + closeButton->reference.yoffset;
				break;
			case BR_TR:
				closeButton->coords.xpos = xpos + width + closeButton->reference.xoffset + xroffset;
				closeButton->coords.ypos = ypos + closeButton->reference.yoffset;
				break;
			case BR_BL:
				closeButton->coords.xpos = xpos + width + closeButton->reference.xoffset + xloffset;
				closeButton->coords.ypos = ypos - height + closeButton->reference.yoffset;
				break;
			case BR_BR:
				closeButton->coords.xpos = xpos + width + closeButton->reference.xoffset + xroffset;
				closeButton->coords.ypos = ypos - height  + closeButton->reference.yoffset;
				break;
		}
	}
	
	// Maximise or reduce button
	if (reduceButton)
	{
		switch (reduceButton->reference.refcorner)
		{
			case BR_TL:
				reduceButton->coords.xpos = xpos + reduceButton->reference.xoffset + xloffset;
				reduceButton->coords.ypos = ypos + reduceButton->reference.yoffset;
				break;
			case BR_TR:
				reduceButton->coords.xpos = xpos + width + reduceButton->reference.xoffset + xroffset;
				reduceButton->coords.ypos = ypos + reduceButton->reference.yoffset;
				break;
			case BR_BL:
				reduceButton->coords.xpos = xpos + reduceButton->reference.xoffset + xloffset;
				reduceButton->coords.ypos = ypos - height + reduceButton->reference.yoffset;
				break;
			case BR_BR:
				reduceButton->coords.xpos = xpos + width + reduceButton->reference.xoffset + xroffset;
				reduceButton->coords.ypos = ypos - height + reduceButton->reference.yoffset;
				break;
		}
	}

	if (maximiseButton)
	{
		switch (maximiseButton->reference.refcorner)
		{
			case BR_TL:
				maximiseButton->coords.xpos = xpos + maximiseButton->reference.xoffset + xloffset;
				maximiseButton->coords.ypos = ypos + maximiseButton->reference.yoffset;
				break;
			case BR_TR:
				maximiseButton->coords.xpos = xpos + width + maximiseButton->reference.xoffset + xroffset;
				maximiseButton->coords.ypos = ypos + maximiseButton->reference.yoffset;
				break;
			case BR_BL:
				maximiseButton->coords.xpos = xpos + maximiseButton->reference.xoffset + xloffset;
				maximiseButton->coords.ypos = ypos - height + maximiseButton->reference.yoffset;
				break;
			case BR_BR:
				maximiseButton->coords.xpos = xpos + width + maximiseButton->reference.xoffset + xroffset;
				maximiseButton->coords.ypos = ypos - height + maximiseButton->reference.yoffset;
				break;
		}
	}

	// Rollup or unroll button
	if (unrollButton)
	{
		switch (unrollButton->reference.refcorner)
		{
			case BR_TL:
				unrollButton->coords.xpos = xpos + unrollButton->reference.xoffset + xloffset;
				unrollButton->coords.ypos = ypos + unrollButton->reference.yoffset;
				break;
			case BR_TR:
				unrollButton->coords.xpos = xpos + width + unrollButton->reference.xoffset + xroffset;
				unrollButton->coords.ypos = ypos + unrollButton->reference.yoffset;
				break;
			case BR_BL:
				unrollButton->coords.xpos = xpos + unrollButton->reference.xoffset + xloffset;
				unrollButton->coords.ypos = ypos - height + unrollButton->reference.yoffset;
				break;
			case BR_BR:
				unrollButton->coords.xpos = xpos + width + unrollButton->reference.xoffset + xroffset;
				unrollButton->coords.ypos = ypos - height + unrollButton->reference.yoffset;
				break;
		}
	}

	if (rollupButton)
	{
		switch (rollupButton->reference.refcorner)
		{
			case BR_TL:
				rollupButton->coords.xpos = xpos + rollupButton->reference.xoffset + xloffset;
				rollupButton->coords.ypos = ypos + rollupButton->reference.yoffset;
				break;
			case BR_TR:
				rollupButton->coords.xpos = xpos + width + rollupButton->reference.xoffset + xroffset;
				rollupButton->coords.ypos = ypos + rollupButton->reference.yoffset;
				break;
			case BR_BL:
				rollupButton->coords.xpos = xpos + rollupButton->reference.xoffset + xloffset;
				rollupButton->coords.ypos = ypos - height + rollupButton->reference.yoffset;
				break;
			case BR_BR:
				rollupButton->coords.xpos = xpos + width + rollupButton->reference.xoffset + xroffset;
				rollupButton->coords.ypos = ypos - height + rollupButton->reference.yoffset;
				break;
		}
	}

	// Title
	if (titleBar->title_map)
	{
		switch (titleBar->reference.refcorner)
		{
			case BR_TL:
				titleBar->title_map->coords.xpos = xpos + xloffset + titleBar->reference.xoffset;
				titleBar->title_map->coords.ypos = ypos + titleBar->reference.yoffset;
				break;
			case BR_TR:
				titleBar->title_map->coords.xpos = xpos + width + xroffset + titleBar->reference.xoffset;
				titleBar->title_map->coords.ypos = ypos + titleBar->reference.yoffset;
				break;
			case BR_BL:
				titleBar->title_map->coords.xpos = xpos + xloffset + titleBar->reference.xoffset;
				titleBar->title_map->coords.ypos = ypos - height + titleBar->reference.yoffset;
				break;
			case BR_BR:
				titleBar->title_map->coords.xpos = xpos + width + xroffset + titleBar->reference.xoffset;
				titleBar->title_map->coords.ypos = ypos - height + titleBar->reference.yoffset;
				break;
		}
		if (titleBar->centered)
		{
			if (titleBar->border == TB_TOP || titleBar->border == TB_BOTTOM)
				titleBar->title_map->coords.xpos = xpos + (width - titleBar->title_map->coords.width)/2;
			else if (titleBar->border == TB_LEFT || titleBar->border == TB_RIGHT)
				titleBar->title_map->coords.ypos = ypos - (height - titleBar->title_map->coords.height)/2;
		}
	}

	// Vertical scroll
	vScrollBar->coords.xpos   = xpos + ((vScrollBar->pos == VSP_RIGHT)?width:-vScrollBar->coords.width);
	vScrollBar->coords.ypos   = ypos - vScrollBar->up.coords.height;
	vScrollBar->coords.height = height - vScrollBar->up.coords.height - vScrollBar->down.coords.height;
	
	vScrollBar->topScroller.coords.xpos        =
	vScrollBar->middleScroller.coords.xpos     =
	vScrollBar->bottomScroller.coords.xpos     =
	vScrollBar->extendableScroller.coords.xpos = vScrollBar->coords.xpos;

	vScrollBar->up.coords.xpos = vScrollBar->coords.xpos;
	vScrollBar->up.coords.ypos = ypos;

	vScrollBar->down.coords.xpos = vScrollBar->coords.xpos;
	vScrollBar->down.coords.ypos = ypos - height + vScrollBar->down.coords.height;
	
	// Border
	if (vScrollBar->pos == VSP_RIGHT)
	{
		border->leftBorder.coords.xpos			= xpos - border->leftBorder.coords.width;
		border->rightBorder.coords.xpos			= xpos + width + vScrollBar->coords.width;
		border->topBorder.coords.xpos			= xpos + border->topleftCorner.xoffset
												  + border->topleftCorner.coords.width;
		border->bottomBorder.coords.xpos		= xpos + border->bottomleftCorner.xoffset
												  + border->bottomleftCorner.coords.width;
		
		border->topleftCorner.coords.xpos		= xpos + border->topleftCorner.xoffset;
		border->toprightCorner.coords.xpos		= xpos + width
												  + vScrollBar->coords.width
												  + border->toprightCorner.xoffset;
		border->bottomleftCorner.coords.xpos	= xpos + border->bottomleftCorner.xoffset;
		border->bottomrightCorner.coords.xpos	= xpos + width
												  + vScrollBar->coords.width
												  + border->bottomrightCorner.xoffset;
	}
	else
	{
		border->leftBorder.coords.xpos			= xpos - vScrollBar->coords.width - border->leftBorder.coords.width;
		border->rightBorder.coords.xpos			= xpos + width;
		border->topBorder.coords.xpos			= xpos
												  - vScrollBar->coords.width
												  + border->topleftCorner.xoffset
												  + border->topleftCorner.coords.width;
		border->bottomBorder.coords.xpos		= xpos - vScrollBar->coords.width
												  + border->bottomleftCorner.xoffset
												  + border->bottomleftCorner.coords.width;;

		border->topleftCorner.coords.xpos		= xpos - vScrollBar->coords.width + border->topleftCorner.xoffset;
		border->toprightCorner.coords.xpos		= xpos + width + border->toprightCorner.xoffset;
		border->bottomleftCorner.coords.xpos	= xpos - vScrollBar->coords.width + border->bottomleftCorner.xoffset;
		border->bottomrightCorner.coords.xpos	= xpos + width + border->bottomrightCorner.xoffset;
	}
	border->topBorder.coords.ypos			= ypos + border->topBorder.coords.height;
	border->leftBorder.coords.ypos			= ypos
											  + border->topleftCorner.yoffset
											  - border->topleftCorner.coords.height;
	border->rightBorder.coords.ypos			= ypos
											  + border->toprightCorner.yoffset
											  - border->toprightCorner.coords.height;
	border->bottomBorder.coords.ypos		= ypos - height;

	border->topleftCorner.coords.ypos		= ypos + border->topleftCorner.yoffset;
	border->toprightCorner.coords.ypos		= ypos + border->toprightCorner.yoffset;
	border->bottomleftCorner.coords.ypos	= ypos - height + border->bottomleftCorner.yoffset;
	border->bottomrightCorner.coords.ypos	= ypos - height + border->bottomrightCorner.yoffset;
	
	border->leftBorder.coords.height	= height
										  + border->topleftCorner.yoffset
										  - border->topleftCorner.coords.height
										  - border->bottomleftCorner.yoffset;
	border->rightBorder.coords.height	= height
										  + border->toprightCorner.yoffset
										  - border->toprightCorner.coords.height
										  - border->bottomrightCorner.yoffset;
	border->topBorder.coords.width		= width + vScrollBar->coords.width
										  - border->topleftCorner.xoffset
										  - border->topleftCorner.coords.width
										  + border->toprightCorner.xoffset;
	border->bottomBorder.coords.width	= width + vScrollBar->coords.width
										  - border->bottomleftCorner.xoffset
										  - border->bottomleftCorner.coords.width
										  + border->bottomrightCorner.xoffset;
	
	// Background
	background.x = xpos;
	background.y = ypos;
}

void Skin::UpdateScroller(float pos, float height)
{
	if (state != LOADED) return;

	if (vScrollBar->extendable)
	{
		float newval = height;
		float rheight = (float) (vScrollBar->coords.height);
		float minprop = (vScrollBar->topScroller.coords.height
						 + vScrollBar->bottomScroller.coords.height
						 + vScrollBar->middleScroller.coords.height) / rheight;
		if (newval < minprop) newval = minprop;
		if (newval > 1) newval = 1;
		vScrollBar->height = (int) (newval*rheight);
		vScrollBar->extendableScroller.coords.height = vScrollBar->height
												 - vScrollBar->topScroller.coords.height
												 - vScrollBar->bottomScroller.coords.height;
	}

	vScrollBar->topScroller.coords.ypos        = vScrollBar->up.coords.ypos
												 - vScrollBar->up.coords.height
												 - (int)(pos * (vScrollBar->coords.height - vScrollBar->height));
	vScrollBar->extendableScroller.coords.ypos = vScrollBar->topScroller.coords.ypos-vScrollBar->topScroller.coords.height;
	vScrollBar->middleScroller.coords.ypos     = vScrollBar->topScroller.coords.ypos
												 - (vScrollBar->height - vScrollBar->middleScroller.coords.height)/2;
	vScrollBar->bottomScroller.coords.ypos     = vScrollBar->topScroller.coords.ypos
												 - vScrollBar->height + vScrollBar->bottomScroller.coords.height;
}

void Skin::UpdateStates(void)
{
	if (state != LOADED) return;

	bool btnDown = (mouse.button[0] == BUTTON_DOWN || mouse.button[1] == BUTTON_DOWN || mouse.button[2] == BUTTON_DOWN);
	
	if (IsMouseOnSkin())
	{
		border->topleftCorner.state		=
		border->toprightCorner.state	=
		border->bottomleftCorner.state  = 
		border->bottomrightCorner.state =
		border->topBorder.state			=
		border->leftBorder.state        =
		border->rightBorder.state       =
		border->bottomBorder.state      = BES_ACTIVE;
	}
	else
	{
		border->topleftCorner.state		=
		border->toprightCorner.state	=
		border->bottomleftCorner.state  = 
		border->bottomrightCorner.state =
		border->topBorder.state			=
		border->leftBorder.state        =
		border->rightBorder.state       =
		border->bottomBorder.state      = BES_INACTIVE;
	}
	
	if (closeButton)
	{
		if (IsMouseOnCloseButton()) closeButton->state = btnDown?BS_PRESSED:BS_ACTIVE;
		else closeButton->state = BS_INACTIVE;
	}
	
	if (reduceButton)
	{
		if (IsMouseOnMaximiseOrReduceButton()) reduceButton->state = maximiseButton->state = btnDown?BS_PRESSED:BS_ACTIVE;
		else if (reduceButton && maximiseButton) reduceButton->state = maximiseButton->state = BS_INACTIVE;
	}

	if (unrollButton)
	{
		if (IsMouseOnRollUpOrUnrollButton()) unrollButton->state = rollupButton->state = btnDown?BS_PRESSED:BS_ACTIVE;
		else if (unrollButton && rollupButton) unrollButton->state = rollupButton->state = BS_INACTIVE;
	}

	if (IsMouseOnScrollerDownButton()) vScrollBar->down.state = btnDown?BS_PRESSED:BS_ACTIVE; else vScrollBar->down.state = BS_INACTIVE;
	if (IsMouseOnScrollerUpButton())  vScrollBar->up.state = btnDown?BS_PRESSED:BS_ACTIVE; else vScrollBar->up.state = BS_INACTIVE;
	if (IsMouseOnVScroller())
		vScrollBar->topScroller.state        =
		vScrollBar->middleScroller.state     =
		vScrollBar->bottomScroller.state     = 
		vScrollBar->extendableScroller.state = btnDown?BS_PRESSED:BS_ACTIVE;
	else
		vScrollBar->topScroller.state        =
		vScrollBar->middleScroller.state     =
		vScrollBar->bottomScroller.state     = 
		vScrollBar->extendableScroller.state = BS_INACTIVE;
}

void Skin::DrawRegion(Region *r, Coords *c, bool tiled)
{
	if (skin_gridmode.ivalue <= 0)
	{
		glBindTexture(GL_TEXTURE_2D, r->TexName);
		glBegin(GL_TRIANGLE_STRIP);

		if (tiled)
		{
			// Calculate tiling
			float xtile = (float)c->width/(float) r->coords.width;
			float ytile = (float) c->height/(float) r->coords.height;
			
			// Top left
			glTexCoord2f(0, ytile);
			glVertex2i(c->xpos, c->ypos);
			
			// Top right
			glTexCoord2f(xtile, ytile);
			glVertex2i(c->xpos+c->width, c->ypos);
			
			// Bottom left
			glTexCoord2f(0, 0);
			glVertex2i(c->xpos, c->ypos-c->height);
			
			// Bottom right
			glTexCoord2f(xtile, 0);
			glVertex2i(c->xpos+c->width, c->ypos-c->height);
		}
		else
		{
			// Top left
			glTexCoord2f(0, 1);
			glVertex2i(c->xpos, c->ypos);
			
			// Top right
			glTexCoord2f(1, 1);
			glVertex2i(c->xpos+c->width, c->ypos);
			
			// Bottom left
			glTexCoord2f(0, 0);
			glVertex2i(c->xpos, c->ypos-c->height);
			
			// Bottom right
			glTexCoord2f(1, 0);
			glVertex2i(c->xpos+c->width, c->ypos-c->height);
		}

		glEnd();
	}

	if (skin_gridmode.ivalue)
	{	
		glColor4f(1, 1, 1, 1);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_COLOR_MATERIAL);

		glBegin(GL_LINE_LOOP);

		glVertex2i(c->xpos, c->ypos);
		glVertex2i(c->xpos+c->width, c->ypos);
		glVertex2i(c->xpos+c->width, c->ypos-c->height);
		glVertex2i(c->xpos, c->ypos-c->height);

		glEnd();

		glEnable(GL_TEXTURE_2D);
		glEnable(GL_COLOR_MATERIAL);
	}
}

void Skin::DrawSkin(int width, int height)
{
	if (state != LOADED) return;

	// Draw background
	if (!isRolledUp)
	{
		if (!background.sprite || background.style == NOTIMAGED)
		{
			glColor4fv(background.color);
			glDisable(GL_TEXTURE_2D);
			glDisable(GL_COLOR_MATERIAL);	
			glBegin(GL_TRIANGLE_STRIP);
				glVertex2i(background.x, background.y);
				glVertex2i(background.x + width, background.y);
				glVertex2i(background.x, background.y - height);
				glVertex2i(background.x + width, background.y - height);
			glEnd();
		}
		else
		{
			if (background.style == IMAGED)
			{
				background.sprite->BindAndDraw(background.x, background.y - height);
			}
			else if (background.style == ANIMATED)
			{
				timer.Refresh();
				background.sprite->BindTexture();

				background.sprite->xOffset = backup_x += 0.1f * (GLfloat) timer.frametime;
				background.sprite->yOffset = backup_y = 0;
				if (background.sprite->xOffset >  1.f) background.sprite->xOffset -= 1.f;
				if (background.sprite->xOffset < -1.f) background.sprite->xOffset += 1.f;
				if (background.sprite->yOffset >  1.f) background.sprite->yOffset -= 1.f;
				if (background.sprite->yOffset < -1.f) background.sprite->yOffset += 1.f;
				background.sprite->Draw(background.x, background.y - height);

				backup_color[0] = background.sprite->color[0];
				backup_color[1] = background.sprite->color[1];
				backup_color[2] = background.sprite->color[2];
				backup_color[3] = background.sprite->color[3];
				backup_sint = (float) sin((float) timer.fTime/2.5f);
				backup_t = (float) timer.fTime/1.5f;		
				background.sprite->color[0] = 0.5f*(float) sin(backup_t)+.5f;
				background.sprite->color[1] = 0.5f*(float) sin(backup_t+g_PI_DIV_4)+.5f;
				background.sprite->color[2] = 0.5f*(float) cos(backup_t)+.5f;
				background.sprite->color[3] = 0.25f;
				background.sprite->xOffset =  backup_t * (float) cos(C_ALPHA) + (float) sin(C_ALPHA) * backup_sint;
				background.sprite->yOffset = -backup_t * (float) sin(C_ALPHA) + (float) cos(C_ALPHA) * backup_sint;
				if (background.sprite->xOffset >  1.f) background.sprite->xOffset -= 1.f;
				if (background.sprite->xOffset < -1.f) background.sprite->xOffset += 1.f;
				if (background.sprite->yOffset >  1.f) background.sprite->yOffset -= 1.f;
				if (background.sprite->yOffset < -1.f) background.sprite->yOffset += 1.f;
				background.sprite->Draw(background.x, background.y - height);
				background.sprite->color[0] = backup_color[0];
				background.sprite->color[1] = backup_color[1];
				background.sprite->color[2] = backup_color[2];
				background.sprite->color[3] = backup_color[3];
			}
		}
	}

	glShadeModel(GL_SMOOTH);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_COLOR_MATERIAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glColor4f(1, 1, 1, 1);

	// Draw border
	if (isRolledUp)
	{
		if (titleBar->border == TB_TOP)
		{
			DrawRegion(&border->topBorder.region[border->topBorder.state], &border->topBorder.coords, border->topBorder.tiled);
			DrawRegion(&border->topleftCorner.region[border->topleftCorner.state], &border->topleftCorner.coords);
			DrawRegion(&border->toprightCorner.region[border->toprightCorner.state], &border->toprightCorner.coords);
		}
		else if (titleBar->border == TB_LEFT)
		{
			DrawRegion(&border->leftBorder.region[border->leftBorder.state], &border->leftBorder.coords, border->leftBorder.tiled);
			DrawRegion(&border->topleftCorner.region[border->topleftCorner.state], &border->topleftCorner.coords);
			DrawRegion(&border->bottomleftCorner.region[border->bottomleftCorner.state], &border->bottomleftCorner.coords);
		}
		else if (titleBar->border == TB_RIGHT)
		{
			DrawRegion(&border->rightBorder.region[border->rightBorder.state], &border->rightBorder.coords, border->rightBorder.tiled);
			DrawRegion(&border->toprightCorner.region[border->toprightCorner.state], &border->toprightCorner.coords);
			DrawRegion(&border->bottomrightCorner.region[border->bottomrightCorner.state], &border->bottomrightCorner.coords);
		}
		else if (titleBar->border == TB_BOTTOM)
		{
			DrawRegion(&border->bottomBorder.region[border->bottomBorder.state], &border->bottomBorder.coords, border->bottomBorder.tiled);
			DrawRegion(&border->bottomleftCorner.region[border->bottomleftCorner.state], &border->bottomleftCorner.coords);
			DrawRegion(&border->bottomrightCorner.region[border->bottomrightCorner.state], &border->bottomrightCorner.coords);
		}
	}
	else
	{
		DrawRegion(&border->leftBorder.region[border->leftBorder.state], &border->leftBorder.coords, border->leftBorder.tiled);
		DrawRegion(&border->topBorder.region[border->topBorder.state], &border->topBorder.coords, border->topBorder.tiled);
		DrawRegion(&border->bottomBorder.region[border->bottomBorder.state], &border->bottomBorder.coords, border->bottomBorder.tiled);
		DrawRegion(&border->rightBorder.region[border->rightBorder.state], &border->rightBorder.coords, border->rightBorder.tiled);
	
		DrawRegion(&border->topleftCorner.region[border->topleftCorner.state], &border->topleftCorner.coords);
		DrawRegion(&border->toprightCorner.region[border->toprightCorner.state], &border->toprightCorner.coords);
		DrawRegion(&border->bottomleftCorner.region[border->bottomleftCorner.state], &border->bottomleftCorner.coords);
		DrawRegion(&border->bottomrightCorner.region[border->bottomrightCorner.state], &border->bottomrightCorner.coords);
	}

	// Affiche la barre de titre et les icônes de réduction, de maximisation et de fermeture
	if (titleBar->title_map) DrawRegion(titleBar->title_map, &titleBar->title_map->coords);
/*	else if (strlen(titleBar->title_str))
	{	
		SetModes(_3D_MODE_, false);		// On quitte le mode car on va écrire quelque chose, et le false sera true
	
		int titleLen = (int) strlen(titleBar->title_str);

		// Display the console title
		if (titleBar->font && titleLen)
		{
			char buf[CONSOLE_LINELENGTH] = { '\0' };
			int left = titleBar->coords.xpos + titleBar->fontsize[1];
			if (titleBar->centered) left = titleBar->middle.coords.xpos + (titleBar->middle.coords.width - titleLen*titleBar->fontsize[0])/2;
			strncpy(buf, titleBar->title_str, titleBar->middle.coords.width/titleBar->fontsize[1]);

			titleBar->font->Size(titleBar->fontsize[0], titleBar->fontsize[1]);
			titleBar->font->Draw(left, (int) (titleBar->coords.ypos)-(titleBar->coords.height-titleBar->fontsize[1])/2-titleBar->fontsize[1], "%cb+^0%s", TEXT_STYLE_KEYWORD, buf);
		}
		SetModes(_2D_MODE_, false);
	}*/

	// Draw buttons
	if (closeButton) DrawRegion(&closeButton->region[closeButton->state], &closeButton->coords);

	if (isMaximized && reduceButton) DrawRegion(&reduceButton->region[reduceButton->state], &reduceButton->coords);
	else if (!isMaximized && maximiseButton) DrawRegion(&maximiseButton->region[maximiseButton->state], &maximiseButton->coords);

	if (isRolledUp && unrollButton)
	{
		DrawRegion(&unrollButton->region[unrollButton->state], &unrollButton->coords);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_COLOR_MATERIAL);
		return;
	}
	else if (!isRolledUp && rollupButton) DrawRegion(&rollupButton->region[rollupButton->state], &rollupButton->coords);

	// Draw scrollbar
	DrawRegion(&vScrollBar->background, &vScrollBar->coords, vScrollBar->tiled);
	DrawRegion(&vScrollBar->up.region[vScrollBar->up.state], &vScrollBar->up.coords);
	DrawRegion(&vScrollBar->down.region[vScrollBar->down.state], &vScrollBar->down.coords);
	if (vScrollBar->extendable)
		DrawRegion(&vScrollBar->extendableScroller.region[vScrollBar->extendableScroller.state], &vScrollBar->extendableScroller.coords);
	DrawRegion(&vScrollBar->topScroller.region[vScrollBar->topScroller.state], &vScrollBar->topScroller.coords);
	DrawRegion(&vScrollBar->bottomScroller.region[vScrollBar->bottomScroller.state], &vScrollBar->bottomScroller.coords);
	DrawRegion(&vScrollBar->middleScroller.region[vScrollBar->middleScroller.state], &vScrollBar->middleScroller.coords);

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_COLOR_MATERIAL);
}

void Skin::SetBackgroundStyle(enum_BackgroundStyle style)
{
	backup_x = backup_y = 0.f;
	background.style = style;
}

void Skin::SetBackgroundColor(float r, float g, float b, float a)
{
	background.color[0] = r;
	background.color[1] = g;
	background.color[2] = b;
	background.color[3] = a;
}

void Skin::GiveMousePos(int x, int y)
{
	mouse.x = x;
	mouse.y = y;
	UpdateStates();
}

void Skin::GiveMouseState(MouseState *m)
{
	memcpy(&this->mouse, m, sizeof(MouseState));
	UpdateStates();
}

bool Skin::IsMouseOnSkin(void)
{
	if (isRolledUp) return IsMouseOnTitleBar();
	else return (border && titleBar &&
				 mouse.x >= border->leftBorder.coords.xpos &&
				 mouse.x <= border->rightBorder.coords.xpos + border->rightBorder.coords.width &&
				 mouse.y <= border->topBorder.coords.ypos &&
				 mouse.y >= border->bottomBorder.coords.ypos - border->bottomBorder.coords.height);
}

bool Skin::IsMouseOnVScroller(void)
{
	return (vScrollBar &&
			mouse.x >= vScrollBar->coords.xpos &&
			mouse.x <= vScrollBar->coords.xpos + vScrollBar->coords.width &&
			mouse.y <= vScrollBar->topScroller.coords.ypos &&
			mouse.y >= vScrollBar->bottomScroller.coords.ypos - vScrollBar->bottomScroller.coords.height);
}

bool Skin::IsMouseOnScrollerUpButton(void)
{
	return (vScrollBar &&
			mouse.x >= vScrollBar->up.coords.xpos &&
			mouse.x <= vScrollBar->up.coords.xpos + vScrollBar->up.coords.width &&
			mouse.y <= vScrollBar->up.coords.ypos &&
			mouse.y >= vScrollBar->up.coords.ypos - vScrollBar->up.coords.height);
}

bool Skin::IsMouseOnScrollerDownButton(void)
{
	return (vScrollBar &&
			mouse.x >= vScrollBar->down.coords.xpos &&
			mouse.x <= vScrollBar->down.coords.xpos + vScrollBar->down.coords.width &&
			mouse.y <= vScrollBar->down.coords.ypos &&
			mouse.y >= vScrollBar->down.coords.ypos - vScrollBar->down.coords.height);
}

bool Skin::IsMouseOnVScrollZone(void)
{
	return (vScrollBar &&
			mouse.x >= vScrollBar->coords.xpos &&
			mouse.x <= vScrollBar->coords.xpos + vScrollBar->coords.width &&
			mouse.y <= vScrollBar->coords.ypos &&
			mouse.y >= vScrollBar->coords.ypos - vScrollBar->coords.height);
}

bool Skin::IsMouseOnTitleBar(void)
{
	if (titleBar->border == TB_NONE) return false;
	return ((titleBar->border == TB_TOP &&
			 mouse.x >= border->topleftCorner.coords.xpos &&
			 mouse.x <= border->toprightCorner.coords.xpos + border->toprightCorner.coords.width &&
			 mouse.y <= border->topBorder.coords.ypos &&
			 mouse.y >= border->topBorder.coords.ypos - border->topBorder.coords.height) ||
			(titleBar->border == TB_LEFT &&
			 mouse.x >= border->leftBorder.coords.xpos &&
			 mouse.x <= border->leftBorder.coords.xpos + border->leftBorder.coords.width &&
			 mouse.y <= border->topleftCorner.coords.ypos &&
			 mouse.y >= border->bottomleftCorner.coords.ypos - border->bottomleftCorner.coords.height) ||
			(titleBar->border == TB_BOTTOM &&
			 mouse.x >= border->bottomleftCorner.coords.xpos &&
			 mouse.x <= border->bottomrightCorner.coords.xpos + border->bottomrightCorner.coords.width &&
			 mouse.y <= border->bottomBorder.coords.ypos &&
			 mouse.y >= border->bottomBorder.coords.ypos - border->bottomBorder.coords.height) ||
			(titleBar->border == TB_RIGHT &&
			 mouse.x >= border->rightBorder.coords.xpos &&
			 mouse.x <= border->rightBorder.coords.xpos + border->rightBorder.coords.width &&
			 mouse.y <= border->toprightCorner.coords.ypos &&
			 mouse.y >= border->bottomrightCorner.coords.ypos - border->bottomrightCorner.coords.height));
}

bool Skin::IsMouseOnCloseButton(void)
{
	return (closeButton &&
			mouse.x >= closeButton->coords.xpos &&
			mouse.x <= closeButton->coords.xpos + closeButton->coords.width &&
			mouse.y <= closeButton->coords.ypos &&
			mouse.y >= closeButton->coords.ypos - closeButton->coords.height);
}

bool Skin::IsMouseOnMaximiseOrReduceButton(void)
{
	if (isMaximized && reduceButton)
	{
		return (mouse.x >= reduceButton->coords.xpos &&
				mouse.x <= reduceButton->coords.xpos + reduceButton->coords.width &&
				mouse.y <= reduceButton->coords.ypos &&
				mouse.y >= reduceButton->coords.ypos - reduceButton->coords.height);
	}
	else if (!isMaximized && maximiseButton)
	{
		return (mouse.x >= maximiseButton->coords.xpos &&
				mouse.x <= maximiseButton->coords.xpos + maximiseButton->coords.width &&
				mouse.y <= maximiseButton->coords.ypos &&
				mouse.y >= maximiseButton->coords.ypos - maximiseButton->coords.height);
	}
	else return false;
}

bool Skin::IsMouseOnRollUpOrUnrollButton(void)
{
	if (isRolledUp && unrollButton)
	{
		return (mouse.x >= unrollButton->coords.xpos &&
				mouse.x <= unrollButton->coords.xpos + unrollButton->coords.width &&
				mouse.y <= unrollButton->coords.ypos &&
				mouse.y >= unrollButton->coords.ypos - unrollButton->coords.height);
	}
	else if (!isRolledUp && rollupButton)
	{
		return (mouse.x >= rollupButton->coords.xpos &&
				mouse.x <= rollupButton->coords.xpos + rollupButton->coords.width &&
				mouse.y <= rollupButton->coords.ypos &&
				mouse.y >= rollupButton->coords.ypos - rollupButton->coords.height);
	}
	else return false;
}
