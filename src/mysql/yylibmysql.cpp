#include "stdafx.h"
#include "yylibmysql.h"
#include "mysql/errmsg.h"

YYLibMysql::YYLibMysql()
{
	_sql = NULL;
	_sql = mysql_init(NULL);
}

YYLibMysql::~YYLibMysql()
{
	if (_sql)
		mysql_close(_sql);
	_sql = NULL;
}

bool YYLibMysql::Connect(const char * host, unsigned int port, const char * user, const char * pass)
{
	if (!_sql) return false;
	// my_bool reconnect = true;
	// mysql_options(_sql, MYSQL_OPT_RECONNECT, &reconnect);

	if (!mysql_real_connect(_sql, host, user, pass, NULL, port, NULL,
					  CLIENT_INTERACTIVE
					| CLIENT_MULTI_RESULTS
					| CLIENT_MULTI_STATEMENTS
					| CLIENT_REMEMBER_OPTIONS
					))
	{
		SQL_ERR(_sql);
		return false;
	}
	return true;
}

void YYLibMysql::Use(const char * db)
{
	if (!_sql) return;
	SQL_VERIFY(_sql, mysql_select_db(_sql, db));
}

bool YYLibMysql::Execute(const char * cmd)
{
	if (!_sql) return false;
	if (mysql_query(_sql, cmd))
	{
		SQL_ERR(_sql);
		return false;
	}
	return true;
}

/* The following SQL statements can be used as prepared statements:
ALTER TABLE
ANALYZE TABLE
CACHE INDEX
CALL
CHANGE MASTER
CHECKSUM {TABLE | TABLES}
COMMIT
{CREATE | DROP} DATABASE
{CREATE | RENAME | DROP} USER
CREATE INDEX
CREATE TABLE
DELETE
DO
DROP INDEX
DROP TABLE
FLUSH {TABLE | TABLES | TABLES WITH READ LOCK | HOSTS | PRIVILEGES
| LOGS | STATUS | MASTER | SLAVE | DES_KEY_FILE | USER_RESOURCES}
GRANT
INSERT
INSTALL PLUGIN
KILL
LOAD INDEX INTO CACHE
OPTIMIZE TABLE
RENAME TABLE
REPAIR TABLE
REPLACE
RESET {MASTER | SLAVE | QUERY CACHE}
REVOKE
SELECT
SET
SHOW BINLOG EVENTS
SHOW CREATE {PROCEDURE | FUNCTION | EVENT | TABLE | VIEW}
SHOW {AUTHORS | CONTRIBUTORS | WARNINGS | ERRORS}
SHOW {MASTER | BINARY} LOGS
SHOW {MASTER | SLAVE} STATUS
SLAVE {START | STOP}
TRUNCATE TABLE
UNINSTALL PLUGIN
UPDATE
*/
YYLibMysql::StmtItem & YYLibMysql::StmtItem::Prepare(const char * cmd)
{
	Clear();
	if (mysql_stmt_prepare(_stmt, cmd, strlen(cmd)))
	{
		SQL_STMT_ERR(_stmt);
		return *this;
	}
	valTotal = mysql_stmt_param_count(_stmt);
	if (valTotal > 0)
	{
		_bind = new MYSQL_BIND[valTotal];
		memset(_bind, 0, sizeof(MYSQL_BIND) * valTotal);
	}
	return *this;
}

