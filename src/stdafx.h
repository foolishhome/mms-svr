#include <assert.h>
#include <string.h>

#ifdef WIN32
	#include <errno.h>
	#include <WinSock2.h>
	#include "ws2tcpip.h"

	#include "include/winlogger.h"
	#define LOG	server::win::Logger

	#define WIN32_LEAN_AND_MEAN             // 从 Windows 头中排除极少使用的资料
	// Windows 头文件:
	#include <windows.h>

	// C 运行时头文件
	#include <stdlib.h>
	#include <malloc.h>
	#include <memory.h>
	#include <tchar.h>

#else
	#include <unistd.h>
	#include <stdlib.h>
	#include <stdio.h>
	#include <errno.h>

	// libevent
	#include <netinet/in.h>
	#include <sys/socket.h>
	#include <arpa/inet.h>
	#include <fcntl.h>

	#include "include/logger.h"
	#define LOG	server::xlinux::Logger
	#define closesocket	close
	#define MAX_PATH	260
	#define WORD		short
#endif


