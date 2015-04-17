// mmsclient.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <winsock2.h>
#include <assert.h>

int _tmain(int argc, _TCHAR* argv[])
{
	WSADATA wsaData = {0};
	WSAStartup(MAKEWORD(2, 2), &wsaData);

#define DATA_SIZE 10001

	char * address = "172.19.40.32";
	int port = 10000;


	char * senddata = (char*)malloc(DATA_SIZE);
	memset(senddata, 0, DATA_SIZE);
	for (int i = 0; i < DATA_SIZE - 1; i++)
	{
		senddata[i] = '9';
	}

	char resvdata[DATA_SIZE];

	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = inet_addr(address);
	sin.sin_port = htons(port);


	int loops = 100000;
	while (loops != 0)
	{
		int sd = socket(AF_INET, SOCK_STREAM, 0);
		if (connect(sd, (const sockaddr*)&sin, sizeof(sin)) != -1)
		{
			OutputDebugStringA("connect succeed\n");
			int sz = send(sd, senddata, (int)strlen(senddata), 0);
			if (sz != strlen(senddata))
			{
				assert(false);
			}

			memset(resvdata, 0, DATA_SIZE);
			int rz = recv(sd, resvdata, DATA_SIZE, 0);
			if (rz != sz)
			{
				// assert(false);
			}
		}
		closesocket(sd);

		loops--;
	}

	free(senddata);

	WSACleanup();
	return 0;
}

