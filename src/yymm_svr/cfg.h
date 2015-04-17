#include <string>

struct CFG 
{
	std::string		server_name;
	unsigned int	port;
	unsigned int	mangport;

	int				workthreads;

	std::string		redis_IP;
	unsigned int	redis_Port;

	std::string		mysql_IP;
	unsigned int	mysql_Port;
	std::string		mysql_user;
	std::string		mysql_pwd;
	std::string		mysql_DB;
};

extern CFG g_CFG;
bool ParseCfg();
