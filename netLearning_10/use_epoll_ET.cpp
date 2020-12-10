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

void et(epoll_event *events, int number, int epollfd, int listenfd){
    char buf[BUFFER_SIZE];
    for(int i = 0; i < number; ++i){
        int sockfd = events[i].data.fd;

        if(sockfd == listenfd){
            int connfd = accept(sockfd, NULL, NULL);
            addfd(epollfd, connfd, true);
        }else if(events[i].events & EPOLLIN){
            /*
                这段代码不会被重复触发， 所以我们循环读取数据， 以确保把socket读缓存中的所有数据读出
            */
           while(true){
               memset(buf, '\0', sizeof(buf));
               int ret = recv(sockfd, buf, sizeof(buf), 0);
               if(ret < 0){
                   /*
                    对于非阻塞IO， 下面的条件成立表示数据已经全部读取完毕， 此后epoll就能再次触发
                    sockfd上的EPOLLIN事件， 以驱动下一次读操作
                    EAGAIN:  这个错误在非阻塞IO中会出现，表示当前已经没有数据，我们需要等待稍后在读
                   */
                  if( (errno == EAGAIN) || (errno == EWOULDBLOCK)){
                      break;
                  }
                  close(sockfd);
                  delfd(epollfd, sockfd);
                  break;
               }else if(ret == 0){
                   delfd(epollfd, sockfd);
                   close(sockfd);
                   break;
               }else
                    send(sockfd, buf, strlen(buf), 0);
           }
        }else 
            cout<<"something else happened"<<endl;
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
        et(events, nums, epollfd, listenfd);
    }

    return 0;
}