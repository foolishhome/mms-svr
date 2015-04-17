#ifndef YYMMS_LIBMYSQL
#define YYMMS_LIBMYSQL

#include <mysql.h>
#include <istream>
#include "boost/shared_ptr.hpp"

#ifndef MEM_STUFF
#include "memstuff.h"
#endif

#define SQL_ERR(sq)	LOG::Error("# SQL_ERR in %s on line %d # ERR:%s (MySQL error code: %d )", __FILE__, __LINE__, mysql_error(sq), mysql_errno(sq))
#define SQL_STMT_ERR(st)	LOG::Error("# SQL_STMT_ERR in %s on line %d # ERR:%s (MySQL_STMT error code: %d )", __FILE__, __LINE__, mysql_stmt_error(st), mysql_stmt_errno(st))
#define SQL_VERIFY(sql, e)	do{if(e) SQL_ERR(sql);} while (0)
#define SQL_STMT_VERIFY(st, e)	do{if(e) SQL_STMT_ERR(st);} while (0)

class YYLibMysql
{
public:
	struct StmtRes
	{
		StmtRes(MYSQL_STMT * stmt);
		~StmtRes();
		bool BindResult();

		MYSQL_RES	*	_res;
		MYSQL_STMT	*	_stmt;
		MYSQL_BIND	*	_bind;

	protected:
		memstuff<256>	_stuff;
	};
	struct StmtItem
	{
		enum Stmt_Enum
		{
			Exc_Enum_Execute = 1
		};
		StmtItem(MYSQL * sq);
		~StmtItem();

		MYSQL_STMT	*	_stmt;
		MYSQL_BIND	*	_bind;	

		my_ulonglong	affected_rows;

		StmtItem & operator<<(const char * s);
		StmtItem & operator<<(std::istream * blob);
		StmtItem & operator<<(signed char value);
		StmtItem & operator<<(short value);
		StmtItem & operator<<(int value);
		StmtItem & operator<<(long long value);
		StmtItem & operator<<(float value);
		StmtItem & operator<<(double value);
		StmtItem & operator<<(struct tm & value);

		//virtual void setDateTime(unsigned int parameterIndex, const sql::SQLString& value) = 0;
		//virtual void setNull(unsigned int parameterIndex, int sqlType) = 0;

		void operator<<(Stmt_Enum);

		StmtItem & Prepare(const char * cmd);
		StmtRes * GetRes();

	protected:
		unsigned int valIndex;
		unsigned int valTotal;
		void Clear();
		memstuff<256>	_stuff;
	};
	YYLibMysql();
	virtual ~YYLibMysql();

public:
	bool Connect(const char * host, unsigned int port, const char * user, const char * pass);

	void Use(const char * db);
	bool Execute(const char * cmd);

public:
	MYSQL	*		_sql;
};

YYLibMysql::StmtItem::Stmt_Enum Execute();

#endif // YYMMS_LIBMYSQL
