#include "stdafx.h"
#include "cmdproc.h"
#include "yymms_svr.h"
#include <signal.h>
#include "cfg.h"
#include "yyssl.h"

MmsServer::MmsServer() :
		hash_map_lock<int, boost::shared_ptr<Item_data> >(boost::shared_ptr<Item_data>((Item_data*)NULL))
		,_redis(this, &MmsServer::redis_cb)
		,UTIL::Bossmultiworker<BossInfo, thread_data*>(BossInfo(), NULL)
		,_userData(boost::shared_ptr<Item_data>((Item_data*)NULL))
{
	_threadnum = 0;
	YYSSL::SSLInit();
}

MmsServer::~MmsServer()
{
	for (int i = 0; i < _threadnum; i++)
	{
		thread_data* data = GetWorkerTouch(i);
		if (data)
		{
			delete data;
			SetWorker(i, NULL);
		}
	}
}

bool MmsServer::init(int port, int ThreadNum)
{
	_threadnum = ThreadNum;

	if (!_event.Init(this))
		return false;
	if (!_event.Bind(NULL, port))
		return false;
	if (!InitMmsExec(&_event, this))
		return false;

	_redis.Init(&_event, g_CFG.redis_IP.c_str(), g_CFG.redis_Port, _event.Base());
	_redis.Connect();

	Init(_threadnum);
	pthread_t pid;
	for (int i = 0; i < _threadnum; i++)
	{
		thread_data * dt = new thread_data;
		dt->_index = i;
		dt->_svr = this;
		SetWorker(i, dt);

		pthread_create(&pid, NULL, MmsServer::work_thread, (void*)dt);
		dt->_t_handle = pid;
	}
	return true;
}

void MmsServer::exit()
{
	_event.Exit();
}

void MmsServer::run()
{
	_event.Run();

	thread_data* data;
	int i;
	for (i = 0; i < _threadnum; i++)
	{
		data = GetWorkerTouch(i);
		if (!data)
			continue;
		data->_exitflag = true;
		sem_post(&data->_sem);
	}

	for (i = 0; i < _threadnum; i++)
	{
		data = GetWorkerTouch(i);
		if (!data)
			continue;
		pthread_join(data->_t_handle, NULL);
	} 
}

bool MmsServer::data_receive(int fd, fastbuffer * readbuffer, unsigned int listen_port)
{
	if (!readbuffer)
		return true;

	BossInfo info;
	info.buf = readbuffer;
	info.fd = fd;
	info.port = listen_port;

	/*
	 *	Bossworker
	 */
	// 这里采用分配算法将fd分到固定的线程，避免多个线程处理同一个socket
	int threadid = fd % _threadnum; assert(threadid >= 0 && threadid < _threadnum);
	PushJob(threadid, info);

	thread_data * data = GetWorkerTouch(threadid);
	if (data)
		sem_post(&data->_sem);
	return false;	// keep readbuffer alive
}

void * MmsServer::work_thread(void * arg)
{
	thread_data * data = (thread_data*)arg;
	if (!data) return NULL;

	MmsServer * svr = data->_svr;
	if (!svr) return NULL;

	int index = data->_index;

	YYLibMysql mysql;
	if (!mysql.Connect(g_CFG.mysql_IP.c_str(), g_CFG.mysql_Port, g_CFG.mysql_user.c_str(), g_CFG.mysql_pwd.c_str()))
	{
		LOG::Error("connect mysql error");
	}
	mysql.Use(g_CFG.mysql_DB.c_str());

	BossInfo info;
	do
	{
		sem_wait(&data->_sem);
		if (data->_exitflag)
		{
			LOG::Info("thread exit id = %d", pthread_self());
			break;
		}

		info = svr->PopJob(index);
		if (info.fd == 0 || info.buf == NULL)
			continue;

		svr->_process_data(info, &mysql);

	} while (true);
	pthread_exit(NULL);
	return NULL;
}

void MmsServer::_process_data(BossInfo & info, YYLibMysql * mysql)
{
	if (!info.buf)
		return;

	if (info.port == g_CFG.mangport)
	{
		ProcessMmsExecData(info.fd, info.buf);
	}
	else
	{	// process data in readbuffer
		JsonProcess(info.fd, info.buf, mysql);
	}

	// access redis
	// std::string rs(buf->buf(), buf->size());
	// the next two comments will spend almost 1s if connect is break;
	//	_redis.AsyncSendCommand(fd, "LPUSH MMS %s", rs.c_str());
	//	_redis.AsyncSendCommand(fd, "LRANGE MMS 0 -1");

	// buf->append("\0", 1);
}

void MmsServer::redis_cb(redisReply *reply, long fd)
{
	/*
#define REDIS_REPLY_STRING 1
#define REDIS_REPLY_ARRAY 2
#define REDIS_REPLY_INTEGER 3
#define REDIS_REPLY_NIL 4
#define REDIS_REPLY_STATUS 5
#define REDIS_REPLY_ERROR 6
	*/
	if (!reply) return;

	if (reply->type == REDIS_REPLY_ARRAY)
	{
		fastbuffer * echostr = new fastbuffer;

		echostr->append("reply is:\r\n", strlen("reply is:\rn"));
		for (size_t i = 0; i < reply->elements; i++)
		{
			echostr->append(reply->element[i]->str, reply->element[i]->len + 1);
		}

		if (echostr->size() > 0)
		{
			_event.data_send(fd, echostr);
		}
		else
		{
			delete echostr;
		}
	}
}

/*
 *	global function
 */
MmsServer g_svr;


