#ifndef PSL_MY_MESSAGE_H
#define PSL_MY_MESSAGE_H

class MyMessage
{
public:
	virtual unsigned int getMsgSize() = 0;
	virtual bool serialize(void * pbuffer, unsigned int size) = 0;
};

#endif // !PSL_MY_MESSAGE_H

