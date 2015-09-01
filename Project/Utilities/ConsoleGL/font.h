#ifndef __FONT_H__
#define __FONT_H__

#include "sprite.h"

// colors are divided by 2 if we have overbright bits !!
static const int text_colors[10][4]=
{
	{ 1, 1, 1, 1 },	// white	0
	{ 1, 0, 0, 1 },	// red		1
	{ 0, 1, 0, 1 },	// green	2
	{ 0, 0, 1, 1 },	// blue		3
	{ 1, 0, 1, 1 },	// magenta	4
	{ 1, 1, 0, 1 },	// yellow	5
	{ 0, 1, 1, 1 },	// cyan		6
	{ 0, 0, 0, 1 },	// black	7
	{ 0, 0, 0, 1 },	// black	8
	{ 0, 0, 0, 1 }	// black	9
};

/**
 * Font class.
 * @todo Write a new font system that support fixed (and not fixed ?) TrueType fonts
 */
class Font
{
	private:
		Sprite charmap;					/**< Texture containing characters. */
		float tPoints[256][2];			/**< Table contenant la position de chaque lettre */
		float tWidth[256];				/**< Table of character width in the texture map.*/
		int width[256];					/**< Table of character width for rendering.*/
		float tHeight;					/**< Char height in texture map. */
		int height;						/**< Char height. */

	protected:
		int fontMaxLen;					/**< Max lenght for a string. */
		int fontTabSpace;				/**< Tab size. */
		float fontITOF;					/**< Color percent. */
		int fontItalic;					/**< Italic font size. */

		float ratio;					/**< Ratio between width and height. */
		float scale;					/**< Scaling factor. */

		float fgColor[4];				/**< Foreground color, default white. */
		float gdColor[4];   			/**< Gradient color, default gray. */
		float bgColor[4];   			/**< Background color, default gray. */
		int ysize;      				/**< Size of text, default 12. */
		int shadow;         			/**< Shadow text? default 0. */
		int gradient;       			/**< Gradient? default 0. */
		int italic;         			/**< Italic amount, defaul 0. */
		int bold;           			/**< Bold text?. */
		unsigned int texId; 			/**< Texture id. */

		void Reset(void);
		void ColorCopy(float *dest, float *src);
		void DrawChar(unsigned char c, int x, int y, int ysize, int shadow);
		void RenderChar(unsigned char c, int x, int y, int ysize);
		void ForeColorReset(void);
		void ShadowColorReset(void);
		void GradientColorReset(void);
		int SlashParser(char *buffPtr, int *x, int *y);
		int HutParser(char *buffPtr, int *x, int *y);
		void WalkString(char *buffPtr, int xpos, int ypos, int *vPort, int dir = 1);
		int SetColorFromToken(char *s);
		int ItalicsMode(char *s);
		int BoldMode(char *s);
		int GetCharHits(char *s, char f);
		int GetTrueLen(char *s);
	
	public:
	
		Font(void);
		~Font(void);
		int Load(char *name);
		virtual void Draw(int xpos, int ypos, char *s, ...);		/**< Display a string on screen (left to right) */
		void BackDraw(int xpos, int ypos, char *s, ...);			/**< Display a string on screen (right to left) */
		int GetSize(void);
		void SetRatio(float r);
		float GetRatio(void);
		void SetScale(float s);
		float GetScale(void);
		void Shadow(void);
		void Gradient(void);
		void Color3f(float r, float g, float b);
		void Color4f(float r, float g, float b, float a);
		void Color3fv(float *clr);
		void Color4fv(float *clr);
		void ShadowColor3f(float r, float g, float b);
		void ShadowColor4f(float r, float g, float b, float a);
		void ShadowColor3fv(float *clr);
		void ShadowColor4fv(float *clr);
		void GradientColor3f(float r, float g, float b);
		void GradientColor4f(float r, float g, float b, float a);
		void GradientColor3fv(float *clr);
		void GradientColor4fv(float *clr);

		/**
		 * Gets the width of a character in pixels and current ratio.
		 * The given width is scaled by 'ratio' factor.
		 * @param c The character to get width.
		 * @return The character width.
		 */
		float GetWidth(char c);

		/**
		 * Generate a destination string free of all keycodes.
		 * The function remove all codes (bold, italic, color...) from
		 * the source string and put the new string in dest string.
		 * @param dest The destination string.
		 * @param source The source string.
		 */
		void RemoveKeyCodes(char *dest, char *source);
};

#endif	/* __FONT_H__ */