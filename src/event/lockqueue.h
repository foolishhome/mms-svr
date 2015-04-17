#ifndef YYMMS_LOCKQUEUE
#define YYMMS_LOCKQUEUE

#include <pthread.h>
#include <queue>

template<class T, class T1 = int>
class LockQueue
{
public:
	pthread_mutex_t	_mutex;
	std::queue<T>	_fd_queue;

public:
	LockQueue(T TdefValue,  T1 dt = 0)
	{
		_dt = dt;
		_TdefValue = TdefValue;
		pthread_mutex_init(&_mutex, NULL);
	}
	~LockQueue()
	{
		pthread_mutex_destroy(&_mutex);
	}
	T pop()
	{
		T t = _TdefValue;
		pthread_mutex_lock(&_mutex);
		{
			if (!_fd_queue.empty())
			{
				t = _fd_queue.front();
				_fd_queue.pop();
			}
		}
		pthread_mutex_unlock(&_mutex);
		return t;
	}

	void push(T & t)
	{
		pthread_mutex_lock(&_mutex);
		{
			_fd_queue.push(t);
		}
		pthread_mutex_unlock(&_mutex);
	}
private:
	T	_TdefValue;
public:
	T1	_dt;
};

#endif // YYMMS_LOCKQUEUE