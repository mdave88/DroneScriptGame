//-----------------------------------------------------------------------------
// Skin
//-----------------------------------------------------------------------------

#ifndef __SKIN_H__
#define __SKIN_H__

#ifdef WIN32
	#include <windows.h>
#endif
#include <IL/il.h>
#include <GL/gl.h>
#include "glutils.h"
#include "timer.h"

class Parser;												/**< Parser class declaration */
class Sprite;												/**< Sprite class declaration */
class Font;													/**< Font class declaration */

#define C_ALPHA 0.785398163f								/**< Sinus rotation angle for animation */
#define BUTTON_GAP 1.2f										/**< Gap between buttons */

typedef int vecpos2[2];										/**< Integer 2 coordinates position */

// Keywords list
#define str_OPENBLOCK			"{"
#define str_CLOSEBLOCK			"}"
#define str_SKIN_MAIN			"SKIN_MAIN"
#define str_SKIN_MAP			"SKIN_MAP"
#define str_SKIN_BACKGROUND		"SKIN_BACKGROUND"
#define str_SKIN_BACKCOLOR		"SKIN_BACKCOLOR"
#define str_INACTIVE			"INACTIVE"
#define str_ACTIVE				"ACTIVE"
#define str_PRESSED				"PRESSED"
#define str_RELEASED			"RELEASED"
#define str_CLOSE_BUTTON		"CLOSE_BUTTON"
#define str_MAXIMISE_BUTTON		"MAXIMISE_BUTTON"
#define str_REDUCE_BUTTON		"REDUCE_BUTTON"
#define str_ROLLUP_BUTTON		"ROLLUP_BUTTON"
#define str_UNROLL_BUTTON		"UNROLL_BUTTON"
#define str_TITLE_BAR			"TITLE_BAR"
#define str_VSCROLL_BAR			"VSCROLL_BAR"
#define str_TITLE_BORDER		"TITLE_BORDER"
#define str_TITLE_STRING		"TITLE_STRING"
#define str_TITLE_MAP			"TITLE_MAP"
#define str_TITLE_FONTSIZE		"TITLE_FONTSIZE"
#define str_TITLE_MODE			"TITLE_MODE"
#define str_CENTERED			"CENTERED"
#define str_UP_BUTTON			"UP_BUTTON"
#define str_DOWN_BUTTON			"DOWN_BUTTON"
#define str_SCROLLER			"SCROLLER"
#define str_TOP					"TOP"
#define str_MIDDLE				"MIDDLE"
#define str_BOTTOM				"BOTTOM"
#define str_LEFT				"LEFT"
#define str_RIGHT				"RIGHT"
#define str_NONE				"NONE"
#define str_EXTENDABLE			"EXTENDABLE"
#define str_BACKGROUND			"BACKGROUND"
#define str_BORDER				"BORDER"
#define str_TOP_LEFT_CORNER		"TOP_LEFT_CORNER"
#define str_TOP_RIGHT_CORNER	"TOP_RIGHT_CORNER"
#define str_BOTTOM_LEFT_CORNER	"BOTTOM_LEFT_CORNER"
#define str_BOTTOM_RIGHT_CORNER	"BOTTOM_RIGHT_CORNER"
#define str_TOP_BORDER			"TOP_BORDER"
#define str_LEFT_BORDER			"LEFT_BORDER"
#define str_BOTTOM_BORDER		"BOTTOM_BORDER"
#define str_RIGHT_BORDER		"RIGHT_BORDER"
#define str_DIMS				"DIMS"
#define str_FILLMODE			"FILLMODE"
#define str_TILED				"TILED"
#define str_STRETCHED			"STRETCHED"
#define str_POSITION			"POSITION"
#define str_RELATIVE			"RELATIVE"
#define str_TL					"TL"
#define str_TR					"TR"
#define str_BL					"BL"
#define str_BR					"BR"

/**
 * Skin state.
 */
typedef enum
{
	UNLOADED,					/**< No skin loaded */
	LOADED,						/**< Skin loaded and working */
	LOADING						/**< Skin loading */
} SkinState;

/**
 * Background styles.
 */
typedef enum
{
	NOTIMAGED,					/**< Console background has no image */
	IMAGED,						/**< Console background has an image */
	ANIMATED					/**< Console background is animated */
} enum_BackgroundStyle;

/**
 * Element coordinates
 */
typedef struct
{
	int xpos;					/**< X position of region top left corner for screen display */
	int ypos;					/**< Y position of region top left corner for screen display */
	int width;					/**< Rendered region widht */
	int height;					/**< Rendered region height */
} Coords;

