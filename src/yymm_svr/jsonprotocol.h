#ifndef YY_JSON_PROTOCOL
#define YY_JSON_PROTOCOL

#ifndef YY_JSON
#include "yyjson.h"
#endif
#include <string>
#ifndef YYSSL_H
#include "yyssl.h"
#endif

#define YY_JSON_CMD_MIN				1000

// comment between client and server
#define YY_JSON_COMPRESS_RSA		1000
#define YY_JSON_COMPRESS_AES		1001

#define YY_JSON_LOGIN				1011
#define YY_JSON_LOGIN_RES			1012
#define YY_JSON_LOGIN1				1013
#define YY_JSON_LOGIN1_RES			1014
#define YY_JSON_LOGOUT				1015
#define YY_JSON_LOGOUT_RES			1016

#define YY_JSON_OPENURL				1020
#define YY_JSON_OPENURL_RES			1021
// comment between yymms_svr and shellclient
#define YY_JSON_MMS_OPENURL			2000
#define YY_JSON_MMS_OPENURL_RES		2001
#define YY_JSON_MMS_STOPPLAY		2002
#define YY_JSON_MMS_STOPPLAY_RES	2003
#define YY_JSON_MMS_EXIT			2004

// comment between shellclient and yymms_svr
#define YY_JSON_SVR_INIT			3000

#define YY_JSON_CMD_MAX				4000

#define is_key(a, t)	(strcmp(key, a) == 0 && json_object_get_type(val) == t)

class YYJ
{
public:
	int	_id;
public:
	YYJ(int id) {_id = id;}
	virtual bool Serialize(json_object * obj) = 0;
	virtual bool Serialize(std::string & cmd) = 0;
};

/*
 *	Login step
 *	1. client->server: YJson_Login			with YYID, token and RSA public key
 *	2. server->client: YYJson_Login_res		answer token verify succeed or not and take with AES encrypted by RSA public key
 *	3. client->server: YJson_Login1			with password encrypted by AES key
 *	4. server->client: YYJson_Login1_res	return login succeed or not
 */
/*
 *	Login step(version 2)
 *	1. client->server: YJson_Login			with YYID and RSA public key
 *	2. server->client: YYJson_Login_res		answer succeed if YYID is in whilelist and take with AES encrypted by RSA public key
 *	3. client->server: YJson_Login1			with token encrypted by AES
 *	4. server->client: YYJson_Login1_res	return token verification succeed or not
 *	5. client->server: YJson_Login2			with password encrypted by AES key
 *	6. server->client: YYJson_Login2_res	return login succeed or not
 */
class YYJson_Login : public YYJ
{
public:
	YYJson_Login() : YYJ(YY_JSON_LOGIN)
	{
		yyID = 0;
	}

	static YYJ * clone() {return new YYJson_Login;}
	virtual bool Serialize(json_object * obj)
	{
		json_object_object_foreach(obj, key, val)
		{
			if (is_key("yyID", json_type_int))
			{
				yyID = json_object_get_int(val);
			}
			else if (is_key("token", json_type_string))
			{
				token = json_object_get_string(val);
			}
			else if (is_key("rsa", json_type_string))
			{
				std::string rsaStr = json_object_get_string(val);
				if (!YYSSL::YYBase64::base64decode(rsaStr, rsakey))
					return false;
			}
		}
		return true;
	}
	virtual bool Serialize(std::string & cmd)
	{
		std::string rsaStr;
		if (!YYSSL::YYBase64::base64encode(rsakey, rsaStr))
			return false;
		cmd = "{" + YYJson::toCmd("yyID", yyID) + "," + YYJson::toCmd("token", token) + "," + YYJson::toCmd("rsa", rsaStr) + "}";
		return true;
	}
public:
	unsigned int	yyID;
	std::string		token;
	std::string		rsakey;
};

