#ifndef YYMMS_EVENT_MSGQUEUE
#define YYMMS_EVENT_MSGQUEUE

class msg_queue
{
public:
	msg_queue() : fd(0)
	{
	}

	void init(int fdin)
	{
		fd = fdin;
	}

	bool msg_send(int sockfd)
	{
		int sz = send(fd, (char*)&sockfd, sizeof(int), 0);
		if (sz == sizeof(int))
			return true;
		if (sz <= 0)
		{
			int err;
#ifdef WIN32
			err = WSAGetLastError();
#else
			err = errno;
#endif
			LOG::Error("msgqueue is block, reconnecting..... %d", err);
			fd = -1;
		}
		return true;
	}
private:
	int fd;
	struct sockaddr_in sin;
};



#endif // YYMMS_EVENT_MSGQUEUE