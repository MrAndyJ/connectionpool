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
	//Andy1 初始化数据库连接
	Connection()
	{
		_conn = mysql_init(nullptr);  // 
	}

	// 	//Andy1 关闭链接
	~Connection()
	{
		if (_conn != nullptr)
		{
			mysql_close(_conn);
		}

	}
	//Andy1 连接数据库
	bool connect(string ip, unsigned short port, string username, string password, string dbname)
	{
		MYSQL* p = mysql_real_connect(_conn, ip.c_str(), username.c_str(), password.c_str(), dbname.c_str(), port, nullptr, 0);
		return p != nullptr;
	}

	//Andy1  更新操作 insert、delete、update
	bool update(string sql)
	{
		if (mysql_query(_conn, sql.c_str()))
		{
			LOG("更新失败" + sql);
			cout << mysql_error(_conn) << endl;
			return false;
		}
		return true;
	}
	// 查询操作 select
	MYSQL_RES* query(string sql)
	{
		if (mysql_query(_conn, sql.c_str()))
		{
			LOG("查询失败" + sql);
			return nullptr;
		}
		return mysql_use_result(_conn);
	}

	// Andy5: 刷新一下链接的起始的空闲时间点
	void refreshAliveTime() { _alivetime = clock(); }
	// 返回存活的时间
	clock_t getAliveeTime()const { return clock() - _alivetime; }

private:
	MYSQL* _conn;
	clock_t _alivetime; // 记录进入空闲状态后的起始存活时间
};