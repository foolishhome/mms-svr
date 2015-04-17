#ifndef MEM_STUFF
#define MEM_STUFF

template<int SZ>
class memstuff
{
	struct data 
	{
		size_t _size;
		size_t _bufsize;
		char * _buf;
		data * _next;
	};

public:
	memstuff()
	{
		_data = NULL;
	}
	~memstuff()
	{
		if (!_data) return;
		data * buf = _data;
		data * nexbuf;
		do{
			nexbuf = buf->_next;
			free(buf->_buf);
			free(buf);
			buf = nexbuf;
		} while(buf);
	}
	
public:
	void clear()
	{
		if (!_data) return;
		data * buf = _data;
		do{
			buf->_size = 0;
			buf = buf->_next;
		} while(buf);
	}
	char * alloc(size_t bz)
	{
		data * buf = _data;

		if (!_data)
		{
			if (!appendbuf(_data, bz))
				return NULL;
			buf = _data;
		}
		else
		{
			data * prebuf;
			do
			{
				prebuf = buf;
				if (buf->_bufsize - buf->_size >= bz)
				{
					buf->_size += bz;
					return buf->_buf + buf->_size - bz;
				}
			} while ((buf = buf->_next) != NULL);

			if (!appendbuf(prebuf->_next, bz))
				return NULL;
			buf = prebuf->_next;
		}

		buf->_size += bz;
		return buf->_buf + buf->_size - bz;
	}
	bool appendbuf(data *& buf, size_t bz)
	{
		buf = (data*)malloc(sizeof(data));
		if (buf == NULL)
			return false;
		memset(buf, 0, sizeof(data));
		size_t sz = (bz < SZ)? SZ: bz * 2;
		buf->_buf = (char*)malloc(sz);
		buf->_bufsize = sz;
		return true;
	}

private:
	data * _data;
};

#endif // MEM_STUFF
