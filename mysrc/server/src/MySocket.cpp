#include <assert.h>
#include "MySocket.h"

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

MySocket::MySocket(MyIoService * pservice) :m_ioservice(pservice), m_socket(m_ioservice->getIoService())
{
	m_readedSize = 0;
	m_processedSize = 0;
	m_isClosed = false;
	m_isSending = false;
}

MySocket::~MySocket()
{
	assert(m_msgsDeque.empty());
}

void MySocket::startRead()
{
	std::shared_ptr<MySocket> psocket = shared_from_this();
	m_socket.async_read_some(boost::asio::buffer(m_readBuffer + m_readedSize, sizeof(m_readBuffer) - m_readedSize), std::bind(&MySocket::onRead, this, std::move(psocket), std::placeholders::_1, std::placeholders::_2));
}

void MySocket::startConnect(boost::asio::ip::tcp::endpoint local_endpoint, boost::asio::ip::tcp::endpoint remote_endpoint)
{
	m_socket.open(local_endpoint.protocol());
	m_socket.bind(local_endpoint);
	std::shared_ptr<MySocket> psocket = shared_from_this();
	m_socket.async_connect(remote_endpoint, std::bind(&MySocket::onConnect, this, std::move(psocket), std::placeholders::_1));
}

boost::asio::ip::tcp::socket & MySocket::getSocket()
{
	return m_socket;
}

MyIoService* MySocket::getIoService()
{
	return m_ioservice;
}

void MySocket::sendMsg(MyMessage msg)
{
	if (nullptr == msg.pdata)
	{
		return;
	}
	if (0 == msg.size)
	{
		if (0 != msg.retSize)
		{
			m_ioservice->getMemoryPool().free(msg.pdata, msg.retSize);
		}
		return;
	}
	if (m_isClosed)
	{
		m_ioservice->getMemoryPool().free(msg.pdata, msg.retSize);
		return;
	}
	if (m_isSending)
	{
		m_msgsDeque.push_back(msg);
	}
	else
	{
		assert(m_msgsDeque.empty());
		m_isSending = true;
		std::shared_ptr<MySocket> psocket = shared_from_this();
		boost::asio::async_write(m_socket, boost::asio::buffer(msg.pdata, msg.size), std::bind(&MySocket::onWrite, this, std::move(psocket), msg, std::placeholders::_1, std::placeholders::_2));
	}
}

void MySocket::close()
{
	if (!m_isClosed)
	{
		m_isClosed = true;
		boost::system::error_code ec;
		m_socket.close(ec);
		for (auto & i : m_msgsDeque)
		{
			m_ioservice->getMemoryPool().free(i.pdata, i.retSize);
		}
		m_msgsDeque.clear();
		closeCallback();
	}
}

size_t MySocket::getReadBufferSize()
{
	return sizeof(m_readBuffer);
}

void MySocket::procData(std::shared_ptr<MySocket> pself)
{
	if (m_isClosed)
	{
		return;
	}
	bool procFinish = false;
	uint32_t processedSize = 0;
	while (true)
	{
		processedSize = procData(m_readBuffer + m_processedSize, m_readedSize - m_processedSize, procFinish);
		m_processedSize += processedSize;
		if (0 == processedSize)
		{
			assert(false == procFinish);
			break;
		}
		if (false == procFinish)
		{
			break;
		}
	}
	if (0 == processedSize)
	{
		if (m_processedSize != m_readedSize)
		{
			memcpy(m_readBuffer, m_readBuffer + m_processedSize, m_readedSize - m_processedSize);
			m_readedSize -= m_processedSize;
		}
		else
		{
			m_readedSize = 0;
		}
		m_processedSize = 0;
		size_t empty_size = sizeof(m_readBuffer) - m_readedSize;
		if (empty_size != 0)
		{
			m_socket.async_read_some(boost::asio::buffer(m_readBuffer + m_readedSize, empty_size), std::bind(&MySocket::onRead, this, std::move(pself), std::placeholders::_1, std::placeholders::_2));
		}
		else
		{
			close();
		}
	}
}

void MySocket::onRead(std::shared_ptr<MySocket> pself, const boost::system::error_code& error, std::size_t bytesTransferred)
{
	if (error)
	{
		close();
		return;
	}
	m_readedSize += (uint32_t)bytesTransferred;
	procData(std::move(pself));
}

void MySocket::onConnect(std::shared_ptr<MySocket> pself, const boost::system::error_code & error)
{
	if (error)
	{
		close();
		return;
	}
	if (m_isClosed)
	{
		return;
	}
	login();
	m_socket.async_read_some(boost::asio::buffer(m_readBuffer + m_readedSize, sizeof(m_readBuffer) - m_readedSize), std::bind(&MySocket::onRead, this, std::move(pself), std::placeholders::_1, std::placeholders::_2));
}

void MySocket::onWrite(std::shared_ptr<MySocket> pself, MyMessage msg, const boost::system::error_code& error, std::size_t bytesTransferred)
{
	m_ioservice->getMemoryPool().free(msg.pdata, msg.retSize);
	if (error)
	{
		close();
		return;
	}
	assert(msg.size == bytesTransferred);
	if (m_isClosed)
	{
		return;
	}
	if (m_msgsDeque.empty())
	{
		m_isSending = false;
	}
	else
	{
		MyMessage msg = m_msgsDeque.front();
		m_msgsDeque.pop_front();
		boost::asio::async_write(m_socket, boost::asio::buffer(msg.pdata, msg.size), std::bind(&MySocket::onWrite, this, std::move(pself), msg, std::placeholders::_1, std::placeholders::_2));
	}
}