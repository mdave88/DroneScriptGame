#pragma once

#include <set>
#include <map>
#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include <time.h>

// rarely changed header files -> precompiled header
#include "Common/GameDefines.h"
#include "Common/Assert.h"
#include "Common/Singleton.h"

#ifdef CLIENT_SIDE
// Glew
#include <GL/glew.h>

// OpenGL
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif

#define USE_GLM
#ifdef USE_GLM
#include <glm/glm.hpp>
#endif

#include "Math/vec2.h"
#include "Math/vec3.h"
#include "Math/matrix.h"

#include "Common/Directory.h"

#include "Common/Utils.h"
#include "Common/ConstantManager.h"
#include "Console/CrimsonConsole.h"

#include "GameLogic/SerializationDefs.h"


#ifdef CLIENT_SIDE
void CheckGLError();
#endif


// error checking
//#define CHKHEAP()  (check_heap(__FILE__, __LINE__))
//void check_heap(char *file, int line);
