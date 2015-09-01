//-----------------------------------------------------------------------------
// Commands
//-----------------------------------------------------------------------------

#ifndef __COMMANDS_H__
#define __COMMANDS_H__

#define COMMAND_LINELENGTH				1025			/**< Maximum length for a command */
#define MAX_SCRIPT_LINES				1025			/**< Maximum number of lines for a script. */

typedef void (*xcommand_t) (int argc, char *argv[]);	/**< Command type */

/**
 * Command structure.
 */
typedef struct cmd_function_s
{
	struct cmd_function_s	*next;			/**< Pointer to next command */
	char					*name;			/**< Command name */
	xcommand_t				function;		/**< Pointer to corresponding function */
	char *					description;	/**< Function description */
} cmd_function_t;

typedef char commline[COMMAND_LINELENGTH];

typedef struct
{
	commline *outputData;					/**< Output data tables. */
	int outputDataSize;						/**< Size of output data table. */
	int outputDataAllocatedSize;			/**< Size of output data table allocation (used for blocks allocations). */
} DataTable;

/**
 * String comparator.
 * This function is used by qsort function.
 * @param arg1 A pointer to the key for the search.
 * @param arg2 A pointer to the array element to be compared with the key.
 * @return A value specifying the relationship between parameters. The function return a negative
 * value if first parameter is less than second ; it return a null value if both parameters are
 * equivalent ; and it returns a positive value if first parameter is greater than second.
 */
int StrCompare(const void *arg1, const void *arg2);

/**
 * Commands class
 * The commands are used in the console to do some modifications on the
 * configuration of the engine. A command is typically a structure containing
 * a name, an attached function and an eventual description.
 * @see cmd_function_t
 * @todo Improve scripts manager, and make it possible to have scripts longer than MAX_SCRIPT_LINES.
 */
class Commands
{
	private:
		int nbrAllocatedHistLines;				/**< Number of allocated lines for history */
		int nbrCommHistLines;					/**< Number of history lines */
		commline *commandsHistory;				/**< Commands history
												 *   The allocation is made block by block, so the console has more allocated lines than filled lines. */

		cmd_function_t *cmd_functions;			/**< Pointer on the first function of the list. */
	
		bool isPiped;							/**< The currently proceced command must take input from outputData table. */
		bool isWaitingForOutput;				/**< Command class is waiting for output. */
		DataTable *dataTables;					/**< Output data tables. */
		int nbrTables;							/**< Number of tables. */
		int currTable;							/**< Table currently being completed. */

		bool isMakingRecursion;					/**< The command execution function makes recursions. */

	public:
		bool AddToConsole;						/**< Defines if commands must be added to console. */
		bool AddToHistory;						/**< Defines if commands must be added to history. */
		void DoNotAddToScript(void);

		Commands(void);
		~Commands(void);

		/**
		 * Add new command.
		 * Add a new command to commands list. The function stops if new
		 * command already exists.
		 * @param cmd_name the new command name
		 * @param function the command associated function
		 * @param description_message a description message for the command
		 */
		void AddCommand(char *cmd_name, xcommand_t function, const char* description_message = NULL);

		/**
		 * Set a new description for an existing command.
		 * The command description can be set later than command registry
		 * (AddCommand), and doesn't require to create manually a new message
		 * in the global messages list. You can set a new description or
		 * modify an existing description, or also add a description to the
		 * function in another language.
		 * @param cmd_name the command name
		 * @param description a null ended string containing command description
		 */
		void SetDescription(char *cmd_name, const char *description);

		/**
		 * Remove an existing command of the list.
		 * Commands can be removed from commands list. Remove it by entering
		 * the command name. If the command cannot be found, the function has
		 * no effect.
		 * @param cmd_name the command name
		 */
		void RemoveCommand(char *cmd_name);

		/**
		 * Check for existing command.
		 * You can check if a command is already defined in commands list.
		 * @param cmd_name the command name
		 * @return a boolean value that has 1 if the command was found, else 0
		 */
		bool Exists(char *cmd_name);

