//-----------------------------------------------------------------------------
// Console
//-----------------------------------------------------------------------------

#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include "const.h"
#include "glutils.h"
#include "skin.h"
#include "consline.h"
#include "font.h"
#include "commands.h"
#include "alias.h"
#include "timer.h"

/**
 * Console elements.
 */
typedef enum
{
	CLOSE_BUTTON,						/**< Console close button */
	MAXIMISE_BUTTON,					/**< Console maximise button */
	REDUCE_BUTTON,						/**< Console reduce button */
	SCROLL_BUTTON,						/**< Console scroll button */
	SCROLLER_UP_BUTTON,					/**< Console scroller up button */
	SCROLLER_DOWN_BUTTON,				/**< Console scroller down button */
	BACKGROUND							/**< Console background */
} enum_ConsoleElements;

/**
 * Console states.
 */
typedef enum
{
	OPEN,								/**< Console is open. */
	OPENING,							/**< Console is opening. */
	CLOSED,								/**< Console is closed. */
	CLOSING								/**< Console is closing. */
} enum_ConsoleStates;

/**
 * Console types.
 */
typedef enum
{
	C_STATIC,							/**< Console is static and covers the superior half of window (similar to quake console). */
	C_FLYING							/**< Console is movable and resizable. */
} enum_ConsoleType;

/**
 * Console vertical scrolling directions.
 */
typedef enum
{
	UP,									/**< Scroll the console lines up. */
	DOWN,								/**< Scroll the console lines down. */
	TOP,								/**< Go to the top of the console. */
	BOTTOM								/**< Go to the last line of the console. */
} enum_ConsoleScrollDir;

/**
 * Console cursor scrolling directions.
 */
typedef enum
{
	LEFT = -1,							/**< Move the cursor to the left. */
	RIGHT = 1,							/**< Move the cursor to the right. */
	C_EXTREM_LEFT = -(CONSOLE_LINELENGTH+1),	/**< Move the cursor to the beginning of the line. */
	C_EXTREM_RIGHT = CONSOLE_LINELENGTH+1		/**< Move the cursor to the end of the line. */
} enum_ConsoleCursorScroll;

/**
 * Command search modes.
 */
typedef enum
{
	LISTED,								/**< Commands are displayed in a list. */
	SEQUENCED							/**< Commands are given one after other. */
} enum_ConsoleCommandSearchMode;

typedef void (*FuncPointer)(void);		/**< Function pointer. */

/**
 * Console class.
 * The console appears on the top of the screen and can be used to enter
 * commands. It is also used to display messages or variables values.
 * The console is composed with to parts : the console and the miniconsole.
 * The miniconsole is a bunch of text lines that appear at the top of the
 * window. It can be used to display messages while console is closed.
 * @todo Replace the console tables with lists or vectors.
 * @todo Code a default font. Console is unreadable if no font was loaded. At least, change console
 *       constructor so it requires a charmap as parameter.
 * @todo Code a default skin.
 * @todo Think about derivating the console from an iostram class.
 * @todo Don't use GLUT in the library, and don't make difference with "keys" and "special keys".
 * @todo Separate the core of console and graphical part of the console. This should allow to make a skinable console.
 * @todo Improve vertical scrolling with mouse (specially when scroller is big - the console scroll only if mouse goes upper the scroller).
 * @todo Change some public variables to Var type.
 * @todo Think about having "class Console : public Skin".
 * @todo Implement Levenstein algo.
 * @todo Implement grep function.
 * @todo Implement pipes support.
 * @bug There is a bug with scrolling and maximisation and resizing (ex. with scrolling to first line).
 */
class Console
{
	protected:
		ConsLine messageLines[C_NMESSAGELINES];				/**< Miniconsole lines */
		int currently_used_message_lines;					/**< Current message line buffer */
		float messageLife[C_NMESSAGELINES];					/**< Miniconsole message lifetime */
		bool isActive;										/**< Is console active */

		int nbrAllocatedLines;								/**< Number of allocated lines */
		int nbrUsedLines;									/**< Number of console lines */
		ConsLine *consoleLines;								/**< Console content */

		int maxTextLineLength;								/**< Maximum length for a line text. */

		int scrollVal;										/**< Scrolling cursor value */
		int initvScrollY;									/**< Initial mouse position when stating scrolling */
		int vScrollY;										/**< Vertical scroll position */

