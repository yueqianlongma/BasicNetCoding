#include<iostream>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<sys/epoll.h>
#include<fcntl.h>
#include<cstring>
#include<unistd.h>
#include<pthread.h>
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

/*
    将fd上的EPOLLIN和EPOLLET事件注册到epollfd指示的epoll内核事件表中， 参数oneshot指定是否注册
    fd上的EPOLLONESHOT事件
*/
void addfd(int epollfd, int fd, bool oneshot){
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET;
    if(oneshot) event.events |= EPOLLONESHOT;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}

/*
    重置fd上的事件， 这样的操作之后， 尽管fd上的EPOLLONESHOT事件被注册， 但是操作系统仍然会
    触发fd上的EPOLLIN事件， 且只触发一次
*/
void reset_oneshot(int epollfd, int fd) {
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
    setnonblocking(fd);
}


struct fds{
    int epollfd;
    int sockfd;
};

void* worker(void *arg){
    int sockfd = ( (fds*) arg)->sockfd;
    int epollfd = ( (fds*) arg)->epollfd;
    cout<<"start new thread to receive data on fd: "<<sockfd<<endl;

    char buf[BUFFER_SIZE];
    while(true){
        memset(buf, '\0', sizeof(buf));
        int ret = recv(sockfd, buf, sizeof(buf), 0);
        if(ret == 0){
            close(sockfd);
            delfd(epollfd, sockfd);
            cout<<"foreiner closed the connection\n";
            break;
        }
        else if(ret < 0){
            if(errno == EAGAIN){
                reset_oneshot(epollfd, sockfd);
                cout<<"read later\n";
                break;
            }
        }else{
            send(sockfd, buf, strlen(buf), 0);
            sleep(5);
        }
    }
    cout<<"end thread receiving data on fd: "<<sockfd<<endl;
    return nullptr;
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
    //注意， 监听sockfd上是不能注册EPOLLONESHOT事件的， 否则应用程序只能处理一个客户
    //连接， 因为后续的客户连接请求将不再触发sockfd上的EPOLLIN事件
    addfd(epollfd, listenfd, false);

    epoll_event events[MAX_EVENT_NUMBER];
    while(true)
    {
        int nums = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
        if(nums < 0){
            cout<<"epoll failure"<<endl;
            break;
        }
        for(int i = 0; i < nums; ++i){
            int fd = events[i].data.fd;
            if(fd == listenfd){
                int connfd = accept(listenfd, NULL, NULL);
                addfd(epollfd, connfd, true);
            }
            else if(events[i].events & EPOLLIN){
                pthread_t thread;
                fds fds_for_new_worker;
                fds_for_new_worker.epollfd = epollfd;
                fds_for_new_worker.sockfd = fd;
                //新启动一个工作线程为fd服务
                pthread_create(&thread, NULL, worker, (void *)&fds_for_new_worker);
            }
            else{
                cout<<"something else happed"<<endl;
            }
        }
    }

    return 0;
}