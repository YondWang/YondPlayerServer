#pragma once
#include "Thread.h"
#include "Epoll.h"
#include "Socket.h"
#include <list>
#include <sys/timeb.h>

class LogInfo {
public:
	LogInfo() {}
	~LogInfo() {}
	operator Buffer() {

	}
};

class CLoggerServer {
public:
	CLoggerServer() :
		m_thread(CLoggerServer::ThreadFunc, this)
	{
		m_server = NULL;
		m_path = "./log/" + GetTimeStr() + ".log";
		printf("%s(%d):[%s]path=%s\n", __FILE__, __LINE__, __FUNCTION__, (char*)m_path);
	}
	~CLoggerServer() {
		Close();
	}
public:
	CLoggerServer(const CLoggerServer&) = delete;
	CLoggerServer operator=(const CLoggerServer&) = delete;
public:
	int Start() {
		if (m_server != NULL) return -1;
		if (access("log", W_OK | R_OK) != 0) {
			mkdir("log", S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
		}
		m_file = fopen(m_path, "w+");
		if (m_file == NULL) {
			return -2;
		}
		int ret = m_epoll.Create(1);
		if (ret != 0) return -3;
		m_server = new CLocalSocket();
		if (m_server == NULL) {
			m_epoll.Close();
			return -4;
		}
		ret = m_server->Init(CSockParam("./log/server.sock", SOCK_ISSERVER));
		if (ret != 0) {
			Close();
			return -5;
		}
		m_thread.Start();
		if (ret != 0) {
			Close();
			return -6;
		}
		return 0;
	}
	int ThreadFunc() {
		EPEvents events;
		std::map<int, CSocketBase*> mapClients;
		while ((m_thread.isValid()) && (m_epoll != -1) && (m_server != NULL)) {
			ssize_t ret = m_epoll.WaitEvents(events, 1);
			if (ret < 0) break;
			if (ret > 0) {
				ssize_t i = 0;
				for (; i < ret; i++) {
					if (events[i].events & EPOLLERR) {
						break;
					}
					else if (events[i].events & EPOLLIN) {
						if (events[i].data.ptr == m_server) {
							CSocketBase* pClient = NULL;
							int r = m_server->Link(&pClient);
							if (r < 0) continue;
							r = m_epoll.Add(*pClient, EpollData((void*)pClient), EPOLLIN);
							if (r < 0) {
								delete pClient;
								continue;
							}
							auto it = mapClients.find(*pClient);
							if (it->second != NULL) {
								delete it->second;
							}
							mapClients[*pClient] = pClient;
						}
						else {
							CSocketBase* pClient = (CSocketBase*)events[i].data.ptr;
							if (pClient != NULL) {
								Buffer data(1024 * 1024);
								int r = pClient->Recv(data);
								if (r <= 0) {
									delete pClient;
									mapClients[*pClient] = NULL;
								}
								else {
									WriteLog(data);
								}
							}
						}
					}
				}
				if (i != ret) {
					break;
				}
			}
		}
		for (auto it = mapClients.begin(); it != mapClients.end(); it++) {
			if (it->second) {
				delete it->second;
			}
		}
		mapClients.clear();
		return 0;
	}
	int Close() {
		if (m_server != NULL) {
			CSocketBase* p = m_server;
			m_server = NULL;
			delete p;
		}
		m_epoll.Close();
		m_thread.Stop();
		return 0;
	}

	//for not log thread and process
	static void Trace(const LogInfo& info) {
		static thread_local CLocalSocket client;
		if (client == -1) {
			int ret = 0;
			ret = client.Init(CSockParam("./log/server.sock", 0));
			if (ret != 0) {
#ifdef _DEBUG_
				printf("%s(%d):[%s]ret=%s\n", __FILE__, __LINE__, __FUNCTION__, ret);
#endif // _DEBUG_
			return;
			}
		}
		client.Send(info);
	}
	static Buffer GetTimeStr() {
		Buffer result(128);
		timeb tmb;
		ftime(&tmb);
		tm* pTm = localtime(&tmb.time);
		snprintf(result, result.size(),
			"%04d-%02d-%02d %02d-%02d-%02d %03d",
			pTm->tm_year + 1900, pTm->tm_mon + 1, pTm->tm_mday,
			pTm->tm_hour, pTm->tm_min, pTm->tm_sec,
			tmb.millitm
		);
	}
private:
	void WriteLog(const Buffer& data) {
		if (m_file != NULL) {
			FILE* pFile = m_file;
			fwrite((char*)data, 1, data.size(), pFile);
			fflush(pFile);
#ifdef _DEBUG_
			printf("%s", (char*)data);
#endif // _DEBUG_
		}
	}
private:
	CThread m_thread;
	CEpoll m_epoll;
	CSocketBase* m_server;
	Buffer m_path;
	FILE* m_file;
};