#include <sys/types.h>
#include <event.h>
#include "hiredis.h"
#include "async.h"

#ifdef __cplusplus
extern "C" {
#endif

void redisLibeventReadEvent(int fd, short event, void *arg);

void redisLibeventWriteEvent(int fd, short event, void *arg);

void redisLibeventAddRead(void *privdata);

void redisLibeventDelRead(void *privdata);

void redisLibeventAddWrite(void *privdata);

void redisLibeventDelWrite(void *privdata);

void redisLibeventCleanup(void *privdata);

int redisLibeventAttach(redisAsyncContext *ac, struct event_base *base);

namespace COMMON
{
	unsigned int GetTickCount();
}

#ifdef __cplusplus
}
#endif

