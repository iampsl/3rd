#include "log_manager.h"
#include <string.h>
#include <string>
#include <stdlib.h>
#include <stdarg.h>
#include <functional>
#include <assert.h>


int get_mmddhhmmss(const time_t & t)
{
    tm tout;
#ifdef PSL_WINDOWS
    localtime_s(&tout, &t);
#else
    localtime_r(&t, &tout);
#endif
    int ret = (tout.tm_mon + 1) * 100000000 + (tout.tm_mday) * 1000000 + (tout.tm_hour) * 10000 + (tout.tm_min) * 100 + tout.tm_sec;
    return ret;
}


log_manager::log_manager()
{
	static_assert((sizeof(m_logs) / sizeof(m_logs[0])) != 0, "");
	size_t nums = (sizeof(m_logs) / sizeof(m_logs[0]));
	for (size_t i = 0; i < nums; ++i)
	{
		m_logs[i].pfile = NULL;
		m_logs[i].file_cur_size = 0;
		m_logs[i].file_max_size = 1024*1024*10;
		memset(m_logs[i].file_save_directory, 0, sizeof(m_logs[i].file_save_directory));
		memset(m_logs[i].file_name_suffix, 0, sizeof(m_logs[i].file_name_suffix));
        snprintf(m_logs[i].file_name_prefix, sizeof(m_logs[i].file_name_prefix), "%u_", (unsigned int)i);
		m_logs[i].bstart = false;
		m_logs[i].cur_insert_index = 0;
		m_logs[i].cur_worker_index = 1;
	}
}

log_manager::~log_manager()
{
}

log_manager & log_manager::get_instance()
{
	static log_manager s_manager;
	return s_manager;
}

void log_manager::set_log_info(int whichlog, const char * pfile_prefix, const char * pfile_suffix, const char * pfile_dir, size_t max_size)
{
	size_t nums = (sizeof(m_logs) / sizeof(m_logs[0]));
    assert(whichlog >= 0 && whichlog < nums);
	if (whichlog < 0 || whichlog >= nums)
	{
		return;
	}
	if (NULL != pfile_prefix)
	{
        snprintf(m_logs[whichlog].file_name_prefix, sizeof(m_logs[whichlog].file_name_prefix), "%s", pfile_prefix);
	}
	if (NULL != pfile_suffix)
	{
        snprintf(m_logs[whichlog].file_name_suffix, sizeof(m_logs[whichlog].file_name_suffix), "%s", pfile_suffix);
	}
	if (NULL != pfile_dir)
	{
		char backslash[3] = {0};
		size_t file_dir_len = strlen(pfile_dir);
		if (0 != file_dir_len)
		{
			if ('/' != pfile_dir[file_dir_len-1])
			{
				backslash[0] = '/';
			}
		}
        snprintf(m_logs[whichlog].file_save_directory, sizeof(m_logs[whichlog].file_save_directory), "%s%s", pfile_dir, backslash);
	}
	m_logs[whichlog].file_max_size = max_size;
}

void log_manager::start()
{
	size_t nums = (sizeof(m_logs) / sizeof(m_logs[0]));
	for (size_t i = 0; i < nums; ++i)
	{
		std::lock_guard<std::mutex> lock(m_logs[i].messages_mutex);
		assert(m_logs[i].bstart == false);
		m_logs[i].bstart = true;
	}
	m_pthread = std::make_shared<std::thread>(&log_manager::work_thread, this);
}

void log_manager::output_message( int whichlog, const message_data & msg)
{
	size_t nums = (sizeof(m_logs) / sizeof(m_logs[0]));
    assert(whichlog >= 0 && whichlog < nums);
	if (whichlog < 0 || whichlog >= nums)
	{
		return;
	}
	if (msg.size == 0)
	{
		return;
	}
	if (0 == m_logs[whichlog].file_max_size)
	{
		return;
	}
	std::lock_guard<std::mutex> lock(m_logs[whichlog].messages_mutex);
	m_logs[whichlog].messages[m_logs[whichlog].cur_insert_index].push_back(msg);
}

