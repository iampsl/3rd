#include <assert.h>
#include <openssl/sha.h>
#include "MyWebSocket.h"

static const char * g_websocketReq = "GET / HTTP/1.1\r\nSec-Websocket-Version: 13\r\nSec-Websocket-Key: Q6NIX4mSwu68sFLoWMi6pA==\r\nConnection: Upgrade\r\nUpgrade: websocket\r\n\r\n";
static const char * g_websocketAccept = "9zVwHv3w6sZ+Mj0ucxp4gAacjKk=";

bool http_message::parse(const char * pchar, uint32_t endpos)
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

const char * http_message::get_first() const
{
	return m_first.c_str();
}

const char * http_message::get_second() const
{
	return m_second.c_str();
}

const char * http_message::get_three() const
{
	return m_three.c_str();
}

const char * http_message::get_http_head(const char * key) const
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

bool http_message::parse_line(const char * pchar, uint32_t & lineLength)
{
	uint32_t i = 0;
	while (pchar[i] != ' ' && (pchar[i] != '\r' || pchar[i + 1] != '\n'))
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
	m_first.assign(pchar, i);
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
	m_second.assign(pchar + i, j - i);
	while (pchar[j] == ' ')
	{
		++j;
	}
	uint32_t k = j;
	while (pchar[k] != '\r' || pchar[k + 1] != '\n')
	{
		++k;
	}
	if (j < k)
	{
		m_three.assign(pchar + j, k - j);
		lineLength = k + 2;
		return true;
	}
	else
	{
		return false;
	}
}

