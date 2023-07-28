# myLog
我自己写的一个日志库 ，兼容 windows，linux

## 异步日志，通过使用一个专门的线程来处理日志写入，避免阻塞主线程

* 定义一个队列，用于保存日志消息
* 定义一个队列, 用于保存日志文件，保存最新7天的日志
* 定义一个单独的线程，该线程循环等待队列中是否有新的日志消息，如果有则将其写入日志文件
* 主线程向队列中添加日志消息时，直接将消息放入队列即可

```
#include <iostream>
#include "Logger.h"
void fun1()
{
	int t = 0;
	while(true)
	{	
		LOG_INFO("%d", t++);
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
}
void fun2()
{
	int t = 0;
	while (true)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10000));
		LOG_ERROR("-----%d", t++);
	}
}
int main()
{
	std::thread t1(fun1);
	std::thread t2(fun2);
	t1.join();
	t2.join();
}
```



