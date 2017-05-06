#ifndef PSL_MY_MESSAGE_H
#define PSL_MY_MESSAGE_H

class MyMessage
{
public:
	virtual unsigned int getMsgSize() = 0;
	//1代表文本数据　２代表二进制数据
	virtual uint8_t getMsgType() = 0;
	virtual bool serialize(void * pbuffer, unsigned int size) = 0;
};

#endif // !PSL_MY_MESSAGE_H

