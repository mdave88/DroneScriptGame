#ifndef __GLUTILS_H__
#define __GLUTILS_H__

/**
 * OpenGL states.
 */
typedef struct
{
	int matrixMode;
	int polyMode[2];
	int lightingOn;
	int blendOn;
	int depthOn;
	int textureOn;
	int blendSrc;
	int blendDst;
} _state_values;

/**
 * Backup modes for OpenGL state restoration.
 */
enum
{
	_2D_MODE_ = 2,
	_3D_MODE_ = 3
};

/**
 * Mouse button state.
 */
typedef enum
{
	BUTTON_UP,
	BUTTON_DOWN
} MouseButtonState;

/**
 * Mouse state.
 */
typedef struct
{
	int x, y;							/**< Mouse position with origin in bottom left screen corner. */
	MouseButtonState button[3];			/**< Mouse buttons states. */
} MouseState;

typedef float Color[4];					/**< Color type. */

#define arg_is(u,v) !strcmp(argv[u],v)	/**< Macro for argument test. */

bool SetModes(int state);

bool IsPowerOfTwo(int n);				/**< Return true if the value is a power of 2. */
int FindNearestPowerOf2(int n);			/**< Get the nearest power of 2. */
double GetTime(void);
int GetArg(char* command, int i, char *dest);

#endif	/* __GLUTILS_H__ */