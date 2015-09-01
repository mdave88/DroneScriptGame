#ifndef __CONSLINE_H__
#define __CONSLINE_H__

#include "const.h"
#include "font.h"

typedef char ConsLineContent[CONSOLE_LINELENGTH];

#define CUTTING_CHARS	" _.:,;\\¦|/*-+<>!?"	/** Chars that can make a line cut */

class ConsLine
{
	private:
		Font *font;							/**< Pointer on the font class (to get character width). */

		/**
		 * Recopy the last font parameters (bold, color, italic, ...) at the begin of the new line
		 * @param n The currently supported line.
		 */
		void RecopyFormatting(int n);

	public:
		ConsLineContent content;			/**< Line content (simple content). */
		ConsLineContent *lines;				/**< Line content (autocutted content - updated by Update()). */
		int length;							/**< Line length (number of characters). */
		int nlines;							/**< Number of lines that content takes when displayed. */
		int *breakPos;						/**< Postions of line breaks. */
		int breakPosSize;					/**< Break position table size. */
		
		/**
		 * Initialize the line.
		 * @param f The font that is used for rendering.
		 */
		void Init(Font *f);

		/**
		 * Destroy the line.
		 */
		void Shut(void);

		/**
		 * Reinitialize the line.
		 * Don't realloc breakPos table.
		 */
		void ReInit(void);

		/**
		 * Update the line.
		 * The line update itself in function of given line length.
		 * @param linewidth The line width.
		 * @return The number of lines occupied by the line
		 */
		int Update(int linewidth);

		/**
		 * Set the line content.
		 * @param c The line content.
		 */
		void SetContent(const char *c, ...);

		/**
		 * Get the line content.
		 * @param p The number of desired content line.
		 * @return A pointer to a null ended line. If parameter is negative or bigger
		 * than the number of lines, the full line content is returned.
		 */
		char *GetContent(int p = -1);

		/**
		 * Sets the used font.
		 * @param f The font that is used for rendering.
		 */
		void SetFont(Font *f);

		/**
		 * Gets the line width.
		 * The function adds the width of all (or len, if > 0) characters of the line.
		 * The function is only designed for command line.
		 * @param len The number of character to consider.
		 * @param start The start caracter position.
		 * @return The line width.
		 */
		int GetWidth(int len = -1, int start = 0);

		/**
		 * Gets the number of char that can take place in a given width.
		 * @param width The width that is used for test.
		 * @param start The start position for getting the len
		 * @return The number of characters that can be drawn in the fiven width
		 */
		int GetLen(int width, int start = 0);

		/**
		 * Find the character that is at a certain distance from a start point.
		 * This function is only designed for command line.
		 * @param dist The distance between characters (in term of width).
		 * @param start The position of start character of the string.
		 * @return The index of required character.
		 */
		int FindChar(int dist, int start);
};

#endif	/* __CONSLINE_H__ */