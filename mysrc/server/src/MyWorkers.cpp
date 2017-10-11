#include <assert.h>
#include "MyWorkers.h"

MyWorker::MyWorker()
{
	m_brun = false;
	m_cur_insert_index = 0;
	m_cur_worker_index = 1;
}

MyWorker::~MyWorker()
{

}

void MyWorker::start()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	assert(!m_brun);
	m_brun = true;
	m_work_thread = std::make_shared<std::thread>(&MyWorker::do_work, this);
}

void MyWorker::push_back(std::function<void()> w)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	m_work[m_cur_insert_index].push_back(std::move(w));
}

void MyWorker::push_front(std::function<void()> w)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	m_work[m_cur_insert_index].push_front(std::move(w));
}

void MyWorker::stop()
{
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		assert(m_brun);
		m_brun = false;
	}
	m_work_thread->join();
}

void MyWorker::do_work()
{
	bool bexit = false;
	while (!bexit)
	{
		if (m_work[m_cur_worker_index].empty())
		{
			{
				std::lock_guard<std::mutex> lock(m_mutex);
				bexit = !m_brun;
				if (!m_work[m_cur_insert_index].empty())
				{
					size_t temp = m_cur_insert_index;
					m_cur_insert_index = m_cur_worker_index;
					m_cur_worker_index = temp;
				}
			}
			if (m_work[m_cur_worker_index].empty())
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(15));
			}
		}
		else
		{
			try
			{
				m_work[m_cur_worker_index].front()();
			}
			catch (...)
			{
			}
			m_work[m_cur_worker_index].pop_front();
		}
	}
}


MyWorkers::MyWorkers()
{
	m_cur_index = 0;
}

MyWorkers::~MyWorkers()
{

}

void MyWorkers::start(unsigned int num)
{
	if (0 == num)
	{
		num = 1;
	}
	for (unsigned int i = 0; i < num; ++i)
	{
		m_vecWorkers.push_back(std::make_shared<MyWorker>());
		m_vecWorkers.back()->start();
	}
}

void MyWorkers::push_back(std::function<void()> w)
{
	const size_t size = m_vecWorkers.size();
	m_cur_index = (m_cur_index + 1) % size;
	size_t index = m_cur_index % size;
	m_vecWorkers[index]->push_back(std::move(w));
}

void MyWorkers::push_front(std::function<void()> w)
{
	const size_t size = m_vecWorkers.size();
	m_cur_index = (m_cur_index + 1) % size;
	size_t index = m_cur_index % size;
	m_vecWorkers[index]->push_front(std::move(w));
}

void MyWorkers::stop()
{
	for (auto & i : m_vecWorkers)
	{
		i->stop();
	}
}

MyWorkerList::MyWorkerList()
{
	m_cur_insert_index = 0;
	m_cur_worker_index = 1;
}

MyWorkerList::~MyWorkerList()
{
}

void MyWorkerList::push_back(bool bmultiPush, std::function<void()> w)
{
	if (bmultiPush)
	{
		m_mutex.lock();
		m_work[m_cur_insert_index].push_back(std::move(w));
		m_mutex.unlock();
	}
	else
	{
		while (!m_mutex.try_lock())
		{
		}
		m_work[m_cur_insert_index].push_back(std::move(w));
		m_mutex.unlock();
	}
}

bool MyWorkerList::process()
{
	if (!m_mutex.try_lock())
	{
		return true;
	}
	m_cur_insert_index = m_cur_worker_index;
	m_mutex.unlock();
	m_cur_worker_index = (m_cur_worker_index + 1) % 2;
	if (m_work[m_cur_worker_index].empty())
	{
		return false;
	}
	for (auto & value : m_work[m_cur_worker_index])
	{
		value();
	}
	m_work[m_cur_worker_index].clear();
	return true;
}