class YYJson_Login_res : public YYJ
{
public:
	YYJson_Login_res() : YYJ(YY_JSON_LOGIN_RES)
	{
		bTokenSucc = false;
	}
	static YYJ * clone() {return new YYJson_Login_res;}
	virtual bool Serialize(json_object * obj)
	{
		json_object_object_foreach(obj, key, val)
		{
			if (is_key("bSucc", json_type_boolean))
			{
				bTokenSucc = json_object_get_boolean(val);
			}
			else if (is_key("key", json_type_string))
			{
				std::string aesStr = json_object_get_string(val);
				if (!YYSSL::YYBase64::base64decode(aesStr, aeskey))
					return false;
			}
		}
		return true;
	}
	virtual bool Serialize(std::string & cmd)
	{
		std::string aesStr;
		if (!YYSSL::YYBase64::base64encode(aeskey, aesStr))
			return false;
		cmd = "{" + YYJson::toCmd("bSucc", bTokenSucc) + "," + YYJson::toCmd("key", aesStr) + "}";
		return true;
	}
public:
	bool		bTokenSucc;
	std::string	aeskey;
};

class YYJson_Login1 : public YYJ
{
public:
	YYJson_Login1() : YYJ(YY_JSON_LOGIN1)
	{
		uid = 0;
	}

	static YYJ * clone() {return new YYJson_Login1;}
	virtual bool Serialize(json_object * obj)
	{
		json_object_object_foreach(obj, key, val)
		{
			if (is_key("uid", json_type_int))
			{
				uid = json_object_get_int(val);
			}
			else if (is_key("user", json_type_string))
			{
				user = json_object_get_string(val);
			}
			else if (is_key("pwd", json_type_string))
			{
				pwd = json_object_get_string(val);
			}
		}
		return true;
	}
	virtual bool Serialize(std::string & cmd)
	{
		cmd = "{" + YYJson::toCmd("user", user) + "," + YYJson::toCmd("uid", uid) + "," + YYJson::toCmd("pwd", pwd) + "}";
		return true;
	}
public:
	unsigned int	uid;
	std::string		user;
	std::string		pwd;
};

class YYJson_Login1_res : public YYJ
{
public:
	YYJson_Login1_res() : YYJ(YY_JSON_LOGIN1_RES)
	{
		bSucc = false;
	}
	static YYJ * clone() {return new YYJson_Login1_res;}
	virtual bool Serialize(json_object * obj)
	{
		json_object_object_foreach(obj, key, val)
		{
			if (is_key("bSucc", json_type_boolean))
			{
				bSucc = (bool)json_object_get_boolean(val);
			}
		}
		return true;
	}
	virtual bool Serialize(std::string & cmd)
	{
		cmd = "{" + YYJson::toCmd("bSucc", bSucc) + "}";
		return true;
	}
public:
	bool	bSucc;
};

class YYJson_Logout : public YYJ
{
public:
	YYJson_Logout(int id = YY_JSON_LOGOUT) : YYJ(id)
	{
	}

	static YYJ * clone() {return new YYJson_Logout;}
	virtual bool Serialize(json_object * obj)
	{
		json_object_object_foreach(obj, key, val)
		{
			if (is_key("yyID", json_type_int))
			{
				yyID = json_object_get_int(val);
			}
		}
		return true;
	}
	virtual bool Serialize(std::string & cmd)
	{
		cmd = "{" + YYJson::toCmd("yyID", yyID) + "}";
		return true;
	}
public:
	unsigned int yyID;
};

class YYJson_Logout_res : public YYJson_Logout
{
public:
	YYJson_Logout_res() : YYJson_Logout(YY_JSON_LOGOUT_RES)
	{
	}
	static YYJ * clone() {return new YYJson_Logout_res;}	
};

class YYJson_OpenUrl : public YYJ
{
public:
	YYJson_OpenUrl() : YYJ(YY_JSON_OPENURL)
	{
	}