/**
 * Region structure.
 */
typedef struct
{
	Coords coords;				/**< Region data coordinates in texture */
	GLuint TexName;				/**< OpenGL texture name. */
} Region;

// Elements states ------------------------------------------------------------

/**
 * Button states
 */
typedef enum
{
	BS_INACTIVE = 0,			/**< Button is inactive */
	BS_ACTIVE   = 1,			/**< Button is active */
	BS_PRESSED  = 2				/**< Button is pressed */
} ButtonState;

/**
 * Button reference corner
 */
typedef enum
{
	BR_TL = 0,					/**< Reference is text zone top left corner */
	BR_TR = 1,					/**< Reference is text zone top right corner */
	BR_BL = 2,					/**< Reference is text zone bottom left corner */
	BR_BR = 3					/**< Reference is text zone bottom right corner */
} ButtonReferenceCorner;

/**
 * Titlebar states
 */
typedef enum
{
	TS_INACTIVE = 0,			/**< Titlebar is inactive */
	TS_ACTIVE   = 1				/**< Titlebar is active */
} TitleBarState;

/**
 * Titlebar border
 */
typedef enum
{
	TB_NONE		= 0,			/**< The console has no titlebar */
	TB_TOP		= 1,			/**< Titlebar role is done by top border */
	TB_LEFT		= 2,			/**< Titlebar role is done by left border */
	TB_RIGHT	= 3,			/**< Titlebar role is done by right border */
	TB_BOTTOM	= 4				/**< Titlebar role is done by bottom border */
} TitleBarBorder;

typedef enum
{
	VSP_RIGHT = 0,				/**< Scrollbar is positioned at the right of text zone */
	VSP_LEFT = 1				/**< Scrollbar is positioned at the left of the text zone */
} ScrollbarPos;

/**
 * Border elements states
 */
typedef enum
{
	BES_INACTIVE = 0,			/**< Border element is inactive */
	BES_ACTIVE   = 1			/**< Border element is active */
} BorderElementState;

// Skin elements --------------------------------------------------------------

/**
 * Button reference
 */
typedef struct
{
	ButtonReferenceCorner refcorner;	/**< Button reference corner */
	int xoffset;				/**< Offset on the horizontal axis */
	int yoffset;				/**< Offset on the vertical axis */
} ButtonReference;

/**
 * Button structure.
 */
typedef struct
{
	Region region[3];			/**< Inactive, active and pressed button skins */
	ButtonState state;			/**< Button state */
	Coords coords;				/**< Button screen coordinates */
	ButtonReference reference;	/**< Button reference for drawing */
} Button;

/**
 * Titlebar structure.
 */
typedef struct
{
	Region *title_map;			/**< Title map region */
	char *title_str;			/**< Title bar title string */
	int fontsize[2];			/**< Title bar font size (height and width) */
	bool centered;				/**< Title is centered on titlebar */
	Font* font;					/**< Title font (same as console) */
	TitleBarBorder border;		/**< Border which makes title bar role */
	ButtonReference reference;	/**< Title bar reference for drawing title string map */
} TitleBar;

/**
 * Vertical scrollbar structure.
 */
typedef struct
{
	Button up;					/**< Scroller up button */
	Button down;				/**< Scroller down button */
	Button topScroller;			/**< Scroller top part */
	Button middleScroller;		/**< Scroller middle part */
	Button bottomScroller;		/**< Scroller bottom part */
	Button extendableScroller;	/**< Scroller extendable part */
	Region background;			/**< Scroll bar zone background */
	Coords coords;				/**< Scroll bar background screen coordinates */
	int height;					/**< Scroller total height (including top and bottom parts) */
	bool tiled;					/**< Scrollbar background filling mode */
	bool extendable;			/**< Scroller is extendable (if false, extendableScroller is not required) */
	ScrollbarPos pos;			/**< Scrollbar position */
} VScrollBar;

/**
 * Border element.
 */
typedef struct
{
	Region region[2];			/**< Inactive and active region skins */
	BorderElementState state;	/**< Border element state */
	Coords coords;				/**< Border element screen coordinates */
	bool tiled;					/**< Border filling mode (only available for borders, not for corners) */
	int xoffset;				/**< Offset on the horizontal axis (only for border corners) */
	int yoffset;				/**< Offset on the vertical axis (only for border corners) */
} BorderElement;

/**
 * Border structure.
 */
typedef struct s_Border
{
	BorderElement topleftCorner;
	BorderElement toprightCorner;
	BorderElement bottomleftCorner;
	BorderElement bottomrightCorner;
	BorderElement topBorder;
	BorderElement leftBorder;
	BorderElement bottomBorder;
	BorderElement rightBorder;
} Border;

