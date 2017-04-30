#include <assert.h>
#include "workers.h"

worker::worker()
{
	m_brun = false;
	m_cur_insert_index = 0;
	m_cur_worker_index = 1;
}

worker::~worker()
{

}

void worker::start()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	assert(!m_brun);
	m_brun = true;
	m_work_thread = std::make_shared<std::thread>(&worker::do_work, this);
}

void worker::push_back(std::function<void()> w)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	m_work[m_cur_insert_index].push_back(std::move(w));
}

void worker::push_front(std::function<void()> w)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	m_work[m_cur_insert_index].push_front(std::move(w));
}

void worker::stop()
{
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		assert(m_brun);
		m_brun = false;
	}
	m_work_thread->join();
}

void worker::do_work()
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
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
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


workers::workers()
{
	m_cur_index = 0;
}

workers::~workers()
{

}

void workers::start(unsigned int num)
{
	if (0 == num)
	{
		num = 1;
	}
	for (unsigned int i = 0; i < num; ++i)
	{
		m_pworkers.push_back(std::make_shared<worker>());
		m_pworkers.back()->start();
	}
}

void workers::push_back(std::function<void()> w)
{
	size_t index;
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		index = m_cur_index;
		m_cur_index = ((m_cur_index + 1) % (m_pworkers.size()));
	}
	m_pworkers[index]->push_back(std::move(w));
}

void workers::push_front(std::function<void()> w)
{
	size_t index;
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		index = m_cur_index;
		m_cur_index = ((m_cur_index + 1) % (m_pworkers.size()));
	}
	m_pworkers[index]->push_front(std::move(w));
}

void workers::stop()
{
	for (auto & i : m_pworkers)
	{
		i->stop();
	}
}