	static YYJ * clone() {return new YYJson_OpenUrl;}
	virtual bool Serialize(json_object * obj)
	{
		json_object_object_foreach(obj, key, val)
		{
			if (is_key("channelID", json_type_int))
			{
				channelID = json_object_get_int(val);
			}
			else if (is_key("subChannelID", json_type_int))
			{
				subChannelID = json_object_get_int(val);
			}
			else if (is_key("url", json_type_string))
			{
				url = json_object_get_string(val);
			}
		}
		return true;
	}
	virtual bool Serialize(std::string & cmd)
	{
		cmd = "{" + YYJson::toCmd("url", url) + "," + YYJson::toCmd("channelID", channelID) + "," + YYJson::toCmd("subChannelID", subChannelID) + "}";
		return true;
	}
public:
	unsigned int channelID;
	unsigned int subChannelID;
	std::string url;
};

class YYJson_OpenUrl_res : public YYJ
{
public:
	YYJson_OpenUrl_res() : YYJ(YY_JSON_OPENURL_RES)
	{
		bSucc = false;
	}

	static YYJ * clone() {return new YYJson_OpenUrl_res;}
	virtual bool Serialize(json_object * obj)
	{
		json_object_object_foreach(obj, key, val)
		{
			if (is_key("bSucc", json_type_boolean))
			{
				bSucc = (bool)json_object_get_boolean(val);
			}
			else if (is_key("url", json_type_string))
			{
				url = json_object_get_string(val);
			}
		}
		return true;
	}
	virtual bool Serialize(std::string & cmd)
	{
		cmd = "{" + YYJson::toCmd("bSucc", bSucc) + "," + YYJson::toCmd("url", url) + "}";
		return true;
	}
public:
	bool		bSucc;
	std::string url;
};


/*
 *	YYJson_MMS 表示yymms_svr向shellclient发送的命令
 */
class YYJson_MMS_OpenUrl : public YYJ
{
public:
	YYJson_MMS_OpenUrl() : YYJ(YY_JSON_MMS_OPENURL)
	{
		uid = yyID = channelID = subChannelID = 0;
	}

	static YYJ * clone() {return new YYJson_MMS_OpenUrl;}
	virtual bool Serialize(json_object * obj)
	{
		json_object_object_foreach(obj, key, val)
		{
			if (is_key("uid", json_type_int))
			{
				uid = json_object_get_int(val);
			}
			else if (is_key("yyID", json_type_int))
			{
				yyID = json_object_get_int(val);
			}
			else if (is_key("channelID", json_type_int))
			{
				channelID = json_object_get_int(val);
			}
			else if (is_key("subChannelID", json_type_int))
			{
				subChannelID = json_object_get_int(val);
			}
			else if (is_key("url", json_type_string))
			{
				url = json_object_get_string(val);
			}
			else if (is_key("user", json_type_string))
			{
				user = json_object_get_string(val);
			}
			else if (is_key("tick", json_type_string))
			{
				tick = json_object_get_string(val);
			}
		}
		return true;
	}
	virtual bool Serialize(std::string & cmd)
	{
		cmd = "{" + YYJson::toCmd("uid", uid) +
				"," + YYJson::toCmd("yyID", yyID) + 
				"," + YYJson::toCmd("user", user) + 
				"," + YYJson::toCmd("tick", tick) + 
				"," + YYJson::toCmd("channelID", channelID) + 
				"," + YYJson::toCmd("subChannelID", subChannelID) + 
				"," + YYJson::toCmd("url", url) + "}";
		return true;
	}
public:
	unsigned int	uid;
	unsigned int	yyID;
	unsigned int	channelID;
	unsigned int	subChannelID;
	std::string		url;
	std::string		user;
	std::string		tick;
};

class YYJson_MMS_StopPlay : public YYJ
{
public:
	YYJson_MMS_StopPlay() : YYJ(YY_JSON_MMS_STOPPLAY)
	{
		yyID = 0;
	}

