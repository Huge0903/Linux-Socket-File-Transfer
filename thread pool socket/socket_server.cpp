#include "socket_server.h"
#include "communication.h"

#if 0
#include "MemRecord.h"
#if defined(MEM_DEBUG)
#define new DEBUG_NEW
#define delete DEBUG_DELETE
#endif
#endif

SERVERPARAM Server;

int main(void)
{
    signal(SIGINT, ServerSingal);
    Server.ClientStructMutex = PTHREAD_MUTEX_INITIALIZER;

    pthread_t server_pthread_id;
    Server.addr.sin_family = AF_INET;    //IPv4协议
    Server.addr.sin_port = htons(8000);
    Server.addr.sin_addr.s_addr = htonl(INADDR_ANY);

    /*****************创建服务端结点*******************/
    Server.fd = socket( Server.addr.sin_family, SOCK_STREAM, IPPROTO_TCP);
    if(Server.fd == -1)
    {
        printf("socket create failed!\r\n");
        return 0;
    }
    int optval = 1;
    if(setsockopt(Server.fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
        printf("socket SO_REUSEADDR option set failed!\r\n");
    
    if(bind(Server.fd, (struct sockaddr *)&Server.addr, (socklen_t)sizeof(struct sockaddr_in)) == -1)
    {
        printf("bind failed!\r\n");
        return 0;
    }

    if(listen(Server.fd, MAX_WAIT_CONNECTION) == -1)
    {
        printf("listen failed!\r\n");
        return 0;
    }

    
    if(pthread_create(&server_pthread_id, NULL, server_accept, NULL) == -1)
    {
        printf("server_accept pthread create failed\r\n");
        return 0;
    }
    /********************************************/

    while (1)
    {
        if(Server.TURNDOWN == true)
        {
            pthread_cancel(server_pthread_id);
            Server.revPool.ClearPool();
            pthread_mutex_lock(&Server.ClientStructMutex);
            for(auto i : Server.ClientStructMap)
                delete i.first;
            pthread_mutex_unlock(&Server.ClientStructMutex);
            break;
        }
        sleep(1);
    }
    
    close(Server.fd);
    return 0;
}

/*
 *  @Description: pthread of server accept client connect.
 *  @Para       : NULL
 *  @return     : NULL
 *  @Author     : Huge
 *  @Data       : 2020.02.10 22:50
**/
void * server_accept(void * arg)
{
    printf("Waiting for client connect!\r\n");
    pthread_detach(pthread_self()); //线程退出后自行删除线程资源
    while (1)
    {
        CLIENTPARAM * ClientNow = new CLIENTPARAM;  //为客户端申请临时空间
        if(ClientNow == NULL)
        {
            cout << "server_accept malloc CLIENTPARAM failed!\r\n" << endl;
            continue;
        }
        
        ClientNow -> client_struct_len = sizeof(struct sockaddr);  //使用accept之前一定要设置为存储结构体的长度
        ClientNow -> client_fd = accept(Server.fd, (struct sockaddr *)&(ClientNow -> client_addr), &(ClientNow -> client_struct_len));
        if(ClientNow -> client_fd == -1)
            continue;
        
        Server.revPool.FunQueuePush(server_rev, ClientNow);     //将服务端接收处理函数推入队列

        pthread_mutex_lock(&Server.ClientStructMutex);
        Server.ClientStructMap[ClientNow] = true;
        pthread_mutex_unlock(&Server.ClientStructMutex);
    }
    return NULL;
}

/*
 *  @Description: pthread of server receive data from client.
 *  @Para       : the struct CLIENTPARAM address of client.
 *  @return     : NULL
 *  @Author     : Huge
 *  @Data       : 2020.02.10 22:50
**/
void * server_rev(void * arg)
{
    pthread_detach(pthread_self()); //线程退出后自行删除线程资源
    CLIENTPARAM * ClientNow = (CLIENTPARAM *)arg;
    while (1)
    {
        struct timeval timeout={5,0};   //接收的timeout延时为5s
        int ret = setsockopt(ClientNow -> client_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout,sizeof(timeout));
        if(ret != 0)
        {
            DeleteClient(ClientNow);
            break;
        }
        
        int revlen = recv(ClientNow -> client_fd, ClientNow -> revbuf, REVBUFFLEN - 1, 0);
        if(revlen == -1) //超时
        {
            switch (errno)
            {
                case EAGAIN:
                {
                    printf("IP %s PORT %d this client timeout!\r\n", 
                           inet_ntoa(ClientNow -> client_addr.sin_addr),
                           ClientNow -> client_addr.sin_port);
                    break;
                }
                default:
                    printf("recv IP %s PORT %d data failed!\r\n", 
                           inet_ntoa(ClientNow -> client_addr.sin_addr),
                           ClientNow -> client_addr.sin_port);
                    break;
            }
            DeleteClient(ClientNow);
            break;  //退出线程，删除该线程的接收任务
        }
        else    //正常接收
        {
            ClientNow -> revbuf[revlen] = '\0';
            printf("IP %s PORT %d : %s\r\n", 
                   inet_ntoa(ClientNow -> client_addr.sin_addr), 
                   ClientNow -> client_addr.sin_port, 
                   ClientNow -> revbuf);
            OrderCompiler(ClientNow -> client_fd, ClientNow -> revbuf);
        }
    }
    return NULL;
}

/*
 *  @Description: Delete useless client connection, and free the malloc RAM.
 *  @Para       : the struct CLIENTPARAM address of client.
 *  @return     : void
 *  @Author     : Huge
 *  @Data       : 2020.02.10 22:50
**/
void DeleteClient(CLIENTPARAM * ClientNow)
{
    close(ClientNow -> client_fd);
    printf("Client IP %s PORT %d closed!\r\n", 
            inet_ntoa(ClientNow -> client_addr.sin_addr), 
            ClientNow -> client_addr.sin_port);
    pthread_mutex_lock(&Server.ClientStructMutex);
    Server.ClientStructMap.erase(ClientNow);
    pthread_mutex_unlock(&Server.ClientStructMutex);
    delete ClientNow;
}

/*
 *  @Description: Chang signal action
 *  @Para       : int sig : type of signal
 *  @return     : void
 *  @Author     : Huge
 *  @Data       : 2020.03.16 11:42
**/
void ServerSingal(int sig)
{
    if(sig == SIGINT)   //关闭服务端
    {
        printf("ready to turn down server!\r\n");
        Server.TURNDOWN = true;
    }
}