		char cursorSymbol;									/**< Cursor symbol */
		bool isCursorCentered;								/**< Cursor symbol alignement */
		int cursorPos;										/**< Cursor position (0 = at the end of command line) */
		bool insert;										/**< Console command line is in insertion mode */

		int height;											/**< Console text zone height */
		int width;											/**< Console text zone width */
		int leftPos;										/**< Console text zone left position */
		int topPos;											/**< Console text zone top position */
		int nbrMaxLines;									/**< Max number of displayable lines */

		bool enableOpeningClosingAnimations;				/**< Activate opening and closing animation  (only used with STATIC type) */
		float opening_or_closing_progress;					/**< Console opening and closing animation progress */

		bool consoleIsMaximized;							/**< Maximisation console state */
		bool consoleIsRolledUp;								/**< Rolling up console state */
		int hBackup;										/**< Console window height backup */
		int wBackup;										/**< Console window width backup */
		int lBackup;										/**< Console window left backup */
		int tBackup;										/**< Console window top backup */

		int screen_Width;									/**< Main window width */
		int screen_Height;									/**< Main window height */
		int screen_xPosition;								/**< Main window left position */
		int screen_yPosition;								/**< Main window top position */

		ConsLine prompt;									/**< Prompt symbol */
		enum_ConsoleStates state;							/**< Console state */
		enum_ConsoleType type;								/**< Console type */

		enum_ConsoleCommandSearchMode searchMode;			/**< Console command searching mode */

		Font* font;											/**< Console font */
		Font* minifont;										/**< Miniconsole font */

		Skin *skin;											/**< Console skin */

		MouseState mouse;									/**< Mouse state (used to know if mouse is on console or not) */

		int histLine;										/**< Current history line */
		int lastkey;										/**< Last key pressed backup */
		int cons_left;										/**< Buffer variable that store console left position */
		int cons_top;										/**< Buffer variable that store console top position */
		int cons_width;										/**< Buffer variable that store console width */
		int cons_height;									/**< Buffer variable that store console height */
		bool console_move;									/**< Buffer variable that indicate if console is moving */
		bool console_resize;								/**< Buffer variable that indicate if console is resizing */
		bool cons_scroll;									/**< Buffer variable that indicate if console is scrolling */
		int cons_resizefrom;								/**< Buffer variable that store which corner was clicked for resizing */
		int dx, dy, dz;										/**< Backup variables for dropping */
		bool lButton, mButton, rButton;						/**< Mouse buttons state */
		double clic_delay;									/**< Double-clic counter */
		double mouse_repeat_start;							/**< Storing time for a clic */
		bool isRepeating;									/**< Defining if mouse is doing a repeat. */
		
		float backup_x, backup_y, backup_t, backup_sint;	/**< Variables used for console animation */
		Color backup_color;									/**< Variable used for console animation */

		Timer *timer;										/**< Console timer. */

		FuncPointer displayFunc;							/**< Display function pointer. */

		/**
		 * Number of lines that covers the console lines.
		 * During rendering, lines can be splitted in more than only one line in console.
		 * This variable stores the number of lines that are necessary to display all the
		 * console.
		 */
		int nbrTrueLines;

		/**
		 * Recalculates the number of displayable lines in the console.
		 * The function automatically calculates the number of lines that can
		 * be displayed in the console, in function of console height.
		 * It also call the UpdateConsoleLinesWidth() function to make console lines update.
		 */
		void Recalculate_NLines(void);

		/**
		 * Update all console lines.
		 * The function send an Update() command to all lines of the console, with
		 * the given width. All the console lines will then update themselves and
		 * compute the break line positions.
		 * @param w The new console line width.
		 */
		void UpdateConsoleLinesWidth(int w);

		/**
		 * Draw the console background.
		 * @todo Make this function customizable.
		 */
		void DrawBackground(void);

	public:

		bool addToMessages;									/**< Add messages to miniconsole */
		bool showMessages;									/**< Display the miniconsole */
	
		float msgLifeLen;									/**< Maximum message life length */
	
		float openSpeed;									/**< Console opening speed */
		float closeSpeed;									/**< Console closing speed */
		float cursorSpeed;									/**< Console cursor speed */
		float doubleclicSpeed;								/**< Speed for double clic detection */
		float mouseRepeatSpeed;								/**< Mouse repetition clic speed */

