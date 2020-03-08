#include "communication.h"
#include "socket_client.h"

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
    
    if(action == "foundfile")
    {
        fileReceive(client_fd, Para.c_str());
    }
    else
    {
        std::cout << order << std::endl;
    }
}

/*
 *  @Description: transfer file from server to client
 *  @Para       : int client_fd : the socket fd of client
 *                const char * filename : name of file
 *  @return     : void
 *  @Author     : Huge
 *  @Data       : 2020.03.06 12:12
**/
void fileReceive(int client_fd, const char * filename)
{
    printf("ready to save file!\r\n");
    char revbuf[REVBUFFLEN * 10];
    int file = open( filename, O_RDWR | O_CREAT | O_TRUNC, 666);

    while(1)
    {
        int revlen = readLine(client_fd, revbuf, REVBUFFLEN * 10);
        if(revlen > 0)
        {
            std::cout << revbuf;
            write(file, revbuf, revlen);
        }
        else
            break;
    }
    close(file);
}

ssize_t readLine(int fd, char * buffer, size_t n)
{
    ssize_t numRead = 0;
    size_t totRead = 0;
    char ch;
    if(n <= 0 || buffer == NULL)
    {
        errno = EINVAL;
        return -1;
    }

    while(1)
    {
        numRead = recv(fd, &ch, 1, 0);
        if(numRead == -1)
        {
            if(errno == EINTR)
                continue;
            else
                return -1;
        }
        else if(numRead == 0)
        {
            if(totRead == 0)
                return 0;
            else
                break;
        }
        else
        {
            if(totRead < n-1)
            {
                totRead++;
                *buffer++ = ch;
            }
            if(ch == '\n')
                break;
        }
    }
    *buffer = '\0';
    return totRead;
}
