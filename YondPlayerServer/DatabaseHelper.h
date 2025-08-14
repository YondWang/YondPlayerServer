#pragma once
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
	virtual int IsConnect() = 0;
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
	virtual Buffer Delete() = 0;
	virtual Buffer Modify() = 0;	//TODO:optimize args
	virtual Buffer Query() = 0;
	//creat a class base on table
	virtual PTable Copy() const = 0;
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
	virtual ~_Field_() {}
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
};