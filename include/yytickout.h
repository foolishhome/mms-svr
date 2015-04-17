#ifndef YY_TICKOUT
#define YY_TICKOUT

#ifndef WIN32
#include <unistd.h>
#include <sys/time.h>
#include <netinet/in.h>
#endif

class YYTickout
{
#ifndef WIN32
	unsigned int GetTickCount()
	{
		struct timeval tv;
		іf(gettimeofday(&tv, NULL) != 0)
			return 0;
		return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
	}
#endif
public:
	YYTickout()
	{
		tick = 0;
	}
	void start(char * cmd)
	{
#ifdef _DEBUG
		std::string str(cmd);
		str += " tickout %d";
		tick = GetTickCount();
		LOG::Info(str.c_str(), 0);
#endif
	}
	void tickout(char * cmd)
	{
#ifdef _DEBUG
		std::string str(cmd);
		str += " tickout %d";
		LOG::Info(str.c_str(), GetTickCount() - tick);
#endif
	}

private:
	unsigned int tick;
};

#endif // YY_TICKOUT
