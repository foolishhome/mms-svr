#include "stdafx.h"
#include "yylibevent.h"

fd_state * YYLibEvent::alloc_fd_state(YYLibEvent * libevent, evutil_socket_t fd, listener_state * liststate)
{
	struct event_base * base = libevent->base;
    fd_state * state = (fd_state*)malloc(sizeof(fd_state));
    if (!state)
        return NULL;
	memset(state, 0, sizeof(fd_state));
	state->libevent = libevent;
	state->listen_port = liststate->listen_port;

    state->read_event = event_new(base, fd, EV_ET|EV_READ|EV_PERSIST, do_read, state);
    if (!state->read_event)
	{
        free(state);
        return NULL;
    }
    state->write_event = event_new(base, fd, EV_ET|EV_WRITE|EV_PERSIST, do_write, state);

    if (!state->write_event)
	{
        event_free(state->read_event);
        free(state);
        return NULL;
    }

    assert(state->write_event);
	libevent->add_state(fd, state);

    return state;
}

void YYLibEvent::free_fd_state(evutil_socket_t fd, fd_state *state)
{
	if (state->libevent->_callback)
		state->libevent->_callback->fd_close(fd);
	state->libevent->del_state(fd);
    event_free(state->read_event);
    event_free(state->write_event);
	closesocket(fd);
	state->libevent = NULL;
	delete state;
}

void YYLibEvent::do_read(evutil_socket_t fd, short events, void *arg)
{
    fd_state *state = (fd_state*)arg;
	if (!state || !state->libevent)
		return;

    static char buf[1024];
    int result;
	int err;
	fastbuffer * readbuffer = NULL;

    while (true)
	{
        assert(state->write_event);
        result = recv(fd, buf, sizeof(buf), 0);
		if (result < 0)
		{
#ifdef WIN32
			err = WSAGetLastError();
#else
			err = errno;
#endif
			if (err == EAGAIN || err == EWOULDBLOCK || err == EINTR)
			{
				if (readbuffer && state->libevent->_callback)
					if (!state->libevent->_callback->data_receive(fd, readbuffer, state->listen_port))
						readbuffer = NULL;
			}
			else
			{	// error occuse
				state->libevent->free_fd_state(fd, state);
			}
			break;
		}
		else if (result == 0)
		{	// fd shutdown
			state->libevent->free_fd_state(fd, state);
			break;
		}
		else
		{
			if (!readbuffer)
			{
				readbuffer = new fastbuffer();
				if (!readbuffer)
					break;
			}
			readbuffer->append(buf, result);
		}
    }
	if (readbuffer)
		delete readbuffer;
}

void YYLibEvent::do_write(evutil_socket_t fd, short events, void *arg)
{
	int err;
	fd_state * state = (fd_state *)arg;
	if (!state || !state->libevent)
		return;

	int sz = state->writebuffer.size();
	if (sz > 0)
	{
		int result = send(fd, state->writebuffer.buf(), sz, 0);
		if (result < 0)
		{
#ifdef WIN32
			err = WSAGetLastError();
#else
			err = errno;
#endif
			if (err == EAGAIN || err == EWOULDBLOCK || err == EINTR)
			{
			}
			else
			{	// error occuse
				state->libevent->free_fd_state(fd, state);
			}
		}
		else if (result == 0)
		{	// fd shutdown
			state->libevent->free_fd_state(fd, state);
		}
		else
		{
			if (result == sz)
			{
				state->writebuffer.clear();
				event_del(state->write_event);
			}
			else
			{
				state->writebuffer.read(NULL, sz);
			}
		}
	}
	else
	{
		event_del(state->write_event);
	}
}

void YYLibEvent::do_accept(evutil_socket_t listener, short event, void *arg)
{
	listener_state * liststate = (listener_state*)arg;
	if (!liststate)
		return;
	YYLibEvent * libevent = liststate->libevent;
	if (!libevent) return;

    struct sockaddr_storage ss;
    socklen_t slen = sizeof(ss);
    int fd = accept(listener, (struct sockaddr*)&ss, &slen);
	if (fd < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
		{	// there is no connect

		}
		else if (errno == EMFILE)
		{
			closesocket(fd);
			LOG::Error("fd is not enougth!!");
		}
	}
	else
	{
        fd_state *state;
        evutil_make_socket_nonblocking(fd);
        state = alloc_fd_state(libevent, fd, liststate);
		assert(state);
        assert(state->write_event);
        event_add(state->read_event, NULL);
    }
}

YYLibEvent::YYLibEvent()
		: base(NULL)
		,hash_map_unlock<int, fd_state*>(NULL)
		,_callback(NULL)
		,_queueback(NULL)
		,_bufqueue_map(0)
{

}

YYLibEvent::~YYLibEvent()
{
	if (base)
		event_base_free(base);
	base = NULL;
}

bool YYLibEvent::Init(ILibEvent	* callback)
{
	_callback = callback;

	base = event_base_new();
	if (!base)
	{
		LOG::Error("YYLibEvent init event base failed");
		return false;
	}

	AddQueueEvent(WRITE_EVENT_FLAG, this);

	if (!initqueue())
		return false;

	return true;
}

