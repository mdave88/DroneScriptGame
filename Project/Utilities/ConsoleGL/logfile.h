//-----------------------------------------------------------------------------
// LogFile
//-----------------------------------------------------------------------------

#ifndef __LOGFILE_H__
#define __LOGFILE_H__

#include <stdio.h>
#define OUTPUT_FILENAME					"logfile.html"

/**
 * LogFile class.
 * The logfile class is an interface to the log file. It is possible to save
 * log data to this log file through methods of the log class. The log
 * files can store messages from the system or the user. In fact, it store
 * every console message and some other messages.
 * The log file can be used for debug by reading the successive events that
 * occured during engine execution.
 * The log file content is replaced with new data, each time the engine is
 * executed, so the log file stores only the last execution of the engine.
 * @todo Manage file with seperate threads. The Log class should store
 *       messages in a buffer and write the buffer into the file when there
 *       is available CPU time.
 */
class LogFile
{
	private:
		FILE* logfile;							/**< logfile pointer */
		char outputfile[FILENAME_LENGTH];		/**< logfile name */

	public:
		LogFile(void);
		~LogFile(void);

		/**
		 * Adds a line into the log file
		 * @param s the new line to write
		 */
		void Insert(const char* s, ...);

		/**
		 * Opens the logfile for multiple insertions.
		 * This function can be used when multiple insertions in logfile has to be done.
		 * This should improve insertions speed.
		 * @return Logfile pointer if file could be opended, NULL pointer if not.
		 */
		FILE* OpenFile(void);

		/**
		 * Closes the logfile after multiple insertions.
		 * @return 0 if the stream is successfully closed.
		 */
		int CloseFile(void);
};

#endif	/* __LOGFILE_H__ */