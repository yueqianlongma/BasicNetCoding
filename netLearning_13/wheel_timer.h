
#ifndef WHEEL_TIMER
#define WHEEL_TIMER

#include <arpa/inet.h>
#define BUFFER_SIZE 64
class tw_timer;

//需要保存的用户数据
struct client_data
{
    sockaddr_in address;
    int sockfd;
    char buf[BUFFER_SIZE];
    tw_timer *timer;
};

//定时器
class tw_timer
{
    public:
        tw_timer *next, *prev;
        //记录定时器在时间轮转多少圈后生效
        int rotation;
        //定时器在时间轮中的位置
        int time_slot;
        void (*cb_func) (client_data *);
        client_data *user_data;

    public:
        tw_timer(int rot, int ts): next(nullptr), prev(nullptr), rotation(rot), time_slot(ts){}
};
//时间轮容器， 管理操作定时器
class time_wheel
{
    private:
        //时间轮上槽的数量
        static const int N = 60;
        // 每1s时间轮转动一次
        static const int SI = 1;
        // 时间轮的槽， 其中每个元素指向一个定时链表， 链表无序
        tw_timer* slots[N];
        int cur_slot;   //时间轮的当前槽
    public:
        time_wheel();
        ~time_wheel();

        //添加
        tw_timer *add_timer( int timeout );
        //删除
        void del_timer( tw_timer *timer );
        //外接口， 心博函数
        void tick();

};
#endif