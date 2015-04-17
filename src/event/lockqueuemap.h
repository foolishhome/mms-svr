#ifndef YYMMS_LOCKQUEUEMAP
#define YYMMS_LOCKQUEUEMAP

#include <pthread.h>
#include <queue>
#include <map>

template<class T, class T1>
class LockQueueMap
{
	typedef typename std::map<T, std::queue<T1> >				QUEUE_MAP;
	typedef typename std::map<T, std::queue<T1> >::iterator		QUEUE_MAP_ITER;
	typedef typename std::map<T, std::queue<T1> >::value_type	QUEUE_MAP_TYPE;

public:
	pthread_mutex_t	_mutex;
	std::map<T, std::queue<T1> >	_fd_queue_map;

public:
	LockQueueMap(T1 TdefValue)
	{
		_TdefValue = TdefValue;
		pthread_mutex_init(&_mutex, NULL);
	}
	~LockQueueMap()
	{
		pthread_mutex_destroy(&_mutex);
	}
	T1 pop(T s)
	{
		T1 t = _TdefValue;
		pthread_mutex_lock(&_mutex);
		{
			QUEUE_MAP_ITER iter = _fd_queue_map.find(s);
			if (iter != _fd_queue_map.end())
			{
				if (!iter->second.empty())
				{
					t = iter->second.front();
					iter->second.pop();
				}
			}
		}
		pthread_mutex_unlock(&_mutex);
		return t;
	}

	void push(T s, T1 & t)
	{
		pthread_mutex_lock(&_mutex);
		{
			QUEUE_MAP_ITER iter = _fd_queue_map.find(s);
			if (iter == _fd_queue_map.end())
			{
				std::queue<T1> qu;
				_fd_queue_map.insert(QUEUE_MAP_TYPE(s, qu));
				iter = _fd_queue_map.find(s);
			}
			if (iter != _fd_queue_map.end())
			{
				iter->second.push(t);
			}
		}
		pthread_mutex_unlock(&_mutex);
	}
private:
	T1	_TdefValue;
};

#endif // YYMMS_LOCKQUEUEMAP
