#include "communication.h"
#include "socket_client.h"
using namespace std;

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
    cout << s << endl;
    for(int i = 0; i < len; i++)
    {
        if(start == false && s[i] != ' ')
        {
            action += s[i];
        }
        else
        {
            start = true;
            if(action == "foundfile")
            {
                s.erase(s.begin(), s.begin() + i + 1);
                start = false;
                break;
            }
        }
        
        if(start == true && s[i] != ' ')
            Para += s[i];
    }
    
    if(action == "foundfile")
    {
        cout << "I am here!" << endl;
        action.clear();
        Para.clear();
        for(auto c : s)
        {
            if(c != ' ' && start == false)
                action += c;
            else
                start = true;
            
            if(start == true && c != ' ')
                Para += c;
        }
        int filesize = stoi(Para);
        fileReceive(client_fd, action.c_str(), filesize);
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
void fileReceive(int client_fd, const char * filename, int filesize)
{
    char temp[] = "client ready, pleas send!";
    send( client_fd, temp, sizeof(temp), 0);

    int filefd = open( filename, O_RDWR | O_CREAT | O_TRUNC , 666);
    if(ftruncate( filefd, filesize) != 0)
    {
        cout << "failed to ftruncate file!" << endl;
        return;
    }

    void * fileaddr = mmap(NULL, filesize, PROT_READ | PROT_WRITE , MAP_SHARED, filefd, 0);
    if(fileaddr == MAP_FAILED)
    {
        cout << "mmap " << filename << " failed!" << endl;
        return;
    }

    int revlen = recv(client_fd, fileaddr, filesize, 0);
    if(revlen == -1)
    {
        cout << "recv " << filename << " failed!" << endl;
        munmap(fileaddr, filesize);
        return;
    }
    
    
    if(msync(fileaddr, filesize, MS_SYNC) == -1)
    {
        cout << "msync " << filename << " failed!"<< endl;
    }
    if(munmap(fileaddr, filesize) == -1)
    {
        cout << "munmap " << filename << " failed!"<< endl;
    }
    
    #if 0   //之前的方案使用readline函数，但是逐个去读取socket中的数据太麻烦
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
    #endif
    close(filefd);
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
