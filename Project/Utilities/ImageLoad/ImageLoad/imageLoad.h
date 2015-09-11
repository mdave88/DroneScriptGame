#pragma once

//OpenGL
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <GL/glew.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <map>
#include <string.h>

typedef std::map<const std::string, GLuint>	TextureDirectory;
typedef std::map<const std::string, GLuint>	CubeTextureDirectory;

void devilInit(bool enableDebugMessages = false);

bool loadTexture(const std::string& filename, GLuint& texid, bool resize = false, int resolutionDiv = 2);
bool loadTexture(const std::string& filename, GLuint& texid, TextureDirectory& textureDirectory, bool resize = false, int resolutionDiv = 2);
bool loadCubeMap(const std::string& mapname, const std::string& extension, GLuint& texid, TextureDirectory& textureDirectory);

// debug
bool isDevILDebugEnabled();
void setDevILDebug(bool debugEnabled);

