#ifndef PSL_MY_WEB_SOCKET_H
#define PSL_MY_WEB_SOCKET_H

#include <inttypes.h>
#include <deque>
#include <string>
#include <memory>
#include "MyMessage.h"
#include "MyIoService.h"

class http_request
{
public:
	bool parse(const char * pchar, uint32_t endpos);
	const char * get_http_method() const;
	const char * get_http_version() const;
	const char * get_resource_path() const;
	const char * get_http_head(const char * key) const;
protected:
	bool parse_line(const char * pchar, uint32_t & lineLength);
	bool parse_head(const char * pchar, uint32_t & headLength);
	void trim(const char * pchar, uint32_t begPos, uint32_t endPos, std::string & outStr);
private:
	std::string m_http_method;
	std::string m_http_version;
	std::string m_resource_path;
	std::list<std::pair<std::string, std::string>> m_http_heads;
};

class MyWebSocket : public std::enable_shared_from_this<MyWebSocket>
{
public:
	MyWebSocket(MyIoService * pservice);
	MyWebSocket(const MyWebSocket &) = delete;
	MyWebSocket & operator = (const MyWebSocket &) = delete;
	virtual ~MyWebSocket();
	void startRead();
	boost::asio::ip::tcp::socket & getSocket();
	MyIoService* getIoService();
	void send_message(const void * pdata, size_t size, uint8_t data_type);
	void send_ping();
	void close();
protected:
	virtual void close_session_callback() = 0;
	virtual void http_handshake_callback() = 0;
	virtual void heart_beat_callback() = 0;
	virtual void proc_msg_packet(uint8_t * pdata, uint32_t size, uint8_t type, bool & procFinish) = 0;
private:
	void send_message(buffer_t* pdata);
	uint32_t proc_data(uint8_t * pdata, uint32_t size, bool & procFinish);
	void proc_data(std::shared_ptr<ws_session> psession);
private:
	struct MessageData
	{
		void *pdata;
		unsigned int size;
		unsigned int retSize;
	};
private:
	void on_read(std::shared_ptr<ws_session> psession, const boost::system::error_code& error, std::size_t bytes_transferred);
	void on_write(std::shared_ptr<ws_session> psession, buffer_t * pdata, const boost::system::error_code& error, std::size_t bytes_transferred);
private:
	MyIoService * m_ioservice;
	boost::asio::ip::tcp::socket m_socket;
	uint8_t m_readBuffer[1024];
	uint32_t m_readedSize;
	uint32_t m_processedSize;
	bool m_isClosed;
	bool m_isSending;
	bool m_http_handshake;
	uint8_t m_frames_opcode;
	std::vector<MessageData> m_frames;
	size_t m_frames_total_size;
	std::deque<MessageData> m_msgsDeque;
};

#endif // !PSL_MY_WEB_SOCKET_H


