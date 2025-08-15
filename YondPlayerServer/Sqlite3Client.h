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
	virtual int Close();
	//是否连接 true连接中 false未连接
	virtual bool IsConnect();
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

class _sqlite3_table_ :
	public _Table_
{
public:
	_sqlite3_table_() : _Table_() {}
	_sqlite3_table_(const _sqlite3_table_& table);
	virtual ~_sqlite3_table_() {}
	//返回创建的sql语句
	virtual Buffer Creat();
	//delete
	virtual Buffer Drop();
	//crud
	virtual Buffer Insert(const _Table_& values);
	virtual Buffer Delete(const _Table_& values);
	virtual Buffer Modify(const _Table_& values);	//TODO:optimize args
	virtual Buffer Query();
	//creat a class base on table
	virtual PTable Copy() const;
	virtual void ClearFieldUsed();
public:
	//get table full name
	virtual operator const Buffer() const;
};



class _sqlite3_field_ :
	public _Field_ 
{
public:
	_sqlite3_field_();
	virtual ~_sqlite3_field_() {}
	virtual Buffer Craete();
	virtual void LoadFromStr(const Buffer& str);
	//use for where sql
	virtual Buffer toEqualExp() const;
	virtual Buffer toSqlString() const;
	//列的全名
	virtual operator const Buffer() const;
private:
	Buffer Str2Hex(const Buffer& data) const;
	union {
		bool Bool;
		int Integer;
		double Double;
		Buffer* String;
	}Value;
	int nType;
};