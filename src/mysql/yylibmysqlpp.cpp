#include "stdafx.h"
#include "yylibmysqlpp.h"
#include "cppconn/prepared_statement.h"
#include "cppconn/driver.h"

sql::Driver *					YYLibMysqlPP::_driver;
std::auto_ptr<sql::Connection>	YYLibMysqlPP::_con;

YYLibMysqlPP::YYLibMysqlPP()
{
	if (_driver)
		_driver->threadInit();
}

YYLibMysqlPP::~YYLibMysqlPP()
{
	if (_driver)
		_driver->threadEnd();
}

bool YYLibMysqlPP::Connect(char * host, char * user, char * pass)
{
	SQL_TRY
	{
		if (!_driver)
		{
			_driver = sql::mysql::get_driver_instance();
		}
		if (_driver)
			_con.reset(_driver->connect(host, user, pass));
	}
	SQL_END

	return (_con.get() != NULL);
}

bool YYLibMysqlPP::Use(char * db)
{
	if (!db || _con.get() == NULL) return false;

	SQL_TRY
	{
		_con->setSchema(db);
	}
	SQL_END	
	return true;
}

bool YYLibMysqlPP::Execute(char * cmd)
{
	if (!cmd || _con.get() == NULL) return false;
	std::auto_ptr<sql::Statement> stmt(_con->createStatement());
	return stmt->execute(cmd);
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
YYLibMysqlPP::ExcItem & YYLibMysqlPP::Prepare(char * cmd)
{
	_excItem.Clear();
	if (!cmd || _con.get() == NULL) return _excItem;
	_excItem._stmt.reset(_con->prepareStatement(cmd));
	return _excItem;
}
YYLibMysqlPP::ExcItem & YYLibMysqlPP::ExcItem::operator<<(const char * s)
{
	if (_stmt.get())
		_stmt->setString(++valIndex, s);
	return *this;
}
YYLibMysqlPP::ExcItem & YYLibMysqlPP::ExcItem::operator<<(const sql::SQLString& value)
{
	if (_stmt.get())
		_stmt->setString(++valIndex, value);
	return *this;
}
YYLibMysqlPP::ExcItem & YYLibMysqlPP::ExcItem::operator<<(bool bt)
{
	if (_stmt.get())
		_stmt->setBoolean(++valIndex, bt);
	return *this;
}
YYLibMysqlPP::ExcItem & YYLibMysqlPP::ExcItem::operator<<(std::istream * blob)
{
	if (_stmt.get())
		_stmt->setBlob(++valIndex, blob);
	return *this;
}
YYLibMysqlPP::ExcItem & YYLibMysqlPP::ExcItem::operator<<(double value)
{
	if (_stmt.get())
		_stmt->setDouble(++valIndex, value);
	return *this;
}
YYLibMysqlPP::ExcItem & YYLibMysqlPP::ExcItem::operator<<(int32_t value)
{
	if (_stmt.get())
		_stmt->setInt(++valIndex, value);
	return *this;
}
YYLibMysqlPP::ExcItem & YYLibMysqlPP::ExcItem::operator<<(uint32_t value)
{
	if (_stmt.get())
		_stmt->setUInt(++valIndex, value);
	return *this;
}
YYLibMysqlPP::ExcItem & YYLibMysqlPP::ExcItem::operator<<(int64_t value)
{
	if (_stmt.get())
		_stmt->setInt64(++valIndex, value);
	return *this;
}
YYLibMysqlPP::ExcItem & YYLibMysqlPP::ExcItem::operator<<(uint64_t value)
{
	if (_stmt.get())
		_stmt->setUInt64(++valIndex, value);
	return *this;
}
YYLibMysqlPP::ExcItem & YYLibMysqlPP::ExcItem::operator<<(struct tm & value)
{
	if (_stmt.get())
	{
		
	}
	return *this;
}

void YYLibMysqlPP::ExcItem::operator<<(Exc_Enum eu)
{
	if (!_stmt.get()) return;
	switch (eu)
	{
	case Exc_Enum_Query:
		{
			_set = _stmt->executeQuery();
		}
		break;
	case Exc_Enum_Update:
		_stmt->executeUpdate();
		break;
	case Exc_Enum_Execute:
		_stmt->execute();
		break;
	}
}

YYLibMysqlPP::ExcItem::ExcItem()
{
	Clear();
}

void YYLibMysqlPP::ExcItem::Clear()
{
	valIndex = 0;
	_stmt.reset();
	_set = NULL;
}

YYLibMysqlPP::ExcItem::Exc_Enum Execute()
{
	return YYLibMysqlPP::ExcItem::Exc_Enum_Execute;
}
YYLibMysqlPP::ExcItem::Exc_Enum ExecuteQuery()
{
	return YYLibMysqlPP::ExcItem::Exc_Enum_Query;
}
YYLibMysqlPP::ExcItem::Exc_Enum ExecuteUpdate()
{
	return YYLibMysqlPP::ExcItem::Exc_Enum_Update;
}
