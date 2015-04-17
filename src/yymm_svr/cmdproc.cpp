#include "stdafx.h"
#include "jsonprotocol.h"
#include "cmdproc.h"
#include "yymms_svr.h"
#include "cfg.h"

#ifndef WIN32
unsigned int GetTickCount()
{
	struct timeval tv;
	if (gettimeofday(&tv, NULL) != 0)
		return 0;

	return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}
#endif


bool MmsServer::_sendCmd(int fd, YYJ * cmd)
{
	std::string str;
	if (!YYmmsJson::FormatCmd(cmd, str) || str.length() == 0)
		return false;

	fastbuffer * writebuffer = new fastbuffer;
	if (!writebuffer)
		return false;
	writebuffer->append(str.c_str(), str.length());

	_event.data_send(fd, writebuffer);
	// you can't del writebuffer, data_send will deliver to _event
	// delete writebuffer;
	return true;
}

/*
 *	return true, if have something needed to send
 *	you need to delete readbuffer if not use anymore;
 */
void MmsServer::JsonProcess(int fd, fastbuffer * readbuffer, YYLibMysql * mysql)
{
	if (fd <= 0 || !readbuffer)
		return;

	fastbuffer * buf = NULL;
	Item_data_ptr state = get_state(fd);
	if (state)
	{
		if (state->readbuffer)
		{
			state->readbuffer->append(readbuffer->buf(), readbuffer->size());
		}
		else
		{
			state->readbuffer = readbuffer;
			readbuffer = NULL;
		}
		buf = state->readbuffer;
	}
	else
	{
		buf = readbuffer;
	}

	// json process
	YYJ * jobj;
	do 
	{
		int offset = 0;
		jobj = NULL;
		YYMMSJSON_ERROR err;
		err = YYmmsJson::ParseCmd(buf->buf(), buf->size(), offset, jobj);
		switch (err)
		{
		case YYMMSJSON_ERROR_SUCCESS:
			{
				if (jobj->_id == YY_JSON_LOGIN)
				{	// check user account
					YYJson_Login * login = (YYJson_Login*)jobj;
					if (!state)
					{
						state = boost::shared_ptr<Item_data>(new Item_data);
						if (!state) break;
						state->fd = fd;
						state->readbuffer = readbuffer;
						readbuffer = NULL;
						add_state(fd, state);
						_userData.add_state(login->yyID, state);
					}
					state->yyID = login->yyID;
					state->token = login->token;
					state->rsaStr = login->rsakey;
					std::string prik;
					state->rsa.SetRSAKey(state->rsaStr, prik);
					state->loginTime = GetTickCount();
					LoginCmd(fd, state, mysql);
				}
				else
				{
					if (!state)
					{
						LOG::Info("YY_JSON_LOGIN not login fd=%d", fd);
						break;
					}
NextRun:
					switch (jobj->_id)
					{
					case YY_JSON_COMPRESS_RSA:
						{
							assert(false);
						}
						break;
					case YY_JSON_COMPRESS_AES:
						{
							YYJson_Comp_AES * cmd = (YYJson_Comp_AES*)jobj;
							YYJ * realobj;
							if (!cmd->UnFormat(state->aes, realobj))
								break;

							YYmmsJson::FreeCmd(jobj);
							jobj = realobj;
							goto NextRun;
						}
						break;
					case YY_JSON_LOGIN1:
						{
							YYJson_Login1 * cmd = (YYJson_Login1*)jobj;
							state->uid = cmd->uid;
							state->account = cmd->user;
							state->pwd = cmd->pwd;

							YYJson_Login1_res resp;
							resp.bSucc = true;
							_sendCmd(fd, &resp);
						}
						break;
					case YY_JSON_LOGOUT:
						{
							LOG::Info("YY_JSON_LOGIN logout fd=%d, uid=%u, account:%s, logintime=%u, logouttime=%u", fd, state->uid, state->account.c_str(), state->loginTime, GetTickCount());
							YYJson_Logout_res resp;
							resp.yyID = state->yyID;
							_sendCmd(fd, &resp);

							ShellExit(state);

							_userData.del_state(state->yyID);
							del_state(fd);
						}
						break;
					case YY_JSON_OPENURL:
						{
							YYJson_OpenUrl * cmd = (YYJson_OpenUrl*)jobj;
							state->channelID = cmd->channelID;
							state->subChannelID = cmd->subChannelID;
							state->url = cmd->url;

							bool bSucc = true;
							if (state->isConnectShellClient())
								openURL(state->shellClient_fd, state);
							else
							{
								bSucc = ShellExec(state);
							}
							YYJson_OpenUrl_res resp;
							resp.bSucc = (json_bool)bSucc;
							_sendCmd(fd, &resp);
						}
						break;
					}
					// LOG::Info("json %s", json_object_to_json_string(jobj->_jobj));
				}
				buf->read(NULL, offset);
				YYmmsJson::FreeCmd(jobj);
			}
			break;
		// case YYMMSJSON_ERROR_ESC:
		// case YYMMSJSON_ERROR_JSON:
		default:
			buf->clear();
			_event.closefd(fd);
			break;
		}
	} while (buf->size() > 0);
	
	if (readbuffer)
		delete readbuffer;
}

