#include <stddef.h>
#include <assert.h>
#include <string.h>
#include "buffer_t.h"


buffer_t::buffer_t(size_t buffer_size)
{
	pnew_buffer_ = NULL;
	data_size_ = 0;
	buffer_size_ = 0;
	if (buffer_size <= sizeof(pbuffer_))
	{
		buffer_size_ = buffer_size;
	}
	else
	{
		pnew_buffer_ = new uint8_t[buffer_size];
		buffer_size_ = buffer_size;
	}
}

buffer_t::buffer_t(const buffer_t & other)
{
	pnew_buffer_ = NULL;
	data_size_ = 0;
	buffer_size_ = 0;
	if (other.pnew_buffer_)
	{
		pnew_buffer_ = new uint8_t[other.buffer_size_];
		buffer_size_ = other.buffer_size_;
		memcpy(pnew_buffer_, other.pnew_buffer_, other.data_size_);
		data_size_ = other.data_size_;
	}
	else
	{
		buffer_size_ = other.buffer_size_;
		data_size_ = other.data_size_;
		memcpy(pbuffer_, other.pbuffer_, data_size_);
	}
}

buffer_t::buffer_t(buffer_t && other)
{
	data_size_ = other.data_size_;
	buffer_size_ = other.buffer_size_;
	other.buffer_size_ = 0;
	other.data_size_ = 0;
	if (other.pnew_buffer_)
	{
		pnew_buffer_ = other.pnew_buffer_;
		other.pnew_buffer_ = NULL;
	}
	else
	{
		pnew_buffer_ = NULL;
		memcpy(pbuffer_, other.pbuffer_, data_size_);
	}
}

buffer_t & buffer_t::operator = (const buffer_t & other)
{
	if (other.pnew_buffer_)
	{
		uint8_t * ptemp = new uint8_t[other.buffer_size_];
		memcpy(ptemp, other.pnew_buffer_, other.data_size_);
		if (pnew_buffer_)
		{
			delete[]pnew_buffer_;
		}
		pnew_buffer_ = ptemp;
		data_size_ = other.data_size_;
		buffer_size_ = other.buffer_size_;
	}
	else
	{
		if (pnew_buffer_)
		{
			delete[] pnew_buffer_;
			pnew_buffer_ = NULL;
		}
		memcpy(pbuffer_, other.pbuffer_, other.data_size_);
		data_size_ = other.data_size_;
		buffer_size_ = other.buffer_size_;
	}
	return *this;
}

buffer_t & buffer_t::operator = (buffer_t && other)
{
	if (other.pnew_buffer_)
	{
		if (pnew_buffer_)
		{
			delete[]pnew_buffer_;
		}
		pnew_buffer_ = other.pnew_buffer_;
		data_size_ = other.data_size_;
		buffer_size_ = other.buffer_size_;
		other.pnew_buffer_ = NULL;
		other.data_size_ = 0;
		other.buffer_size_ = 0;
	}
	else
	{
		if (pnew_buffer_)
		{
			delete[]pnew_buffer_;
			pnew_buffer_ = NULL;
		}
		data_size_ = other.data_size_;
		buffer_size_ = other.buffer_size_;
		memcpy(pbuffer_, other.pbuffer_, data_size_);
	}
	return *this;
}

buffer_t::~buffer_t()
{
	if (NULL != pnew_buffer_)
	{
		delete[]pnew_buffer_;
	}
}

uint8_t * buffer_t::get_buffer()
{
	if (pnew_buffer_)
	{
		return pnew_buffer_;
	}
	return pbuffer_;
}

size_t buffer_t::get_data_size()
{
	return data_size_;
}

size_t buffer_t::get_buffer_size()
{
	return buffer_size_;
}

void buffer_t::copy_data_to_buffer(const void * pdata, size_t data_size)
{
	assert(pdata);
	assert(data_size <= buffer_size_);
	if (pnew_buffer_)
	{
		memcpy(pnew_buffer_, pdata, data_size);
	}
	else
	{
		memcpy(pbuffer_, pdata, data_size);
	}
	data_size_ = data_size;
}

void buffer_t::set_data_size(size_t size)
{
	assert(size <= buffer_size_);
	data_size_ = size;
}