bool http_message::parse_head(const char * pchar, uint32_t & headLength)
{
	uint32_t i = 0;
	while (pchar[i] != ':' && (pchar[i] != '\r' || pchar[i + 1] != '\n'))
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
	while (pchar[j] != '\r' || pchar[j + 1] != '\n')
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

void http_message::trim(const char * pchar, uint32_t begPos, uint32_t endPos, std::string & outStr)
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

void makeMask(uint8_t * psrc, uint64_t size, uint32_t mask)
{
	uint8_t * pmask = (uint8_t*)(&mask);
	for (uint64_t i = 0; i < size; ++i)
	{
		psrc[i] = psrc[i] ^ pmask[i % sizeof(mask)];
	}
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

MyWebSocket::MyWebSocket(MyIoService * pservice) : MySocket(pservice)
{
	m_isClient = false;
	m_isHandshaked = false;
}

MyWebSocket::~MyWebSocket()
{

}

void MyWebSocket::close()
{
	auto ioservice = getIoService();
	for (auto & value : m_frames)
	{
		ioservice->getMemoryPool().free(value.pdata, value.retSize);
	}
	m_frames.clear();
	MySocket::close();
}

void MyWebSocket::sendPing()
{
	uint8_t opcode = 137;
	uint8_t payload = 0;
	MessageData data;
	data.size = 2;
	data.pdata = getIoService()->getMemoryPool().malloc(data.size, data.retSize);
	if (nullptr == data.pdata)
	{
		return;
	}
	uint8_t * pbuffer = (uint8_t*) (data.pdata);
	pbuffer[0] = opcode;
	pbuffer[1] = payload;
	doSendMsg(data);
}

void MyWebSocket::sendMsg(MyMessage & msg)
{
	unsigned int size = msg.getMsgSize();
	if (0 == size)
	{
		return;
	}
	const uint8_t data_type = msg.getMsgType();
	assert(2 == data_type|| 1 == data_type);
	uint8_t firstByte = (15 & data_type) | 128;
	bool bmask = m_isClient ? true : false;
	uint32_t maskValue = (uint32_t)size;
	if (size <= 125)
	{
		uint8_t secondByte = (bmask?((uint8_t)(128|size)):((uint8_t)size));
		MessageData data;
		data.size = (bmask ? (6+size):(2+size));
		data.pdata = getIoService()->getMemoryPool().malloc(data.size, data.retSize);
		if (nullptr == data.pdata)
		{
			return;
		}
		uint8_t * pbuffer = (uint8_t*)(data.pdata);
		pbuffer[0] = firstByte;
		pbuffer[1] = secondByte;
		if (bmask)
		{
			uint32_t * pmask = (uint32_t*)(pbuffer+2);
			*pmask = maskValue;
		}
		bool bserialize = msg.serialize(pbuffer + data.size - size, size);
		assert(true == bserialize);
		if (bmask)
		{
			makeMask(pbuffer + data.size - size, size, maskValue);
		}
		doSendMsg(data);
	}
	else if (size <= 65535)
	{
		uint8_t secondByte = (bmask ? ((uint8_t)254):((uint8_t)126));
		MessageData data;
		data.size = (bmask ? (8 + size) : (4 + size));
		data.pdata = getIoService()->getMemoryPool().malloc(data.size, data.retSize);
		if (nullptr == data.pdata)
		{
			return;
		}
		uint8_t * pbuffer = (uint8_t*)(data.pdata);
		pbuffer[0] = firstByte;
		pbuffer[1] = secondByte;
		uint16_t * plength = (uint16_t*)(pbuffer + 2);
		*plength = htons((uint16_t)size);
		if (bmask)
		{
			uint32_t * pmask = (uint32_t*)(pbuffer + 4);
			*pmask = maskValue;
		}
		bool bserialize = msg.serialize(pbuffer + data.size - size, size);
		assert(true == bserialize);
		if (bmask)
		{
			makeMask(pbuffer + data.size - size, size, maskValue);
		}
		doSendMsg(data);
	}
	else
	{
		uint8_t secondByte = (bmask ? ((uint8_t)255) : ((uint8_t)127));
		MessageData data;
		data.size = (bmask ? (14 + size) : (10 + size));
		data.pdata = getIoService()->getMemoryPool().malloc(data.size, data.retSize);
		if (nullptr == data.pdata)
		{
			return;
		}
		uint8_t * pbuffer = (uint8_t*)(data.pdata);
		pbuffer[0] = firstByte;
		pbuffer[1] = secondByte;
		uint64_t * plength = (uint64_t*)(pbuffer + 2);
		*plength = htonll(size);
		if (bmask)
		{
			uint32_t * pmask = (uint32_t*)(pbuffer + 10);
			*pmask = maskValue;
		}
		bool bserialize = msg.serialize(pbuffer + data.size - size, size);
		assert(true == bserialize);
		if (bmask)
		{
			makeMask(pbuffer + data.size - size, size, maskValue);
		}
		doSendMsg(data);
	}
}

uint32_t MyWebSocket::procData(uint8_t * pdata, uint32_t size, bool & procFinish)
{
	if (false == m_isHandshaked)
	{
		if (m_isClient)
		{
			if (size < 4)
			{
				return 0;
			}
			if (pdata[0] != 'H' || pdata[1] != 'T' || pdata[2] != 'T' || pdata[3] != 'P')
			{
				close();
				return 0;
			}
			uint32_t fendpos = find_http_request_end_pos((const char *)pdata, size);
			if (0 == fendpos)
			{
				return 0;
			}
			http_message hreq;
			if (!hreq.parse((const char*)pdata, fendpos - 2))
			{
				close();
				return 0;
			}
			const char *pwebsocket_accept = hreq.get_http_head("Sec-WebSocket-Accept");
			if (nullptr == pwebsocket_accept)
			{
				close();
				return 0;
			}
			if (0 != strcmp(pwebsocket_accept, g_websocketAccept))
			{
				close();
				return 0;
			}
			m_isHandshaked = true;
			httpFinishCallback();
			procFinish = true;
			return fendpos;
		}
		else
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
			http_message hreq;
			if (!hreq.parse((const char*)pdata, fendpos - 2))
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
			MessageData data;
			data.size = (unsigned int)(str_len + websocket_accept.size());
			data.pdata = getIoService()->getMemoryPool().malloc(data.size, data.retSize);
			if (nullptr == data.pdata)
			{
				close();
				return 0;
			}
			memcpy(data.pdata, pstr, str_len);
			memcpy((uint8_t*)(data.pdata) + str_len, &(websocket_accept[0]), websocket_accept.size());
			doSendMsg(data);
			m_isHandshaked = true;
			httpFinishCallback();
			procFinish = true;
			return fendpos;
		}
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
		MessageData data{nullptr, 0, 0};
		if (payload_data_size)
		{
			data.size = payload_data_size;
			data.pdata = getIoService()->getMemoryPool().malloc(data.size, data.retSize);
			if (nullptr == data.pdata)
			{
				close();
				return 0;
			}
			uint8_t * pbuffer = (uint8_t*)(data.pdata);
			if (bmask)
			{
				makeMask(pdata + frame_total_size - payload_data_size, payload_data_size, frame_mask_value);
			}
			else
			{
				memcpy(pbuffer, pdata + frame_total_size - payload_data_size, payload_data_size);
			}
		}
		if (m_frames.empty())
		{
			m_frames.push_back(data);
			m_frames_opcode = opcode;
			m_frames_total_size = payload_data_size;
		}
		else
		{
			m_frames.push_back(data);
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
					MessageData tempData = m_frames.front();
					procMsg((uint8_t*)(tempData.pdata), (uint32_t)(tempData.size), m_frames_opcode, procFinish);
				}
				else
				{
					MessageData tempData;
					tempData.size = m_frames_total_size;
					tempData.pdata = getIoService()->getMemoryPool().malloc(tempData.size, tempData.retSize);
					if (nullptr != tempData.pdata)
					{
						uint8_t * ptotalBegin = (uint8_t*)(tempData.pdata);
						size_t curTotalSize = 0;
						for (auto & value : m_frames)
						{
							if (nullptr != value.pdata)
							{
								memcpy(ptotalBegin + curTotalSize, value.pdata, value.size);
								curTotalSize += value.size;
							}
						}
						procMsg((uint8_t*)(tempData.pdata), (uint32_t)(tempData.size), m_frames_opcode, procFinish);
						getIoService()->getMemoryPool().free(tempData.pdata, tempData.retSize);
					}
				}
			}
				break;
			case 8:
				close();
				break;
			case 9:
				pingCallback();
				break;
			case 10:
				pongCallback();
				break;
			default:
				break;
			}
			MyMemoryPool & mpool = getIoService()->getMemoryPool();
			for (auto & value : m_frames)
			{
				if (value.pdata)
				{
					mpool.free(value.pdata, value.retSize);
				}
			}
			m_frames.clear();
		}
		return (uint32_t)frame_total_size;
	}
}


void MyWebSocket::login()
{
	m_isClient = true;
	size_t len = strlen(g_websocketReq);
	MessageData data;
	data.size = len;
	data.pdata = getIoService()->getMemoryPool().malloc(data.size, data.retSize);
	if (nullptr == data.pdata)
	{
		close();
		return;
	}
	memcpy(data.pdata, g_websocketReq, len);
	doSendMsg(data);
}