#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>

const static int MAXLINE = 1024;
const static int SERV_PORT = 33333;

int main()
{
    int i , maxi , maxfd, listenfd , connfd , sockfd ;
    /*nready 描述字的数量*/
    int nready ,client[FD_SETSIZE];
    int n ;
    /*创建描述字集合，由于select函数会把未有事件发生的描述字清零，所以我们设置两个集合*/
    fd_set rset , allset;
    char buf[MAXLINE];
    socklen_t clilen;
    struct sockaddr_in cliaddr , servaddr;
    /*创建socket*/
    listenfd = socket(AF_INET , SOCK_STREAM , 0);
    /*定义sockaddr_in*/
    memset(&servaddr , 0 ,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(listenfd, (struct sockaddr *) & servaddr , sizeof(servaddr));
    listen(listenfd , 100);
    /*listenfd 是第一个描述字*/
    /*最大的描述字，用于select函数的第一个参数*/
    maxfd = listenfd;
    /*client的数量，用于轮询*/
    maxi = -1;
    /*init*/
    for(i=0 ;i<FD_SETSIZE ; i++)
        client[i] = -1;
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);

    for (;;)
    {
        rset = allset;
        /*只select出用于读的描述字，阻塞无timeout*/
        nready = select(maxfd+1 , &rset , NULL , NULL , NULL);
        if(FD_ISSET(listenfd,&rset))
        {
            clilen = sizeof(cliaddr);
            connfd = accept(listenfd , (struct sockaddr *) & cliaddr , &clilen);
            /*寻找第一个能放置新的描述字的位置*/
            for (i=0;i<FD_SETSIZE;i++)
            {
                if(client[i]<0)
                {
                    client[i] = connfd;
                    break;
                }
            }
            /*找不到，说明client已经满了*/
            if(i==FD_SETSIZE)
            {
                printf("Too many clients , over stack .\n");
                return -1;
            }
            FD_SET(connfd,&allset);//设置fd
            /*更新相关参数*/
            if(connfd > maxfd) maxfd = connfd;
            if(i>maxi) maxi = i;
            if(nready<=1) continue;
            else nready --;
        }

        for(i=0 ; i<=maxi; i++)
        {
            if (client[i]<0) continue;
            sockfd = client[i];
            if(FD_ISSET(sockfd,&rset))
            {
                n = read(sockfd , buf , MAXLINE);
                if (n==0)
                {
                    /*当对方关闭的时候，server关闭描述字，并将set的sockfd清空*/
                    close(sockfd);
                    FD_CLR(sockfd,&allset);
                    client[i] = -1;
                }
                else
                {
                    buf[n]='\0';
                    printf("Socket %d said : %s\n",sockfd,buf);
                    write(sockfd,buf,n); //Write back to client
                }
                nready --;
                if(nready<=0) break;
            }
        }

    }
    return 0;
}