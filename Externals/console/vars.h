//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

#ifndef __VARS_H__
#define __VARS_H__

/**
 * Variable status.
 * Status defines variable type and domain in which variable can be
 * modified.
 */
enum
{
	VF_NONE			= 0x00,	/**< no flag */
	VF_CHEAT		= 0x01,	/**< value can only be changed if cheating */
	VF_SYSTEM		= 0x02,	/**< value can only be changed by the system */
	VF_PERSISTENT	= 0x04,	/**< value is stored in config.ini */
	VF_LATCH		= 0x08,	/**< change needs restart */
	VF_USER			= 0x10,	/**< created by the user */
	VF_TEMP			= 0x20,	/**< temporal variables are to be replaced with local varibles. */
	VF_DEBUG		= 0x40
};

/**
 * A variable.
 * A variable is an object that can store a value. The value can be an integer,
 * a float or a string. The variables have a direct impact on the engine
 * comportment. Some features are enabled or disabled following the
 * corresponding variable values.
 */
class Var
{
	public:
		/**
		 * Constructor for variable storing int value.
		 * @param name The variable's name.
		 * @param ivalue The new value (default is 0).
		 * @param flags The flags for the variable (default is VF_NONE).
		 */
		Var(const char *name, int ivalue = 0, int flags = 0);

		/**
		 * Constructor for variable storing float value.
		 * @param name The variable's name.
		 * @param fvalue The new value.
		 * @param flags The flags for the variable (default is VF_NONE).
		 */
		Var(const char *name, float fvalue, int flags = 0);

		/**
		 * Constructor for variable storing a string.
		 * @param name The variable's name.
		 * @param svalue The new value.
		 * @param flags The flags for the variable (default is VF_NONE).
		 */
		Var(const char *name, const char *svalue, int flags = 0);


		/**
		 * Default destructor.
		 * The destructor will automatically destroy the variable and his
		 * content.
		 */
		~Var(void);


		/**
		 * Set a string value to the variable.
		 * @param svalue The new value of the variable.
		 */
		void SetString(const char *svalue);

		/**
		 * Set an integer value to the variable.
		 * @param ivalue The new value of the variable.
		 */
		void SetInteger(int ivalue);

		/**
		 * Set a float value to the variable.
		 * @param fvalue The new value of the variable.
		 */
		void SetFloat(float fvalue);


		/**
		 * Set a new value to the variable under the form of a string
		 * @param svalue The new value of the variable.
		 * @see SetString()
		 */
		void SetDefault(const char *svalue);
		

		/**
		 * Equal operator for variable storing an integer value. We can use
		 * this operator to set a new integer value to the variable.
		 * @param ivalue The new value of the variable.
		 * @return The new value of the variable.
		 */
		int operator= (const int ivalue);

		/**
		 * Equal operator for variable storing a float value. We can use this
		 * operator to set a new float value to the variable.
		 * @param fvalue The new value of the variable.
		 * @return The new value of the variable.
		 */
		float operator= (const float fvalue);

		/**
		 * Equal operator for variable storing a string. We can use this
		 * operator to set a new value under the form of a string.
		 * @param svalue The new value of the variable.
		 */
		void operator= (const char *svalue);

		char *	name;			/**< Variable name. */
		float	fvalue;			/**< Variable value in the floating form */
		int		ivalue;			/**< Variable value in the integer form. */
		char *	svalue;			/**< Variable value in the string form. */
		int		flags;			/**< Variable flags. */

		/**
		 * Variable default value.
		 * A default value is assigned to the variable when the variable
		 * constructor is called. The constructor stores the given value
		 * in this variable. The default value is used by the console to
		 * display the default value into parenthesis.
		 */
		char *  default_value;

		/**
		 * Pointer to the next variable in a variables list.
		 * If next is here, the cost of recorrer the list is very hight,
		 * because vars are stored in very different places of the memory.
		 * In facts this pointer isn't used at all. Vars class is a much
		 * better method to have variables list.
		 */
		Var *	next;
};

/**
 * Variables list.
 * This class defines a list that contains variables.
 */
class Vars
{
	private:
		Var *list;				/**< The variables list. */

	public:
		Vars(void);				/**< Default constructor. */

		/**
		 * Default destructor.
		 * The destructor destroys all temporary (VF_TEMP flag) variables
		 * of the list.
		 */
		~Vars(void);


