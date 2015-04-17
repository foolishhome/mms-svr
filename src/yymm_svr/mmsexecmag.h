#ifndef YY_MMSEXECMANAGER
#define YY_MMSEXECMANAGER

#ifndef CMD_PROC_H
#include "cmdproc.h"
#endif

class YYLibEvent;
class MmsServer;
class fastbuffer;
class MmsExecManager
{
public:
	bool InitMmsExec(YYLibEvent * event, MmsServer * parent);
	bool ShellExec(const Item_data_ptr state);

	void ProcessMmsExecData(int fd, fastbuffer * readbuffer);
	void openURL(int fd, const Item_data_ptr state);
	void ShellExit(const Item_data_ptr state);

protected:
	MmsServer * _Parent;
};

#endif	// YY_MMSEXECMANAGER