/**
 * Background structure.
 */
typedef struct
{
	int x, y;										/**< Background position (bottom left corner) */
	Sprite* sprite;									/**< Background sprite */
	Color color;									/**< Console backcolor */
	enum_BackgroundStyle style;						/**< Console background style */
} BackGround;

/**
 * SkinMap class.
 * This class manage the skin map image.
 */
class SkinMap
{
	public:
		ILuint ImgId;								/**< IL Image ID */
		int map_width;								/**< Texture width. */
		int map_height;								/**< Texture height. */
		int map_bpp;								/**< Texture byte per pixel. */

		/**
		 * Default constructor.
		 * Load the map using DevIL library.
		 * @param filename The name of file to load.
		 */
		SkinMap(char *filename);
		~SkinMap(void);								/**< Default destructor. */

		/**
		 * Create an OpenGL texture using a region of skin map data.
		 * @param r The destination region.
		 */
		void GetRegion(Region *r);
};

/**
 * Skin class.
 * The skin class is responsible for loading console skin. It support format
 * described in "skin.txt" file.
 * The skin active, inactive or other states cannot be updated in the Skin class.
 * It must be done by the function that manage inputs. In our case, this is done
 * by Console class.
 * @todo Terminate the skin class structure and implement it. The class should
 *       also manage mouse position for interaction, because Console shouldn't
 *       know border width, button size, etc.
 * @bug There is a bug with right border: the border is not correctly rendered.
 */
class Skin
{
	private:
		SkinState	state;					/**< Internal skin state */

		Parser		*p;						/**< File parser */
		MouseState	mouse;					/**< Current mouse state */

		Color		backup_color;
		float		backup_sint;
		float		backup_t;
		float		backup_x;
		float		backup_y;		

		/**
		 * Parse a button from skin file.
		 * @param b The destination button
		 */
		int ParseButton(Button *b);

		/**
		 * Parse a title bar from skin file.
		 * @param t The destination title bar
		 */
		int ParseTitleBar(TitleBar *t);

		/**
		 * Parse a vertical scroll bar from skin file.
		 * @param v The destination scroll bar
		 */
		int ParseVScrollBar(VScrollBar *v);

		/**
		 * Parse a border element from skin file.
		 * @param b The destination border element
		 */
		int ParseBorderElement(BorderElement *b);

		/**
		 * Parse a border from skin file.
		 * @param b The destination border
		 */
		int ParseBorder(Border *b);

		/**
		 * Draw a region of skin map.
		 * @param r The region to draw.
		 * @param c The coordinates for display destination zone.
		 * @param tiled Define if texture must be tiled if zone is bigger than texture.
		 */
		void DrawRegion(Region *r, Coords *c, bool tiled = true);

		/**
		 * Update the skin elements states.
		 */
		void UpdateStates(void);

	public:
		SkinMap		*map;									/**< Skin map containing skin elements */
		BackGround	background;								/**< Console background */
		Button		*closeButton;							/**< Close button */
		Button		*maximiseButton;						/**< Maximise button */
		Button		*reduceButton;							/**< Reduce button */
		Button		*rollupButton;							/**< Roll up button */
		Button		*unrollButton;							/**< Unroll button */
		TitleBar	*titleBar;								/**< Title bar */
		VScrollBar	*vScrollBar;							/**< Vertical scroll bar */
		Border		*border;								/**< Border */

		Timer		timer;									/**< Skin timer */

		bool isMaximized;									/**< Maximisation console state */
		bool isRolledUp;									/**< Rolling up console state */
		
		Skin(void);											/**< Default constructor */
		~Skin(void);										/**< Default destructor */

		void Skin::Init(void);								/**< Initialize the skin elements. */
		void Skin::Shut(void);								/**< Destroy the skin elements. */
			
		/**
		 * Load a skin.
		 * The function load a skin represented by its description. The file is
		 * parsed and structures are loaded. If the skin cannot be loaded, the
		 * function return 0, else it return a non null value.
		 * @param filename The skin filename to load.
		 * @param width The console width (used to resize background)
		 * @param height The console height (used to resize background)
		 * @param font The font used for title
		 * @return A non null value is function could load skin, else 0.
		 */
		int LoadSkin(const char *filename, int width, int height, Font* font);

