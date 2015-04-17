#ifndef YYMMS_LIBREDIS
#define YYMMS_LIBREDIS

#ifndef HIREDIS_ADAPTERS_LIBEVENT
#include "libeventdef.h"
#endif

#ifndef YYMMS_LIBEVENT
#include "event/yylibevent.h"
#endif
#ifndef YYMMS_LOCKQUEUE
#include "lockqueue.h"
#endif

#include <string>
#include <queue>

#define REDIS_EVENT_FLAG	2

template<class Class, typename Parameter>
class Redis_CallBack 
{
    typedef void (Class::*Method)(redisReply*, Parameter);
public:   
    Redis_CallBack(Class* class_instance, Method method)
	{
		_class_instance = class_instance;
		_method         = method;
	}

    void operator()(redisReply * reply, Parameter parameter)  
    {
        return (_class_instance->*_method)(reply, parameter);  
    }

private:
    Class*  _class_instance;  
    Method  _method;  
};  
  
struct QueueItem
{
	QueueItem() : privdata(NULL)
	{
	}
	std::string format;
	void * privdata;
};

template<class T, class Class, typename Parameter>
class YYRedis : public ILibQueueEvent
				, public Redis_CallBack<Class, Parameter>
				, LockQueue<QueueItem>
{
    typedef void (Class::*Method)(redisReply*, Parameter);
public:
	YYRedis(Class* _class_instance, Method _method) : Redis_CallBack<Class, Parameter>(_class_instance, _method)
													,LockQueue<QueueItem>(QueueItem())
    {
		_delayDisconnect = 0;
		_delayConnectTime = 0;
		_connecting = false;
		_c = NULL;
		_base = NULL;
		_port = 0;
    }
    virtual ~YYRedis()
    {
		_base = NULL;
		_connecting = false;
		_c = NULL;
    }
	static void getCallback(redisAsyncContext *c, void *r, void *privdata)
	{
		if (!c || !c->data) return;

		redisReply *reply = (redisReply*)r;
		if (reply == NULL)
			return;

		YYRedis<T, Class, Parameter> * redis = (YYRedis<T, Class, Parameter>*)c->data;
		if (!redis) return;

		(*redis)(reply, (Parameter)privdata);
	}

#ifdef WIN32
	static void connectCallback(const redisAsyncContext *c)
	{
		if (!c) return;
#else
	static void connectCallback(const redisAsyncContext *c, int status)
	{
		if (!c || status != REDIS_OK)
		{
			if (c)
			{
				YYRedis<T, Class, Parameter> * redis = (YYRedis<T, Class, Parameter>*)c->data;
				if (redis)
					redis->_connecting = false;
				LOG::Error("redisAsyncConnect Error: %s", c->errstr);
			}	
			return;
		}

#endif

		YYRedis<T, Class, Parameter> * redis = (YYRedis<T, Class, Parameter>*)c->data;
		if (!redis) return;

		redis->_c = (redisAsyncContext*)c;
		redis->_connecting = false;
		LOG::Info("connectCallback connected...");
	}

	static void disconnectCallback(const redisAsyncContext *c, int status)
	{
		if (!c) return;

		if (status != REDIS_OK)
		{
			LOG::Info("disconnectCallback Error: %s", c->errstr);
		}
		YYRedis<T, Class, Parameter> * redis = (YYRedis<T, Class, Parameter>*)c->data;
		if (!redis) return;

		redis->_c = NULL;
		redis->_delayDisconnect = COMMON::GetTickCount();
		LOG::Info("disconnectCallback disconnected succeed...");
	}
	void Init(T t, const char *ip, unsigned int port, struct event_base *base, int delayConnectTime = 10000)
	{
		_ip = ip;
		_port = port;
		_base = base;
		_delayConnectTime = delayConnectTime;
		_t = t;
		_t->AddQueueEvent(REDIS_EVENT_FLAG, this);
	}
	bool Connect()
	{
		if (_connecting || (_c && _c->c.flags & (REDIS_DISCONNECTING | REDIS_FREEING)))
			return true;

		if (_delayDisconnect != 0 && COMMON::GetTickCount() - _delayDisconnect < _delayConnectTime)
			return false;
		_delayDisconnect = COMMON::GetTickCount();

		redisAsyncContext * c = redisAsyncConnect(_ip.c_str(), _port);
		if (!c || c->err)
		{
			if (c)
				LOG::Error("redisAsyncConnect Error: %s at %u", c->errstr, _delayDisconnect);
			return false;
		}
	
		_connecting = true;
		c->data = (void*)this;

		redisLibeventAttach(c,_base);
		redisAsyncSetConnectCallback(c,connectCallback);
		redisAsyncSetDisconnectCallback(c,disconnectCallback);

		return true;
	}

	bool Close()
	{
		_base = NULL;
		if (_c)
			redisAsyncDisconnect(_c);
	}

	int AsyncSendCommand(Parameter privdata, const char *format, ...)
	{
		if (!_c)
		{
			if (_base)
				Connect(_base);
			return REDIS_ERR;
		}

		QueueItem token;
		token.privdata = (void*)privdata;

		int size = 256;
		std::string str;

		do
		{
			str.resize(size);
			va_list ap;
			va_start(ap,format);
			int n = vsnprintf((char *)str.c_str(), size, format, ap);
			va_end(ap);
			if (n > -1 && n < size) {
				str.resize(n);
				break;
			}
			if (n > -1)
				size=n + 1;
			else
				size *= 2;
		} while(1);

		if (str.empty())
			return REDIS_ERR_EOF;

		token.format = str;

		push(token);
		_t->flag_send(REDIS_EVENT_FLAG);
		return REDIS_OK;
	}

	virtual void queue_receive(char flag)
	{
		if (flag != REDIS_EVENT_FLAG)
			return;
		int status = REDIS_ERR;
		if (_c && _base)
		{
			QueueItem token = pop();
			status = redisvAsyncCommand(_c, getCallback, token.privdata, token.format.c_str(), NULL);
		}
		if (status != REDIS_OK)
		{
			if (_base)
			{
				Connect();
			}
		}		
	}
public:
	redisAsyncContext * _c;
	struct event_base * _base;
	T					_t;

	bool				_connecting;
	unsigned int		_delayConnectTime;
	unsigned int		_delayDisconnect;

	std::string			_ip;
	unsigned int		_port;
};

#endif // YYMMS_LIBREDIS

