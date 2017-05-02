#include <assert.h>
#include "MyIoService.h"

void get_local_remote(boost::asio::ip::tcp::socket & socket, std::string & local, std::string & remote)
{
	local.clear();
	remote.clear();
	boost::system::error_code local_ec;
	auto local_endpoint = socket.local_endpoint(local_ec);
	if (!local_ec)
	{
		local += local_endpoint.address().to_string(local_ec) + ":" + std::to_string(local_endpoint.port());
	}
	boost::system::error_code remote_ec;
	auto remote_endpoint = socket.remote_endpoint(remote_ec);
	if (!remote_ec)
	{
		remote += remote_endpoint.address().to_string(remote_ec) + ":" + std::to_string(remote_endpoint.port());
	}
}

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
