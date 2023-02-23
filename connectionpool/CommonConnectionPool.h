#pragma once

#include "public.h"
#include "connection.h"
#include <queue>
#include <mutex>
#include <thread>
#include <functional>
#include <condition_variable>
// Andy: 懒汉式单例：不定义静态局部变量，直接返回
class ConnectioPool
{
public:
	static ConnectioPool* getConnectionPool()	// Andy2 : 1 单例模式构造 
	{
		static ConnectioPool pool;  // 属于全局作用域，所以要分开，
		return &pool;
	}

	// Andy4: 给客户提供接口，从连接池中获取一个可用的空闲链接
	shared_ptr<Connection> getConnection()
	{
		unique_lock<mutex> lock(queueMutex_);
		// Andy4: wait_for 返回值是枚举类型: 
		// 如果当前连接池是空，那就等100毫秒
		while (connectionQue_.empty())
		{
			// sleep  // names for wait returns
			// 等待100ms，如果等待过程中被wait了，那就继续执行，判断到while() 不为空，就跳出while了
			if (cv_status::timeout == cond_.wait_for(lock, chrono::milliseconds(connectionTimeout_)))
			{
				if (connectionQue_.empty())
				{
					LOG("获取空闲连接超时了...获取连接失败!");
					return nullptr;
				}
			}
		}

		// Andy4 : 需要给智能指针重新定义删除器 
		shared_ptr<Connection> sp(connectionQue_.front(),
			[&](Connection *pcon) {
				unique_lock<mutex> lock(queueMutex_);
				pcon->refreshAliveTime();			// Andy5: 刷新一下开始空闲时间
				connectionQue_.push(pcon);
			});

		connectionQue_.pop();
		cond_.notify_all();
		return sp;
	}

private:

	// Andy2 : 1 单例模式构造 
	ConnectioPool()
	{
		if (!loadConfigFile())	// Andy 已经加载配置项了,直接返回
		{
			return;
		}

		// Andy 创建初始数量够的链接
		// bool connect(string ip, unsigned short port, string username, string password, string dbname)
		for (int i = 0; i < initSize_; ++i)
		{
			Connection* p = new Connection();
			p->connect(ip_, port_, username_, password_, dbname_);
			p->refreshAliveTime();
			connectionQue_.push(p);
			connectionCnt_++;
		}

		// 启动一个新线程，作为链接的生产者 
		thread produre(std::bind(&ConnectioPool::procedureConnectionTask, this));
		produre.detach();

		// Andy5: 启动一个新的线程，扫描超过maxIdleTime时间的空闲时间，进行连接回收
		thread scanner(std::bind(&ConnectioPool::scannerConnectionTask, this));
		scanner.detach();
	}

	bool loadConfigFile()			// Andy2 从配置文件中加载配置项
	{
		FILE* fp = fopen("mysql.ini", "r");
		if (fp == nullptr)
		{
			LOG("fopen err");
			return false;
		}

		while (!feof(fp))
		{
			char line[1024] = { 0 };
			fgets(line, 1024, fp);
			string str = line;				// Andy2转为字符串，方便处理
			int idx = str.find('=', 0);     // 从0开始，找到 = 的索引位置
			if (idx == -1)
			{
				continue;
			}

			int endidx = str.find('\n', idx);  // 找到该行最后一个位置
			string key = str.substr(0, idx);   // 截取从0开始的截取到等号位置个元素，=位置索引值正好是key个数
			string value = str.substr(idx + 1, endidx - idx - 1);			// 从=后一个位置，到\n前一个位置

			// cout << "key = " << key << "value = " << value << endl;
			if (key == "ip")
			{
				ip_ = value;
			}
			else if (key == "port")
			{
				port_ = atoi(value.c_str());
			}
			else if (key == "username")
			{
				username_ = value;
			}
			else if (key == "password")
			{
				password_ = value;
			}
			else if (key == "dbname")
			{
				dbname_ = value;
			}
			else if (key == "initSize")
			{
				initSize_ = atoi(value.c_str());
			}
			else if (key == "maxSize")
			{
				maxSize_ = atoi(value.c_str());
			}
			else if (key == "maxIdleTime")
			{
				maxIdleTime_ = atoi(value.c_str());
			}
			else if (key == "connectionTimeOut")
			{
				connectionTimeout_ = atoi(value.c_str());
			}
		}

		return true;
	}

	void scannerConnectionTask()
	{
		for (;;)
		{
			// Andy5: 模拟maxTime时间, 在这个时间内，新添加的链接没有被使用，将被回收掉
			this_thread::sleep_for(chrono::seconds(maxIdleTime_)); 

			// Andy5: 扫描这个队列，释放多余的链接
			unique_lock<mutex> lock(queueMutex_);
			while (connectionCnt_ > initSize_)
			{
				Connection* p = connectionQue_.front();
				if (p->getAliveeTime() >= (maxIdleTime_ * 1000))
				{
					connectionQue_.pop();
					connectionCnt_--;
					delete p;
				}
				else
				{
					break;
				}
			}
		}
	}

	// Andy3: 运行在独立的线程中，专门负责生产新链接
	void procedureConnectionTask()
	{
		while (1)
		{
			unique_lock<mutex> lock(queueMutex_);
			while (!connectionQue_.empty())  // Andy3: !connectionQue_.empty() 不空的判断，不空就等待
			{
				cond_.wait(lock);
			}

			// 如果链接数量是空

			if (connectionCnt_ < maxSize_)
			{
				Connection* p = new Connection();
				p->connect(ip_, port_, username_, password_, dbname_);
				p->refreshAliveTime();
				connectionQue_.push(p);
				connectionCnt_++;
			}

			cond_.notify_all();
		}
	
	}
private:

	string ip_;					// mysql ip 
	unsigned short port_;
	string username_;
	string password_;
	string dbname_;
	int initSize_;			// 连接池的初始连接量
	int maxSize_;
	int maxIdleTime_;
	int connectionTimeout_;

	queue<Connection*> connectionQue_;		// Andy2 存储mysql 连接队列
	mutex	queueMutex_;					// Andy2 维护连接队列的线程安全互斥锁
	atomic_int connectionCnt_;				// Andy3: 链接数量

	condition_variable cond_;
};
