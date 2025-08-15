﻿#pragma once
#include "public.h"
#include <map>
#include <list>
#include <memory>
#include <vector>

class _Table_;
class _Field_;

using PTable = std::shared_ptr<_Table_>;
using PField = std::shared_ptr<_Field_>;

using KeyValue = std::map<Buffer, Buffer>;
using Result = std::list<PTable>;
using FieldArray = std::vector<PField>;
using FieldMap = std::map<Buffer, PField>;

class CDatabaseClient
{
public:
	CDatabaseClient(const CDatabaseClient&) = delete;
	CDatabaseClient& operator=(const CDatabaseClient&) = delete;
public:
	CDatabaseClient() {}
	virtual ~CDatabaseClient(){}

public:
	//连接
	virtual int Connect(const KeyValue& args) = 0;
	//执行
	virtual int Exec(const Buffer& sql) = 0;
	//带结果执行
	virtual int Exec(const Buffer sql, Result& result, const _Table_& table) = 0;
	//开启事务
	virtual int StartTransaction() = 0;
	//提交事务
	virtual int CommitTransaction() = 0;
	//回滚事务
	virtual int RollbackTransaction() = 0;
	//关闭连接
	virtual int Close() = 0;
	//是否连接
	virtual bool IsConnect() = 0;
};

class _Table_ {
public:
	_Table_() {}
	virtual ~_Table_() {}
	//返回创建的sql语句
	virtual Buffer Creat() = 0;
	//delete
	virtual Buffer Drop() = 0;
	//crud
	virtual Buffer Insert(const _Table_& values) = 0;
	virtual Buffer Delete(const _Table_& values) = 0;
	virtual Buffer Modify(const _Table_& values) = 0;	//TODO:optimize args
	virtual Buffer Query() = 0;
	//creat a class base on table
	virtual PTable Copy() const = 0;
	virtual void ClearFieldUsed() = 0;
public:
	//get table full name
	virtual operator const Buffer() const = 0;
public:
	//db name
	Buffer Database;
	Buffer Name;
	FieldArray FieldDefine;		//save result
	FieldMap Fields;		//列定义的映射表

};

enum {
	SQL_INSERT = 1,		//插入的列
	SQL_MODIFY = 2,		//修改的列
	SQL_CONDITION = 4,	//查询的条件
};

enum {
	NOT_NULL = 1,
	DEFAULT = 2,
	UNIQUE = 4,
	PRIMARY_KEY = 8,
	CHECK = 16,
	AUTOINCREMENT = 32
};

using SqlType = enum {
	TYPE_NULL = 0,
	TYPE_BOOL = 1,
	TYPE_INT = 2,
	TYPE_DATETIME = 4,
	TYPE_REAL = 8,
	TYPE_VARCHAR = 16,
	TYPE_TEXT = 32,
	TYPE_BLOB = 64
};

class _Field_ {
public:
	_Field_() {}
	_Field_(const _Field_& field) {
		Name = field.Name;
		Type = field.Type;
		Attr = field.Attr;
		Default = field.Default;
		Check = field.Check;
	}
	virtual _Field_& operator=(const _Field_& field) {
		if (this != &field) {
			Name = field.Name;
			Type = field.Type;
			Attr = field.Attr;
			Default = field.Default;
			Check = field.Check;
		}
		return *this;
	}
	~_Field_() {}
public:
	virtual Buffer Craete() = 0;
	virtual void LoadFromStr(const Buffer& str) = 0;
	//use for where sql
	virtual Buffer toEqualExp() const = 0;
	virtual Buffer toSqlString() const = 0;
	//列的全名
	virtual operator const Buffer() const = 0;
public:
	Buffer Name;
	Buffer Type;
	Buffer Size;
	unsigned Attr;
	Buffer Default;
	Buffer Check;

public:
	//操作条件
	unsigned Condition;
};