void log_manager::stop()
{
	size_t nums = (sizeof(m_logs) / sizeof(m_logs[0]));
	for (size_t i = 0; i < nums; ++i)
	{
		std::lock_guard<std::mutex> lock(m_logs[i].messages_mutex);
		assert(m_logs[i].bstart == true);
		m_logs[i].bstart = false;
	}
	m_pthread->join();
}

void log_manager::work_thread()
{
	size_t nums = (sizeof(m_logs) / sizeof(m_logs[0]));
	bool bexit = false;
	while (!bexit)
	{
		bool bsleep = true;
		bexit = true;
		for (size_t i = 0; i < nums; ++i)
		{
			log_info & info = m_logs[i];
			if (info.messages[info.cur_worker_index].empty())
			{
				bool bstart;
				{
					std::lock_guard<std::mutex> lock(info.messages_mutex);
					bstart = info.bstart;
					if (!info.messages[info.cur_insert_index].empty())
					{
						size_t temp = info.cur_insert_index;
						info.cur_insert_index = info.cur_worker_index;
						info.cur_worker_index = temp;
					}
				}
				if (info.messages[info.cur_worker_index].empty())
				{
					if (bstart)
					{
						if (nullptr != info.pfile)
						{
							fflush(info.pfile);
						}
						bexit = false;
					}
					else
					{
						if (nullptr != info.pfile)
						{
							fclose(info.pfile);
							info.pfile = nullptr;
						}
					}
				}
				else
				{
					bexit = false;
					bsleep = false;
				}
			}
			else
			{
				bexit = false;
				bsleep = false;
				while (!info.messages[info.cur_worker_index].empty())
				{
					if (NULL == info.pfile)
					{
						char file_path[2048];
						time(&(info.create_time));
						gen_file_path(file_path, sizeof(file_path), info.file_save_directory, info.file_name_prefix, info.file_name_suffix, info.create_time);
#ifdef PSL_WINDOWS
						info.pfile = _fsopen(file_path, "a", _SH_DENYWR);
#else
						info.pfile = fopen(file_path, "a");
#endif
						if (NULL != info.pfile)
						{
							fseek(info.pfile, 0, SEEK_END);
							long pos = ftell(info.pfile);
							if (pos < 0)
							{
								info.file_cur_size = info.file_max_size;
							}
							else
							{
								info.file_cur_size = pos;
							}
						}
					}
					if (NULL == info.pfile)
					{
						info.messages[info.cur_worker_index].clear();
					}
					else
					{
						if (info.file_cur_size < info.file_max_size)
						{
							size_t iwrite = fwrite(info.messages[info.cur_worker_index].front().buffer, info.messages[info.cur_worker_index].front().size, 1, info.pfile);
							if (iwrite == 1)
							{
								info.file_cur_size += info.messages[info.cur_worker_index].front().size;
							}
							info.messages[info.cur_worker_index].pop_front();
						}
						else
						{
							fclose(info.pfile);
							info.pfile = NULL;
						}
					}
				}
			}
		}
		if (bsleep)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}
	}
}

void log_manager::gen_file_path(void * pout, size_t out_size, const char * pfile_dir, const char * pfile_prefix, const char * pfile_suffix, const time_t & t)
{
    assert(pout && out_size);
	tm tout;
#ifdef PSL_WINDOWS
    localtime_s(&tout, &t);
#else
	localtime_r(&t, &tout);
#endif
	char date_string[15];
    snprintf(date_string, sizeof(date_string), "%04d%02d%02d%02d%02d%02d", tout.tm_year + 1900, tout.tm_mon + 1, tout.tm_mday, tout.tm_hour, tout.tm_min, tout.tm_sec);
    snprintf((char*)pout, out_size, "%s%s%s%s", pfile_dir, pfile_prefix, date_string, pfile_suffix);
}
