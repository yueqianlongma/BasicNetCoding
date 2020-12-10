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

int udp_connect(const char *host, const char *serv, sockaddr **saptr, socklen_t *lenp)
{
    int sockfd, n;
    struct addrinfo *res, *ressave;
    res = host_serv(host, serv, AI_CANONNAME, AF_UNSPEC, SOCK_DGRAM, &n);
    if(res == NULL)
    {
        cout<<"udp connect error "<<host<<" , "<<serv<<" "<<gai_strerror(n);
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
        perror("udp connect error");
        exit(0);
    }

    if(saptr)
    {
        *saptr = (struct sockaddr*)malloc(res->ai_addrlen);
        memcpy(*saptr, res->ai_addr, res->ai_addrlen);
        *lenp = res->ai_addrlen;
    }

    freeaddrinfo(ressave);
    return sockfd;
}

int main(int argc, char **argv)
{
    if(argc != 3)
    {
        cout<<"Usage ip / port"<<endl;
        return -1;
    }
    char buff[100], temp='a';
    sockaddr *serv_addr;
    socklen_t len;
    int sockfd = udp_connect(argv[1], argv[2], &serv_addr, &len);
    sendto(sockfd, &temp, 1, 0, serv_addr, len);
    bzero(buff, sizeof(buff));
    recvfrom(sockfd, buff, sizeof(buff), 0, nullptr, nullptr);
    write(fileno(stdout), buff, strlen(buff));
    close(sockfd);
    return 0;
}