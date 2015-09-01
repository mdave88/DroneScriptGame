#include "cl.h"

#ifdef WIN32
	#include <windows.h>
#endif
#include <GL/gl.h>		// OpenGL
#include <GL/glu.h>
#include <IL/ilut.h>	// DevIL
#include <stdlib.h>

bool m3d_logo_has_toggled_mode;

Sprite::Sprite(void)
{
	xOffset = yOffset = 0;
	xTiling = yTiling = 1;
	size[0] = size[1] = 1;
	color[0] = color[1] = color[2] = color[3] = 1;
	TexName = 0;
	isDefined = false;
}

Sprite::~Sprite (void)
{
	if (isDefined) glDeleteTextures(1, &TexName);  // Delete our OpenGL texture.
	isDefined = false;
}

void Sprite::SetSize(int x, int y)
{
	if (x >= 0) size[0] = x;
	if (y >= 0) size[1] = y;
}

int Sprite::LoadImage(char *filename)
{
	// IL image ID
	ILuint ImgId = 0;

	// Generate the main image name to use
	ilGenImages(1, &ImgId);
	// Bind this image name
	ilBindImage(ImgId);
	// Loads the image specified by File into the ImgId image
	if (!ilLoadImage(filename))
	{
		ILenum err = ilGetError();
		printf("An error occured while loading %s: %d (%s)\n", filename, err, iluErrorString(err));
		return 0;
	}
	
	// Get image width and height
	width = (int) ilGetInteger(IL_IMAGE_WIDTH);
	height = (int) ilGetInteger(IL_IMAGE_HEIGHT);
	bpp = (int) ilGetInteger(IL_IMAGE_BYTES_PER_PIXEL);

	// Enable texturing
	glEnable(GL_TEXTURE_2D);

	// Goes through all steps of sending the image to OpenGL
	TexName = ilutGLBindTexImage();

	// We're done with our image, so we go ahead and delete it
	ilDeleteImages(1, &ImgId);

	isDefined = true;

	return 1;
}

void Sprite::BindTexture(void)
{
	if (isDefined) glBindTexture(GL_TEXTURE_2D, TexName);
}

void Sprite::Draw(int x, int y)
{
	if (!isDefined) return;
	
	// setup various opengl things that we need
	m3d_logo_has_toggled_mode = SetModes(_2D_MODE_);	

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_COLOR_MATERIAL);
	glShadeModel(GL_SMOOTH);
	glColor4fv(color);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBegin(GL_TRIANGLE_STRIP);																	
		// Top Left
		glTexCoord2f(xOffset, yTiling + yOffset);
		glVertex2i(x, y+size[1]);
		// Top Right
		glTexCoord2f(xTiling + xOffset, yTiling + yOffset);
		glVertex2i(x+size[0], y+size[1]);
		// Bottom Left
		glTexCoord2f(xOffset, yOffset);
		glVertex2i(x, y);
		// Bottom Right
		glTexCoord2f(xTiling + xOffset, yOffset);
		glVertex2i(x+size[0], y);
	glEnd();

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_COLOR_MATERIAL);

	if (m3d_logo_has_toggled_mode) SetModes(_3D_MODE_);
}

void Sprite::BindAndDraw(int x, int y)
{
	BindTexture();
	Draw(x, y);
}

int Sprite::GetWidth(void)
{
	return width;
}

int Sprite::GetHeight(void)
{
	return height;
}

int Sprite::GetBPP(void)
{
	return bpp;
}