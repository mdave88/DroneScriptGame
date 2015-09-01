#include "glutils.h"

#ifdef WIN32
	#include <windows.h>
#else
	#include <sys/time.h>
#endif

#include <GL/gl.h>
#include <stdlib.h>
#include <string.h>

int vPort[4];
_state_values sval;
int DrawingState;

bool SetModes(int state)
{
	if (DrawingState == state) return false;

	DrawingState = state;

	/* grab the modes that we might need to change */
	if (state == _2D_MODE_)
	{
		// get current viewport
		glGetIntegerv(GL_VIEWPORT, vPort);
		// setup various opengl things that we need
		
		glGetIntegerv(GL_POLYGON_MODE, sval.polyMode);
		
		if (sval.polyMode[0] != GL_FILL) glPolygonMode(GL_FRONT, GL_FILL);
		if (sval.polyMode[1] != GL_FILL) glPolygonMode(GL_BACK, GL_FILL);
		
		sval.textureOn = glIsEnabled(GL_TEXTURE_2D);
		if (!sval.textureOn) glEnable(GL_TEXTURE_2D);
		
		sval.depthOn = glIsEnabled(GL_DEPTH_TEST);
		if (sval.depthOn) glDisable(GL_DEPTH_TEST);
		
		sval.lightingOn = glIsEnabled(GL_LIGHTING);        
		if (sval.lightingOn) glDisable(GL_LIGHTING);
		
		glGetIntegerv(GL_MATRIX_MODE, &sval.matrixMode); 
		
		/* i don't know if this is correct */
		sval.blendOn= glIsEnabled(GL_BLEND);        
		glGetIntegerv(GL_BLEND_SRC, &sval.blendSrc);
		glGetIntegerv(GL_BLEND_DST, &sval.blendDst);
		
		if (!sval.blendOn) glEnable(GL_BLEND);
		
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		
		glOrtho(0, vPort[2], 0, vPort[3], -1, 1);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
	}
	else if (state == _3D_MODE_)
	{
		/* put everything back where it was before */
		if (sval.polyMode[0] != GL_FILL) glPolygonMode(GL_FRONT, sval.polyMode[0]);
		if (sval.polyMode[1] != GL_FILL) glPolygonMode(GL_BACK, sval.polyMode[1]);
		
		if (sval.lightingOn) glEnable(GL_LIGHTING);
		
		/* i don't know if this is correct */
		if (!sval.blendOn)
		{
			glDisable(GL_BLEND);
			glBlendFunc(sval.blendSrc, sval.blendDst);
		}
		else glBlendFunc(sval.blendSrc, sval.blendDst);
		
		if (sval.depthOn) glEnable(GL_DEPTH_TEST);
		
		if (!sval.textureOn) glDisable(GL_TEXTURE_2D);
		
		glMatrixMode(sval.matrixMode);

		glMatrixMode(GL_PROJECTION);
		glPopMatrix();   
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();		
	}
	return true;
}

bool IsPowerOfTwo(int n)
{
	if (!n) return false;
	return (((n)&(n-1)) == 0);
}

int FindNearestPowerOf2(int n)
{
	int p2;

	for (p2 = 1; p2 < n; p2 <<= 1);

	if (p2 - n <= n-p2/2) return p2;
	else return p2/2;
}

double GetTime(void)
{
	#ifdef WIN32
		return timeGetTime();
	#else
		return (double) gethrtime();
	#endif
}

#include <stdio.h>
int GetArg(char* command, int i, char *dest)
{
	int b = 0, j = 0, k = i;
	
	while (command[b] == 32 && b < (int) strlen(command)) ++b;		// Skip white spaces at command begin
	j = b;
	while (k > 0 && j < (int) strlen(command))						// Search the start of argument i	
	{
		if (command[j] == 32)
		{
			--k;
			while (command[j] == 32 && j < (int) strlen(command)) ++j;
		}
		else ++j;
	}
	if (j >= (int) strlen(command))
	{
		strcpy(dest, command);
		return 0;	// Error
	}
	
	// Search the end of argument i
	k = j+1;
	while (command[k] != 32 && k < (int) strlen(command)) ++k;
	--k;

	memset(dest, '\0', k-j+2);
	strncpy(dest, &command[j], k-j+1);
	return 1;
}