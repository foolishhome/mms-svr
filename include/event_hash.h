#ifndef YYMMS_HASH_MAP
#define YYMMS_HASH_MAP

#include <hash_map>
#include <pthread.h>

#ifdef WIN32
	#define HASH_MAP_SPACE	stdext
#else
	#define HASH_MAP_SPACE	__gnu_cxx
#endif

template<class T1, class T2>
class hash_map_lock
{
	typedef typename HASH_MAP_SPACE::hash_map<T1, T2>				HASH_MAP;
	typedef typename HASH_MAP_SPACE::hash_map<T1, T2>::iterator		HASH_MAP_ITER;
	typedef typename HASH_MAP_SPACE::hash_map<T1, T2>::value_type	HASH_MAP_TYPE;
public:
	hash_map_lock(T2 defValue)
	{
		_T1defValue = defValue;
		pthread_mutex_init(&_mutexmap, NULL);
	}
	~hash_map_lock()
	{
		pthread_mutex_destroy(&_mutexmap);
	}
	void add_state(T1 fd, T2 state)
	{
		pthread_mutex_lock(&_mutexmap);
		{
			if (_hash_map.find(fd) != _hash_map.end())
				_hash_map[fd] = state;
			else
				_hash_map.insert(HASH_MAP_TYPE(fd, state));
		}
		pthread_mutex_unlock(&_mutexmap);
	}

	void del_state(T1 fd)
	{
		pthread_mutex_lock(&_mutexmap);
		{
			HASH_MAP_ITER iter = _hash_map.find(fd);
			if (iter != _hash_map.end())
			{
				_hash_map.erase(iter);
			}
		}
		pthread_mutex_unlock(&_mutexmap);
	}

	T2 get_state(T1 fd)
	{
		T2 state = _T1defValue;
		pthread_mutex_lock(&_mutexmap);
		{
			HASH_MAP_ITER iter = _hash_map.find(fd);
			if (iter != _hash_map.end())
			{
				state = iter->second;
			}
		}
		pthread_mutex_unlock(&_mutexmap);
		return state;
	}
protected:
	pthread_mutex_t	_mutexmap;
	HASH_MAP		_hash_map;
	T2 _T1defValue;
};

template<class T1, class T2>
class hash_map_unlock
{
	typedef typename HASH_MAP_SPACE::hash_map<T1, T2>				HASH_MAP;
	typedef typename HASH_MAP_SPACE::hash_map<T1, T2>::iterator		HASH_MAP_ITER;
	typedef typename HASH_MAP_SPACE::hash_map<T1, T2>::value_type	HASH_MAP_TYPE;
public:
	hash_map_unlock(T2 defValue)
	{
		_T1defValue = defValue;
	}
	void add_state(T1 fd, T2 state)
	{
		if (_hash_map.find(fd) != _hash_map.end())
			_hash_map[fd] = state;
		else
			_hash_map.insert(HASH_MAP_TYPE(fd, state));
	}

	void del_state(T1 fd)
	{
		HASH_MAP_ITER iter = _hash_map.find(fd);
		if (iter != _hash_map.end())
		{
			_hash_map.erase(iter);
		}
	}

	T2 get_state(T1 fd)
	{
		T2 state = _T1defValue;
		HASH_MAP_ITER iter = _hash_map.find(fd);
		if (iter != _hash_map.end())
		{
			state = iter->second;
		}
		return state;
	}
protected:
	HASH_MAP _hash_map;
	T2 _T1defValue;
};

#endif // YYMMS_HASH_MAP
