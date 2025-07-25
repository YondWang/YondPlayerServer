﻿#include "Process.h"
#include "Logger.h"
#include "ThreadPool.h"

int CreateLogServer(CProcess* proc) {
	//printf("%s(%d):<%s> pid = %d\n", __FILE__, __LINE__, __FUNCTION__, getpid());
	CLoggerServer server;
	int ret = server.Start();
	if (ret != 0) {
		printf("%s(%d):<%s> pid = %d errno %d msg:%s ret=%d\n",
			__FILE__, __LINE__, __FUNCTION__, getpid(), errno, strerror(errno), ret);
	}
	int fd = 0;
	while (true) {
		ret = proc->RecvFD(fd);
		printf("%s(%d):<%s> fd = %d\n", __FILE__, __LINE__, __FUNCTION__, fd);
		if (fd <= 0) break;
	}
	ret = server.Close();
	printf("%s(%d):<%s> ret = %d\n", __FILE__, __LINE__, __FUNCTION__, ret);
	return 0;
}

int CreateClientServer(CProcess* proc) {
	//printf("%s(%d):<%s> pid = %d\n", __FILE__, __LINE__, __FUNCTION__, getpid());
	int fd = -1;
	int ret = proc->RecvFD(fd);
	//printf("%s(%d):<%s> ret = %d\n", __FILE__, __LINE__, __FUNCTION__, ret);
	if (ret != 0) printf("error:%d msg:%s\n", errno, strerror(errno));
	//printf("%s(%d):<%s> fd = %d\n", __FILE__, __LINE__, __FUNCTION__, fd);
	sleep(1);
	char buf[10] = "";
	lseek(fd, 0, SEEK_SET);
	read(fd, buf, sizeof(buf));
	//printf("%s(%d):<%s> buf = %s\n", __FILE__, __LINE__, __FUNCTION__, buf);
	close(fd);


	return 0;
}

int LogTest() {
	char buffer[] = "hello YondWang! 冯老师";
	usleep(1000 * 100);
	TRACEI("HERE IS A LOG %d %c %f %g %s 哈哈哈 嘻嘻", 10, 'A', 1.0f, 2.0, buffer);
	DUMPD((void*)buffer, (size_t)sizeof(buffer));
	LOGE << 100 << " " << "S" << " " << 0.12345f << " " << 1.23456789 << " " << buffer << " 一道云编程";
	return 0;
}

int main()
{
	//CProcess::SwitchDeamon();       //turn to Deamon process

	CProcess proclog, procclient;
	printf("%s(%d):<%s> pid = %d\n", __FILE__, __LINE__, __FUNCTION__, getpid());
	proclog.SetEntryFunction(CreateLogServer, &proclog);
	int ret = proclog.CreateSubProcess();
	if (ret != 0) {
		printf("%s(%d):<%s> pid = %d\n", __FILE__, __LINE__, __FUNCTION__, getpid());
		return -1;
	}

	LogTest();

	CThread thread(LogTest);
	thread.Start();
	procclient.SetEntryFunction(CreateClientServer, &procclient);
	ret = procclient.CreateSubProcess();
	if (ret != 0) {
		printf("%s(%d):<%s> pid = %d\n", __FILE__, __LINE__, __FUNCTION__, getpid());
		return -2;
	}
	printf("%s(%d):<%s> pid = %d\n", __FILE__, __LINE__, __FUNCTION__, getpid());
	usleep(1000 * 1);
	int fd = open("./test.txt", O_RDWR | O_CREAT | O_APPEND);
	printf("%s(%d):<%s> fd = %d\n", __FILE__, __LINE__, __FUNCTION__, fd);
	if (fd == -1) return -3;
	ret = procclient.SendFD(fd);
	printf("%s(%d):<%s> ret = %d\n", __FILE__, __LINE__, __FUNCTION__, ret);
	if (ret != 0) printf("error:%d msg:%s\n", errno, strerror(errno));
	write(fd, "YondWang\n", 10);
	close(fd);

	CThreadPool pool;
	ret = pool.Start(4);
	printf("%s(%d):<%s> ret = %d\n", __FILE__, __LINE__, __FUNCTION__, ret);
	if (ret != 0) printf("error:%d msg:%s\n", errno, strerror(errno));
	ret = pool.AddTask(LogTest);
	printf("%s(%d):<%s> ret = %d\n", __FILE__, __LINE__, __FUNCTION__, ret);
	if (ret != 0) printf("error:%d msg:%s\n", errno, strerror(errno));
	ret = pool.AddTask(LogTest);
	printf("%s(%d):<%s> ret = %d\n", __FILE__, __LINE__, __FUNCTION__, ret);
	if (ret != 0) printf("error:%d msg:%s\n", errno, strerror(errno));
	ret = pool.AddTask(LogTest);
	printf("%s(%d):<%s> ret = %d\n", __FILE__, __LINE__, __FUNCTION__, ret);
	if (ret != 0) printf("error:%d msg:%s\n", errno, strerror(errno));
	ret = pool.AddTask(LogTest);
	printf("%s(%d):<%s> ret = %d\n", __FILE__, __LINE__, __FUNCTION__, ret);
	if (ret != 0) printf("error:%d msg:%s\n", errno, strerror(errno));

	(void)getchar();
	pool.Close();

	proclog.SendFD(-1);
	(void)getchar();

	return 0;
}