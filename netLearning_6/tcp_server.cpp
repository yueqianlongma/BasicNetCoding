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


//获取信息， 监听端口
int tcp_listen(const char *host, const char *serv, socklen_t *addrlenp)
{
    int n, listenfd, on = 1;
    struct addrinfo *res = host_serv(host, serv, AI_PASSIVE, AF_UNSPEC, SOCK_STREAM, &n), *ressave;
    if(res == NULL)
    {
        cout<<"tcp listen error for "<<host<<" , "<<serv<<gai_strerror(n);
        exit(0);
    }
    ressave = res;

    for(; res != NULL; res = res->ai_next)
    {
        listenfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if(listenfd < 0)
            continue;
        
        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
        if(bind(listenfd, res->ai_addr, res->ai_addrlen) == 0)
            break;
        close(listenfd);
    }
    if(res == NULL)
    {
        perror("tcp_listen error");
        exit(0);
    }
    listen(listenfd, 5);
    if(addrlenp)
        *addrlenp = res->ai_addrlen;
    
    freeaddrinfo(ressave);
    return listenfd;
}

void service(char* serv)
{
    int listenfd, connfd;
    socklen_t len;
    char buff[100];
    time_t ticks;
    struct sockaddr_storage cliaddr;

    listenfd = tcp_listen(nullptr, serv, nullptr);
    while(true)
    {
        len = sizeof(cliaddr);
        connfd = accept(listenfd, (sockaddr*)&cliaddr, &len);
        cout<<"connection from "<<show((sockaddr*)&cliaddr)<<endl;
                
        ticks = time(NULL);
        snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks));
        write(connfd, buff, strlen(buff));

        close(connfd);
    }
}


int main(int argc, char **argv)
{
    if(argc != 2)
    {
        cout<<"Usage host"<<endl;
        return 0;
    }
    service(argv[1]);
    return 0;
}