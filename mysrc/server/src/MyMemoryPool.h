#ifndef PSL_MY_MEMORY_POOL_H
#define PSL_MY_MEMORY_POOL_H

#include <boost/pool/pool.hpp>

class MyMemoryPool
{
public:
	MyMemoryPool();
	MyMemoryPool(const MyMemoryPool &) = delete;
	MyMemoryPool & operator = (const MyMemoryPool &) = delete;
	void * malloc(unsigned int size, unsigned int & retSize);
	void free(void * pmem, unsigned int retSize);
private:
	boost::pool<> m_pool32;
	boost::pool<> m_pool64;
	boost::pool<> m_pool128;
	boost::pool<> m_pool256;
	boost::pool<> m_pool512;
};

#endif // !PSL_MY_MEMORY_POOL_H
