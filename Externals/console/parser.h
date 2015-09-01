//-----------------------------------------------------------------------------
// Parser
//-----------------------------------------------------------------------------

#ifndef __PARSER_H__
#define __PARSER_H__

#define FILENAME_LEN	1024		/**< Maximum size for a file name. */

/**
 * File, string and buffer parser.
 * This class defines parsing functions for files, strings and char buffers.
 * There is no limit on tokens size.
 * Commentaries must have C/C++ syntax form. Commentaries on one single line
 * can also begin with # symbol.
 */
class Parser
{
	public:
		char *token;				/**< Current token. */
		int line;					/**< Current line number. */

		Parser(void);				/**< Default constructor. */
		~Parser(void);				/**< Default destructor. */

		/**
		 * Open the file, read content and make the parser ready.
		 * The file is closed and content is stored in memory.
		 * @param name The name of the file to parse.
		 * @return An non null integer if file could be opened and a null value if not.
		 */
		int StartParseFile(const char *name);

		/**
		 * Give the string that will be parsed to the parser and make the parser ready.
		 * @param string The string to parse.
		 */
		void StartParseString(const char *string);

		/**
		 * Give the buffer that will be parsed to the parser and make the parser ready.
		 * @param buffer The buffer to parse.
		 * @param size The size of the buffer.
		 */
		void StartParseBuffer(const unsigned char *buffer, const int size);

		/**
		 * Stop the current parsing.
		 * Free memory and reinit the paser for any further use.
		 */
		void StopParse(void);

		/**
		 * Read the next token.
		 * @param crossline This parameter allows to determine if the function must report line breaks.
		 *                  If the parameter is true, the function doesn't take care of line breaks and
		 *                  return false only if the end of file is found.
		 *                  If the parameter is false, the function will return false when a line break
		 *                  or the end of file is found.
		 * @return By default, if the parser get the end of file, 0 is returned. In other cases, 1 is
		 *         returned. Read the commentary given for "crossline" parameter for more details.
		 */
		bool GetToken(bool crossline = true);
		
		/**
		 * Compare a word with current token.
		 * The function works exactly as strcmp :
		 * <pre>
		 *       return strcmp(token, word);
		 * </pre>
		 * @return The function return zero if the given word and the token are similar. A non null
		 *         value is returned if strings are different. Look the strcmp function for more
		 *         details.
		 */
		int CompToken(const char *word);

		/**
		 * Do an assertion on the token.
		 * The function looks if the expected token is the found one. If this is not the case, it
		 * display an error message in the console and return 0.
		 * @param expected_token The token which must be compared to the current token.
		 * @return The function returns the result of assertion.
		 * @todo Make possible to specify the error message.
		 */
		int Assert(const char *expected_token);

		/**
		 * Move the current position in the file, the string or the buffer.
		 * @param offset Offset to apply to current position.
		 */
		void GoTo(int offset);

		/**
		 * Get the offset of current position from the data beginning.
		 * @return Offset between current position and data beginning.
		 */
		int GetOffset(void);

	private:
		char filename[FILENAME_LEN];		/**< File name to parse. */
		const unsigned char *buffer;		/**< Current position in the buffer. */
		const unsigned char *buf_start;		/**< Start position. */
		const unsigned char *buf_end;		/**< End position. */
		int fsize;							/**< File size. */
		unsigned char *mem;					/**< File content. */

		int tokensize;						/**< Size of current token. */
};

#endif	/* __PARSER_H__ */