		/**
		 * Creates a variable storing a string and add it to the list.
		 * @param name The name of the variable.
		 * @param string The value of the variable under the form of a string.
		 * @param flags The flags of the variable (default is VF_NONE).
		 */
		void CreateVar(const char *name, const char *string, int flags=0);

		/**
		 * Creates a variable storing a float and add it to the list.
		 * @param name The name of the variable.
		 * @param fvalue The float value of the variable.
		 * @param flags The flags of the variable (default is VF_NONE).
		 */
		void CreateVar(const char *name, float fvalue, int flags=0);

		/**
		 * Creates a variable storing an integer and add it to the list.
		 * @param name The name of the variable.
		 * @param ivalue The int value of the variable.
		 * @param flags The flags of the variable (default is VF_NONE).
		 */
		void CreateVar(const char *name, int ivalue, int flags=0);


		/**
		 * Modify the value and the flags of a variable of the list.
		 * @param name The name of the variable.
		 * @param string The value of the variable under the form of a string.
		 * @return True if the operation succeeded, false if not.
		 */
		bool SetKeyValue(const char *name, const char *string);

		/**
		 * Modify the value and the flags of a variable of the list.
		 * @param name The name of the variable.
		 * @param value The float value of the variable.
		 * @return True if the operation succeeded, false if not.
		 */
		bool SetKeyValue(const char *name, const float value);

		/**
		 * Modify the value and the flags of a variable of the list.
		 * @param name The name of the variable.
		 * @param value The int value of the variable.
		 * @return True if the operation succeeded, false if not.
		 */
		bool SetKeyValue(const char *name, const int value);


		/** 
		 * Register a variable if it doesnt exists. If it exists and is
		 * temporal, replace it.
		 * @param var The variable to add to the variables list.
		 */
		void RegisterVar(Var &var);

		/** 
		 * Register a variable if it doesnt exists. If it exists and is
		 * temporal, replace it.
		 * @param var The variable to add to the variables list.
		 */
		void RegisterVar(Var *var);

		/** 
		 * Unregister a variable. If the variable is present in the variables
		 * list, it is removed of it.
		 * @param var The variable to remove from the variables list.
		 */
		void UnregisterVar(Var &var);

		/** 
		 * Unregister a variable. If the variable is present in the variables
		 * list, it is removed of it.
		 * @param var The variable to remove from the variables list.
		 */
		void UnregisterVar(Var *var);


		/**
		 * Get the value of a variable under the form of a string.
		 * @param name The name of the variable.
		 * @return The value of the variable.
		 */
		char *StringForKey(const char * name);

		/**
		 * Get the float value of a variable.
		 * @param name The name of the variable.
		 * @return The value of the variable.
		 */
		float ValueForKey(const char * name);

		/**
		 * Get the integer value of a variable.
		 * @param name The name of the variable.
		 * @return The value of the variable.
		 */
		int IntForKey(const char * name);


		/**
		 * Get the default value of a variable.
		 * @param name The name of the variable.
		 * @return The default value of the variable under the form of a string.
		 */
		char * DefaultForKey(const char *name);


		/**
		 * Informs if the variable is in the variables list.
		 * @param name The name of the variable.
		 * @return A boolean expressions that is true if the variable is present
		 *         in the variables list, false if not.
		 */
		bool isKey(const char *name);

		/**
		 * Get a pointer to a variable.
		 * @param name The name of the variable.
		 * @return The pointer to the variable or a NULL pointer if the variable
		 *         couldn't be found in the list.
		 */
		Var* GetVar(const char *name);


		/**
		 * Print a list of variables contained in the variables list.
		 */
		void printList(void);

		/**
		 * Get the variables list length.
		 * @return The number of variables stored in the list.
		 */
		int GetNumber(void);

		/**
		 * Get the name of the variable at position i in the list.
		 * @param i The index of the variable.
		 * @return The name of the variable at position i or an empty
		 *         expression if the input value is incorrect.
		 */
		char *GetName(int i);

		/**
		 * Load the variables from a file.
		 * @param filename The name of file to read.
		 * @return An integer that indicates if file could be opened (1) or not (0).
		 */
		int LoadFromFile(char *filename);

		/**
		 * Save the variables with VF_PERSISTENT flag to the disk.
		 * @param filename The name of destination file.
		 * @param mode The file opening mode (same as fopen function).
		 */
		void SaveToFile(char *filename, const char *mode = "w");
};

#endif	/* __VARS_H__ */