#pragma once

#include <stdarg.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <fstream>
#include <string>
#include <ctime>



#ifdef _WIN32
    #include <direct.h> 
    #include <io.h>                      //C (Windows)    _access
#else
    #include <dirent.h>
    #include <unistd.h>             //C (Linux)      access  
    #include <sys/stat.h>
    #include <sys/types.h>
#endif


#define LOG_INFO( fmt, ...) \
	 Logger::GetInstance()->log(__FILE__, __func__, __LINE__,LOG_LEVEL::INFO, fmt,__VA_ARGS__)
#define LOG_ERROR( fmt, ...) \
	 Logger::GetInstance()->log(__FILE__, __func__, __LINE__,LOG_LEVEL::ERROR, fmt,__VA_ARGS__)

struct TimeFormat
{
	std::string date;
	std::string date_time;
	std::string one_week_ago;
};

enum LOG_LEVEL
{
	INFO,
	WARN,
	DEBUG,
	ERROR
};
static inline std::string enum_str(enum LOG_LEVEL c) {

}



class Logger
{
public:
	~Logger();
	Logger(Logger &other) = delete;
	void operator=(const Logger &other) = delete;
	static Logger* GetInstance();
	void log(const char *file_name, const char *func_name, int line, LOG_LEVEL lev,const char * fmt, ...);

private:
	Logger();
	static std::mutex m_mutex;
	static Logger* logger;

private:
	std::mutex t_mutex;
	std::condition_variable m_cond;
	std::queue<std::string> m_queue;
	std::queue<std::string> file_queue;
	std::thread m_thread;
	bool m_running;	
	std::ofstream m_file;
	std::string base_file;
	std::string file_name;

private:
	std::string enum_str(enum LOG_LEVEL i);
	void wirte();
	void getTime(struct TimeFormat&);

	void CreatDir();
	void GetDirFiles();
	void InsertFile(const char * path);
	bool isCreatFile();
	void removeFile(const char * path);


};



