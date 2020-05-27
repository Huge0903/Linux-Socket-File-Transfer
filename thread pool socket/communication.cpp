#include "communication.h"
#include "socket_server.h"

/*
 *  @Description: Compiler the receive order
 *  @Para       : int client_fd : the socket fd of client
 *                const char * order : the order client send
 *  @return     : void
 *  @Author     : Huge
 *  @Data       : 2020.03.16 12:12
**/
void OrderCompiler(int client_fd, const char * order)
{
    std::string s = order;
    int len = s.size();
    std::string action, Para;
    bool start = false;
    for(int i = 0; i < len; i++)
    {
        if(start == false && s[i] != ' ')
        {
            action += s[i];
        }
        else
            start = true;
        
        if(start == true && s[i] != ' ')
        {
            Para += s[i];
        }
    }
    
    if( action == "wtf" || action == "WTF")     //????
    {
        char temp[] = "FUCK YOURSELF!\r\n";
        send(client_fd, temp, sizeof(temp), 0);
    }
    else if(action == "getfile")                //????
    {
        fileTransfer(client_fd, Para.c_str());
    }
    else
    {
        char temp[] = "I don't know what do you mean!\r\n";
        send(client_fd, temp, sizeof(temp), 0);
    }
}

/*
 *  @Description: transfer file from server to client
 *  @Para       : int client_fd : the socket fd of client
 *                const char * filename : name of file
 *  @return     : void
 *  @Author     : Huge
 *  @Data       : 2020.03.16 12:12
**/
void fileTransfer(int client_fd, const char * filename)
{
    int file = open(filename, O_RDONLY);
    if(file < 0)
    {
        char temp[] = "The file you need is not exist! Please create it in server device!\r\n";
        send(client_fd, temp, sizeof(temp), 0);
    }
    else
    {
        struct stat filestat;
        fstat(file, &filestat);
        char temp[REVBUFFLEN];
        int len = sprintf(temp, "foundfile %s %d", filename, filestat.st_size);
        send(client_fd, temp, len, 0);
        recv(client_fd, temp, REVBUFFLEN - 1, 0);
        sendfile(client_fd, file, 0, INT_MAX);
    }
    close(file);
}