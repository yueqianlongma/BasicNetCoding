
#ifndef HEAP_TIMER_H
#define HEAP_TIMER_H

#include <arpa/inet.h>
#include <time.h>

#define BUFFER_SIZE 64
class heap_timer;

struct client_data
{
    sockaddr_in address;
    int sockfd;
    char buf[BUFFER_SIZE];
    heap_timer *timer;
};

//定时器类
class heap_timer
{

public:
    time_t expire;
    void (*cb_func) ( client_data* );
    client_data *user_data;
public:
    heap_timer( int delay )
    {
        expire = time( nullptr ) + delay;
    }
};
//时间堆， 管理操作定时器任务
class time_heap
{
private:
    heap_timer **array;
    int capacity;
    int cur_size;

private:
    void down( int hole );
    void resize();

public:
    time_heap( int cap );
    time_heap( heap_timer **init_array, int size, int capacity );
    ~time_heap();

    void add_timer( heap_timer *timer );
    void del_timer( heap_timer *timer );
    heap_timer* top() const;
    void pop_timer();
    void tick();
    bool empty() const;

};
#endif