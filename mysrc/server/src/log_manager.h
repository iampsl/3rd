#ifndef PSL_LOG_MANAGER_H_
#define PSL_LOG_MANAGER_H_

#include <stdio.h>
#include <time.h>
#include <deque>
#include <vector>
#include <memory>
#include <thread>
#include <mutex>

#define LOG_INFO 0
#define LOG_TOTAL 1

int get_mmddhhmmss(const time_t & t);

class log_manager
{
public:
	struct message_data
	{
		uint8_t buffer[512];
		unsigned int size;
	};
public:
	static log_manager & get_instance();
	void set_log_info(int whichlog, const char * pfile_prefix, const char * pfile_suffix, const char * pfile_dir, size_t max_size);
	void start();
	void output_message(int whichlog, const message_data & msg);
	void stop();
private:
	void work_thread();
	static void gen_file_path(void * pout, size_t out_size, const char * pfile_dir, const char * pfile_prefix, const char * pfile_suffix, const time_t & t);
private:
	log_manager();
	log_manager(const log_manager & other) = delete;
	log_manager & operator = (const log_manager & other) = delete;
	~log_manager();
private:
	struct log_info 
	{
		char file_name_prefix[64];
		char file_name_suffix[64];
		char file_save_directory[1024];
		size_t file_max_size;
		size_t file_cur_size;
		time_t create_time;
		FILE * pfile;
		std::deque<message_data> messages[2];
		size_t cur_worker_index;
		std::mutex messages_mutex;
		bool bstart;
		size_t cur_insert_index;
	};
	static_assert(LOG_TOTAL != 0, "");
private:
		log_info m_logs[LOG_TOTAL];
		std::shared_ptr<std::thread>  m_pthread;
};



#define  LOG_MANAGER_OUTPUT(which_log, format_string, ...) \
{\
    log_manager::message_data msg; \
    msg.size = (unsigned)(sizeof(msg.buffer)-1); \
    int iprintf = snprintf((char*)(msg.buffer), sizeof(msg.buffer), format_string, ##__VA_ARGS__); \
    if (iprintf > 0)\
    {\
        if ((size_t)iprintf < sizeof(msg.buffer))\
        {\
            msg.size = (unsigned)iprintf; \
        }\
        log_manager::get_instance().output_message(which_log, msg); \
    }\
}


#endif

