#include "socket_client.h"
#include "communication.h"

using namespace std;

CLIENTPARAM  Client;

int main(void)
{
    signal(SIGINT, ClientSingal);

    /*获取服务端的IP地址和端口号*/
    struct sockaddr_in Server_addr;
    memset((char *)&Server_addr, 0, sizeof(Server_addr));
    unsigned short Server_Port = 0;
    string IP_s;
    cout << "Please input server IP : ";
    cin >> IP_s;
    cout << "Please input server Port : ";
    cin >> Server_Port;
    Server_addr.sin_family = AF_INET;
    Server_addr.sin_port = htons(Server_Port);
    Server_addr.sin_addr.s_addr = inet_addr(IP_s.c_str());

    /*客户端结点建立并连接服务端*/
    Client.fd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(connect( Client.fd, (const struct sockaddr * )&Server_addr, sizeof(Server_addr)) == -1)
    {
        cout << "client connect server failed!" << endl;
        return 0;
    }
    cout << "client connect server success!" << endl;

    while(Client.Turndown == false)
    {
        cout << "please input order : ";    //输入指令
        string order;
        getline(cin, order);
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
                cout << "client connect server failed!" << endl;
                if(Client.Turndown == false)
                    continue;
                else
                    break;
            }
        }

        if(send(Client.fd, order.c_str(), order.size(), 0) == -1)          //发送指令
            cout << "send data failed!" << endl;
        int revlen = recv(Client.fd, Client.revbuf, REVBUFFLEN - 1, 0);     //接收返回的指令
        Client.revbuf[revlen] = '\0';
        OrderCompiler(Client.fd, Client.revbuf);        //解析指令
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
        cout << "ready to close client!" << endl;
        Client.Turndown = true;
    }
}

