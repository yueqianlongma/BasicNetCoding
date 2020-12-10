#include<iostream>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<unistd.h>
#include<cstring>
using namespace std;


void show(sockaddr* addr)
{
    char buf[100];
    switch (addr->sa_family)
    {
    case AF_INET:
    {
        sockaddr_in* addr4 = (sockaddr_in*)addr;
        snprintf(buf, sizeof(buf), "%s:%d", inet_ntoa(addr4->sin_addr), ntohs(addr4->sin_port));
        cout<<"connect: "<<buf<<endl;
        break;
    }
    default:
        break;
    }
}


int main(int argc, char **argv)
{
    sockaddr_in localAddr;
    bzero(&localAddr, sizeof(localAddr));
    localAddr.sin_family = AF_INET;
    localAddr.sin_port = htons(33333);
    localAddr.sin_addr.s_addr = INADDR_ANY;

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    bind(sockfd, (sockaddr*)&localAddr, sizeof(localAddr));
    listen(sockfd, 5);

    char buf[100];
    while (true)
    {
        sockaddr_in peerAddr;
        socklen_t len = sizeof(peerAddr);
        int fd = accept(sockfd, (sockaddr*)&peerAddr, &len);

        pid_t status = fork();
        if(status == 0)
        {
            close(sockfd);
            show((sockaddr*)&peerAddr);
            while(true)
            {   
                bzero(buf, sizeof(buf));
                int n = read(fd, buf, sizeof(buf));
                if(n < 0)
                {
                    if(errno == EINTR)  continue;
                    else break;
                }
                if(n == 0)  break;
                write(STDOUT_FILENO, buf, strlen(buf));
                write(fd, buf, strlen(buf));
            }
            exit(0);
        }
        close(fd);
    }
    
    return 0;
}