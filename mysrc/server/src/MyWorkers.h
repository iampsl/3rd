#ifndef PSL_MY_WORKERS_H
#define PSL_MY_WORKERS_H

#include <deque>
#include <memory>
#include <functional>
#include <thread>
#include <vector>
#include <mutex>

class MyWorker
{
public:
	MyWorker();
	MyWorker & operator = (const MyWorker &) = delete;
	MyWorker(const MyWorker &) = delete;
	~MyWorker();
	void start();
	void push_back(std::function<void()> w);
	void push_front(std::function<void()> w);
	void stop();
private:
	void do_work();
private:
	std::deque<std::function<void()>> m_work[2];
	size_t m_cur_worker_index;
	std::shared_ptr<std::thread> m_work_thread;
	std::mutex m_mutex;
	bool m_brun;
	size_t m_cur_insert_index;
};

class MyWorkers
{
public:
	MyWorkers();
	MyWorkers & operator = (const MyWorkers &) = delete;
	MyWorkers(const MyWorkers &) = delete;
	~MyWorkers();
	void start(unsigned int num);
	void push_back(std::function<void()> w);
	void push_front(std::function<void()> w);
	void stop();
private:
	std::vector<std::shared_ptr<MyWorker>> m_vecWorkers;
	size_t m_cur_index;
};

class MyWorkerList
{
public:
	MyWorkerList();
	MyWorkerList & operator = (const MyWorkerList &) = delete;
	MyWorkerList(const MyWorkerList &) = delete;
	~MyWorkerList();
	void push_back(bool bmultiPush, std::function<void()> w);
	bool process();//just call on a thread
private:
	std::deque<std::function<void()>> m_work[2];
	uint32_t m_cur_worker_index;
	std::mutex m_mutex;
	uint32_t m_cur_insert_index;
};

#endif // !PSL_MY_WORKERS_H
