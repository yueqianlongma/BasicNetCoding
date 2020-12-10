#include<iostream>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<sys/epoll.h>
#include<fcntl.h>
#include<cstring>
#include<unistd.h>
using namespace std;

#define BUFFER_SIZE 100
#define MAX_EVENT_NUMBER 100
#define SERV_PORT  33333

int setnonblocking(int fd)
{
    int oldOpt = fcntl(fd, F_GETFL);
    int newOpt = oldOpt | O_NONBLOCK;
    fcntl(fd, F_SETFL, newOpt);
    return oldOpt;
}

void delfd(int epollfd, int fd)
{
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, nullptr);
}

void addfd(int epollfd, int fd, bool enable_et)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN;
    if(enable_et)   event.events |= EPOLLET;

    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}

void lt(epoll_event *events, int number, int epollfd, int listenfd){
    char buf[BUFFER_SIZE];
    for(int i = 0; i < number; ++i){
        int sockfd = events[i].data.fd;
        
        if(sockfd == listenfd){
            int connfd = accept(listenfd, NULL, NULL);
            addfd(epollfd, connfd, false);
        }else if(events[i].events & EPOLLIN){
            bzero(buf, sizeof(buf));
            int ret = read(sockfd, buf, sizeof(buf));
            if(ret <= 0){
                close(sockfd);
                delfd(epollfd, sockfd);
                continue;
            }
            write(sockfd, buf, strlen(buf));
        }else{
            cout<<"something else happened"<<endl;
        }
    }
}

int main()
{
    struct sockaddr_in servaddr;
    int listenfd = socket(AF_INET , SOCK_STREAM , 0);
    memset(&servaddr,0,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(listenfd , (struct sockaddr *) &servaddr, sizeof(servaddr));
    listen(listenfd,10);
    
    int epollfd = epoll_create(5);
    addfd(epollfd, listenfd, false);

    epoll_event events[MAX_EVENT_NUMBER];
    while(true)
    {
        int nums = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
        if(nums < 0)
        {
            cout<<"epoll failure"<<endl;
            break;
        }
        lt(events, nums, epollfd, listenfd);
    }

    return 0;
}