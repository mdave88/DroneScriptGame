//-----------------------------------------------------------------------------
// Alias
//-----------------------------------------------------------------------------

#ifndef __ALIAS_H__
#define __ALIAS_H__

#define MAX_ALIAS_NAME		32				/**< Maximum length of alias namess */

/**
 * Maximum amount of recursive calls.<br>
 * Used to avoid infinite recursions.
 */
#define MAX_ALIAS_DEPTH		16

/**
 * Alias structure.
 * This structure describes an alias. An alias is a couple name-value. A
 * pointer is also added and used in the list structure of the Alias class.
 * @see Alias
 */
typedef struct alias_s
{
	struct alias_s *next;					/**< Pointer on the next alias in aliases list. */
	char		name[MAX_ALIAS_NAME];		/**< Alias name. */
	char		*value;						/**< Alias value. */
} alias_t;

/**
 * Alias class.
 * Alias can be used to replace existing words, commands or variables
 * with others.
 */
class Alias
{
	alias_t		*alias;			/**< Pointer on the first alias of the list. */

	public:
		Alias(void);			/**< Default constructor. */
		~Alias(void);

		/**
		 * Recursion depth.
		 * Used to avoid infinit recursions.
		 */
		int depth;

		/**
		 * Finds an alias of the list.
		 * @param a_name The name of alias to get.
		 * @return A pointer to the alias or NULL pointer if alias cannot be
		 *         found in the list.
		 */
		alias_t* FindAlias(char *a_name);

		/**
		 * Modifiy the value of an existing alias or add a new alias with
		 * given value.
		 * @param a_name The name of alias.
		 * @param value The new value of alias.
		 */
		void SetAlias(char *a_name, char *value, ...);

		/**
		 * Removes an alias from the list.
		 * The function try to find the alias in the list and removes it if
		 * it can find it. The alias is destroyed and memory is freed.
		 * @param a_name The name of alias to remove.
		 */
		void RemoveAlias(char *a_name);

		/**
		 * Gets the value of an alias.
		 * The function search for alias value in aliases table. If corresponding
		 * alias is found, it return the alias value. If not alias is found, the
		 * function return an empty string.
		 * @param a_name The name of alias to get.
		 * @return The corresponding value for requested alias or an empty string
		 *         if corresponding value coulnd't be found.
		 */
		char *GetAlias(char *a_name);

		/**
		 * Autocompletes alias name.
		 * @param partial The beginning of alias name.
		 * @return The full name for first matching alias.
		 */
		char *CompleteAlias(char *partial);

		/**
		 * Gets the aliases list.
		 * @return A pointer to the first alias of the list.
		 */
		alias_t* GetAliasList(void);

		/**
		 * Prints a list of defined aliases to the console.
		 */
		void printList(void);

		/**
		 * Returns the number of aliases.
		 * @return The number of aliases.
		 */
		int GetNumber(void);

		/**
		 * Get the name of alias at position \a i in the aliases list
		 * @param i The position of alias in the list.
		 * @return The name of alias \a i, a empty string if \a i is not correct.
		 */
		char *GetName(int i);
};

#endif	/* __ALIAS_H__ */