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
    sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(22225);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr.s_addr);

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    char buff[100];
    while(true)
    {
        bzero(buff, sizeof(buff));
        if(read(STDIN_FILENO, buff, sizeof(buff)) <= 0) break;
        sendto(sockfd, buff, strlen(buff), 0, (sockaddr*)&server_addr, sizeof(server_addr));

        int n = recvfrom(sockfd, buff, sizeof(buff), 0, nullptr, nullptr);
        write(STDOUT_FILENO, buff, strlen(buff));
    }
    close(sockfd);
    return 0;
}