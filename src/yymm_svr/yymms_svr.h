#include "stdlib.h"
#include <semaphore.h>

#ifndef YYMMS_LOCKQUEUE
#include "event/lockqueue.h"
#endif

#ifndef YYMMS_LIBEVENT
#include "event/yylibevent.h"
#endif

#ifndef YYMMS_HASH_MAP
#include "event_hash.h"
#endif

#ifndef YYMMS_LIBREDIS
#include "redis/yyredis.h"
#endif

#ifndef YYMMS_UTIL
#include "util.h"
#endif

#ifndef YYMMS_LIBMYSQLPP
// #include "mysql/yylibmysqlpp.h"
#endif
#ifndef YYMMS_LIBMYSQL
#include "mysql/yylibmysql.h"
#endif

#ifndef YY_MMSEXECMANAGER
#include "mmsexecmag.h"
#endif

class Item_data;
class MmsServer;
struct thread_data
{
	thread_data()
	{
		_exitflag = false;
		sem_init(&_sem, 0, 0);
		_index = -1;
		_svr = NULL;
	}
	~thread_data()
	{
		sem_destroy(&_sem);
	}
	pthread_t		_t_handle;
	sem_t			_sem;
	bool			_exitflag;
	int				_index;
	MmsServer		*_svr;
};

struct BossInfo
{
	BossInfo()
	{
		buf = NULL;
		fd = 0;
		port = 0;
	}
	fastbuffer *	buf;
	int				fd;
	unsigned int	port;
};

class YYJ;
class YYJson_Login;
class MmsServer : public ILibEvent
	, public hash_map_lock<int, boost::shared_ptr<Item_data> >
	, UTIL::Bossmultiworker<BossInfo, thread_data*>
	, MmsExecManager
{
	friend class MmsExecManager;
public:
	MmsServer();
	virtual ~MmsServer();

public:
	bool init(int port, int ThreadNum);
	void run();
	void exit();

	virtual bool data_receive(int fd, fastbuffer * readbuffer, unsigned int listen_port);
	virtual void fd_close(int fd);

	void redis_cb(redisReply *reply, long fd);
	bool _sendCmd(int fd, YYJ * cmd);

protected:
	static void * work_thread(void * arg);
	void _process_data(BossInfo & info, YYLibMysql * mysql);
	void JsonProcess(int fd, fastbuffer * readbuffer, YYLibMysql * mysql);
	bool CheckWhiteList(unsigned int uid, YYLibMysql * mysql);
	void LoginCmd(int fd, Item_data_ptr state, YYLibMysql * mysql);

private:
	int											_threadnum;

	YYLibEvent									_event;

	YYRedis<YYLibEvent*, MmsServer, long>		_redis;

	hash_map_lock<unsigned int, boost::shared_ptr<Item_data> >		_userData;		// 保存以yyID为索引的Item_data*
};

