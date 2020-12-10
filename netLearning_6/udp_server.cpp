/*
*   2020/8/29 19:23
*/
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <netdb.h>
using namespace std;

struct addrinfo *host_serv(const char *hostname, const char *service, int flags, int family, int socktype, int *n)
{
    struct addrinfo hints, *res;

    bzero(&hints, sizeof(struct addrinfo));
    hints.ai_family = family;
    hints.ai_flags = flags;
    hints.ai_socktype = socktype;

    if( (*n = getaddrinfo(hostname, service, &hints, &res)) != 0)
    {
        return NULL;
    }
    return res;
}

int udp_server(const char *host, const char *serv, socklen_t *addrlenp)
{
    int sockfd, n;
    struct addrinfo *res, *ressave;
    res = host_serv(host, serv, AI_PASSIVE, AF_UNSPEC, SOCK_DGRAM, &n);
    if(res == NULL)
    {
        cout<<"udp server error for  "<<host<<" , "<<serv<<" "<<gai_strerror(n);
        exit(0);
    }
    ressave = res;

    for(; res != NULL; res = res->ai_next)
    {
        sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if(sockfd < 0)
            continue;
        if(bind(sockfd, res->ai_addr, res->ai_addrlen) == 0)
            break;
        close(sockfd);
    }
    if(res == NULL)
    {
        perror("udp server error ");
        exit(0);
    }
    if(addrlenp)
        *addrlenp = res->ai_addrlen;
    
    freeaddrinfo(ressave);
    return sockfd;
}

int main(int argc, char **argv)
{
    if(argc != 2)
    {
        cout<<"Usage port"<<endl;
        return -1;
    }
    int sockfd = udp_server(nullptr, argv[1], nullptr);
    sockaddr_storage cliaddr;
    socklen_t len;
    char buff[2048], temp;
    time_t ticks;
    while (true)
    {
        recvfrom(sockfd, &temp, 1, 0, (sockaddr*)&cliaddr, &len);
        ticks = time(NULL);
        snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks));
        sendto(sockfd, buff, strlen(buff), 0, (sockaddr*)&cliaddr, len);
    }
    return 0;
}