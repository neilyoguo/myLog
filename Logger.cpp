#include "Logger.h"

#pragma warning(disable: 4996)

Logger* Logger::logger = nullptr;
std::mutex Logger::m_mutex;

Logger* Logger:: GetInstance()
{
	m_mutex.lock();
	if (logger == nullptr)
	{
		logger = new Logger();
	}
	m_mutex.unlock();
	return logger;
}
Logger::Logger():base_file("logs/log_"),m_running(true)
{
	CreatDir();
	GetDirFiles();
	TimeFormat t_fmt;
	getTime(t_fmt);
	file_name = base_file + t_fmt.date;
	m_file.open(file_name, std::ios::out | std::ios::app);
	if (!m_file)
		return;
	InsertFile(file_name.c_str());
	m_thread = std::thread(&Logger::wirte, this);
}
Logger::~Logger()
{
	m_running = false;
	m_thread.join();
	if(m_file)
		m_file.close();

}
std::string Logger::enum_str(enum LOG_LEVEL i)
{
	std::string str[] = { "INFO", "WARN", "DEBUG", "ERROR", };
	return str[i];
}

void Logger::log(const char *file_name, const char *func_name, int line, LOG_LEVEL lev, const char * fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	char buffer[256],tmp[1024];
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);
	TimeFormat t_fmt;
	getTime(t_fmt);
	sprintf(tmp, "%s:%s;%s;%d;[%s]:%s", \
		t_fmt.date_time.c_str() ,file_name, func_name, line,enum_str(lev).c_str(), buffer);
	t_mutex.lock();
	m_queue.push(tmp);
	t_mutex.unlock();
	m_cond.notify_one();

}
void Logger::wirte()
{

	while (m_running) 
	{
		if (m_file)
		{
			std::unique_lock<std::mutex> lock(t_mutex);
			if (m_queue.empty())
			{
				m_cond.wait(lock);
			}
			while (!m_queue.empty())
			{
				const auto& message = m_queue.front();
				bool ret = isCreatFile();
				if (ret)
				{
					m_file << message << std::endl;
				}
				m_queue.pop();
			}
		}
	}
	return;
}

void Logger::getTime(struct TimeFormat& time_fmt)
{
	time_t now = time(0);
	struct tm *stm = localtime(&now);
	char tmp[128];
	sprintf(tmp, "%04d-%02d-%02d %02d:%02d:%02d", \
		1900 + stm->tm_year, 1 + stm->tm_mon, stm->tm_mday, stm->tm_hour, stm->tm_min, stm->tm_sec);
	time_fmt.date_time = tmp;
	sprintf(tmp, "%04d%02d%02d", \
		1900 + stm->tm_year, 1 + stm->tm_mon, stm->tm_mday);
	time_fmt.date = tmp;

	time_t ago = now - 7 * 24 * 60 * 60;
	stm = localtime(&ago);
	sprintf(tmp, "%04d%02d%02d", \
		1900 + stm->tm_year, 1 + stm->tm_mon, stm->tm_mday);
	time_fmt.one_week_ago = tmp;
}

void Logger::CreatDir()
{
#ifdef _WIN32
	mkdir("logs");
#else
	mkdir("logs",S_IRUSR | S_IWUSR | S_IXUSR | S_IRWXG | S_IRWXO);
#endif
}
void Logger::GetDirFiles()
{
#ifdef _WIN32
	long handle;  
	struct _finddata_t fileinfo; 
	std::string search_file = base_file + "*";
	handle = _findfirst(search_file.c_str(), &fileinfo);
	if (-1 == handle)
		return;
	file_queue.push(fileinfo.name);
	while (!_findnext(handle, &fileinfo)) 
	{
		file_queue.push(fileinfo.name);
	}
	_findclose(handle); 
#else
	DIR *dirp = nullptr;
    struct dirent *fileinfo = nullptr;
    std::string str;
    dirp = opendir("logs");
	if(dirp == nullptr)
	{
        return;
    }
    while((fileinfo = readdir(dirp)) != NULL)
	{
		file_queue.push(fileinfo->d_name);              
    }
    closedir(dirp);

#endif

}

void Logger::InsertFile(const char * path)
{
	file_queue.push(path);
	unsigned int one_week = 7;
	if (file_queue.size() > one_week)
	{
		const auto& str = file_queue.front();
		removeFile(str.c_str());
		file_queue.pop();
	}
}

void Logger::removeFile(const char * path)
{

	int ret = 0;
#ifdef _WIN32
	ret = _access(path, 0);
#else
	ret = access(path, 0);
#endif
	if (ret == 0)
	{
		remove(path);
	}
}
bool Logger::isCreatFile()
{
	static int front_date = 0;
	int current_date = 0;
	time_t now = time(0);
	struct tm *stm = localtime(&now);
	current_date = stm->tm_mday;

	if ( (current_date == front_date + 1) || (current_date < front_date) )
	{
		m_file.close();

		TimeFormat t_fmt;
		getTime(t_fmt);
		file_name = base_file + t_fmt.date;
		m_file.open(file_name, std::ios::out | std::ios::app);
		if (!m_file)
			return false;
		InsertFile(file_name.c_str());
		front_date = current_date;
		return true;
	}
	return true;
}
