#include <assert.h>
#include <string.h>

#ifdef WIN32
	#include <errno.h>
	#include <WinSock2.h>
	#include "ws2tcpip.h"

	#include "include/winlogger.h"
	#define LOG	server::win::Logger

	#define WIN32_LEAN_AND_MEAN             // �� Windows ͷ���ų�����ʹ�õ�����
	// Windows ͷ�ļ�:
	#include <windows.h>

	// C ����ʱͷ�ļ�
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


