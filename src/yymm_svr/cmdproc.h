#ifndef CMD_PROC_H
#define CMD_PROC_H

#ifndef FASTBUFFER
#include "fastbuffer.h"
#endif
#include <string>
#include <boost/shared_ptr.hpp>
#ifndef YYSSL_H
#include "yyssl.h"
#endif

class Item_data
{
public:
	Item_data()
	{
		fd = uid = yyID = shellClient_fd = 0;
		channelID = subChannelID = 0;
		readbuffer = NULL;
		loginTime = 0;
	}
	~Item_data()
	{
		if (readbuffer) delete readbuffer;
	}
	int fd;
	fastbuffer * readbuffer;

	bool isConnectShellClient()
	{
		return shellClient_fd != 0;
	}
public:
	// account info
	unsigned int yyID;
	unsigned int uid;
	std::string account;
	std::string pwd;

	unsigned int loginTime;

	// openurl info
	unsigned int channelID;
	unsigned int subChannelID;
	std::string  url;
	
	int shellClient_fd;

	YYSSL::YYAES aes;
	std::string token;

	std::string rsaStr;
	YYSSL::YYRSA rsa;
};
typedef boost::shared_ptr<Item_data>	Item_data_ptr;

#endif	// CMD_PROC_H