	static YYJ * clone() {return new YYJson_MMS_StopPlay;}
	virtual bool Serialize(json_object * obj)
	{
		json_object_object_foreach(obj, key, val)
		{
			if (is_key("yyID", json_type_int))
			{
				yyID = json_object_get_int(val);
			}
		}
		return true;
	}
	virtual bool Serialize(std::string & cmd)
	{
		cmd = "{" + YYJson::toCmd("yyID", yyID) + "}";
		return true;
	}
public:
	unsigned int	yyID;
};

class YYJson_MMS_StopPlay_res : public YYJ
{
public:
	YYJson_MMS_StopPlay_res() : YYJ(YY_JSON_MMS_STOPPLAY_RES)
	{
		yyID = 0;
	}

	static YYJ * clone() {return new YYJson_MMS_StopPlay_res;}
	virtual bool Serialize(json_object * obj)
	{
		json_object_object_foreach(obj, key, val)
		{
			if (is_key("yyID", json_type_int))
			{
				yyID = json_object_get_int(val);
			}
		}
		return true;
	}
	virtual bool Serialize(std::string & cmd)
	{
		cmd = "{" + YYJson::toCmd("yyID", yyID) + "}";
		return true;
	}
public:
	unsigned int	yyID;
};

class YYJson_MMS_Exit : public YYJ
{
public:
	YYJson_MMS_Exit() : YYJ(YY_JSON_MMS_EXIT)
	{
		yyID = 0;
	}

	static YYJ * clone() {return new YYJson_MMS_Exit;}
	virtual bool Serialize(json_object * obj)
	{
		json_object_object_foreach(obj, key, val)
		{
			if (is_key("yyID", json_type_int))
			{
				yyID = json_object_get_int(val);
			}
		}
		return true;
	}
	virtual bool Serialize(std::string & cmd)
	{
		cmd = "{" + YYJson::toCmd("yyID", yyID) + "}";
		return true;
	}
public:
	unsigned int	yyID;
};

/*
 *	YYJson_Svr 表示shellclient向yymms_svr发送的命令
 */
class YYJson_Svr_Init : public YYJ
{
public:
	YYJson_Svr_Init() : YYJ(YY_JSON_SVR_INIT)
	{
		yyID = 0;
		bSucc = false;
	}

	static YYJ * clone() {return new YYJson_Svr_Init;}
	virtual bool Serialize(json_object * obj)
	{
		json_object_object_foreach(obj, key, val)
		{
			if (is_key("bSucc", json_type_boolean))
			{
				bSucc = (bool)json_object_get_boolean(val);
			}
			else if (is_key("yyID", json_type_int))
			{
				yyID = json_object_get_int(val);
			}
		}
		return true;
	}
	virtual bool Serialize(std::string & cmd)
	{
		cmd = "{" + YYJson::toCmd("bSucc", bSucc) + "," + YYJson::toCmd("yyID", yyID) + "}";
		return true;
	}
public:
	unsigned int yyID;
	bool	bSucc;
};

enum YYMMSJSON_ERROR{
	YYMMSJSON_ERROR_SUCCESS = 0,// 成功
	YYMMSJSON_ERROR_BUFSIZE,	// 缓冲区异常
	YYMMSJSON_ERROR_ESC,		// 首字符错误
	YYMMSJSON_ERROR_JSON,		// JSON数据错误
	YYMMSJSON_ERROR_UNKNOWED	// 不明错误
};

