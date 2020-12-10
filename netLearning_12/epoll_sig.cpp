#include <iostream>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <assert.h>
#include <cstring>
#include <fcntl.h>
#include <sys/epoll.h>
#include <signal.h>
#include <string>
using namespace std;

#define MAX_EVENT_NUMBER 1024
static int pipefd[2];


// 信号处理函数
void sig_handler(int sig){
    int save_errno = errno;
    int msg = sig;
    //向主循环发送信号， 主循环监听pipefd[0]
    send(pipefd[1], (char*)&msg, sizeof(msg), 0);
    errno = save_errno;
}

//设置信号处理函数
void addsig(int sig){
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = sig_handler;
    sa.sa_flags |= SA_RESTART;
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, NULL) != -1);
}

int setnonblocking(int fd)
{
    int oldOpt = fcntl(fd, F_GETFL);
    int newOpt = oldOpt | O_NONBLOCK;
    fcntl(fd, F_SETFL, newOpt);
    return oldOpt;
}

void addfd(int epollfd, int fd){
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}


void server(int port=33333){
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = INADDR_ANY;

    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(listenfd >= 0);

    int ret = bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    assert(ret != -1);

    ret = listen(listenfd, 5);
    assert(ret != -1);

    epoll_event events[MAX_EVENT_NUMBER];
    int epollfd = epoll_create(5);
    assert(epollfd != -1);
    addfd(epollfd, listenfd);

    //使用socketpair 创建管道， 注册pipefd[0]上的可读事件
    ret = socketpair(PF_UNIX, SOCK_STREAM, 0, pipefd);
    assert(ret != -1);
    setnonblocking(pipefd[1]);
    addfd(epollfd, pipefd[0]);

    //设置一些信号处理函数
    addsig(SIGHUP);
    addsig(SIGCHLD);
    addsig(SIGTERM);
    addsig(SIGINT);
    bool stop_server = false;

    while(!stop_server){
        int number = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
        if( (number < 0) && (errno != EINTR)){
            cout<<"epoll failure"<<endl;
            break;
        }

        for(int i = 0; i < number; ++i){
            int sockfd = events[i].data.fd;
            if(sockfd == listenfd){
                int connfd = accept(sockfd, NULL, NULL);
                addfd(epollfd, connfd);
            }
            else if( (sockfd == pipefd[0]) && events[i].events & EPOLLIN){
                char signals[1024];
                ret = recv(pipefd[0], signals, sizeof(signals), 0);
                if(ret == -1)   
                    continue;
                else if(ret == 0)
                    continue;
                else {
                    // 因为每个信号值占1个字节， 所以按每个字节逐个接收信号。
                    for(int j = 0; j < ret; ++j){
                        switch (signals[j])
                        {
                        case SIGCHLD:
                             cout<<"SIGCHLD"<<endl;
                        case SIGHUP:
                            cout<<"SIGHUP"<<endl;
                            continue;
                        case SIGTERM:
                            cout<<"SIGTERM"<<endl;
                        case SIGINT:    
                            cout<<"SIGINT"<<endl;
                            stop_server = true;
                        default:
                            break;
                        }
                    }
                }
            }
            else {

            }
        }
    }
    cout<<"close fds"<<endl;
    close(listenfd);
    close(pipefd[1]);
    close(pipefd[0]);
}

int main()
{
    server();
    return 0;
}