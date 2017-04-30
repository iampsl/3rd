#ifndef PSL_WORKERS_H_
#define PSL_WORKERS_H_

#include <deque>
#include <memory>
#include <functional>
#include <thread>
#include <vector>
#include <mutex>

class worker
{
public:
	worker();
	~worker();
	void start();
	void push_back(std::function<void()> w);
	void push_front(std::function<void()> w);
	void stop();
private:
	void do_work();
	worker & operator = (const worker & other) = delete;
	worker(const worker & other) = delete;
private:
	std::deque<std::function<void()>> m_work[2];
	size_t m_cur_worker_index;
	std::shared_ptr<std::thread> m_work_thread;
	std::mutex m_mutex;
	bool m_brun;
	size_t m_cur_insert_index;
};

class workers
{
public:
	workers();
	~workers();
	void start(unsigned int num);
	void push_back(std::function<void()> w);
	void push_front(std::function<void()> w);
	void stop();
private:
	workers & operator = (const workers & other) = delete;
	workers(const workers & other) = delete;
private:
	std::vector<std::shared_ptr<worker>> m_pworkers;
	std::mutex m_mutex;
	size_t m_cur_index;
};

#endif // !PSL_WORKERS_H_
