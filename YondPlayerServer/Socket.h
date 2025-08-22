#pragma once
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <fcntl.h>
#include "Public.h"

enum SockAttr {
	SOCK_ISSERVER = 1,	//是否服务器	1是 0客户端
	SOCK_ISNONBLOCK = 2,	//受否阻塞	1表示非阻塞 0阻塞
	SOCK_ISUDP = 4,		//是否UDP，1表示UDP，0表示TCP
	SOCK_ISIP = 8,		//是否为IP协议，1表示IP协议，0表示本地套接字
};

class CSockParam {
public:
	CSockParam() {
		bzero(&addr_in, sizeof(addr_in));
		bzero(&addr_un, sizeof(addr_un));
		port = -1;
		attr = 0;			//默认客户端，阻塞，tcp
	}
	CSockParam(const Buffer& ip, short port, int attr) {
		this->ip = ip;
		this->port = port;
		this->attr = attr;
		addr_in.sin_family = AF_INET;
		addr_in.sin_port = port;
		addr_in.sin_addr.s_addr = inet_addr(ip);
	}
	CSockParam(const sockaddr_in* addrin, int attr) {
		this->ip = ip;
		this->port = port;
		this->attr = attr;
		memcpy(&addr_in, addrin, sizeof(addr_in));

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
	CSocketBase() {
		m_socket = -1;
		m_status = 0;		//unfinish init
	}
	//传递析构操作
	virtual ~CSocketBase() {
		Close();
	}
public:
	//初始化 服务器 套接字创建 bind listen   客户端 套接字创建
	virtual int Init(const CSockParam& param) = 0;
	//连接  服务器 accept 客户端 connect   对udp这里可以忽略
	virtual int Link(CSocketBase** pClient = NULL) = 0;

	virtual int Send(const Buffer& data) = 0;
	virtual int Recv(Buffer& data) = 0;
	virtual int Close() {
		m_status = 3;
		if (m_socket != -1) {
			if ((m_param.attr & SOCK_ISSERVER) && 
				((m_param.attr & SOCK_ISIP) == 0))
				unlink(m_param.ip);
			int fd = m_socket;
			m_socket = -1;
			close(fd);
		}
		return 0;
	}
	virtual operator int() { return m_socket; }
	virtual operator int() const { return m_socket; }
	virtual operator const sockaddr_in* () const { return &m_param.addr_in; }
	virtual operator sockaddr_in* () { return &m_param.addr_in; }

protected:
	//套接字描述符，默认是 -1
	int m_socket;
	//0未初始化 1初始化完成 2连接完成 3已经关闭
	int m_status;
	CSockParam m_param;
};

class CSocket : public CSocketBase {
public:	CSocket() : CSocketBase() {}
	CSocket(int sock) : CSocketBase() {
		m_socket = sock;
	}
	//传递析构操作
	virtual ~CSocket() {
		Close();
	}
public:
	//初始化 服务器 套接字创建 bind listen   客户端 套接字创建
	virtual int Init(const CSockParam& param) {
		if (m_status != 0) return -1;
		m_param = param;
		int type = (m_param.attr & SOCK_ISUDP) ? SOCK_DGRAM : SOCK_STREAM;
		if (m_socket == -1) {
			if (param.attr & SOCK_ISIP) {
				m_socket = socket(PF_INET, type, 0);
			}
			else {
				m_socket = socket(PF_LOCAL, type, 0);
			}
		}
		else
			m_status = 2;
		if (m_socket == -1) return -2;
		int ret = 0;
		if (m_param.attr & SOCK_ISSERVER) {
			if (param.attr & SOCK_ISIP) {
				ret = bind(m_socket, m_param.addrin(), sizeof(sockaddr_in));
			}
			else {
				ret = bind(m_socket, m_param.addrun(), sizeof(sockaddr_un));
			}
			if (ret == -1) return -3;
			ret = listen(m_socket, 32);
			if (ret == -1) return -4;
		}
		if (m_param.attr & SOCK_ISNONBLOCK) {
			int option = fcntl(m_socket, F_GETFL);
			if (option == -1) return -5;
			option |= O_NONBLOCK;
			ret = fcntl(m_socket, F_SETFL, option);
			if (ret == -1) return -6;
		}
		if(m_status == 0)
			m_status = 1;
		return 0;
	}
	//连接  服务器 accept 客户端 connect   对udp这里可以忽略
	virtual int Link(CSocketBase** pClient = NULL) {
		if (m_status <= 0 || m_socket == -1) return -1;
		int ret = 0;
		if (m_param.attr & SOCK_ISSERVER) {
			if (pClient == NULL) return -2;
			CSockParam param;
			int fd = -1;
			socklen_t len = 0;
			if (m_param.attr & SOCK_ISIP) {
				param.attr |= SOCK_ISIP;
				len = sizeof(sockaddr_in);
				fd = accept(m_socket, param.addrin(), &len);
			}
			else {
				len = sizeof(sockaddr_un);
				fd = accept(m_socket, param.addrun(), &len);
			}
			if (fd == -1) return -3;
			*pClient = new CSocket(fd);
			if (*pClient == NULL) return -4;
			ret = (*pClient)->Init(param);
			if (ret != 0) {
				delete (*pClient);
				*pClient = NULL;
				return -5;
			}
		}
		else {
			if (m_param.attr & SOCK_ISIP) 
				ret = connect(m_socket, m_param.addrin(), sizeof(sockaddr_in));
			else
				ret = connect(m_socket, m_param.addrun(), sizeof(sockaddr_un));
			if (ret != 0) return -6;
		}
		m_status = 2;
		return 0;
	}

	
	virtual int Send(const Buffer& data) {
		if ((m_status < 2) || (m_socket == -1)) return -1;
		ssize_t index = 0;
		while (index < (ssize_t)data.size()) {
			ssize_t len = write(m_socket, (char*)data + index, data.size() - index);
			if (len == 0) return -2;
			if (len < 0) return -3;
			index += len;
		}
		return 0;
	}

	// >0 success recv <0 recv failed =0 recv data empty
	virtual int Recv(Buffer& data) {
		if (m_status < 2 || m_socket == -1) return -1;
		ssize_t len = read(m_socket, data, data.size());
		if (len > 0) {
			data.resize(len);
			return (int)len;
		}
		if (len < 0) {
			if (errno == EINTR || (errno == EAGAIN)) {
				data.clear();
				return 0;		//none data recived
			}
			return -2;	//error
		}
		return -3;	//socket is closed
	}
	virtual int Close() {
		return CSocketBase::Close();
	}
private:
};