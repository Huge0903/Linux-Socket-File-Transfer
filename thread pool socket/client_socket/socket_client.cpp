

#include "socket_client.h"
#include "communication.h"

CLIENTPARAM  Client;

int main(void)
{
    signal(SIGINT, ClientSingal);

    Client.fd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in Server_addr;
    memset((char *)&Server_addr, 0, sizeof(Server_addr));
    Server_addr.sin_family = AF_INET;
    Server_addr.sin_port = htons(8000);
    Server_addr.sin_addr.s_addr = inet_addr("192.168.1.50");

    if(connect( Client.fd, (const struct sockaddr * )&Server_addr, sizeof(Server_addr)) == -1)
    {
        printf("client connect server failed!\r\n");
        return 0;
    }
    printf("client connect server success!\r\n");

    while(Client.Turndown == false)
    {
        std::cout << "please input order : ";
        std::string order;
        getline(std::cin, order);
        if(order == "exit")
        {
            Client.Turndown = true;
            break;
        }

        struct tcp_info info; 
        int len = sizeof(info); 
        getsockopt(Client.fd, IPPROTO_TCP, TCP_INFO, &info, (socklen_t *)&len); 
        if(info.tcpi_state != TCP_ESTABLISHED)  //检测当前TCP连接是否处于“正常连接”状态
        {
            close(Client.fd);   //当服务端断开连接后，需要新建客户端，再连接。
            Client.fd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if(connect( Client.fd, (const struct sockaddr * )&Server_addr, sizeof(Server_addr)) == -1)
            {
                printf("client connect server failed!\r\n");
                if(Client.Turndown == false)
                    continue;
                else
                    break;
            }
        }

        if(send(Client.fd, order.c_str(), order.size(), 0) == -1)
            printf("send data failed!\r\n");
        int revlen = recv(Client.fd, Client.revbuf, REVBUFFLEN - 1, 0);
        Client.revbuf[revlen] = '\0';
        OrderCompiler(Client.fd, Client.revbuf);
    }

    close(Client.fd);
    return 0;
}


/*
 *  @Description: Chang signal action
 *  @Para       : int sig : type of signal
 *  @return     : void
 *  @Author     : Huge
 *  @Data       : 2020.03.16 11:42
**/
void ClientSingal(int sig)
{
    if(sig == SIGINT)   //关闭服务端
    {
        printf("ready to close client!\r\n");
        Client.Turndown = true;
    }
}

