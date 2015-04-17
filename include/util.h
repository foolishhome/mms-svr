#ifndef YYMMS_UTIL
#define YYMMS_UTIL

#include <vector>

#ifndef YYMMS_HASH_MAP
#include "event_hash.h"
#endif

namespace UTIL
{
	template<typename WorkerID, class T, class TOUCH>
	class Bossworker
	{
	public:
		typedef typename hash_map_unlock<WorkerID, std::pair<std::queue<T>, TOUCH>*>::iterator WORKER_ITER;
		Bossworker(T defValue, TOUCH defTVal) : _workers(NULL)
		{
			_TdefValue = defValue;
			_TdefTouch = defTVal;
			pthread_mutex_init(&_mutex, NULL);
		}
		~Bossworker()
		{
			pthread_mutex_lock(&_mutex);
			{
				WORKER_ITER iter;
				for (iter = _workers.begin(); iter != _workers.end(); iter++)
				{
					if (iter->second)
						delete iter->second;
				}
				_workers.clear();
			}
			pthread_mutex_unlock(&_mutex);

			pthread_mutex_destroy(&_mutex);
		}
		void SetWorker(WorkerID id, TOUCH data)
		{
			pthread_mutex_lock(&_mutex);
			{
				std::pair<std::queue<T>, TOUCH> * q = _workers.get_state(id);
				if (q)
					q->second = data;
				else
					_workers.add_state(id, new std::pair<std::queue<T>, TOUCH>(std::queue<T>(), data));
			}
			pthread_mutex_unlock(&_mutex);
		}
		TOUCH GetWorkerTouch(WorkerID id)
		{
			TOUCH data = _TdefTouch;
			pthread_mutex_lock(&_mutex);
			{
				std::pair<std::queue<T>, TOUCH> * q = _workers.get_state(id);
				if (q)
					data = q->second;
			}
			pthread_mutex_unlock(&_mutex);
			return data;
		}
		bool PushJob(WorkerID id, T t)
		{
			bool bRet = false;
			pthread_mutex_lock(&_mutex);
			{
				std::pair<std::queue<T>, TOUCH> * q = _workers.get_state(id);
				if (q)
				{
					q->first.push(t);
					bRet = true;
				}
			}
			pthread_mutex_unlock(&_mutex);
			return bRet;
		}
		T PopJob(WorkerID id)
		{
			T t = _TdefValue;
			pthread_mutex_lock(&_mutex);
			{
				std::pair<std::queue<T>, TOUCH> * q = _workers.get_state(id);
				if (q && !q->first.empty())
				{
					t = q->first.front();
					q->first.pop();
				}
			}
			pthread_mutex_unlock(&_mutex);
			return t;
		}
	private:
		pthread_mutex_t	_mutex;

		hash_map_unlock<WorkerID, std::pair<std::queue<T>, TOUCH>*> _workers;

		T		_TdefValue;
		TOUCH	_TdefTouch;
	};

	template<class T, class TOUCH>
	class Bossmultiworker
	{
	public:
		typedef typename std::pair<std::queue<T>, TOUCH>		WORKER;

		Bossmultiworker(T defValue, TOUCH defTVal) : _workers(NULL)
		{
			_TdefValue = defValue;
			_TdefTouch = defTVal;
		}
		~Bossmultiworker()
		{
			if (_mutexs)
			{
				for (int i = 0; i < _works; i++)
					pthread_mutex_destroy(&_mutexs[i]);
				delete[] _mutexs;
			}
			if (_workers)
			{
				for (int i = 0; i < _works; i++)
					delete _workers[i];
				delete[] _workers;
			}
		}
		bool Init(int numWorkers)
		{
			_works = numWorkers;
			_mutexs = new pthread_mutex_t[numWorkers];
			_workers = new WORKER*[numWorkers];
			for (int i = 0; i < numWorkers; i++)
			{
				pthread_mutex_init(&_mutexs[i], NULL);
				_workers[i] = new std::pair<std::queue<T>, TOUCH>();
				_workers[i]->second = _TdefTouch;
			}
			return true;
		}
		void SetWorker(int id, TOUCH data)
		{
			assert(id >= 0 && id < _works);
			if (id < 0 || id >= _works) return;
			_workers[id]->second = data;
		}
		TOUCH GetWorkerTouch(int id)
		{
			assert(id >= 0 && id < _works);
			if (id < 0 || id >= _works) return _TdefTouch;
			return _workers[id]->second;
		}
		bool PushJob(int id, T t)
		{
			assert(id >= 0 && id < _works);
			if (id < 0 || id >= _works) return false;
			pthread_mutex_lock(&_mutexs[id]);
			{
				_workers[id]->first.push(t);
			}
			pthread_mutex_unlock(&_mutexs[id]);
			return true;
		}
		T PopJob(int id)
		{
			assert(id >= 0 && id < _works);
			if (id < 0 || id >= _works) return _TdefValue;
			T t = _TdefValue;
			pthread_mutex_lock(&_mutexs[id]);
			{
				if (!_workers[id]->first.empty())
				{
					t = _workers[id]->first.front();
					_workers[id]->first.pop();
				}
			}
			pthread_mutex_unlock(&_mutexs[id]);
			return t;
		}
	private:
		int					_works;
		pthread_mutex_t	*	_mutexs;

		WORKER **			_workers;

		T		_TdefValue;
		TOUCH	_TdefTouch;
	};
}

#endif // YYMMS_UTIL
