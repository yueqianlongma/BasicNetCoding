#include<iostream>
#include<cstring>
#include<arpa/inet.h>
#include<netinet/in.h>
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
        cout<<"connect: "<<buf<<" ";
        break;
    }
    default:
        break;
    }
}


int main()
{
    sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(22225);
    addr.sin_addr.s_addr = INADDR_ANY;

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    bind(sockfd, (sockaddr*)&addr, sizeof(addr));

    char buff[100];
    sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);
    while(true)
    {
        bzero(buff, sizeof(buff));
        int n = recvfrom(sockfd, buff, sizeof(buff), 0, (sockaddr*)&client_addr, &len);
        show((sockaddr*)&client_addr);
        cout<<buff<<endl;
        sendto(sockfd, buff, strlen(buff), 0, (sockaddr*)&client_addr, len);
    }
    close(sockfd);
    return 0;
}