#ifndef PSL_MY_WEB_SOCKET_H
#define PSL_MY_WEB_SOCKET_H

#include "MySocket.h"

class http_message
{
public:
	bool parse(const char * pchar, uint32_t endpos);
	const char * get_first() const;
	const char * get_second() const;
	const char * get_three() const;
	const char * get_http_head(const char * key) const;
protected:
	bool parse_firstline(const char * pchar, uint32_t & lineLength);
	bool parse_head(const char * pchar, uint32_t & headLength);
	void trim(const char * pchar, uint32_t begPos, uint32_t endPos, std::string & outStr);
private:
	std::string m_first;
	std::string m_second;
	std::string m_three;
	std::list<std::pair<std::string, std::string>> m_http_heads;
};

class MyWebSocket : public MySocket
{
public:
	MyWebSocket(MyIoService * pservice);
	MyWebSocket(const MyWebSocket &) = delete;
	MyWebSocket & operator = (const MyWebSocket &) = delete;
	virtual ~MyWebSocket() override;
	virtual void sendMsg(MyMessage & msg) override;
	virtual void close() override final;
	void sendPing();
	void sendPong();
protected:
	virtual void httpFinishCallback() = 0;
	virtual void pingCallback() = 0;
	virtual void pongCallback() = 0;
	virtual uint32_t procData(uint8_t * pdata, uint32_t size, bool & procFinish) override final;
	virtual void login() override final;
	virtual void procMsg(uint8_t * pdata, uint32_t size, uint8_t type, bool & procFinish) = 0;
private:
	bool m_isClient;
	bool m_isHandshaked;
	uint8_t m_frames_opcode;
	unsigned int m_frames_size;
	std::vector<MessageData> m_frames;
	MessageData m_finishFrame;
};

#endif // !PSL_MY_WEB_SOCKET_H