YYLibMysql::StmtItem & YYLibMysql::StmtItem::operator<<(const char * s)
{
	if (valIndex < valTotal)
	{
		_bind[valIndex].buffer_type = MYSQL_TYPE_STRING;
		_bind[valIndex].buffer = _stuff.alloc(strlen(s) + 1);
		memcpy(_bind[valIndex].buffer, s, strlen(s) + 1);
		_bind[valIndex].buffer_length = strlen(s) + 1;
		_bind[valIndex].is_null = 0;
		_bind[valIndex].length = (unsigned long*)_stuff.alloc(sizeof(unsigned long));
		*_bind[valIndex].length = strlen(s);

		valIndex++;
	}
	return *this;
}
YYLibMysql::StmtItem & YYLibMysql::StmtItem::operator<<(signed char value)
{
	if (valIndex < valTotal)
	{
		_bind[valIndex].buffer_type = MYSQL_TYPE_TINY;
		_bind[valIndex].buffer = _stuff.alloc(sizeof(signed char));
		*(signed char*)(_bind[valIndex].buffer) = value;
		_bind[valIndex].is_null = 0;
		_bind[valIndex].length = 0;

		valIndex++;
	}
	return *this;
}
YYLibMysql::StmtItem & YYLibMysql::StmtItem::operator<<(short value)
{
	if (valIndex < valTotal)
	{
		_bind[valIndex].buffer_type = MYSQL_TYPE_SHORT;
		_bind[valIndex].buffer = _stuff.alloc(sizeof(short));
		*(short*)(_bind[valIndex].buffer) = value;
		_bind[valIndex].is_null = 0;
		_bind[valIndex].length = 0;

		valIndex++;
	}
	return *this;
}
YYLibMysql::StmtItem & YYLibMysql::StmtItem::operator<<(std::istream * blob)
{

	return *this;
}
YYLibMysql::StmtItem & YYLibMysql::StmtItem::operator<<(double value)
{
	if (valIndex < valTotal)
	{
		_bind[valIndex].buffer_type = MYSQL_TYPE_DOUBLE;
		_bind[valIndex].buffer = _stuff.alloc(sizeof(double));
		*(double*)(_bind[valIndex].buffer) = value;
		_bind[valIndex].is_null = 0;
		_bind[valIndex].length = 0;

		valIndex++;
	}
	return *this;
}
YYLibMysql::StmtItem & YYLibMysql::StmtItem::operator<<(float value)
{
	if (valIndex < valTotal)
	{
		_bind[valIndex].buffer_type = MYSQL_TYPE_FLOAT;
		_bind[valIndex].buffer = _stuff.alloc(sizeof(float));
		*(float*)(_bind[valIndex].buffer) = value;
		_bind[valIndex].is_null = 0;
		_bind[valIndex].length = 0;

		valIndex++;
	}
	return *this;
}
YYLibMysql::StmtItem & YYLibMysql::StmtItem::operator<<(int value)
{
	if (valIndex < valTotal)
	{
		_bind[valIndex].buffer_type = MYSQL_TYPE_LONG;
		_bind[valIndex].buffer = _stuff.alloc(sizeof(int));
		*(int*)(_bind[valIndex].buffer) = value;
		_bind[valIndex].is_null = 0;
		_bind[valIndex].length = 0;

		valIndex++;
	}
	return *this;
}
YYLibMysql::StmtItem & YYLibMysql::StmtItem::operator<<(long long value)
{
	if (valIndex < valTotal)
	{
		_bind[valIndex].buffer_type = MYSQL_TYPE_LONGLONG;
		_bind[valIndex].buffer = _stuff.alloc(sizeof(long long));
		*(long long*)(_bind[valIndex].buffer) = value;
		_bind[valIndex].is_null = 0;
		_bind[valIndex].length = 0;

		valIndex++;
	}
	return *this;
}
YYLibMysql::StmtItem & YYLibMysql::StmtItem::operator<<(struct tm & value)
{

	return *this;
}

void YYLibMysql::StmtItem::operator<<(Stmt_Enum eu)
{
	if (!_stmt) return;
	if (eu == Exc_Enum_Execute && valIndex == valTotal)
	{
		SQL_STMT_VERIFY(_stmt, mysql_stmt_bind_param(_stmt, _bind));
		SQL_STMT_VERIFY(_stmt, mysql_stmt_execute(_stmt));
		affected_rows = mysql_stmt_affected_rows(_stmt);
		valIndex = 0;
		_stuff.clear();
	}
}

YYLibMysql::StmtItem::StmtItem(MYSQL * sq)
{
	_bind = NULL;
	_stmt = mysql_stmt_init(sq);
	Clear();
}
YYLibMysql::StmtItem::~StmtItem()
{
	Clear();
	if (_stmt)
	{
		mysql_stmt_close(_stmt);
		_stmt = NULL;
	}
}
void YYLibMysql::StmtItem::Clear()
{
	if (_bind)
	{
		delete[] _bind;
		_bind = NULL;
	}
	valIndex = valTotal = 0;
	_stuff.clear();
}
YYLibMysql::StmtItem::Stmt_Enum Execute()
{
	return YYLibMysql::StmtItem::Exc_Enum_Execute;
}
YYLibMysql::StmtRes * YYLibMysql::StmtItem::GetRes()
{
	return new StmtRes(_stmt);
}
YYLibMysql::StmtRes::StmtRes(MYSQL_STMT * stmt) : _bind(NULL), _stmt(stmt)
{
	_res = mysql_stmt_result_metadata(stmt);
	do 
	{
		if (!_res)
			break;
		int column_count = mysql_num_fields(_res);
		_bind = new MYSQL_BIND[column_count];
		if (!_bind)
			break;
		memset(_bind, 0, sizeof(MYSQL_BIND) * column_count);

		for (int i = 0; i < column_count; i++)
		{
			_bind[i].buffer_type = _res->fields[i].type;
			_bind[i].buffer_length = _res->fields[i].length;
			_bind[i].buffer = _stuff.alloc(_res->fields[i].length + 1);
			_bind[i].is_null = _stuff.alloc(sizeof(my_bool));
			_bind[i].error = _stuff.alloc(sizeof(my_bool));
		}
	} while (0);
}

bool YYLibMysql::StmtRes::BindResult()
{
	if (!_res || !_bind || !_stmt)
		return false;
	if (mysql_stmt_bind_result(_stmt, _bind))
	{
		SQL_STMT_ERR(_stmt);
		return false;
	}
	if (mysql_stmt_store_result(_stmt))
	{
		SQL_STMT_ERR(_stmt);
		return false;
	}
	return true;
}

YYLibMysql::StmtRes::~StmtRes()
{
	_stmt = NULL;
	if (_bind)
		delete[] _bind;
	if (_res)
		mysql_free_result(_res);
}

