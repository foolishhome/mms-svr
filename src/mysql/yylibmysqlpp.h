#ifndef YYMMS_LIBMYSQLPP
#define YYMMS_LIBMYSQLPP

#include "mysql_driver.h"
#include "mysql_connection.h"
#include "cppconn/exception.h"
#include "cppconn/config.h"
#include "cppconn/resultset.h"

#define SQL_TRY		try
#define SQL_END		catch (sql::SQLException &e){	\
	LOG::Error("# ERR: SQLException in %s on line %d # ERR:%s (MySQL error code: %d, SQLState: %s )", __FILE__, __LINE__, e.what(), e.getErrorCode(), e.getSQLStateCStr());	\
}catch (std::runtime_error &e)	\
{LOG::Error("#ERR: SQL runtime_err: %s", e.what());}

class YYLibMysqlPP
{
public:
	struct ExcItem
	{
		enum Exc_Enum
		{
			Exc_Enum_Query = 1,
			Exc_Enum_Update = 2,
			Exc_Enum_Execute = 3
		};
		ExcItem();

		int valIndex;
		std::auto_ptr<sql::PreparedStatement> _stmt;
		sql::ResultSet * _set;

		ExcItem & operator<<(const char * s);
		ExcItem & operator<<(bool bt);
		ExcItem & operator<<(const sql::SQLString& value);
		ExcItem & operator<<(std::istream * blob);
		ExcItem & operator<<(double value);
		ExcItem & operator<<(int32_t value);
		ExcItem & operator<<(uint32_t value);
		ExcItem & operator<<(int64_t value);
		ExcItem & operator<<(uint64_t value);
		ExcItem & operator<<(struct tm & value);

		//virtual void setDateTime(unsigned int parameterIndex, const sql::SQLString& value) = 0;
		//virtual void setNull(unsigned int parameterIndex, int sqlType) = 0;

		void operator<<(Exc_Enum);

		void Clear();
	};
	YYLibMysqlPP();
	virtual ~YYLibMysqlPP();

public:
	static bool Connect(char * host, char * user, char * pass);

	bool Use(char * db);
	bool Execute(char * cmd);
	ExcItem & Prepare(char * cmd);

public:
	static sql::Driver *					_driver;
	static std::auto_ptr<sql::Connection>	_con;

	ExcItem _excItem;
};

YYLibMysqlPP::ExcItem::Exc_Enum Execute();
YYLibMysqlPP::ExcItem::Exc_Enum ExecuteUpdate();
YYLibMysqlPP::ExcItem::Exc_Enum ExecuteQuery();

#endif // YYMMS_LIBMYSQLPP
