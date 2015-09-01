//-----------------------------------------------------------------------------
// LogFile
//-----------------------------------------------------------------------------

#include "cl.h"
#include <stdarg.h>						// for va_start, va_end
#include <time.h>

#ifdef WIN32
	#include <direct.h>					// for _getcwd
	#include <tchar.h>					// for _vsntprintf
#else
	#include <strings.h>
	#include <unistd.h>
	#define _getcwd getcwd
	#define _vsntprintf vsnprintf
#endif

#define LOG_LINELENGTH	32768			// Maximm linelength for log file (is it sufficient ? ;)

LogFile::LogFile(void)
{
	// Creates output file name
	_getcwd(outputfile, PATH_LENGTH);
	strcat(outputfile, "/");
	strcat(outputfile, OUTPUT_FILENAME);
	
	// Creates the logfile
	if ((logfile = fopen(outputfile, "w")))
	{
		fprintf(logfile, "<html>\n<head>\n");
		fprintf(logfile, "\t<title>console logfile</title>\n");
		fprintf(logfile, "</head>\n<body text=\"#FFFFFF\" bgcolor=\"#000000\">\n");
		fprintf(logfile, "\t<tt>\n");
		fprintf(logfile, "\t<table width=\"100%%\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\">\n");
		fclose(logfile);
	}
	else printf("ERROR: couldn't create logfile !");

	logfile = NULL;
}

LogFile::~LogFile(void)
{
	if (logfile = fopen(outputfile, "at"))
	{
		fprintf(logfile, "\t</tr></table>\t</font></tt>\n</body>\n</html>\n");
		fclose(logfile);
	}
	logfile = NULL;
}

void LogFile::Insert(const char* s, ...)
{
	bool alreadyOpened = logfile?true:false;

	if (!alreadyOpened)
	{
		if (!(logfile = fopen(outputfile, "at"))) return;
		if (!logfile)
		{
			gConsole->Insert("^1Error while opening logfile.");
			return;
		}
	}

	va_list	msg;
	char buffer[LOG_LINELENGTH] = { '\0' };
	char output[LOG_LINELENGTH] = { '\0' };

	va_start(msg, s);
	_vsntprintf(buffer, LOG_LINELENGTH-1, s, msg);	
	va_end(msg);


	strcpy(output, "\t<tr valign=\"top\"><td width=\"10%%\"><font size=\"3\" color=\"#FFFFFF\">");

	time_t osBinaryTime;	// C run-time time (defined in <time.h>)
	time(&osBinaryTime);	// Get the current time from the operating system.
	char str_time[32] = { '\0' };
	strftime(str_time, 32, "%H:%M:%S", localtime(&osBinaryTime));
	strcat(output, str_time);
	strcat(output, "</font></td>");

	strcat(output, "<td width=\"90%%\"><font size=\"3\" color=\"#FFFFFF\">");
	
	int l = (int) strlen(buffer);
	for (int i = 0, j = (int) strlen(output); i < l; ++i)
	{
		if (buffer[i] == '^')
		{
			++i;
			if (i >= l) break;
			char str_color[8] = { '\0' };
			if (buffer[i] == '0' || buffer[i] == '7') sprintf(str_color, "#FFFFFF");
			else sprintf(str_color, "#%02x%02x%02x",
				255*text_colors[buffer[i]-'0'][0],
				255*text_colors[buffer[i]-'0'][1],
				255*text_colors[buffer[i]-'0'][2]);
			strcat(output, "</font><font color=\"");
			strcat(output, str_color);
			strcat(output, "\">");
			j = (int) strlen(output);
		}
		else if (buffer[i] == '\t')
		{
			for (int k = 0; k < TAB_LEN; ++k) strcat(output, "&nbsp;");
			j = (int) strlen(output);
		}
		else output[j++] = buffer[i];
	}
	strcat(output, "</font></td></tr>\n");

	// Add the line in the logfile
	fprintf(logfile, "%s", output);
	if (!alreadyOpened)
	{
		fclose(logfile);
		logfile = NULL;
	}
}

FILE* LogFile::OpenFile(void)
{
	logfile = fopen(outputfile, "at");
	return logfile;
}

int LogFile::CloseFile(void)
{
	int ret = fclose(logfile);
	logfile = NULL;
	return ret;
}