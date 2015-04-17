#include "stdafx.h"
#include "yymms_svr.h"
#include <signal.h>
#include "cfg.h"

extern MmsServer g_svr;

#ifdef WIN32
int _tmain(int argc, _TCHAR* argv[])
{
	WSADATA wsaData = {0};
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	/*
	 *	add underline to syslog.conf
	 *	local7.*   /var/log/yymmssvr.log
	 */
	LOG::openlog("yymmssvr", 0, 0);

	if (!ParseCfg())
		return 0;
 
	if (!g_svr.init(g_CFG.port, g_CFG.workthreads))
	{
		LOG::Error("init error");
	}
	else
	{
		g_svr.run();
	}
	
	LOG::Info("MmsServer exist!");

	LOG::closelog();
	WSACleanup();
	return 0;
}
#else
void system_error_cb(int err)
{
	LOG::Info("MmsServer failed, exit excation %d!", err);
}

void signal_handle(int sig)
{
	if (sig == SIGUSR1)
	{
		g_svr.exit();
	}
}

int main(int c, char **v)
{
	// error callback
	event_set_fatal_callback(system_error_cb);
	
	/*
	 *	add underline to syslog.conf
	 *	local7.*   /var/log/yymmssvr.log
	 */
	openlog("yymmssvr", LOG_PID, LOG_LOCAL7);

    setvbuf(stdout, NULL, _IONBF, 0);

	signal(SIGUSR1, signal_handle);

	if (!ParseCfg())
		return 0;

	if (!g_svr.init(g_CFG.port, g_CFG.workthreads))
	{
		LOG::Error("init error");
	}
	else
	{
		g_svr.run();
	}
	
	LOG::Info("MmsServer exist!");

	closelog();

	return 0;
}
#endif
