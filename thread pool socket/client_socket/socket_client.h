#ifndef _SOCKET_CLIENT_H_
#define _SOCKET_CLIENT_H_

#include "public.h"

#define REVBUFFLEN      1024     //接收消息缓存区大小

typedef struct ClientParam
{
    bool Turndown;               //是否关闭客户端
    unsigned int fd;             //客户端文件表示符
    struct sockaddr_in addr;     //服务端端参数
    char revbuf[REVBUFFLEN];     //接受缓存区
}CLIENTPARAM;


void ClientSingal(int sig);

#endif