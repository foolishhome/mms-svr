#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <stdarg.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>

namespace server { namespace xlinux { 

class Logger
{
public:
	static void openlog (char * ident,int option ,int facility)
	{
		::openlog(ident, option, facility);
	}

	static void closelog()
	{
		::closelog();
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
		#define current_thread_id() syscall(SYS_gettid)

		char buf[1024];
		int len = 0;

		len += snprintf(buf + len, sizeof(buf) - len, "Logger::Log[%ld] ", current_thread_id());
		len += vsnprintf(buf + len, sizeof(buf) - len, fmt, va);

		syslog(level, buf, NULL);
	}
};	//Logger

}}
#endif	//LOGGER_H
