#include "stdafx.h"
#include "tinyxml2/tinyxml2.h"
#include "cfg.h"

using namespace tinyxml2;

CFG g_CFG;

std::string GetCfg()
{
	std::string path;
	char buffer[MAX_PATH];

#ifdef WIN32
	GetModuleFileNameA(NULL, buffer, sizeof(buffer));
	char * p = buffer + strlen(buffer);
	while(p != buffer && !strchr(p,'\\'))
	{
		p--;
	}
	*p = '\0';
#else
	getcwd(buffer, MAX_PATH);
#endif

	path = buffer;
	path += "/config/cfg.xml";
	return path;
}

bool ParseCfg()
{
	tinyxml2::XMLDocument doc;
	if (XML_SUCCESS != doc.LoadFile(GetCfg().c_str()))
		return false;

	XMLElement* confElement = doc.FirstChildElement("conf"); if (!confElement) return false;
	XMLElement* serversElement = confElement->FirstChildElement("servers"); if (!serversElement) return false;

	XMLElement* mmsElement = serversElement->FirstChildElement("mms_service");
	if (mmsElement)
	{
		g_CFG.server_name = mmsElement->FirstChildElement("name")->GetText();
		mmsElement->FirstChildElement("port")->QueryUnsignedText(&g_CFG.port);
		mmsElement->FirstChildElement("mangport")->QueryUnsignedText(&g_CFG.mangport);
		mmsElement->FirstChildElement("workthreads")->QueryIntText(&g_CFG.workthreads);
	}

	XMLElement* redisElement = serversElement->FirstChildElement("redis");
	if (redisElement)
	{
		g_CFG.redis_IP = redisElement->FirstChildElement("redis_Ip")->GetText();
		redisElement->FirstChildElement("redis_Port")->QueryUnsignedText(&g_CFG.redis_Port);
	}

	XMLElement* mysqlElement = serversElement->FirstChildElement("mysql");
	if (mysqlElement)
	{
		g_CFG.mysql_IP = mysqlElement->FirstChildElement("ip")->GetText();
		mysqlElement->FirstChildElement("port")->QueryUnsignedText(&g_CFG.mysql_Port);
		g_CFG.mysql_user = mysqlElement->FirstChildElement("user")->GetText();
		g_CFG.mysql_pwd = mysqlElement->FirstChildElement("pwd")->GetText();
		g_CFG.mysql_DB = mysqlElement->FirstChildElement("db")->GetText();
	}

	return true;
}
