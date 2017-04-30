#ifndef PSL_SESSION_H_
#define PSL_SESSION_H_

#include <inttypes.h>
#include <deque>
#include <string>
#include <memory>
#include <boost/asio.hpp>
#include "buffer_t.h"


void get_local_remote(boost::asio::ip::tcp::socket & socket, std::string & local, std::string & remote);

class session : public std::enable_shared_from_this<session>
{
public:
	session(boost::asio::io_service & io_service);
	virtual ~session();
	void start_read();
	void start_connect(boost::asio::ip::tcp::endpoint local_endpoint, boost::asio::ip::tcp::endpoint remote_endpoint);
	boost::asio::ip::tcp::socket & get_socket();
	boost::asio::io_service & get_io_service();
	void send_message(buffer_t * pdata);
	void close();
	size_t get_read_data_buffer_size();
protected:
	virtual void close_session_callback() = 0;
	virtual uint32_t proc_data(uint8_t * pdata, uint32_t size, bool & procFinish) = 0;
	virtual void login() = 0;
public:
	void proc_data(std::shared_ptr<session> psession);
private:
	void on_read(std::shared_ptr<session> psession, const boost::system::error_code& error, std::size_t bytes_transferred);
	void on_connect(std::shared_ptr<session> psession, const boost::system::error_code& error);
	void on_write(std::shared_ptr<session> psession, buffer_t * pdata, const boost::system::error_code& error, std::size_t bytes_transferred);
private:
	session(const session & other) = delete;
	session & operator = (const session & other) = delete;
private:
	boost::asio::io_service & m_io_service;
	boost::asio::ip::tcp::socket m_socket;
	uint8_t m_read_data_buffer[10240];
	uint32_t m_read_data_size;
	uint32_t m_process_data_size;
	bool m_close_socket;
	bool m_bsending;
	std::deque<buffer_t*> m_buffers_deque;
};

#endif // !PSL_SESSION_H_

