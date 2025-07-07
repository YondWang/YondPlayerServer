#include "Process.h"

int CreateLogServer(CProcess* proc) {
    printf("%s(%d):<%s> pid = %d\n", __FILE__, __LINE__, __FUNCTION__, getpid());
    exit(0);
}

int CreateClientServer(CProcess* proc) {
    printf("%s(%d):<%s> pid = %d\n", __FILE__, __LINE__, __FUNCTION__, getpid());
    int fd = -1;
    int ret = proc->RecvFD(fd);
    printf("%s(%d):<%s> ret = %d\n", __FILE__, __LINE__, __FUNCTION__, ret);
    if (ret != 0) printf("error:%d msg:%s\n", errno, strerror(errno));
    printf("%s(%d):<%s> fd = %d\n", __FILE__, __LINE__, __FUNCTION__, fd);
    sleep(1);
    char buf[10] = "";
    lseek(fd, 0, SEEK_SET);
    read(fd, buf, sizeof(buf));
    printf("%s(%d):<%s> buf = %s\n", __FILE__, __LINE__, __FUNCTION__, buf);
    close(fd);


    return 0;
}


int main()
{
    CProcess::SwitchDeamon();       //turn to Deamon process

    CProcess proclog, procclient;
    printf("%s(%d):<%s> pid = %d\n", __FILE__, __LINE__, __FUNCTION__, getpid());
    proclog.SetEntryFunction(CreateLogServer, &proclog);
    int ret = proclog.CreateSubProcess();
    if (ret != 0) {
        printf("%s(%d):<%s> pid = %d\n", __FILE__, __LINE__, __FUNCTION__, getpid());
        return -1;
    }
    ret = procclient.SetEntryFunction(CreateClientServer, &procclient);
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

    return 0;
}