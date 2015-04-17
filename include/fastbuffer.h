#ifndef FASTBUFFER
#define FASTBUFFER

class fastbuffer
{
public:
	fastbuffer(fastbuffer * ot)
	{
		_buf = NULL;
		_bufsize = 0;
		_size = 0;

		append(ot->buf(), ot->size());
		fd = ot->fd;
	}
	fastbuffer()
	{
		_buf = NULL;
		_bufsize = 0;
		_size = 0;
		fd = 0;
	}
	~fastbuffer()
	{
		if (_buf)
			free(_buf);
		_buf = NULL;
		_bufsize = 0;
		_size = 0;
		fd = 0;
	}
	
public:
	size_t read(char * buf, size_t bz)
	{
		size_t dz = (bz > _size) ? _size: bz;
		if (buf)
			memcpy(buf, _buf, dz);
		
		memmove(_buf, _buf + dz, _size - dz);
		_size -= dz;
		return dz;
	}
	size_t push_back(const char s)
	{
		return append(&s, sizeof(char));
	}
	size_t append(const char * buf, size_t bz)
	{
		if (_bufsize - _size < bz)
		{
			size_t sz = (_bufsize + bz) * 2;
			_buf = (char*)realloc(_buf, sz);
			if (!_buf)
				return 0;
			_bufsize = sz;
		}
		memcpy(_buf + _size, buf, bz);
		_size += bz;
		return bz;
	}
	size_t size()
	{
		return _size;
	}
	const char * buf()
	{
		return _buf;
	}
	void clear()
	{
		_size = 0;
	}

public:
	int	fd;
private:
	size_t _size;
	size_t _bufsize;
	char * _buf;
};

#endif // FASTBUFFER
