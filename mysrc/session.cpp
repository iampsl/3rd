#include <assert.h>
#include "session.h"
#include "log_manager.h"

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

session::session(boost::asio::io_service & io_service):m_io_service(io_service), m_socket(io_service)
{
	m_read_data_size = 0;
	m_close_socket = false;
	m_bsending = false;
}

session::~session()
{
	
}

void session::start_read()
{
	std::shared_ptr<session> psession = shared_from_this();
	m_socket.async_read_some(boost::asio::buffer(m_read_data_buffer + m_read_data_size, sizeof(m_read_data_buffer) - m_read_data_size), std::bind(&session::on_read, this, std::move(psession), std::placeholders::_1, std::placeholders::_2));
}

void session::start_connect(boost::asio::ip::tcp::endpoint local_endpoint, boost::asio::ip::tcp::endpoint remote_endpoint)
{
	m_socket.open(local_endpoint.protocol());
	m_socket.bind(local_endpoint);
	std::shared_ptr<session> psession = shared_from_this();
	m_socket.async_connect(remote_endpoint, std::bind(&session::on_connect, this, std::move(psession), std::placeholders::_1));
}

boost::asio::ip::tcp::socket & session::get_socket()
{
	return m_socket;
}

boost::asio::io_service & session::get_io_service()
{
	return m_io_service;
}

void session::send_message(buffer_t * pdata)
{
	if (nullptr == pdata)
	{
		return;
	}
	if (0 == pdata->get_data_size())
	{
		delete pdata;
		return;
	}
	if (m_close_socket)
	{
		delete pdata;
		return;
	}
	if (m_bsending)
	{
		m_buffers_deque.push_back(pdata);
	}
	else
	{
		m_bsending = true;
		std::shared_ptr<session> psession = shared_from_this();
		boost::asio::async_write(m_socket, boost::asio::buffer(pdata->get_buffer(), pdata->get_data_size()), std::bind(&session::on_write, this, std::move(psession), pdata, std::placeholders::_1, std::placeholders::_2));
	}
}

void session::close()
{
	if (!m_close_socket)
	{
		m_close_socket = true;
		boost::system::error_code ec;
		m_socket.close(ec);
		for (auto i : m_buffers_deque)
		{
			delete i;
		}
		m_buffers_deque.clear();
		close_session_callback();
	}
}

size_t session::get_read_data_buffer_size()
{
	return sizeof(m_read_data_buffer);
}

void session::proc_data(std::shared_ptr<session> psession)
{
	if (m_close_socket)
	{
		return;
	}
	bool procFinish = false;
	uint32_t process_size;
	while (true)
	{
		process_size = proc_data(m_read_data_buffer + m_process_data_size, m_read_data_size - m_process_data_size, procFinish);
		m_process_data_size += process_size;
		if (0 == process_size || !procFinish)
		{
			break;
		}
	}
	if (0 == process_size)
	{
		if (0 != m_process_data_size)
		{
			memcpy(m_read_data_buffer, m_read_data_buffer + m_process_data_size, m_read_data_size - m_process_data_size);
			m_read_data_size -=  m_process_data_size;
		}
		size_t empty_size = sizeof(m_read_data_buffer) - m_read_data_size;
		if (empty_size != 0)
		{
			if (!m_close_socket)
			{
				m_socket.async_read_some(boost::asio::buffer(m_read_data_buffer + m_read_data_size, empty_size), std::bind(&session::on_read, this, std::move(psession), std::placeholders::_1, std::placeholders::_2));
			}
		}
		else
		{
			close();
		}
	}
}

void session::on_read(std::shared_ptr<session> psession, const boost::system::error_code& error, std::size_t bytes_transferred)
{
	if (error)
	{
		close();
		return;
	}
	m_read_data_size += (uint32_t)bytes_transferred;
	m_process_data_size = 0;
	proc_data(std::move(psession));
}

void session::on_connect(std::shared_ptr<session> psession, const boost::system::error_code & error)
{
	if (error)
	{
		close();
		return;
	}
	if (m_close_socket)
	{
		return;
	}
	login();
	m_socket.async_read_some(boost::asio::buffer(m_read_data_buffer + m_read_data_size, sizeof(m_read_data_buffer) - m_read_data_size), std::bind(&session::on_read, this, std::move(psession), std::placeholders::_1, std::placeholders::_2));
}

void session::on_write(std::shared_ptr<session> psession, buffer_t * pdata, const boost::system::error_code& error, std::size_t bytes_transferred)
{
	delete pdata;
	if (error)
	{
		close();
		return;
	}
	if (m_close_socket)
	{
		return;
	}
	if (m_buffers_deque.empty())
	{
		m_bsending = false;
	}
	else
	{
		buffer_t * psendData = m_buffers_deque.front();
		m_buffers_deque.pop_front();
		boost::asio::async_write(m_socket, boost::asio::buffer(psendData->get_buffer(), psendData->get_data_size()), std::bind(&session::on_write, this, std::move(psession), psendData, std::placeholders::_1, std::placeholders::_2));
	}
}
