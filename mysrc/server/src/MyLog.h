#ifndef PSL_MYLOG_H
#define PSL_MYLOG_H

#include <stdio.h>
#include <time.h>
#include <inttypes.h>
#include <string>
#include <sstream>
#include <stdexcept>
#include <mutex>
#include <type_traits>

#ifdef MY_LOG_A
#error "macro have already defined"
#endif

#ifdef MY_LOG_B
#error "macro have already defined"
#endif

#ifdef MY_LOG_OP
#error "macro have already defined"
#endif

#ifdef MYLOG
#error "macro have already defined"
#endif

#define LOGERROR 0x01
#define LOGINFO 0x02

class MyLog
{
public:
	MyLog():MY_LOG_A(*this), MY_LOG_B(*this)
	{
		m_level = static_cast<unsigned int>(-1);
		m_file = nullptr;
		m_maxSize = 1024 * 1024 * 10;
	}
	void init(const char * filePrefix, const char * fileSuffix, const char * fileDir, size_t maxSize)
	{
		if (nullptr != filePrefix)
		{
			m_filePrefix.assign(filePrefix);
		}
		if (nullptr != fileSuffix)
		{
			m_fileSuffix.assign(fileSuffix);
		}
		if (nullptr != fileDir)
		{
			m_fileDir.assign(fileDir);
			if (!m_fileDir.empty())
			{
				if (m_fileDir.back() != '/')
				{
					m_fileDir.push_back('/');
				}
			}
		}
		if (maxSize > (1024 * 1024))
		{
			m_maxSize = maxSize;
		}
	}
	inline unsigned int getLevel()
	{
		return m_level;
	}
	void setLevel(unsigned int level)
	{
		m_level = level;
	}
	MyLog & capture(const char * file, int line, int level)
	{
		time_t now;
		time(&now);
		if (m_file)
		{
			if (m_curSize >= m_maxSize)
			{
				fflush(m_file);
				fclose(m_file);
				m_file = nullptr;
			}
		}
		if (nullptr == m_file)
		{
			m_ostrstream.str("");
			time(&m_createTime);
			m_curSize = 0;
			m_ostrstream << m_fileDir << m_filePrefix << timeToInt(m_createTime) << m_fileSuffix;
			
#ifdef PSL_WINDOWS
			m_file = _fsopen(m_ostrstream.str().c_str(), "a", _SH_DENYWR);
#else
			m_file = fopen(m_ostrstream.str().c_str(), "a");
#endif
		}
		if (nullptr == m_file)
		{
			return *this;
		}
		m_ostrstream.str("");
		m_ostrstream << "[" << timeToInt(now) << "]" << "[" << getLevelStr(level) << "]" << "[" << file << ":" << line << "]";
		std::string info = m_ostrstream.str();
		size_t iwrite = fwrite(&(info[0]), info.size(), 1, m_file);
		if (1 == iwrite)
		{
			m_curSize += info.size();
		}
		else
		{
			fflush(m_file);
			fclose(m_file);
			m_file = nullptr;
		}
		return *this;
	}
	template<typename T>
	MyLog & capture(const char * name, const T & value)
	{
		if (nullptr == m_file)
		{
			return *this;
		}
		m_ostrstream.str("");
		m_ostrstream << "{" << name << ":" << value << "}";
		std::string info = m_ostrstream.str();
		size_t iwrite = fwrite(&(info[0]), info.size(), 1, m_file);
		if (1 == iwrite)
		{
			m_curSize += info.size();
		}
		else
		{
			fflush(m_file);
			fclose(m_file);
			m_file = nullptr;
		}
		return *this;
	}
	~MyLog()
	{
		if (m_file)
		{
			fflush(m_file);
			fclose(m_file);
			m_file = nullptr;
		}
	}
	class lockguard
	{
	public:
		lockguard(MyLog & ml):m_plog(&ml)
		{
			m_plog->getMutex().lock();
		}
		lockguard(lockguard && other)
		{
			m_plog = other.m_plog;
			other.m_plog = nullptr;
		}
		~lockguard()
		{
			if (m_plog)
			{
				m_plog->newLine();
				m_plog->getMutex().unlock();
			}
		}
		operator bool()
		{
			return true;
		}
	private:
		lockguard(const lockguard &) = delete;
		lockguard & operator =(const lockguard&) = delete;
	private:
		MyLog * m_plog;
	};
public:
	MyLog & MY_LOG_A;
	MyLog & MY_LOG_B;
protected:
	std::mutex & getMutex()
	{
		return m_mutex;
	}
	void newLine()
	{
		if (m_file)
		{
			char nline[] = "\n";
			size_t iwrite = fwrite(nline, sizeof(nline) - 1, 1, m_file);
			if (iwrite == 1)
			{
				m_curSize += (sizeof(nline) - 1);
				fflush(m_file);
			}
			else
			{
				fflush(m_file);
				fclose(m_file);
				m_file = nullptr;
			}
		}
	}
	uint64_t timeToInt(const time_t & t)
	{
		tm tout;
#ifdef PSL_WINDOWS
		localtime_s(&tout, &t);
#else
		localtime_r(&t, &tout);
#endif
		uint64_t ret = static_cast<uint64_t>(tout.tm_year + 1900)*10000000000 + static_cast<uint64_t>(tout.tm_mon + 1) * 100000000 + static_cast<uint64_t>(tout.tm_mday) * 1000000 + static_cast<uint64_t>(tout.tm_hour) * 10000 + static_cast<uint64_t>(tout.tm_min) * 100 + static_cast<uint64_t>(tout.tm_sec);
		return ret;
	}
	const char * getLevelStr(int level)
	{
		switch (level)
		{
		case LOGERROR:
			return "ERROR";
		case LOGINFO:
			return "INFO";
		default:
			return "NULL";
		}
	}
private:
	MyLog(const MyLog &) = delete;
	MyLog & operator = (const MyLog &) = delete;
private:
	std::mutex m_mutex;
	unsigned int m_level;
	std::ostringstream m_ostrstream;
	FILE * m_file;
	std::string m_filePrefix;
	std::string m_fileSuffix;
	std::string m_fileDir;
	size_t m_maxSize;
	size_t m_curSize;
	time_t m_createTime;
};

#define MY_LOG_A(x) MY_LOG_OP(x,B)
#define MY_LOG_B(x) MY_LOG_OP(x,A)
#define MY_LOG_OP(x,next) MY_LOG_A.capture((#x),(x)).MY_LOG_##next
#define MYLOG(expr1,expr2) static_assert(std::is_same<decltype(expr1), MyLog>::value, #expr1" is not MyLog object"); static_assert(((expr2)==LOGERROR || (expr2)== LOGINFO), #expr2" is not defined"); if((expr2)&(expr1).getLevel()) if(MyLog::lockguard tmp = (expr1)) (expr1).capture(__FILE__, __LINE__, (expr2)).MY_LOG_A

#endif