		Console(void);										/**< Default constructor. */
		~Console (void);									/**< Destructor. */

		/**
		 * Initialization function.
		 * The function initializes all variables and allocates memory for console
		 * lines. This function should be called before any other console function.
		 * @param wwidth The main window width.
		 * @param wheight The main window height.
		 */
		void Init(int wwidth, int wheight);					

		void Destroy(void);									/**< Destroy the console. */

		/**
		* Redefine new window settings.
		* This function must be used to inform the console about window parameters (position,
		* size). This can be useful if one doesn't want to have the console out of the screen
		* after a window resizing.
		* A SetWindowSettings() call should be placed in the reshape function of the main
		* program.
		* The most important parameters are width and height. Left and top parameters can
		* often be omitted.
		* @param wwidth The main window width.
		* @param wheight The main window height.
		* @param wleft The main window left.
		* @param wtop The main window top.
		*/
		void SetWindowSettings(int wwidth = -1, int wheight = -1, int wleft = -1, int wtop = -1);


		// Updating and rendering----------------------------------------------


		/**
		 * Update and redraw the console.
		 * The console should be re-updated each frame, scpecially when background is
		 * animated.
		 * @todo Remove the drawing from this function and make a second function for
		 * this.
		 */
		void Update(void);


		// Command line management --------------------------------------------


		/**
		 * Add a new line to console.
		 * The function allocates a new line for the console.
		 * @todo Put this function as private function.
		 */
		void AddLine(void);

		/**
		 * Add a line to the messages list.
		 * @param s The lien to add in messages list.
		 */
		void AddMessageLine(char* s);

		/**
		 * Updates each message lines.
		 */
		void UpdateMessageLines(void);
		
		/**
		 * Add a text line to console.
		 * @param s The new line to add to the console. The syntax is the same that the printf function.
		 */
		void Insert(const char* s, ...);

		/**
		 * Reinit the command line.
		 */
		void ReInitCurrentCommand(void);

		/**
		 * Return the command line.
		 * @return The current command.
		 */
		char* GetCurrentCommand(void);

		/**
		 * Define the command line.
		 * @param s The command to write in command line.
		 */
		void SetCurrentCommand(char* s, ...);

		/**
		 * Number of lines that are currently occupied.
		 * @return The console length.
		 */
		int GetNbrUsedLines(void);

		/**
		 * Add a character to the command line.
		 * @param c The char to add to the command line.
		 * @todo Put this function in ConsLine class.
		 */
		void AddChar(char c);

		/**
		 * Delete the last character of the command line.
		 * @todo Put this function in ConsLine class.
		 */
		void DelChar(void);

		/**
		 * Clear the console content.
		 */
		void Clear(void);

		/**
		 * Defines the autocompletion display mode for similar commands.
		 * @param mode The new mode to use.
		 */
		void SetTabMode(enum_ConsoleCommandSearchMode mode);


		// Title, prompt ------------------------------------------------------


		/**
		 * Define the console title.
		 * @param s The title string.
		 */
		void SetTitle(char *s);

		/**
		 * Define the console prompt.
		 * @param s The prompt string.
		 */
		void SetPrompt(char *s);

		/**
		 * Get the console prompt.
		 * @return A null ended string.
		 */
		char* GetPrompt(void);


		// Opening and closing ------------------------------------------------


		/**
		 * Open the console.
		 */
		void Open(void);

		/**
		 * Close the console.
		 */
		void Close(void);

		/**
		 * Get current console state.
		 * @return The console state (OPEN, CLOSED, OPENING or CLOSING).
		 */
		enum_ConsoleStates GetState(void);

		/**
		 * Toggle the console state.
		 * @return The console state (OPEN, CLOSED, OPENING or CLOSING).
		 */
		enum_ConsoleStates ToggleState(void);

		/**
		 * Define the console state.
		 * @param cs The console state.
		 */
		void SetState(enum_ConsoleStates cs);


		// Scrolling ----------------------------------------------------------


		/**
		 * Scroll console.
		 * @param dir The scrolling direction.
		 */
		void ScrollConsole(enum_ConsoleScrollDir dir);

