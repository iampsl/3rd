#include "MyMemoryPool.h"

MyMemoryPool::MyMemoryPool():m_pool64(64),m_pool128(128),m_pool256(256),m_pool512(512), m_pool1024(1024)
{

}

void * MyMemoryPool::malloc(unsigned int size, unsigned int & retSize)
{
	if (size <= 64)
	{
		retSize = 64;
		return m_pool64.malloc();
	}
	if (size <= 128)
	{
		retSize = 128;
		return m_pool128.malloc();
	}
	if (size <= 256)
	{
		retSize = 256;
		return m_pool256.malloc();
	}
	if (size <= 512)
	{
		retSize = 512;
		return m_pool512.malloc();
	}
	if (size <= 1024)
	{
		retSize = 1024;
		return m_pool1024.malloc();
	}
	retSize = size;
	return ::malloc(size);
}

void MyMemoryPool::free(void * pmem, unsigned int retSize)
{
	switch (retSize)
	{
	case 64:
		m_pool64.free(pmem);
		break;
	case 128:
		m_pool128.free(pmem);
		break;
	case 256:
		m_pool256.free(pmem);
		break;
	case 512:
		m_pool512.free(pmem);
		break;
	case 1024:
		m_pool1024.free(pmem);
	default:
		assert(retSize > 1024);
		::free(pmem);
		break;
	}
}


