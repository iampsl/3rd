#ifndef PSL_MY_IO_SERVICE_H
#define PSL_MY_IO_SERVICE_H

#include <boost/asio.hpp>
#include "MyMemoryPool.h"

class MyIoService
{
public:
	MyIoService();
	MyIoService(const MyIoService &) = delete;
	MyIoService & operator = (const MyIoService &) = delete;
	boost::asio::io_service & getIoService();
	MyMemoryPool & getMemoryPool();
private:
	boost::asio::io_service m_ioservice;
	MyMemoryPool m_memoryPool;
};

#endif // !PSL_MY_IO_SERVICE_H

