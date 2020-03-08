#ifndef _SOCKET_SERVER_H_
#define _SOCKET_SERVER_H_

#include "public.h"
#include "threadpool.h"

using namespace std;

#define MAX_WAIT_CONNECTION  20 //进入侦听状态后，服务端所能缓存的最多connect请求数量；
#define REVBUFFLEN      200     //接收消息缓存区大小

typedef struct ClientParam
{
    unsigned int client_fd;                      //客户端文件表示符
    pthread_t client_pthread_id;        //客户端接收任务的任务id
    socklen_t client_struct_len;        //客户端结构长度
    struct sockaddr_in client_addr;     //客户端参数
    char revbuf[REVBUFFLEN];            //接受缓存区
}CLIENTPARAM;

typedef struct ServerParam
{
    bool TURNDOWN;  //是否关闭服务端
    unsigned int fd;      //服务端文件标识符
    struct sockaddr_in addr;     //服务端地址结构
    pthread_mutex_t ClientStructMutex;      //客户端结构地址表互斥保护
    map<CLIENTPARAM * , bool> ClientStructMap;    //客户端结构地址表
    threadpool revPool;     //接收消息线程池
}SERVERPARAM;

void * server_accept(void *);
void * server_rev(void * arg);
void Ctrl_C_fun(int sig);
void DeleteClient(CLIENTPARAM * ClientNow);
void ServerSingal(int sig);

#endif