#include <assert.h>
#include <openssl/sha.h>
#include "MyWebSocket.h"

MyWebSocket::MyWebSocket(MyIoService * pservice) : MySocket(pservice)
{
	m_isClient = false;
	m_isHandshaked = false;
}

MyWebSocket::~MyWebSocket()
{

}


void MyWebSocket::sendPing()
{
	uint8_t opcode = 137;
	uint8_t payload = 0;
	buffer_t * pmsg = new buffer_t(2);
	uint8_t * pbuffer = pmsg->get_buffer();
	pbuffer[0] = opcode;
	pbuffer[1] = payload;
	pmsg->set_data_size(2);
	send_message(pmsg);
}

bool is_bigend()
{
	uint32_t val = 1;
	uint8_t * p = (uint8_t*)(&val);
	if (*p == 1)
	{
		return false;
	}
	else
	{
		return true;
	}
}

uint64_t htonll(uint64_t val)
{
	if (is_bigend())
	{
		return val;
	}
	else
	{
		return ((uint64_t)htonl((int)(val >> 32 & 0xffffffff))) | (((uint64_t)htonl((int)(val & 0xffffffff))) << 32);
	}
}

uint64_t ntohll(uint64_t val)
{
	if (is_bigend())
	{
		return val;
	}
	else
	{
		return ((uint64_t)ntohl((int)(val >> 32 & 0xffffffff))) | (((uint64_t)ntohl((int)(val & 0xffffffff))) << 32);
	}
}

void MyWebSocket::sendMsg(MyMessage & msg)
{
	if (pdata == nullptr || size == 0)
	{
		return;
	}
	if (data_type != 1 && data_type != 2)
	{
		data_type = 2;
	}
	uint8_t opcode = (15 & data_type) | 128;
	if (size <= 125)
	{
		uint8_t payload = (uint8_t)size;
		buffer_t* pmsg = new buffer_t(2+size);
		uint8_t * pbuffer = pmsg->get_buffer();
		pbuffer[0] = opcode;
		pbuffer[1] = payload;
		memcpy(pbuffer + 2, pdata, size);
		pmsg->set_data_size(2+size);
		send_message(pmsg);
	}
	else if (size <= 65535)
	{
		uint8_t payload = (uint8_t)126;
		buffer_t* pmsg = new buffer_t(4 + size);
		uint8_t * pbuffer = pmsg->get_buffer();
		pbuffer[0] = opcode;
		pbuffer[1] = payload;
		uint16_t * plength = (uint16_t*)(pbuffer + 2);
		*plength = htons((uint16_t)size);
		memcpy(pbuffer + 4, pdata, size);
		pmsg->set_data_size(4 + size);
		send_message(pmsg);
	}
	else
	{
		uint8_t payload = (uint8_t)127;
		buffer_t * pmsg = new buffer_t(10 + size);
		uint8_t * pbuffer = pmsg->get_buffer();
		pbuffer[0] = opcode;
		pbuffer[1] = payload;
		uint64_t *plength = (uint64_t*)(pbuffer + 2);
		*plength = htonll((uint64_t)size);
		memcpy(pbuffer + 10, pdata, size);
		pmsg->set_data_size(10 + size);
		send_message(pmsg);
	}
}

uint32_t find_http_request_end_pos(const char * pchar, uint32_t size)
{
	for (uint32_t i = 0; i <= size - 4; ++i)
	{
		if (pchar[i] == '\r' && pchar[i + 1] == '\n' && pchar[i + 2] == '\r' && pchar[i + 3] == '\n')
		{
			return i + 4;
		}
	}
	return 0;
}

std::string base64_encode(const unsigned char* Data, int DataByte)
{
	const char * EncodeTable = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	std::string strEncode;
	unsigned char Tmp[4] = { 0 };
	for (int i = 0; i<(int)(DataByte / 3); i++)
	{
		Tmp[1] = *Data++;
		Tmp[2] = *Data++;
		Tmp[3] = *Data++;
		strEncode += EncodeTable[Tmp[1] >> 2];
		strEncode += EncodeTable[((Tmp[1] << 4) | (Tmp[2] >> 4)) & 0x3F];
		strEncode += EncodeTable[((Tmp[2] << 2) | (Tmp[3] >> 6)) & 0x3F];
		strEncode += EncodeTable[Tmp[3] & 0x3F];
	}
	int Mod = DataByte % 3;
	if (Mod == 1)
	{
		Tmp[1] = *Data++;
		strEncode += EncodeTable[(Tmp[1] & 0xFC) >> 2];
		strEncode += EncodeTable[((Tmp[1] & 0x03) << 4)];
		strEncode += "==";
	}
	else if (Mod == 2)
	{
		Tmp[1] = *Data++;
		Tmp[2] = *Data++;
		strEncode += EncodeTable[(Tmp[1] & 0xFC) >> 2];
		strEncode += EncodeTable[((Tmp[1] & 0x03) << 4) | ((Tmp[2] & 0xF0) >> 4)];
		strEncode += EncodeTable[((Tmp[2] & 0x0F) << 2)];
		strEncode += "=";
	}
	return std::move(strEncode);
}

