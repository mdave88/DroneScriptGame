#ifndef __SPRITE_H__
#define __SPRITE_H__

#ifdef WIN32
	#include <windows.h>
#endif
#include <GL/gl.h>

/**
 * Sprite class.
 * Sprites are used by the console for displaying background, close button, etc. The
 * font class also use sprite to load char map. The sprite is always rendered in 2D.
 * It also can be used to display logos on screen.
 * The class uses the <a href="http://openil.sourceforge.net">DevIL library</a> for
 * loading textures.
 * @todo Make data accessible (especially for skin)
 */
class Sprite
{
	private:
		GLuint TexName;								/**< OpenGL texture name. */
		int width;									/**< Texture width. */
		int height;									/**< Texture height. */
		int bpp;									/**< Texture bytes per pixel. */
		bool isDefined;								/**< Determine if a texture is loaded and usable.
													 *   Note we could set TexName to -1, but some compilers
													 *   do not tolerate that GLuint has negative value. */

	public:
		float color[4];								/**< Color used while rendering. */
		float xOffset;								/**< Horizontal offset. */
		float yOffset;								/**< Vertical offset. */
		float xTiling;								/**< Horizontal tiling. */
		float yTiling;								/**< Vertical tiling. */
		int size[2];								/**< Bitmap width and height used for rendering. */

		Sprite(void);								/**< Default constructor. */
		~Sprite(void);								/**< Default destructor. */

		/**
		 * Define the bitmap size for rendering.
		 * @param x The horizontal size.
		 * @param y The vertical size.
		 */
		void SetSize(int x, int y);

		/**
		 * Load an image using DevIL library.
		 * @param filename The name of file to load.
		 * @return An integer value defining if loading was successful (1) or not (0).
		 */
		int LoadImage(char *filename);

		/**
		 * Draw the sprite.
		 * The bitmap position is given by parameters. The parameters define the left bottom
		 * corner of the bitmap. Bitmap size is given by the size 2x1 vector.
		 * Note: The texture must have been bound.
		 * @param x The left position of the bitmap.
		 * @param y The right position of the bitmap.
		 */
		void Draw(int x, int y);

		/**
		 * Bind the texture.
		 * This is used to have texture in memory before rendering.
		 */
		void BindTexture(void);

		/**
		 * Bind the texture and draw the bitmap.
		 * @param x The left position of the bitmap.
		 * @param y The right position of the bitmap.
		 */
		void BindAndDraw(int x, int y);

		/**
		 * Get the loaded image width.
		 * @return The sprite width.
		 */
		int GetWidth(void);

		/**
		 * Get the loaded image height.
		 * @return The sprite height.
		 */
		int GetHeight(void);

		/**
		 * Get the loaded image bytes per pixel.
		 * @return The sprite bytes per pixel.
		 */
		int GetBPP(void);
};

#endif	/* __SPRITE_H__ */