		/**
		 * Define a position in scroll bar.
		 * The function converts the position of the mouse on scroller to scrolling
		 * value between 0 and NbrTrueLines.
		 * @param y The new position for scrollbar.
		 * @param y1 The position of mouse clic.
		 * @param mos Specify that mouse was on scroller when clicking.
		 * @todo This function should be rewritten.
		 */
		void SetVScrollYPos(int y, int y1 = -1, int mos = -1);

		/**
		 * Check if mouse is clicking on scroll button up or down.
		 * The function check if mouse left button is still down and scroll the console
		 * if mouse is on scroll button.
		 */
		void CheckScrollWithMouse(void);


		// Type management ----------------------------------------------------


		/**
		 * Get the console type.
		 * @return The console type.
		 */
		enum_ConsoleType GetType(void);

		/**
		 * Define the console type.
		 * @param console_type The new console type.
		 */
		void SetType(enum_ConsoleType console_type);

		/**
		 * Toggle the console type.
		 */
		void ToggleType(void);


		// Resizing and positioning -------------------------------------------

	
		/**
		 * Get the console width.
		 * @return The console width.
		 */
		int GetWidth(void);

		/**
		 * Get the console height.
		 * @return The console height.
		 */
		int GetHeight(void);

		/**
		 * Get the console left position.
		 * @return The console left.
		 */
		int GetLeft(void);

		/**
		 * Get the console top position.
		 * @return The console top.
		 */
		int GetTop(void);

		/**
		 * Resize the console.
		 * @param newwidth The new width for console.
		 * @param newheight The new height for console.
		 * @return An integer that indicates if resizing has effectively occured:
		 * - 0 : the console was resized successfully
		 * - 1 : only one dimension (width or height) has been resized
		 * - 2 : the console size has not changed
		 */
		int Resize(int newwidth, int newheight);

		/**
		 * Maximize the console (only in FLYING mode).
		 */
		void Maximize(void);

		/**
		 * Reduce the console to previous size (only in FLYING mode).
		 */
		void Unmaximize(void);

		/**
		 * Roll up the console.
		 */
		void Rollup(void);

		/**
		 * Unroll the console.
		 */
		void Unroll(void);

		/**
		 * Get maximization state.
		 * @return The maximization status.
		 */
		bool IsMaximized(void);

		/**
		 * Get rolling up state.
		 * @return The rollup status.
		 */
		bool IsRolledUp(void);

		/**
		 * Toggle the console maximization.
		 */
		void ToggleMaximisation(void);

		/**
		 * Toggle the console rolling.
		 */
		void ToggleRolling(void);

		/**
		 * Define the console top position.
		 * @param newtop The new top position.
		 * @param test An integer indicating if function must check for window overtaking:
		 * - test = 0 : avoid that console is going out of window
		 * - test = 1 : equal to test = 0 AND test = 2
		 * - test = 2 : avoid that console moves when maximized
		 * - test < 0 : no test
		 * @param include_skin By default, the skin is not considered as a member of the console.
		 * If the value is non null, the given top position will be the top of the console skin.
		 */
		void SetTopPos(int newtop, int test = 1, int include_skin = 0);

		/**
		 * Define the console left position.
		 * @param newleft The new left position.
		 * @param test An integer indicating if function must check for window overtaking:
		 * - test = 0 : avoid that console is going out of window
		 * - test = 1 : equal to test = 0 AND test = 2
		 * - test = 2 : avoid that console moves when maximized
		 * - test < 0 : no test
		 * @param include_skin By default, the skin is not considered as a member of the console.
		 * If the value is non null, the given left position will be the left of the console skin.
		 */
		void SetLeftPos(int newleft, int test = 1, int include_skin = 0);


		// Mouse control ------------------------------------------------------


		/**
		 * Determine if mouse is on the console.
		 * @return true if mouse is on the console.
		 */
		bool IsMouseOnConsole(void);

		/**
		 * Determine if mouse is on a resize corner.
		 * @return An integer corresponding to mouse position :
		 * 1 -> corner upper left
		 * 2 -> corner upper right
		 * 3 -> corner lower left
		 * 4 -> corner lower right
		 * 0 -> no corner
		 */
		int IsMouseOnResizeSquare(void);

		/**
		 * Determine if mouse is on the scroll button.
		 * @return true if mouse is on scroll button.
		 */
		bool IsMouseOnVScrollSquare(void);

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
		</pre>
		 */
		int MouseIsInsideWindow(int x, int y, int xoffset = 0, int yoffset = 0);