uint32_t ws_session::proc_data(uint8_t * pdata, uint32_t size, bool & procFinish)
{
	if (!m_http_handshake)
	{
		if (size < 4)
		{
			return 0;
		}
		if (pdata[0] != 'G' || pdata[1] != 'E' || pdata[2] != 'T' || pdata[3] != ' ')
		{
			close();
			return 0;
		}
		uint32_t fendpos = find_http_request_end_pos((const char *)pdata, size);
		if (0 == fendpos)
		{
			return 0;
		}
		http_request hreq;
		if (!hreq.parse((const char*)pdata, fendpos-2))
		{
			close();
			return 0;
		}
		const char * pwebsocket_key = hreq.get_http_head("Sec-WebSocket-Key");
		if (nullptr == pwebsocket_key)
		{
			close();
			return 0;
		}
		std::string websocket_key(pwebsocket_key);
		websocket_key += "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
		unsigned char sha_value[SHA_DIGEST_LENGTH];
		SHA1((unsigned char *)(&websocket_key[0]), websocket_key.size(), sha_value);
		std::string websocket_accept = base64_encode(sha_value, sizeof(sha_value));
		const char * pstr = "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: ";
		size_t str_len = strlen(pstr);
		websocket_accept += "\r\n\r\n";
		buffer_t* pbuffer = new buffer_t(str_len + websocket_accept.size());
		memcpy(pbuffer->get_buffer(), pstr, str_len);
		memcpy(pbuffer->get_buffer() + str_len, &(websocket_accept[0]), websocket_accept.size());
		pbuffer->set_data_size(str_len + websocket_accept.size());
		send_message(pbuffer);
		m_http_handshake = true;
		http_handshake_callback();
		procFinish = true;
		return fendpos;
	}
	else
	{
		if (size < 2)
		{
			return 0;
		}
		uint8_t bfin = (pdata[0] & (uint8_t)128);
		uint8_t opcode = pdata[0] & 15;
		uint8_t bmask = (pdata[1] & (uint8_t)128);
		uint8_t payload = pdata[1] & 127;
		uint64_t payload_data_size = 0;
		uint64_t frame_total_size = 2;
		if (bmask)
		{
			frame_total_size += 4;
		}
		uint32_t frame_mask_value = 0;
		if (payload < 126)
		{
			payload_data_size = payload;
			frame_total_size += payload;
			if (frame_total_size > size)
			{
				return 0;
			}
			if(bmask)
			{
				uint32_t * pvalue = (uint32_t*)(pdata + 2);
				frame_mask_value = *pvalue;
			}
		}
		else if (payload == 126)
		{
			frame_total_size += 2;
			if (frame_total_size > size)
			{
				return 0;
			}
			uint16_t * pvalue = (uint16_t*)(pdata + 2);
			payload_data_size = ntohs(*pvalue);
			frame_total_size += payload_data_size;
			if (frame_total_size > size)
			{
				return 0;
			}
			if (bmask)
			{
				uint32_t * pvalue = (uint32_t*)(pdata + 4);
				frame_mask_value = *pvalue;
			}
		}
		else
		{
			frame_total_size += 8;
			if (frame_total_size > size)
			{
				return 0;
			}
			uint64_t * pvalue = (uint64_t*)(pdata + 2);
			payload_data_size = ntohll(*pvalue);
			frame_total_size += payload_data_size;
			if (frame_total_size > size)
			{
				return 0;
			}
			if (bmask)
			{
				uint32_t * pvalue = (uint32_t*)(pdata + 10);
				frame_mask_value = *pvalue;
			}
		}
		buffer_t* pframe = nullptr;
		if (payload_data_size)
		{
			pframe = new buffer_t(payload_data_size);
			pframe->set_data_size(payload_data_size);
			uint8_t * pbuffer = pframe->get_buffer();
			if (bmask)
			{
				uint8_t * pmask = (uint8_t*)(&frame_mask_value);
				uint8_t * psrc = pdata + frame_total_size - payload_data_size;
				for (uint64_t i = 0; i < payload_data_size; ++i)
				{
					pbuffer[i] = psrc[i] ^ pmask[i % sizeof(frame_mask_value)];
				}
			}
			else
			{
				memcpy(pbuffer, pdata + frame_total_size - payload_data_size, payload_data_size);
			}
		}
		if (m_frames.empty())
		{
			m_frames.push_back(pframe);
			m_frames_opcode = opcode;
			m_frames_total_size = payload_data_size;
		}
		else
		{
			m_frames.push_back(pframe);
			m_frames_total_size += payload_data_size;
		}
		procFinish = true;
		if (bfin)
		{
			switch (m_frames_opcode)
			{
			case 1:
			case 2:
			{
				if (m_frames.size() == 1)
				{
					buffer_t * ptempBuffer = m_frames.front();
					if (ptempBuffer)
					{
						proc_msg_packet(ptempBuffer->get_buffer(), (uint32_t)(ptempBuffer->get_data_size()), m_frames_opcode, procFinish);
					}
					else
					{
						proc_msg_packet(nullptr, 0, m_frames_opcode, procFinish);
					}
					
				}
				else
				{
					buffer_t *ptotalBuffer = new buffer_t(m_frames_total_size);
					uint8_t * ptotalBegin = ptotalBuffer->get_buffer();
					for (auto & i : m_frames)
					{
						if (i)
						{
							size_t totalDataSize = ptotalBuffer->get_data_size();
							memcpy(ptotalBegin + totalDataSize, i->get_buffer(), i->get_data_size());
							ptotalBuffer->set_data_size(totalDataSize + i->get_data_size());
						}
					}
					proc_msg_packet(ptotalBuffer->get_buffer(), (uint32_t)(ptotalBuffer->get_data_size()), m_frames_opcode, procFinish);
				}
			}
				break;
			case 8:
				close();
				break;
			case 9:
			{
				uint8_t opcode = 138;
				uint8_t payload = 0;
				buffer_t* pmsg = new buffer_t(2);
				uint8_t * pbuffer = pmsg->get_buffer();
				pbuffer[0] = opcode;
				pbuffer[1] = payload;
				pmsg->set_data_size(2);
				send_message(pmsg);
				heart_beat_callback();
			}
				break;
			case 10:
				heart_beat_callback();
				break;
			default:
				break;
			}
			for (auto i : m_frames)
			{
				if (i)
				{
					delete i;
				}
			}
			m_frames.clear();
		}
		return (uint32_t)frame_total_size;
	}
}

