#include<iostream>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<cstring>
#include<unistd.h>
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
    sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(33333);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr.s_addr);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    int status = connect(sockfd, (sockaddr*)&addr, sizeof(addr));
    if(status < 0)  
        return -1;
    show((sockaddr*)&addr);

    char buf[100];
    while (true)
    {
        bzero(buf, sizeof(buf));
        if(read(STDIN_FILENO, buf, sizeof(buf)) <= 0)   break;
        write(sockfd, buf, strlen(buf));

        bzero(buf, sizeof(buf));
        int n = read(sockfd, buf, sizeof(buf));
        if(n < 0)
        {
            if(errno == EINTR)  continue;
            else break;
        }
        if(n == 0)  break;
        write(STDOUT_FILENO, buf, strlen(buf));
    }
    close(sockfd);
    return 0;
}