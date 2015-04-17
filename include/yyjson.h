#ifndef YY_JSON
#define YY_JSON

#include "json/json.h"
#include <string>

class YYJson
{
public:
	YYJson()
	{
		_tok = json_tokener_new();
	}
	virtual ~YYJson()
	{
		if (_tok)
			json_tokener_free(_tok);
	}
public:
	static std::string tos(int i)
	{
		char sz[40];
		sprintf(sz, "%d", i);
		return sz;
	}
	static std::string tos(unsigned int i)
	{
		char sz[40];
		sprintf(sz, "%u", i);
		return sz;
	}
	static std::string tos(bool b)
	{
		return b? "true": "false";
	}
	static std::string toCmd(const char * cmd, std::string & val)
	{
		return std::string("\"") + cmd + "\":\"" + val + "\"";
	}
	static std::string toCmd(const char * cmd, int i)
	{
		return std::string("\"") + cmd + "\":" + tos(i);
	}
	static std::string toCmd(const char * cmd, unsigned int i)
	{
		return std::string("\"") + cmd + "\":" + tos(i);
	}
	static std::string toCmd(const char * cmd, bool b)
	{
		return std::string("\"") + cmd + "\":" + tos(b);
	}
public:
	void clear()
	{
		if (_tok)
			json_tokener_reset(_tok);
	}
	json_tokener_error Parse(const char * js)
	{
		clear();
		_jobj = json_tokener_parse_ex(_tok, js, -1);
		if (_tok->err != json_tokener_success)
		{
			if (_jobj != NULL)
				json_object_put(_jobj);
			_jobj = NULL;
		}
		return _tok->err;
	}

	struct json_object*		_jobj;

private:
	struct json_tokener*	_tok;
};

#endif // YY_JSON
