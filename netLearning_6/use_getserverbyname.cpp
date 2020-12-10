#include<iostream>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<unistd.h>
#include<netdb.h>
#include<cstring>
using namespace std;

char* show(sockaddr* addr)
{
    static char buff[100];
    switch (addr->sa_family)
    {
    case AF_INET:
    {
        sockaddr_in* addr4 = (sockaddr_in*)addr;
        snprintf(buff, sizeof(buff), "%s:%d", inet_ntoa(addr4->sin_addr), htons(addr4->sin_port));
        break;
    }
    default:
        break;
    }
    return buff;
}

int main(int argc, char **argv)
{
    if(argc < 3)
    {
        return -1;
    }

    hostent* hptr = gethostbyname(argv[1]);
    if(hptr == nullptr)
    {
        cout<<"hostname error for "<<argv[1]<<" : "<<hstrerror(h_errno)<<endl;
        return -1;
    }

    servent* sptr = getservbyname(argv[2], "tcp");    
    if(sptr == nullptr)
    {
        cout<<"getservbyname error for "<<argv[2]<<endl;
        return -1;
    }

    int sockfd = 0;
    sockaddr_in serverAddr;
    char **pptr = nullptr;
    for(pptr = hptr->h_addr_list; *pptr != nullptr; ++pptr)
    {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        bzero(&serverAddr, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = sptr->s_port;
        memcpy(&serverAddr.sin_addr, *pptr, sizeof(struct in_addr));
        cout<<"trying "<<show((sockaddr*)&serverAddr)<<endl;

        if(connect(sockfd, (sockaddr*)&serverAddr, sizeof(serverAddr)) == 0)
            break;
        perror("connect error");
        close(sockfd);
    }

    if(*pptr == NULL){
        perror("unable to conect");
        return -1;
    }

    char recvline[100];
    while( read(sockfd, recvline, sizeof(recvline)) > 0){
        write(STDOUT_FILENO, recvline, strlen(recvline));
        bzero(recvline, sizeof(recvline));
    }

    return 0;
}