#ifndef YYMMS_LIBEVENT
#define YYMMS_LIBEVENT

#ifdef WIN32
	#include <event.h>

	#define EWOULDBLOCK             WSAEWOULDBLOCK
	#define evutil_socket_t			int
	#define EV_ET					0

	typedef void(* event_callback_fn)(evutil_socket_t, short, void *);
	struct event* event_new(struct event_base * base, evutil_socket_t fd, short flag, event_callback_fn fun, void * arg);
	void event_free	(struct event * evt);
#else
	#include <event2/event.h>
#endif

#include <iostream>
#include <vector>
#include "include/fastbuffer.h"
#include "event_msgqueue.h"

#ifndef YYMMS_HASH_MAP
#include "event_hash.h"
#endif

#ifndef YYMMS_LOCKQUEUEMAP
#include "lockqueuemap.h"
#endif

struct ILibEvent
{
	// return true: readbuffer will no used anymore
	// return false: keep readbuffer alive
	virtual bool data_receive(int fd, fastbuffer * readbuffer, unsigned int listen_port)	= 0;
	virtual void fd_close(int fd) = 0;
};

struct ILibQueueEvent
{
	virtual void queue_receive(char flag) = 0;
};

#define WRITE_EVENT_FLAG	1

class YYLibEvent;
struct fd_state
{
	YYLibEvent	* libevent;
	fastbuffer writebuffer;

	struct event *read_event;
	struct event *write_event;
	unsigned int listen_port;
};

struct listener_state
{
	YYLibEvent	* libevent;
	unsigned int	listen_port;
};

class YYLibEvent : public ILibQueueEvent
				, public hash_map_unlock<int, fd_state*>
				, public msg_queue
{
public:
	YYLibEvent();
	virtual ~YYLibEvent();

public:
	bool Init(ILibEvent	* callback);
	bool Bind(char * address, int port);
	bool Run();
	void Exit();
	void AddQueueEvent(char flag, ILibQueueEvent * ev);
	event_base * Base()
	{
		return base;
	}
	void data_send(int fd, fastbuffer * writebuffer, char flag = WRITE_EVENT_FLAG);
	void flag_send(char flag);
	void closefd(int fd);

	virtual void queue_receive(char flag);

protected:
	static fd_state * alloc_fd_state(YYLibEvent * libevent, evutil_socket_t fd, listener_state * liststate);
	static void free_fd_state(evutil_socket_t fd, fd_state *state);
	static void do_read(evutil_socket_t fd, short events, void *arg);
	static void do_write(evutil_socket_t fd, short events, void *arg);
	static void do_accept(evutil_socket_t listener, short event, void *arg);
	
	bool initqueue();
	static void do_queue_read(evutil_socket_t fd, short events, void *arg);

protected:
	struct event_base * base;

	std::vector<evutil_socket_t> listeners;

	ILibEvent *						_callback;
	std::vector<std::pair<char, ILibQueueEvent*> >		_queueback;

	LockQueueMap<char, fastbuffer*>						_bufqueue_map;
};

#endif // YYMMS_LIBEVENT