static YYJ * LookforMap(int id);
class fastbuffer;
class YYmmsJson
{
	static const char PRE_CHAR	= '\27';
public:
	typedef YYJ* (*JFUNC)(void);
	typedef struct _fn_struct
	{
		int id;
		JFUNC fn;
	}fn_struct;
public:
	static YYMMSJSON_ERROR ParseCmd(const char * cmd, int len, int & offset, YYJ *& jcmd)
	{

		char * buf = (char*)cmd;
		if (len < 3)		return YYMMSJSON_ERROR_BUFSIZE;
		if (*buf != '\27')	return YYMMSJSON_ERROR_ESC;
		buf++;
		WORD jsize = *(WORD*)buf;
		if (jsize > len)	return YYMMSJSON_ERROR_BUFSIZE;
		buf += 2;

		std::string js;
		js.append(buf, jsize - 3);
		YYJson json;
		YYMMSJSON_ERROR jserr = YYMMSJSON_ERROR_UNKNOWED;
		do 
		{
			json_tokener_error err = json.Parse(js.c_str());
			if (json_tokener_success != err)
			{
				jserr = YYMMSJSON_ERROR_JSON;
				break;
			}
			json_object * cmd = json_object_object_get(json._jobj, "cmd");
			json_object * obj = json_object_object_get(json._jobj, "obj");
			if (!cmd || !obj || json_object_get_type(cmd) != json_type_int)
			{
				jserr = YYMMSJSON_ERROR_JSON;
				break;
			}
			int id = json_object_get_int(cmd);
			YYJ * jobj = LookforMap(id);
			if (jobj && jobj->Serialize(obj))
			{
				jcmd = jobj;
				offset += jsize;
				return YYMMSJSON_ERROR_SUCCESS;
			}
		} while (0);
		return jserr;
	}
	static void FreeCmd(YYJ * obj)
	{
		if (obj)
			delete obj;
	}
	static bool FormatCmd(YYJ * obj, std::string & cmd)
	{
		if (!obj) return false;
		std::string str, cmdstr;
		if (obj->Serialize(str))
		{
			cmdstr = "{" + YYJson::toCmd("cmd", obj->_id) + ",\"obj\":" + str + "}";

			cmd.push_back(PRE_CHAR);
			WORD len = cmdstr.length() + 3;
			cmd.append((const char*)&len, sizeof(WORD));
			cmd += cmdstr;
			return true;
		}
		return false;
	}
/*
	static bool FormatCmd(YYJ * obj, fastbuffer * cmd)
	{
		if (!obj || !cmd) return false;
		std::string str, cmdstr;
		if (obj->Serialize(str))
		{
			cmdstr = "{\"cmd\":" + YYJson::tos(obj->_id) + ",\"obj\":" + str + "}";

			cmd->push_back(PRE_CHAR);
			WORD len = cmdstr.length() + 3;
			cmd->append((const char*)&len, sizeof(WORD));
			cmd->append(cmdstr.c_str(), cmdstr.length());
			return true;
		}
		return false;
	}
*/
};

class YYJson_Comp_RSA : public YYJ
{
public:
	YYJson_Comp_RSA() : YYJ(YY_JSON_COMPRESS_RSA)
	{
		version = 0;
	}
	~YYJson_Comp_RSA()
	{
	}
	static YYJ * clone() {return new YYJson_Comp_RSA;}
	virtual bool Serialize(json_object * obj)
	{
		json_object_object_foreach(obj, key, val)
		{
			if (is_key("version", json_type_int))
			{
				version = json_object_get_int(val);
			}
			else if (is_key("cmd", json_type_string))
			{
				serString = json_object_get_string(val);
			}
		}
		return true;
	}
	virtual bool Serialize(std::string & cmd)
	{
		cmd = "{" + YYJson::toCmd("version", version) + "," + YYJson::toCmd("cmd", serString) + "}";
		return true;
	}
public:
	bool Format(YYJ * jobj, YYSSL::YYRSA & rsa)
	{
		std::string compstr;
		if (!jobj || !YYmmsJson::FormatCmd(jobj, compstr))
			return false;

		std::string encryStr;
		if (!rsa.RSAPublicEncrypt(compstr, encryStr))
			return false;

		if (!YYSSL::YYBase64::base64encode(encryStr, serString))
			return false;

		return true;
	}
	bool UnFormat(YYSSL::YYRSA & rsa, YYJ *& jobj)
	{
		if (serString.empty())
			return false;

		std::string out;
		if (!YYSSL::YYBase64::base64decode(serString, out))
			return false;

		std::string decryStr;
		if (!rsa.RSAPrivateDecrypt(out, decryStr))
			return false;

		int offset = 0;
		if (YYMMSJSON_ERROR_SUCCESS != YYmmsJson::ParseCmd(decryStr.c_str(), decryStr.length(), offset, jobj))
			return false;

		return true;
	}
	int			version;
	std::string serString;
};

