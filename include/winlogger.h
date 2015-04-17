#ifndef LOGGER_H
#define LOGGER_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef WIN32
#include <yysvr_res.h>
#endif

namespace server { namespace win { 

/*
 * priorities (these are ordered)
 */
#define	LOG_EMERG	0	/* system is unusable */
#define	LOG_ALERT	1	/* action must be taken immediately */
#define	LOG_CRIT	2	/* critical conditions */
#define	LOG_ERR		3	/* error conditions */
#define	LOG_WARNING	4	/* warning conditions */
#define	LOG_NOTICE	5	/* normal but significant condition */
#define	LOG_INFO	6	/* informational */
#define	LOG_DEBUG	7	/* debug-level messages */

class Logger
{
private:
	static HANDLE & _handle()
	{
		static HANDLE h = NULL;
		return h;
	}
public:
	static void openlog (char * ident,int option ,int facility)
	{
		// Get a handle to the event log. 
		_handle() = RegisterEventSourceA(NULL, // use local computer 
			ident); // event source name 
		if (_handle() == NULL) 
		{ 
			printf( "Could not register the event source. "); 
			return; 
		} 
	}

	static void closelog()
	{
		DeregisterEventSource(_handle());
	}

	static void Debug(const char *fmt, ...)
	{
		va_list va;
		va_start(va, fmt);
		Log(LOG_DEBUG, fmt, va);
		va_end(va);
	}

	static void Info(const char *fmt, ...)
	{
		va_list va;
		va_start(va, fmt);
		Log(LOG_INFO, fmt, va);
		va_end(va);
	}

	static void Warn(const char *fmt, ...)
	{
		va_list va;
		va_start(va, fmt);
		Log(LOG_WARNING, fmt, va);
		va_end(va);
	}

	static void Error(const char *fmt, ...)
	{
		va_list va;
		va_start(va, fmt);
		Log(LOG_ERR, fmt, va);
		va_end(va);
	}

private:
	static void Log(int level, const char *fmt, va_list va)
	{
		if (_handle() == NULL)
			return;

		static int types[][2] = {
			 {LOG_WARNING, EVENTLOG_WARNING_TYPE}
			,{LOG_ERR, EVENTLOG_ERROR_TYPE}
			,{LOG_DEBUG, EVENTLOG_AUDIT_SUCCESS}	// use EVENTLOG_AUDIT_SUCCESS replace LOG_DEBUG
			,{LOG_INFO, EVENTLOG_INFORMATION_TYPE}
		};

		char buf[1024];
		memset(buf, 0, sizeof(buf));
		int len = 0;

		len += _snprintf_s(buf + len, sizeof(buf) - len, sizeof(buf) - len, "Logger::Log[%ld]", 100);
		len += vsnprintf_s(buf + len, sizeof(buf) - len, sizeof(buf) - len, fmt, va);

		WORD type = EVENTLOG_SUCCESS;
		for (int i = 0; i < sizeof(types) / sizeof(int) / 2; i++)
		{
			if (types[i][0] == level)
			{
				type = types[i][1];
				break;
			}
		}

		LPCSTR ms[1] = {buf};
		if (!ReportEventA(_handle(), type, SYSTEM_CATEGORY, MMS_COMMAND, NULL, 1, 0, (LPCSTR*)ms, NULL))
		{
			printf("ReportEvent failed with %d for event %d.\n", GetLastError(), MMS_COMMAND);
		}
	}
};	//Logger

}}
#endif	//LOGGER_H
