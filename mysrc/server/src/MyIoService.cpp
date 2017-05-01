#include "MyIoService.h"

MyIoService::MyIoService()
{

}

boost::asio::io_service & MyIoService::getIoService()
{
	return m_ioservice;
}

MyMemoryPool & MyIoService::getMemoryPool()
{
	return m_memoryPool;
}