class YYJson_Comp_AES : public YYJ
{
public:
	YYJson_Comp_AES() : YYJ(YY_JSON_COMPRESS_AES)
	{
		version = 0;
	}
	~YYJson_Comp_AES()
	{
	}
	static YYJ * clone() {return new YYJson_Comp_AES;}
	virtual bool Serialize(json_object * obj)
	{
		json_object_object_foreach(obj, key, val)
		{
			if (is_key("version", json_type_int))
			{
				version = json_object_get_int(val);
			}
			else if (is_key("cmd", json_type_string))
			{
				serString = json_object_get_string(val);
			}
		}
		return true;
	}
	virtual bool Serialize(std::string & cmd)
	{
		cmd = "{" + YYJson::toCmd("version", version) + "," + YYJson::toCmd("cmd", serString) + "}";
		return true;
	}
public:
	bool Format(YYJ * jobj, YYSSL::YYAES & aes)
	{
		std::string compstr;
		if (!jobj || !YYmmsJson::FormatCmd(jobj, compstr))
			return false;

		std::string encryStr;
		if (!aes.AESEncrypt(compstr, encryStr))
			return false;

		if (!YYSSL::YYBase64::base64encode(encryStr, serString))
			return false;

		return true;
	}
	bool UnFormat(YYSSL::YYAES & aes, YYJ *& jobj)
	{
		if (serString.empty())
			return false;

		std::string out;
		if (!YYSSL::YYBase64::base64decode(serString, out))
			return false;

		std::string decryStr;
		if (!aes.AESDecrypt(out, decryStr))
			return false;

		int offset = 0;
		if (YYMMSJSON_ERROR_SUCCESS != YYmmsJson::ParseCmd(decryStr.c_str(), decryStr.length(), offset, jobj))
			return false;

		return true;
	}
	int			version;
	std::string serString;
};

static YYJ * LookforMap(int id)
{
	static YYmmsJson::fn_struct fnMap[] = {
		{YY_JSON_COMPRESS_RSA, YYJson_Comp_RSA::clone}
		,{YY_JSON_COMPRESS_AES, YYJson_Comp_AES::clone}
		,{YY_JSON_LOGIN, YYJson_Login::clone}
		,{YY_JSON_LOGIN_RES, YYJson_Login_res::clone}
		,{YY_JSON_LOGIN1, YYJson_Login1::clone}
		,{YY_JSON_LOGIN1_RES, YYJson_Login1_res::clone}
		,{YY_JSON_LOGOUT, YYJson_Logout::clone}
		,{YY_JSON_LOGOUT_RES, YYJson_Logout_res::clone}
		,{YY_JSON_OPENURL, YYJson_OpenUrl::clone}
		,{YY_JSON_OPENURL_RES, YYJson_OpenUrl_res::clone}
		,{YY_JSON_MMS_OPENURL, YYJson_MMS_OpenUrl::clone}
		// ,{YY_JSON_MMS_OPENURL_RES, YYJson_MMS_OpenUrl_res::clone}
		,{YY_JSON_MMS_STOPPLAY, YYJson_MMS_StopPlay::clone}
		,{YY_JSON_MMS_STOPPLAY_RES, YYJson_MMS_StopPlay_res::clone}
		,{YY_JSON_MMS_EXIT, YYJson_MMS_Exit::clone}

		,{YY_JSON_SVR_INIT, YYJson_Svr_Init::clone}

		,{0, NULL}
	};

	for (int i = 0; i < sizeof(fnMap) / sizeof(YYmmsJson::fn_struct); i++)
	{
		if (fnMap[i].id == id && fnMap[i].fn != NULL)
		{
			return fnMap[i].fn();
		}
	}
	return NULL;
}

#endif	// YY_JSON_PROTOCOL
