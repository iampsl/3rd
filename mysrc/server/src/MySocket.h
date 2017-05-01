#ifndef PSL_MY_SOCKET_H
#define PSL_MY_SOCKET_H

#include <inttypes.h>
#include <deque>
#include <string>
#include <memory>
#include "MyMessage.h"
#include "MyIoService.h"


void get_local_remote(boost::asio::ip::tcp::socket & socket, std::string & local, std::string & remote);

class MySocket : public std::enable_shared_from_this<MySocket>
{
public:
	MySocket(MyIoService * pservice);
	virtual ~MySocket();
	MySocket(const MySocket &) = delete;
	MySocket & operator = (const MySocket &)= delete;
	void startRead();
	void startConnect(boost::asio::ip::tcp::endpoint local_endpoint, boost::asio::ip::tcp::endpoint remote_endpoint);
	boost::asio::ip::tcp::socket & getSocket();
	MyIoService* getIoService();
	void sendMsg(MyMessage msg);
	void close();
	size_t getReadBufferSize();
	void procData(std::shared_ptr<MySocket> pself);
protected:
	virtual void closeCallback() = 0;
	virtual uint32_t procData(uint8_t * pdata, uint32_t size, bool & procFinish) = 0;
	virtual void login() = 0;
private:
	void onRead(std::shared_ptr<MySocket> pself, const boost::system::error_code& error, std::size_t bytesTransferred);
	void onConnect(std::shared_ptr<MySocket> pself, const boost::system::error_code& error);
	void onWrite(std::shared_ptr<MySocket> pself, MyMessage msg, const boost::system::error_code& error, std::size_t bytesTransferred);
private:
	MyIoService * m_ioservice;
	boost::asio::ip::tcp::socket m_socket;
	uint8_t m_readBuffer[1024];
	uint32_t m_readedSize;
	uint32_t m_processedSize;
	bool m_isClosed;
	bool m_isSending;
	std::deque<MyMessage> m_msgsDeque;
};

#endif // !PSL_MY_SOCKET_H

