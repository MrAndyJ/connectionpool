#pragma once

#include <mysql.h>
#include <string>
#include "public.h"
#include <iostream>
#include <mysql.h>
#include <ctime>
using namespace std;

class Connection
{
public:
	//Andy1 ��ʼ�����ݿ�����
	Connection()
	{
		_conn = mysql_init(nullptr);  // 
	}

	// 	//Andy1 �ر�����
	~Connection()
	{
		if (_conn != nullptr)
		{
			mysql_close(_conn);
		}

	}
	//Andy1 �������ݿ�
	bool connect(string ip, unsigned short port, string username, string password, string dbname)
	{
		MYSQL* p = mysql_real_connect(_conn, ip.c_str(), username.c_str(), password.c_str(), dbname.c_str(), port, nullptr, 0);
		return p != nullptr;
	}

	//Andy1  ���²��� insert��delete��update
	bool update(string sql)
	{
		if (mysql_query(_conn, sql.c_str()))
		{
			LOG("����ʧ��" + sql);
			cout << mysql_error(_conn) << endl;
			return false;
		}
		return true;
	}
	// ��ѯ���� select
	MYSQL_RES* query(string sql)
	{
		if (mysql_query(_conn, sql.c_str()))
		{
			LOG("��ѯʧ��" + sql);
			return nullptr;
		}
		return mysql_use_result(_conn);
	}

	// Andy5: ˢ��һ�����ӵ���ʼ�Ŀ���ʱ���
	void refreshAliveTime() { _alivetime = clock(); }
	// ���ش���ʱ��
	clock_t getAliveeTime()const { return clock() - _alivetime; }

private:
	MYSQL* _conn;
	clock_t _alivetime; // ��¼�������״̬�����ʼ���ʱ��
};