bool YYLibEvent::Bind(char * address, int port)
{
	struct sockaddr_in sin;

	sin.sin_family = AF_INET;
	if (address == NULL)
		sin.sin_addr.s_addr = INADDR_ANY;
	else
		sin.sin_addr.s_addr = inet_addr(address);
	sin.sin_port = htons(port);

	evutil_socket_t listener = socket(AF_INET, SOCK_STREAM, 0);
	evutil_make_socket_nonblocking(listener);

#ifndef WIN32
	{
		int one = 1;
		setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
	}
#endif

	if (bind(listener, (struct sockaddr*)&sin, sizeof(sin)) < 0)
	{
		LOG::Error("YYLibEvent bind failed, port: %d", port);
		return false;
	}

	if (listen(listener, 16) < 0)
	{
		LOG::Error("YYLibEvent listen failed");
		return false;
	}

	listener_state * state = (listener_state*)malloc(sizeof(listener_state));
	if (!state)
		return false;
	memset(state, 0, sizeof(listener_state));
	state->libevent = this;
	state->listen_port = port;
	struct event * listener_event = event_new(base, listener, EV_READ|EV_PERSIST, do_accept, (void*)state);
	/*XXX check it */
	event_add(listener_event, NULL);

	LOG::Info("YYLibEvent listen success, address: %s, port: %d", address, port);
	return true;
}

bool YYLibEvent::Run()
{
	event_base_dispatch(base);
	return true;
}

void YYLibEvent::Exit()
{
	event_base_loopexit(base, NULL);
}

void YYLibEvent::closefd(int fd)
{
	fastbuffer * writebuffer = new fastbuffer;
	writebuffer->fd = fd;

	data_send(fd, writebuffer);
}

void YYLibEvent::data_send(int fd, fastbuffer * writebuffer, char flag/*=WRITE_EVENT_FLAG*/)
{
	if (!writebuffer)
		return;
	writebuffer->fd = fd;

	_bufqueue_map.push(flag, writebuffer);

	flag_send(flag);
}

void YYLibEvent::flag_send(char flag)
{
	msg_send(flag);
}

bool YYLibEvent::initqueue()
{
	evutil_socket_t sv[2]; // sv[0] push socket, sv[1] pop socket

	if (evutil_socketpair(AF_UNIX, SOCK_STREAM, 0, sv) != 0)
		return false;

	evutil_make_socket_nonblocking(sv[1]);

	struct event * event_read = event_new(base, sv[1], EV_READ|EV_PERSIST, do_queue_read, this);
	event_add(event_read, NULL);

	init(sv[0]);

	LOG::Info("queue create success, port0:%d, port1:%d", sv[0], sv[1]);
	return true;
}

void YYLibEvent::do_queue_read(evutil_socket_t fd, short events, void *arg)
{
	YYLibEvent * libevent = (YYLibEvent*)arg;
	if (!libevent)
		return;

	static char flag;
	int result;

	ILibQueueEvent * evt;
	do
	{
		result = recv(fd, (char*)&flag, sizeof(char), 0);
		if (result <= 0)
		{
			break;
		}
		else if (flag > 0)
		{
			for (std::vector<std::pair<char, ILibQueueEvent*> >::iterator iter = libevent->_queueback.begin(); iter != libevent->_queueback.end(); iter++)
			{
				if (iter->first == flag && iter->second)
				{
					evt = iter->second;
					if (evt)
						evt->queue_receive(flag);
					break;
				}
			}
		}
	} while(false);
}

void YYLibEvent::AddQueueEvent(char flag, ILibQueueEvent * ev)
{
	_queueback.push_back(std::make_pair(flag, ev));
}

void YYLibEvent::queue_receive(char flag)
{
	if (flag != WRITE_EVENT_FLAG)
		return;

	fastbuffer * writebuffer = _bufqueue_map.pop(flag);
	if (!writebuffer)
		return;

	if (writebuffer->size() == 0)
	{	// Ö÷¶¯¹Ø±Õ
		fd_state * state = get_state(writebuffer->fd);
		if (state && state->libevent)
		{
			state->libevent->free_fd_state(writebuffer->fd, state);
		}
		delete writebuffer;
	}
	else
	{
		do
		{
			fd_state * state = get_state(writebuffer->fd);
			if (state)
			{
				state->writebuffer.append(writebuffer->buf(), writebuffer->size());
				event_add(state->write_event, NULL);
			}
			delete writebuffer;
		} while (0);
	}
}

#ifdef WIN32
struct event* event_new(struct event_base * base, evutil_socket_t fd, short flag, event_callback_fn fun, void * arg)
{
	struct event * evt = (struct event *)malloc(sizeof(event));
	event_set(evt, fd, flag, fun, arg);
	event_base_set(base, evt);
	return evt;
}

void event_free	(struct event * evt)
{
	event_del(evt);
	free(evt);
}
#endif
