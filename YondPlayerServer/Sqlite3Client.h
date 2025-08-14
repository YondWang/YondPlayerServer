#pragma once
#include "public.h"
#include "DatabaseHelper.h"
#include "sqlite3/sqlite3.h"

class CSqlite3Client
	:public CDatabaseClient
{
public:
	CSqlite3Client(const CSqlite3Client&) = delete;
	CSqlite3Client& operator=(const CSqlite3Client&) = delete;
public:
	CSqlite3Client() {
		m_db = NULL;
		m_stmt = NULL;
	}
	virtual ~CSqlite3Client() {
		Close();
	}

public:
	//连接
	virtual int Connect(const KeyValue& args);
	//执行
	virtual int Exec(const Buffer& sql);
	//带结果执行
	virtual int Exec(const Buffer sql, Result& result, const _Table_& table);
	//开启事务
	virtual int StartTransaction();
	//提交事务
	virtual int CommitTransaction();
	//回滚事务
	virtual int RollbackTransaction();
	//关闭连接
	virtual int Close() {

	}
	//是否连接
	virtual int IsConnect();
private:
	static int  ExecCallback(void* arg, int count, char** names, char** values);
	int ExecCallback(Result& result, const _Table_& table, int count, char** names, char** values);
private:
	sqlite3_stmt* m_stmt;
	sqlite3* m_db;
private:
	class ExecParam {
	public:
		ExecParam(CSqlite3Client* obj, Result& result, const _Table_& table) 
			:obj(obj), result(result), table(table)
		{

		}
		CSqlite3Client* obj;
		Result& result;
		const _Table_& table;
	};
};