		/**
		 * Console command autocomplete.
		 * The function helps you to find the name of an existing command. The
		 * function works with global console (gConsole variable).
		 * @param partial the partial command to complete
		 * @param display_solutions displays the list of solutions if more than
		 *        one solution
		 * @param auto_complete_command complete current console command with
		 *        common begin of every potential solution
		 */
		void CompleteCommand(const char *partial, bool display_solutions = 1, bool auto_complete_command = 1);

		/**
		 * Give the first solution for auto-complete.
		 * Solutions are stored in a list that can be accessed with current
		 * function. The function puts the first solution of the list in the
		 * current console command. The function initialize an internal
		 * solution counter and is required to access other solutions.
		 * @param partial the partial command
		 */
		void FirstSolution(const char *partial = NULL);

		/**
		 * Give the last solution for auto-complete.
		 * Solutions are stored in a list that can be accessed with current
		 * function. The function puts the last solution of the list in the
		 * current console command. The function initialize an internal
		 * solution counter and is required to access other solutions.
		 * @param partial the partial command
		 */
		void LastSolution(const char *partial = NULL);

		/**
		 * Give the next solution for auto-complete.
		 * Solutions are stored in a list that can be accessed with current
		 * function. The function puts the next solution of the list in the
		 * current console command. The function FirstSolution should have
		 * been used before to initialize internal solution counter.
		 */
		void NextSolution(void);

		/**
		 * Give the previous solution for auto-complete.
		 * Solutions are stored in a list that can be accessed with current
		 * function. The function puts the previous solution of the list in the
		 * current console command. The function FirstSolution should have
		 * been used before to initialize internal solution counter.
		 */
		void PrevSolution(void);

		/**
		 * Execute a command.
		 * The function execute function that is associated with command. It
		 * also check for variables and aliases, and execute them if the
		 * commandline matches with.
		 * The function supports multiple commands in a single command line. The
		 * commands are separated with a separator char (by default, the ';'
		 * character, but see COMMAND_SEPARATOR constant to be sure).
		 * The function also supports commands pipes. If a (or several) pipe is
		 * used, the command output is redirected to next command. For example
		 * the "cmdlist" output can be redirected to a grep function. The command
		 * "cmdlist | grep console" outputs only commands which have the "console"
		 * substring in their name. The default pipe character is '|', but see
		 * COMMAND_PIPE constant to be sure).
		 * @param commandline the command to execute with arguments
		 * @return a non zero integer if function exists, 0 if not
		 */
		int ExecuteCommand(const char *commandline, ...);

		/**
		 * Displays the commands list.
		 * The commands list is displayed in the console. You can specify if
		 * descpription must be written.
		 * @param display_descriptions a value that indicates if description
		 *        must be given.
		 */
		void DisplayCommands(int display_descriptions = 0);

		/**
		 * Repeat last executed command.
		 * Last command is stored and you can execute it again.
		 */
		void RepeatLastCommand(void);

		/**
		 * Get the function associated to a command.
		 * @param cmd_name the command name
		 * @return a pointer to the associated command
		 */
		cmd_function_t* GetFunction(char *cmd_name);

		/**
		 * Inform if command class is currently waiting for output.
		 * This is used by console to send inputs to commands. The command class can
		 * then use this information for pipes.
		 * @return A boolean value corresponding to waiting state.
		 */
		bool IsWaitingForOutput(void);

		/**
		 * Get an output data.
		 * The data is added to internal output table.
		 * @param data The new output data.
		 */
		void GetOutput(const char* data);

		/**
		 * Grep function.
		 * The grep function is case sensitive.
		 * @see ExecuteCommand
		 * @param token The string to search.
		 */
		void CallGrep(const char *token);

		/**
		 * Sort function.
		 * The function sorts the current output table data, and redirect it
		 * to output.
		 * @param order The sorting order : the function makes an ascending sorting if order
		 *        has a non-negative value, and makes a descending sorting if order is negative.
		 */
		void CallSort(int order = 1);

		/**
		 * Get a line from the history.
		 * @param l The line number to get.
		 * @return The corresponding line of the history.
		 */
		char* GetHistoryLine(int l);

		/**
		 * Add a line to the history.
		 * @param s The char string to add in the history.
		 */
		void AddToCommHistory(char* s);

		/**
		 * Number of lines in the history.
		 * @return The history length.
		 */
		int GetNbrCommHistLines(void);
};

#endif	/* __COMMANDS_H__ */