void MmsServer::LoginCmd(int fd, Item_data_ptr state, YYLibMysql * mysql)
{
	if (!state)
		return;

	bool loginOK = CheckWhiteList(state->uid, mysql);
	if (!loginOK)
	{
		LOG::Info("YY_JSON_LOGIN logined whileList failed fd=%d, uid=%u, account:%s, logintime=%u", fd, state->uid, state->account.c_str(), state->loginTime);
		YYJson_Login_res resp;
		resp.bTokenSucc = loginOK;
		_sendCmd(fd, &resp);
		_event.closefd(fd);
	}
	else
	{
		LOG::Info("YY_JSON_LOGIN logined fd=%d, uid=%u, account:%s, logintime=%u", fd, state->uid, state->account.c_str(), state->loginTime);
		// PostCmdToThread();
		unsigned char aesKey[AES_BLOCK_SIZE * 2];
		YYSSL::YYAES::GenAESKey(aesKey, AES_BLOCK_SIZE * 2);

		std::string aesString;
		aesString.append((const char*)aesKey, sizeof(aesKey));
		state->aes.AESSetKey((unsigned char *)aesString.c_str(), aesString.length());

		YYJson_Login_res  resp;
		resp.bTokenSucc = loginOK;
		resp.aeskey = aesString;

		YYJson_Comp_RSA comp;
		if (comp.Format(&resp, state->rsa))
			_sendCmd(fd, &comp);
	}
}

bool MmsServer::CheckWhiteList(unsigned int uid, YYLibMysql * mysql)
{
	// mysql process
	if (!mysql || !mysql->_sql)
		return false;

	YYLibMysql::StmtItem item(mysql->_sql);
	// mysql->Execute("DROP TABLE test1");
	// item.Prepare("CREATE TABLE IF NOT EXISTS test1(idx int AUTO_INCREMENT, id int, label varchar(250), v1 TINYINT, v2 SMALLINT, v3 BIGINT, v4 FLOAT, v5 DOUBLE, PRIMARY KEY(idx) );") << Execute();

	// item.Prepare("INSERT INTO test1(id, label, v1, v2, v3, v4, v5) VALUES (?,?,?,?,?,?,?)") << 10 << buf->buf() << (signed char)3 << (short int)5 << (long long)100 << (float)4.5 << (double)55.5 << Execute();
	// item << 20 << buf->buf() << (signed char)10 << (short int)500 << (long long)100000 << (float)4.455 << (double)55.534543 << Execute();

	char sz[40]; sprintf(sz, "%u", uid);
	std::string sql = "SELECT * FROM mms_whitelist WHERE mms_whitelist_uid="; sql += sz;
	item.Prepare(sql.c_str()) << Execute();
	std::auto_ptr<YYLibMysql::StmtRes> res(item.GetRes());
	if (res->BindResult())
	{
		int column_count = mysql_num_fields(res->_res);
		int row = 0;
		while (!mysql_stmt_fetch(res->_stmt))
		{
			long value;
			char * buf;
			row++;
			for (int i = 0; i < column_count; i++)
			{
				switch(res->_bind[i].buffer_type)
				{
				case MYSQL_TYPE_LONG:
					{
						value = *(long*)res->_bind[i].buffer;
					}
					break;
				case MYSQL_TYPE_STRING:
				case MYSQL_TYPE_VARCHAR:
				case MYSQL_TYPE_VAR_STRING:
					{
						buf = (char*)res->_bind[i].buffer;
					}
					break;
				}
			}
		}
	}

	return true;
}

// 客户端断开，如果没有收到Logout的，模拟一个Logout命令
void MmsServer::fd_close(int fd)
{
	Item_data_ptr state = get_state(fd);
	if (!state)
		return;

	YYJson_Logout cmd;
	cmd.yyID = state->yyID;

	std::string str;
	if (YYmmsJson::FormatCmd(&cmd, str))
	{
		fastbuffer * writebuffer = new fastbuffer;
		if (writebuffer)
		{
			writebuffer->append(str.c_str(), str.length());
			data_receive(fd, writebuffer, g_CFG.port);
		}
	}
}