		// Cursor management --------------------------------------------------


		/**
		 * Move the cursor horizontally.
		 * @param d The desplacement factor.
		 */
		void MoveCursor(enum_ConsoleCursorScroll d);

		/**
		 * Define the cursor blinking speed.
		 * @param f The new cursor speed.
		 */
		void SetCursorSpeed(float f);

		/**
		 * Define the cursor symbol.
		 * @param c The new cursor symbol.
		 */
		void SetCursorSymbol(char c);


		// Font ---------------------------------------------------------------


		/**
		 * Define the char table for console.
		 * @param c The char table file name.
		 * @param n Determines which part of the console is concerned :
		 * - 1 The normal console
		 * - 2 The miniconsole
		 * - 0 Both consoles
		 */
		void SetCharTable(char* c, int n = 0);

		/**
		 * Get the console font height.
		 * @param n Determine which console font height is requiered :
		 * - 1 The normal console
		 * - 2 The miniconsole
		 * @return The console font height.
		 */
		int GetFontHeight(int n = 1);

		/**
		 * Set the console font ratio.
		 * @param ratio The new font ratio.
		 * @param n Determines which part of the console is concerned :
		 * - 1 The normal console
		 * - 2 The miniconsole
		 * - 0 Both consoles
		 */
		void SetFontRatio(float ratio, int n = 1);

		/**
		 * Gets the console font ratio.
		 * @param n Determine which console font size is requiered :
		 * - 1 The normal console
		 * - 2 The miniconsole
		 */
		float GetFontRatio(int n = 1);

		/**
		* Set the console font scaling factor.
		* @param scale The new font scaling factor.
		* @param n Determines which part of the console is concerned :
		* - 1 The normal console
		* - 2 The miniconsole
		* - 0 Both consoles
		*/
		void SetFontScale(float scale, int n = 1);

		/**
		* Gets the console font scaling factor.
		* @param n Determine which console font size is requiered :
		* - 1 The normal console
		* - 2 The miniconsole
		*/
		float GetFontScale(int n = 1);


		// Aspect (skin) ------------------------------------------------------

		/**
		 * Load the console skin.
		 * @param filename The skin filename.
		 * @return An integer that indicates if loading was successfull (1) or not (0).
		 */
		int LoadSkin(char *filename);


		// Keyboard and mouse management --------------------------------------


		/**
		 * Give to the console the control of keyboard.
		 * @param key The key that was pressed.
		 * @param x the x position
		 * @param y the y position
		 * @return An integer that indicates if console has used the key (1) or not (0).
		 */
		int Keyboard(unsigned char key, int x, int y);

		/**
		 * Give to the console the control of keyboard special keys.
		 * @param key The key that was pressed.
		 * @param x the x position
		 * @param y the y position
		 * @return An integer that indicates if console has used the key (1) or not (0).
		 */
		int KeyboardSpec(unsigned char key, int x, int y);

		/**
		 * Give to the console the control of mouse clics.
		 * @param button The mouse button that has changed his state.
		 * @param s The state mouse button.
		 * @param x The mouse x position
		 * @param y The mouse y position
		 * @return An integer that indicates if console has used the mouse (1) or not (0).
		 */
		int MouseClic(int button, int s, int x, int y);

		/**
		 * Give to the console the control of mouse movements.
		 * @param x The mouse x position
		 * @param y The mouse y position
		 * @return An integer that indicates if console has used the mouse (1) or not (0).
		 */
		int MouseMove(int x, int y);


		// Other functions ----------------------------------------------------


		/**
		 * Get data from the console.
		 * Use this function for debugging.
		 */
		void GetStats(void);

		/**
		 * Give to the console the display func for updating.
		 * @param func A pointer to display func in main program.
		 */
		void SetDisplayFunc(FuncPointer func);

		/**
		 * Update the display using the display function.
		 */
		void UpdateDisplay();

		/**
		 * Get the mouse position.
		 * The function get mouse position and transmit it to skin.
		 * @param x The horizontal mouse position
		 * @param y The vertical mouse position
		 * @param xoff An offset on x axis (the window left position)
		 * @param yoff An offset on y axis (the window top position)
		 */
		void GiveMousePos(int x, int y, int xoff, int yoff);

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
};

#endif	/* __CONSOLE_H__ */