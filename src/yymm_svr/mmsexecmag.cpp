#include "stdafx.h"
#include "mmsexecmag.h"
#include "jsonprotocol.h"
#include "cmdproc.h"
#include "cfg.h"
#include "event/yylibevent.h"
#include "yyjson.h"
#include "yymms_svr.h"

bool MmsExecManager::InitMmsExec(YYLibEvent * event, MmsServer * parent)
{
	if (!event)
		return false;
	_Parent = parent;
	return event->Bind("127.0.0.1", g_CFG.mangport);
}

bool MmsExecManager::ShellExec(const Item_data_ptr state)
{
	if (!state)
		return false;

	if (state->shellClient_fd != 0)
	{	// 已经连接上一个进程
		return false;
	}

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
#ifdef _DEBUG
	path += "/debug/shellclient_d.exe";
#else
	path += "/release/shellclient.exe";
#endif

	std::string paramt = YYJson::tos(state->yyID) + "&127.0.0.1&" + YYJson::tos(g_CFG.mangport);

#ifdef WIN32
	ShellExecuteA(NULL, "open", path.c_str(), paramt.c_str(), NULL, SW_HIDE);
#else
	execl(path.c_str(), paramt.c_str(), NULL);
#endif

	return true;
}


void MmsExecManager::ProcessMmsExecData(int fd, fastbuffer * readbuffer)
{
	if (!_Parent)
		return;

	fastbuffer * buf = readbuffer;
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
				switch (jobj->_id)
				{
				case YY_JSON_SVR_INIT:
					{
						YYJson_Svr_Init * cmd = (YYJson_Svr_Init*)jobj;
						Item_data_ptr state = _Parent->_userData.get_state(cmd->yyID);
						if (!state) break;

						state->shellClient_fd = fd;

						openURL(fd, state);
					}
					break;
				}
				buf->read(NULL, offset);
				YYmmsJson::FreeCmd(jobj);
			}
			break;
		default:
			buf->clear();
			break;
		}
	} while (buf->size() > 0);

	if (readbuffer)
		delete readbuffer;
}

void MmsExecManager::openURL(int fd, const Item_data_ptr state)
{
	if (!state)
		return;

	YYJson_MMS_OpenUrl resp;
	resp.uid = state->uid;
	resp.yyID = state->yyID;
	resp.user = state->account;
	resp.tick = state->pwd;
	resp.channelID = state->channelID;
	resp.subChannelID = state->subChannelID;
	resp.url = state->url;

	_Parent->_sendCmd(fd, &resp);
}

void MmsExecManager::ShellExit(const Item_data_ptr state)
{	// 通知shellclient退出
	if (!state || state->shellClient_fd == 0)
		return;

	YYJson_MMS_Exit cmd;
	cmd.yyID = state->yyID;

	_Parent->_sendCmd(state->shellClient_fd, &cmd);
}
