#ifndef __CONST_H__
#define __CONST_H__

#define C_NMESSAGELINES				10		/**< Number of message lines. */
#define C_NCOMMHISTORYLINES			1024	/**< Number of lines for console commands history. */
#define CONSOLE_LINELENGTH			1025	/**< Console line length. */

#define TEXT_STYLE_KEYWORD			'¬'		/**< Character introducing a style modification. */
#define COMMAND_SEPARATOR			';'		/**< Character that separate two commands. */
#define COMMAND_PIPE				'|'		/**< Pipe character. */
#define STR_ASK						"/?"	/**< Symbol that is used to get help about a command. */

#define LINESBLOC					64		/**< Size of bloc for console lines allocation */
#define HISTBLOC					32		/**< Size of bloc for command history lines allocation */
#define MAXPROMPTLEN				25		/**< Maximum prompt length */
#define RESIZE_SQUARE				16		/**< Size of resize zones
											 *   @todo Change resizing system -> use console borders */
#define TAB_LEN						5		/**< Tabulations size */
#define DELETE_CHAR					127		/**< Number of delete char */
#define N_LINES_MAX					128		/**< Maximum number of lines for a single message */
											/**< @todo What happens is message is longer ?? */

#define g_PI			3.14159265358979323846f						/**< Pi */
#define g_PIL			3.14159265358979323846264338327950288f		/**< Longer Pi */
#define g_PID			3.14159265358979323846264338327950288419716939937510	/**< Longer longer Pi */
#define g_PI_DIV_360	0.0087266462599716478846184538424417		/**< Pi/360 */
#define g_2_PI			6.28318530717958623200f						/**< 2*Pi */
#define g_PI_DIV_2		1.57079632679489655800f						/**< Pi/2 */
#define g_PI_DIV_4		0.78539816339744827900f						/**< Pi/4 */
#define g_2_PI_DIV_3	2.09439510239319549231f						/**< 2*Pi/3 */
#define g_INV_PI		0.31830988618379069122f						/**< 1/Pi */

#define FILENAME_LENGTH				1025	/**< maximum filename length */
#define PATH_LENGTH					1025	/**< maximum path length */

#endif	/* __CONST_H__ */