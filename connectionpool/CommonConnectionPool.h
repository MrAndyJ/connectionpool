#pragma once

#include "public.h"
#include "connection.h"
#include <queue>
#include <mutex>
#include <thread>
#include <functional>
#include <condition_variable>
// Andy: ����ʽ�����������徲̬�ֲ�������ֱ�ӷ���
class ConnectioPool
{
public:
	static ConnectioPool* getConnectionPool()	// Andy2 : 1 ����ģʽ���� 
	{
		static ConnectioPool pool;  // ����ȫ������������Ҫ�ֿ���
		return &pool;
	}

	// Andy4: ���ͻ��ṩ�ӿڣ������ӳ��л�ȡһ�����õĿ�������
	shared_ptr<Connection> getConnection()
	{
		unique_lock<mutex> lock(queueMutex_);
		// Andy4: wait_for ����ֵ��ö������: 
		// �����ǰ���ӳ��ǿգ��Ǿ͵�100����
		while (connectionQue_.empty())
		{
			// sleep  // names for wait returns
			// �ȴ�100ms������ȴ������б�wait�ˣ��Ǿͼ���ִ�У��жϵ�while() ��Ϊ�գ�������while��
			if (cv_status::timeout == cond_.wait_for(lock, chrono::milliseconds(connectionTimeout_)))
			{
				if (connectionQue_.empty())
				{
					LOG("��ȡ�������ӳ�ʱ��...��ȡ����ʧ��!");
					return nullptr;
				}
			}
		}

		// Andy4 : ��Ҫ������ָ�����¶���ɾ���� 
		shared_ptr<Connection> sp(connectionQue_.front(),
			[&](Connection *pcon) {
				unique_lock<mutex> lock(queueMutex_);
				pcon->refreshAliveTime();			// Andy5: ˢ��һ�¿�ʼ����ʱ��
				connectionQue_.push(pcon);
			});

		connectionQue_.pop();
		cond_.notify_all();
		return sp;
	}

private:

	// Andy2 : 1 ����ģʽ���� 
	ConnectioPool()
	{
		if (!loadConfigFile())	// Andy �Ѿ�������������,ֱ�ӷ���
		{
			return;
		}

		// Andy ������ʼ������������
		// bool connect(string ip, unsigned short port, string username, string password, string dbname)
		for (int i = 0; i < initSize_; ++i)
		{
			Connection* p = new Connection();
			p->connect(ip_, port_, username_, password_, dbname_);
			p->refreshAliveTime();
			connectionQue_.push(p);
			connectionCnt_++;
		}

		// ����һ�����̣߳���Ϊ���ӵ������� 
		thread produre(std::bind(&ConnectioPool::procedureConnectionTask, this));
		produre.detach();

		// Andy5: ����һ���µ��̣߳�ɨ�賬��maxIdleTimeʱ��Ŀ���ʱ�䣬�������ӻ���
		thread scanner(std::bind(&ConnectioPool::scannerConnectionTask, this));
		scanner.detach();
	}

	bool loadConfigFile()			// Andy2 �������ļ��м���������
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
			string str = line;				// Andy2תΪ�ַ��������㴦��
			int idx = str.find('=', 0);     // ��0��ʼ���ҵ� = ������λ��
			if (idx == -1)
			{
				continue;
			}

			int endidx = str.find('\n', idx);  // �ҵ��������һ��λ��
			string key = str.substr(0, idx);   // ��ȡ��0��ʼ�Ľ�ȡ���Ⱥ�λ�ø�Ԫ�أ�=λ������ֵ������key����
			string value = str.substr(idx + 1, endidx - idx - 1);			// ��=��һ��λ�ã���\nǰһ��λ��

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
			// Andy5: ģ��maxTimeʱ��, �����ʱ���ڣ�����ӵ�����û�б�ʹ�ã��������յ�
			this_thread::sleep_for(chrono::seconds(maxIdleTime_)); 

			// Andy5: ɨ��������У��ͷŶ��������
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

	// Andy3: �����ڶ������߳��У�ר�Ÿ�������������
	void procedureConnectionTask()
	{
		while (1)
		{
			unique_lock<mutex> lock(queueMutex_);
			while (!connectionQue_.empty())  // Andy3: !connectionQue_.empty() ���յ��жϣ����վ͵ȴ�
			{
				cond_.wait(lock);
			}

			// ������������ǿ�

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
	int initSize_;			// ���ӳصĳ�ʼ������
	int maxSize_;
	int maxIdleTime_;
	int connectionTimeout_;

	queue<Connection*> connectionQue_;		// Andy2 �洢mysql ���Ӷ���
	mutex	queueMutex_;					// Andy2 ά�����Ӷ��е��̰߳�ȫ������
	atomic_int connectionCnt_;				// Andy3: ��������

	condition_variable cond_;
};
