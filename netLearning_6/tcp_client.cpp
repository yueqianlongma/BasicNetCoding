#include<iostream>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<netdb.h>
#include<unistd.h>
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

struct addrinfo* host_serv(const char* hostname, const char* service, int flags, int family, int socktype, int *n)
{
    struct addrinfo hints, *res;
    bzero(&hints, sizeof(hints));
    hints.ai_family = family;
    hints.ai_flags = flags;
    hints.ai_socktype = socktype;

    if( (*n = getaddrinfo(hostname, service, &hints, &res)) != 0)
        return nullptr;
    return res;
}

//获取服务器信息， 连接
int tcp_connect(const char *host, const char *serv)
{
    int sockfd, n;
    struct addrinfo *res = host_serv(host, serv, AI_CANONNAME, AF_UNSPEC, SOCK_STREAM, &n), *ressave;
    if(res == NULL)
    {
        cout<<"tcp connect error for "<<host<<" , "<<serv<<gai_strerror(n);
        exit(0);
    }
    ressave = res;

    for(; res != NULL; res = res->ai_next)
    {
        sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if(sockfd < 0)
            continue;
        if(connect(sockfd, res->ai_addr, res->ai_addrlen) == 0)
            break;
        close(sockfd);
    }
    
    if(res == NULL)
    {
        perror("tcp_connect error");
        exit(0);
    }

    freeaddrinfo(ressave);
    return  sockfd; 
}

void client(char *host, char *serv)
{
    int sockfd, n;
    char recvline[100];
    socklen_t len;
    struct sockaddr_storage ss;

    sockfd = tcp_connect(host, serv);
    len = sizeof(ss);

    getpeername(sockfd, (sockaddr*)&ss, &len);
    cout<<"connected to "<<show((sockaddr*)&ss)<<endl;

    while( (n = read(sockfd, recvline, sizeof(recvline))) > 0)
    {
        write(fileno(stdout), recvline, n);
    }
    exit(0);
}

int main(int argc, char **argv)
{
    if(argc != 3)
    {
        cout<<"Usage serv host"<<endl;
        return 0;
    }
    client(argv[1], argv[2]);
    return 0;
}
