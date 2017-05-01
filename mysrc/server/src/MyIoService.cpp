#include <assert.h>
#include "MyIoService.h"

MyIoService::MyIoService()
{
	m_index = -1;
}

void MyIoService::init(int index)
{
	assert(-1 == m_index);
	m_index = index;
}

boost::asio::io_service & MyIoService::getIoService()
{
	return m_ioservice;
}

MyMemoryPool & MyIoService::getMemoryPool()
{
	return m_memoryPool;
}

int MyIoService::getIndex()
{
	return m_index;
}