		/**
		 * Update the skin elements.
		 * The function update the skin elements position.
		 * The function parameters define the text zone position. The skin will be
		 * drawn around the text zone, following it size. In other words, the skin
		 * will be drawn as border of the given zone.
		 * @param xpos The top left corner x position of text zone.
		 * @param ypos The top left corner y position of text zone.
		 * @param width The width of text zone.
		 * @param height The height of text zone.
		 */
		void UpdateSkin(int xpos, int ypos,
						int width, int height);

		/**
		 * Update the scroller.
		 * The scroller position is defined between 0 and 1. The 0 correspond to the
		 * top position and the 1 correspond to the bottom position. The value is
		 * independent of scroller height.
		 * The scroller height is defined between 0 and 1. The 0 value correspond
		 * to the smaller height (in fact only the top and bottom part of the
		 * scroller will be displayed), and the 1 value correspond to the bigger
		 * height (in this case, the scroller covers all the scrollbar). The value
		 * is independent of scroller position.
		 * @param pos The new scroller position.
		 * @param height The new scroller height.
		*/
		void UpdateScroller(float pos, float height);

		/**
		 * Draw the skin in pixel buffer.
		 * @param width The console (text zone) width
		 * @param height The console (text zone) height
		 */
		void DrawSkin(int width, int height);

		/**
		 * Define the skin background style.
		 * @param style The new background style.
		 */
		void SetBackgroundStyle(enum_BackgroundStyle style);

		/**
		 * Define the skin background color.
		 * @param r The new red color component.
		 * @param g The new green color component.
		 * @param b The new blue color component.
		 * @param a The new alpha color component.
		 */
		void SetBackgroundColor(float r, float g, float b, float a);


		// Mouse control ------------------------------------------------------------


		/**
		 * Define the mouse state.
		 * The mouse coordinates are (0,0) at the bottom left corner of window. The
		 * x axis goes horizontally on the right and the y axis goes vertically up.
		 * The function get the mouse state and update the skin elements states.
		 * @param m The source mouse state.
		 */
		void GiveMouseState(MouseState *m);

		/**
		 * Define the mouse position.
		 * The fcuntion get the mouse position and update the skin elements states.
		 * @param x The mouse x position
		 * @param y The mouse y position
		 */
		void GiveMousePos(int x, int y);

		/**
		 * Determine if mouse is on the console.
		 * @return true if mouse is on the console.
		 */
		bool IsMouseOnSkin(void);

		/**
		 * Determine if mouse is on the scroll button.
		 * @return true if mouse is on scroll button.
		 */
		bool IsMouseOnVScroller(void);

		/**
		 * Determine if mouse is on the scroller up button.
		 * @return true if mouse is on scroller up buttton.
		 */
		bool IsMouseOnScrollerUpButton(void);

		/**
		 * Determine if mouse is on the scroller down button.
		 * @return true if mouse is on scroller down button.
		 */
		bool IsMouseOnScrollerDownButton(void);

		/**
		 * Determine if mouse is on scroll bar.
		 * @return true if mouse is on scroll bar.
		 */
		bool IsMouseOnVScrollZone(void);

		/**
		 * Determine if mouse is on title bar.
		 * @return true if mouse is on titlebar.
		 */
		bool IsMouseOnTitleBar(void);

		/**
		 * Determine if mouse is on close button.
		 * @return true if mouse is on close button.
		 */
		bool IsMouseOnCloseButton(void);

		/**
		 * Determine if mouse is on maximize or reduce button.
		 * @return true if mouse is on maximization (or unmaximization) button.
		 */
		bool IsMouseOnMaximiseOrReduceButton(void);

		/**
		 * Determine if mouse is on rollup or unroll button.
		 * @return true if mouse is on rollup or unroll button.
		 */
		bool IsMouseOnRollUpOrUnrollButton(void);

		/**
		 * Check if mouse is in the window.
		 * @param x The mouse x position
		 * @param y The mouse y position
		 * @param xoffset An eventual x offset relative to window position
		 * @param yoffset An eventual y offset relative to window position
		 * @return The mouse position relative to window 
		 * - 1 = in the window
		 * - 2 = out horizontally, but in vertically
		 * - 3 = in horizontally, but out vertically
		 * - 0 = totally out
		 *
		 * Output can also be seen on the following graph :
		 * <pre>
		 *      |         |
		 *   0  |    3    |  0
		 *      |         |
		 * -----+---------+------
		 *      |         |
		 *   2  |    1    |  2
		 *      |         |
		 * -----+---------+------
		 *      |         |
		 *   0  |    3    |  0
		 *      |         |	
		 * </pre>
		 */
		int MouseIsInsideWindow(int x, int y, int xoffset = 0, int yoffset = 0);
};

#endif	/* __SKIN_H__ */