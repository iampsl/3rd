#ifndef PSL_MY_IO_SERVICE_H
#define PSL_MY_IO_SERVICE_H

#include <boost/asio.hpp>
#include "MyMemoryPool.h"

void get_local_remote(boost::asio::ip::tcp::socket & socket, std::string & local, std::string & remote);

class MyIoService
{
public:
	MyIoService();
	MyIoService(const MyIoService &) = delete;
	MyIoService & operator = (const MyIoService &) = delete;
	void init(int index);
	boost::asio::io_service & getIoService();
	MyMemoryPool & getMemoryPool();
	int getIndex();
private:
	boost::asio::io_service m_ioservice;
	MyMemoryPool m_memoryPool;
	int m_index;
};

#endif // !PSL_MY_IO_SERVICE_H

