#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <poll.h>
#include <unistd.h>

#define INFTIM -1
#define MAXLINE  1024
#define OPEN_MAX  16 //一些系统会定义这些宏
#define SERV_PORT  33333

int main()
{
    int i , maxi ,listenfd , connfd , sockfd ;
    int nready;
    int n;
    char buf[MAXLINE];
    socklen_t clilen;
    struct pollfd client[OPEN_MAX];

    struct sockaddr_in cliaddr , servaddr;
    listenfd = socket(AF_INET , SOCK_STREAM , 0);
    memset(&servaddr,0,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(listenfd , (struct sockaddr *) & servaddr, sizeof(servaddr));
    listen(listenfd,10);
    client[0].fd = listenfd;
    client[0].events = POLLRDNORM;
    for(i=1;i<OPEN_MAX;i++)
    {
        client[i].fd = -1;
    }
    maxi = 0;

    for(;;)
    {
        nready = poll(client,maxi+1,INFTIM);
        if (client[0].revents & POLLRDNORM)
        {
            clilen = sizeof(cliaddr);
            connfd = accept(listenfd , (struct sockaddr *)&cliaddr, &clilen);
            for(i=1;i<OPEN_MAX;i++)
            {
                if(client[i].fd<0)
                {
                    client[i].fd = connfd;
                    client[i].events = POLLRDNORM;
                    break;
                }
            }
            if(i==OPEN_MAX)
            {
                printf("too many clients! \n");
                return -1;
            }
            if(i>maxi) maxi = i;
            nready--;
            if(nready<=0) continue;
        }

        for(i=1;i<=maxi;i++)
        {
            if(client[i].fd<0) continue;
            sockfd = client[i].fd;
            if(client[i].revents & (POLLRDNORM|POLLERR))
            {
                n = read(client[i].fd,buf,MAXLINE);
                if(n<=0)
                {
                    close(client[i].fd);
                    client[i].fd = -1;
                }
                else
                {
                    buf[n]='\0';
                    printf("Socket %d said : %s\n",sockfd,buf);
                    write(sockfd,buf,n); //Write back to client
                }
                nready--;
                if(nready<=0) break; //no more readable descriptors
            }
        }
    }
    return 0;
}