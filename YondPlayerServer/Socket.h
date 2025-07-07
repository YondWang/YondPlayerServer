#pragma once
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>

class Buffer :public std::string {
public:
	Buffer() :std::string() {}
	Buffer(size_t size) :std::string() {
		resize(size);
	}
	operator char* () { return (char*)c_str(); }
	operator char* () const { return (char*)c_str(); }
	operator const char* () const { return c_str(); }

};

enum SockAttr {
	SOCK_ISSERVER = 1,	//是否服务器	1是 0客户端
	SOCK_ISBLOCK = 2,	//受否阻塞	1表示阻塞 0非阻塞
};

class CSockParam {
public:
	CSockParam() {
		bzero(&addr_in, sizeof(addr_in));
		bzero(&addr_un, sizeof(addr_un));
		port = -1;
		attr = 0;
	}
	CSockParam(const Buffer& ip, short port, int attr) {
		this->ip = ip;
		this->port = port;
		this->attr = attr;
		addr_in.sin_family = AF_INET;
		addr_in.sin_port = port;
		addr_in.sin_addr.s_addr = inet_addr(ip);
	}
	CSockParam(const Buffer& path, int attr) {
		ip = path;
		addr_un.sun_family = AF_UNIX;
		strcpy(addr_un.sun_path, path);
		this->attr = attr;
	}
	~CSockParam() {}
	CSockParam(const CSockParam& param) {
		ip = param.ip;
		port = param.port;
		attr = param.attr;
		memcpy(&addr_in, &param.addr_in, sizeof(addr_in));
		memcpy(&addr_un, &param.addr_un, sizeof(addr_un));
	}
	CSockParam& operator=(const CSockParam& param) {
		if (this != &param) {
			ip = param.ip;
			port = param.port;
			attr = param.attr;
			memcpy(&addr_in, &param.addr_in, sizeof(addr_in));
			memcpy(&addr_un, &param.addr_un, sizeof(addr_un));
		}
		return *this;
	}
	sockaddr* addrin() { return (sockaddr*)&addr_in; }
	sockaddr* addrun() { return (sockaddr*)&addr_un; }

public:
	//address
	sockaddr_in addr_in;
	sockaddr_un addr_un;
	//ip
	Buffer ip;
	short port;
	//参考SockAttr
	int attr;
};

class CSocketBase {
public:
	//传递析构操作
	virtual ~CSocketBase() {
		m_status = 3;
		if (m_socket != -1) {
			int fd = m_socket;
			m_socket = -1;
			close(fd);
		}
	}
public:
	//初始化 服务器 套接字创建 bind listen   客户端 套接字创建
	virtual int Init(const CSockParam& param) = 0;
	//连接  服务器 accept 客户端 connect   对udp这里可以忽略
	virtual int Link(CSocketBase** pClient = NULL) = 0;

	virtual int Send(const Buffer& data) = 0;
	virtual int Recv(Buffer& data) = 0;
	virtual int Close() = 0;
protected:
	//套接字描述符，默认是 -1
	int m_socket;
	//0未初始化 1初始化完成 2连接完成 3已经关闭
	int m_status;
};