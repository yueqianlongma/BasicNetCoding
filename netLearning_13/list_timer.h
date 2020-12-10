#ifndef LST_TIMER
#define LST_TIMER

#include <time.h>
#include <arpa/inet.h>
#define BUFFER_SIZE 64
class util_timer;

//定时器需要保存的任务数据
struct client_data
{
    sockaddr_in address;
    int sockfd;
    char buf[BUFFER_SIZE];
    util_timer *timer;
};

//定时器类
class util_timer
{
public:
    time_t expire;
    void ( *cb_func ) ( client_data* );	//回调函数
    client_data *user_data;
    util_timer *prev;
    util_timer *next;
public:
    util_timer(): prev( nullptr ), next( nullptr ){}
};

//管理操作定时器类的容器
class timer_lst
{
private:
    util_timer *head;
    util_timer *tail;

    void add_timer( util_timer *timer, util_timer *lst_head );
public:
    timer_lst(): head( nullptr ), tail( nullptr ) {}
    ~timer_lst();

    void add_timer( util_timer *timer );
    void addjust_timer( util_timer *timer );
    void del_timer( util_timer *timer );
    void tick();
};
#endif