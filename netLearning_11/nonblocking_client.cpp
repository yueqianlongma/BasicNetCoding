#include <iostream>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <stdio.h>
using namespace std;

const int MAXLINE = 1000;

void show(sockaddr* addr)
{
    char buf[100];
    switch (addr->sa_family)
    {
    case AF_INET:
    {
        sockaddr_in* addr4 = (sockaddr_in*)addr;
        snprintf(buf, sizeof(buf), "%s:%d", inet_ntoa(addr4->sin_addr), ntohs(addr4->sin_port));
        cout<<"connect: "<<buf<<endl;
        break;
    }
    default:
        break;
    }
}

int setNonblocking(int sockfd)
{   
    int old_option = fcntl(sockfd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(sockfd, F_SETFL, new_option);
    return old_option;
}

char* gf_time()
{
    struct timeval tv;
    static char str[30];
    char *ptr;

    if(gettimeofday(&tv, nullptr))  perror("gettimeofday error");
    ptr = ctime(&tv.tv_sec);
    strcpy(str, &ptr[11]);
    snprintf(str + 8, sizeof(str) - 8, ".%06ld", tv.tv_usec);
    return str;
}


void str_cli(int sockfd)
{
    //缓冲区
    char to[MAXLINE], from[MAXLINE], *toiptr, *tooptr, *friptr, *froptr;
    toiptr = tooptr = to;
    friptr = froptr = from;
    //标志， 标记标准输入是否结束
    int stdineof = 0;

    //设置非阻塞
    setNonblocking(sockfd);
    setNonblocking(STDIN_FILENO);
    setNonblocking(STDOUT_FILENO);

    fd_set  rset, wset;
    int maxfd = max(max(STDIN_FILENO, STDOUT_FILENO), sockfd) + 1;
    while(true)
    {
        FD_ZERO(&rset);
        FD_ZERO(&wset);
        if(stdineof == 0 && toiptr < &to[MAXLINE])                  FD_SET(STDIN_FILENO, &rset);
        if(friptr < &from[MAXLINE])                                 FD_SET(sockfd, &rset);
        if(tooptr != toiptr)                                        FD_SET(sockfd, &wset);
        if(froptr != friptr)                                       	FD_SET(STDOUT_FILENO, &wset);

        select(maxfd, &rset, &wset, nullptr, nullptr);
        int n = 0, nwritten = 0;
        //标准读入
        if(FD_ISSET(STDIN_FILENO, &rset))
        {
            if( (n = read(STDIN_FILENO, toiptr, &to[MAXLINE] - toiptr)) < 0)
            {
                if(errno != EWOULDBLOCK)    perror("read error on stdin");
            }
            else if(n == 0)
            {
                fprintf(stderr, "%s: EOF stdin\n", gf_time());
                stdineof = 1;
                //关闭socket的写
                if(tooptr == toiptr)    shutdown(sockfd, SHUT_WR);
            }
            else
            {
                fprintf(stderr, "%s: read %d bytes from stdin\n", gf_time(), n);
                toiptr += n;
                FD_SET(sockfd, &wset);  //通知可以写了， 这个通知在本次循环内会完成
            }
        }
        
        //socket读入
        if(FD_ISSET(sockfd, &rset))
        {
            if( (n = read(sockfd, friptr, &from[MAXLINE] - friptr)) < 0)
            {
                if(errno != EWOULDBLOCK)    perror("read error on socket");
            }
            else if(n == 0)
            {
                fprintf(stderr, "%s: EOF socket\n", gf_time());
                if(stdineof == 1)   
                    return;
                else
                    perror("str_cli: server terminated prematurely");
            }
            else
            {
                fprintf(stderr, "%s: read %d bytes from socket\n", gf_time(), n);
                friptr += n;
                FD_SET(STDOUT_FILENO, &wset);  //通知可以写了， 这个通知在本次循环内会完成
            }
        }

        //标准输出
        if(FD_ISSET(STDOUT_FILENO, &wset) && (n = friptr - froptr) > 0)
        {
            if((nwritten = write(STDOUT_FILENO, froptr, n)) < 0)
            {
                if(errno != EWOULDBLOCK)    perror("write error to stdout");
            }
            else
            {
                fprintf(stderr, "%s: wrote %d bytes to stdout\n", gf_time(), nwritten);
                froptr += nwritten;
                if(froptr == friptr)    froptr = friptr = from;
            }
        }
        
        //socket输出
        if(FD_ISSET(sockfd, &wset) && (n = toiptr - tooptr) > 0)
        {
            
            if((nwritten = write(sockfd, tooptr, n)) < 0)
            {
                if(errno != EWOULDBLOCK)    perror("write error to socket");
            }
            else
            {
                fprintf(stderr, "%s: wrote %d bytes to socket\n", gf_time(), nwritten);
                tooptr += nwritten;
                if(tooptr == toiptr)
                {
                    toiptr = tooptr = to;
                    if(stdineof == 1)   shutdown(sockfd, SHUT_WR);
                }
            }
        }
    }
}


int main(int argc, char **argv)
{
    sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(33333);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr.s_addr);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    int status = connect(sockfd, (sockaddr*)&addr, sizeof(addr));
    if(status < 0)  
        return -1;
    show((sockaddr*)&addr);
    str_cli(sockfd);
    close(sockfd);
    return 0;
}