void MyWebSocket::close()
{
	if (!m_close_socket)
	{
		m_close_socket = true;
		boost::system::error_code ec;
		m_socket.close(ec);
		for (auto i : m_frames)
		{
			if (i)
			{
				delete i;
			}
		}
		m_frames.clear();
		for (auto i : m_buffers_deque)
		{
			delete i;
		}
		m_buffers_deque.clear();
		close_session_callback();
	}
}

bool http_request::parse(const char * pchar, uint32_t endpos)
{
	uint32_t lineLength;
	if (!parse_line(pchar, lineLength))
	{
		return false;
	}
	const char * pend = pchar + endpos;
	for (const char * pbegin = pchar + lineLength; pbegin < pend;)
	{
		uint32_t headLength;
		if (!parse_head(pbegin, headLength))
		{
			return false;
		}
		pbegin += headLength;
	}
	return true;
}

const char * http_request::get_http_method() const
{
	return m_http_method.c_str();
}

const char * http_request::get_http_version() const
{
	return m_http_version.c_str();
}

const char * http_request::get_resource_path() const
{
	return m_resource_path.c_str();
}

const char * http_request::get_http_head(const char * key) const
{
	if (nullptr == key)
	{
		return nullptr;
	}
	for (auto & i : m_http_heads)
	{
		if (i.first == key)
		{
			return i.second.c_str();
		}
	}
	return nullptr;
}

bool http_request::parse_line(const char * pchar, uint32_t & lineLength)
{
	uint32_t i = 0;
	while (pchar[i] != ' ' && (pchar[i] != '\r' || pchar[i+1] != '\n'))
	{
		++i;
	}
	if (pchar[i] != ' ')
	{
		return false;
	}
	if (i == 0)
	{
		return false;
	}
	m_http_method.assign(pchar, i);
	while (pchar[i] == ' ')
	{
		++i;
	}
	uint32_t j = i;
	while (pchar[j] != ' ' && (pchar[j] != '\r' || pchar[j + 1] != '\n'))
	{
		++j;
	}
	if (pchar[j] != ' ')
	{
		return false;
	}
	m_resource_path.assign(pchar + i, j - i);
	while (pchar[j] == ' ')
	{
		++j;
	}
	uint32_t k = j;
	while (pchar[k] != '\r' || pchar[k+1] != '\n')
	{
		++k;
	}
	if (j < k)
	{
		m_http_version.assign(pchar + j, k - j);
		lineLength = k + 2;
		return true;
	}
	else
	{
		return false;
	}
}

bool http_request::parse_head(const char * pchar, uint32_t & headLength)
{
	uint32_t i = 0;
	while (pchar[i] != ':' && (pchar[i] != '\r' || pchar[i+1] !='\n'))
	{
		++i;
	}
	if (pchar[i] != ':')
	{
		return false;
	}
	if (i == 0)
	{
		return false;
	}
	std::string key(pchar, i);
	uint32_t j = i + 1;
	while (pchar[j] != '\r' || pchar[j+1] != '\n')
	{
		++j;
	}
	std::string value;
	trim(pchar, i + 1, j, value);
	if (!value.empty())
	{
		m_http_heads.push_back(std::pair<std::string, std::string>(std::move(key), std::move(value)));
	}
	headLength = j + 2;
	return true;
}

void http_request::trim(const char * pchar, uint32_t begPos, uint32_t endPos, std::string & outStr)
{
	outStr.clear();
	while (begPos < endPos)
	{
		if (pchar[begPos] == ' ')
		{
			++begPos;
		}
		else
		{
			break;
		}
	}
	if (begPos < endPos)
	{
		outStr.assign(pchar + begPos, endPos - begPos);
	}
}