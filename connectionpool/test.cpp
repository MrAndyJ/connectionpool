#include <iostream>
#include <thread>
#include "connection.h"
#include "CommonConnectionPool.h"
#include "public.h"
using namespace std;

void test1()
{
	cout << "不使用连接池" << endl;
	clock_t begin = clock();
	for (int i = 0; i < 2000; i++)
	{
		Connection conn;
		char sql[1024] = { 0 };
		sprintf(sql, "insert into user(name, age, sex) values('%s', %d, '%s')", "zhangsan", 20, "W");
		conn.connect("127.0.0.1", 3306, "root", "123000", "link");
		conn.update(sql);
	}
	clock_t end = clock();
	cout << end - begin << "	ms" << endl;
}

void test2()
{
	cout << "使用连接池" << endl;

	ConnectioPool* p = ConnectioPool::getConnectionPool();
	clock_t begin = clock();
	for (int i = 0; i < 2000; i++) 
	{
		shared_ptr<Connection> sp = p->getConnection();
		char sql[1024] = { 0 };
		sprintf(sql, "insert into user(name, age, sex) values('%s', %d, '%s')", "zhangsan", 20, "W");
		// conn.connect("127.0.0.1", 3306, "root", "123000", "link");
		sp->update(sql);
	}
	clock_t end = clock();
	cout << end - begin << "	ms" << endl;
}

void test3()
{
	clock_t begin = clock();

	thread t1([]()->void {
		ConnectioPool* cp = ConnectioPool::getConnectionPool();
		for (int i = 0; i < 2500; ++i)
		{
			char sql[1024] = { 0 };
			sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')", "lisi", 21, "W");
			shared_ptr<Connection> sp = cp->getConnection();
			sp->update(sql);
		}
	});

	thread t2([]()->void {
		ConnectioPool* cp = ConnectioPool::getConnectionPool();
		for (int i = 0; i < 2500; ++i)
		{
			char sql[1024] = { 0 };
			sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')", "lisi", 21, "W");
			shared_ptr<Connection> sp = cp->getConnection();
			sp->update(sql);
		}
	});

	thread t3([]()->void {
		ConnectioPool* cp = ConnectioPool::getConnectionPool();
		for (int i = 0; i < 2500; ++i)
		{
			char sql[1024] = { 0 };
			sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')", "lisi", 21, "W");
			shared_ptr<Connection> sp = cp->getConnection();
			sp->update(sql);
		}
	});

	thread t4([]()->void {
		ConnectioPool* cp = ConnectioPool::getConnectionPool();
		for (int i = 0; i < 2500; ++i)
		{
			char sql[1024] = { 0 };
			sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')", "lisi", 21, "W");
			shared_ptr<Connection> sp = cp->getConnection();
			sp->update(sql);
		}
		});

	t1.join();
	t2.join();
	t3.join();
	t4.join();


	clock_t end = clock();

	cout << end-begin   << endl;

}

void test4()
{
	Connection conn;
	conn.connect("127.0.0.1", 3306, "root", "123000", "link");
	clock_t begin = clock();

	thread t1([]() {
		for (int i = 0; i < 2500; ++i)
		{
			Connection conn;
			char sql[1024] = { 0 };
			sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')",
				"zhang san", 20, "W");
			conn.connect("127.0.0.1", 3306, "root", "123000", "link");
			conn.update(sql);
		}

		});

	thread t2([]() {
		for (int i = 0; i < 2500; ++i)
		{
			Connection conn;
			char sql[1024] = { 0 };
			sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')",
				"zhang san", 20, "M");
			conn.connect("127.0.0.1", 3306, "root", "123000", "link");
			conn.update(sql);
		}

		});

	thread t3([]() {
		for (int i = 0; i < 2500; ++i)
		{
			Connection conn;
			char sql[1024] = { 0 };
			sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')",
				"zhang san", 20, "M");
			conn.connect("127.0.0.1", 3306, "root", "123000", "link");
			conn.update(sql);
		}

		});

	thread t4([]() {
		for (int i = 0; i < 2500; ++i)
		{
			Connection conn;
			char sql[1024] = { 0 };
			sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')",
				"zhang san", 20, "M");
			conn.connect("127.0.0.1", 3306, "root", "123000", "link");
			conn.update(sql);
		}

		});


	t1.join();
	t2.join();
	t3.join();
	t4.join();


	clock_t end = clock();

	cout << end - begin << endl;
}


int main()
{

	// ConnectioPool* p = ConnectioPool::getConnectionPool();	
	test3();
	//ConnectioPool* p2 = ConnectioPool::getConnectionPool();

	//Connection conn;
	//char sql[1024] = { 0 };
	//sprintf(sql, "insert into user(name, age, sex) values('%s', %d, '%s')","zhangsan", 20, "W");
	//conn.connect("127.0.0.1", 3306, "root", "123000", "link");
	//conn.update(sql);

	/*
	clock_t begin = clock();
	for (int i = 0; i < 1000; i++)
	{
		Connection conn;
		char sql[1024] = { 0 };
		sprintf(sql, "insert into user(name, age, sex) values('%s', %d, '%s')", "zhangsan", 20, "W");
		conn.connect("127.0.0.1", 3306, "root", "123000", "link");
		conn.update(sql);
	}
	clock_t end = clock();
	cout << end - begin << "	ms" << endl;
	*/
	return 1;
}