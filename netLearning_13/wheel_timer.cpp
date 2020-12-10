#include "wheel_timer.h"
#include <iostream>
#include <cstring>

//初始化
time_wheel::time_wheel(): cur_slot( 0 )
{
    memset( slots, 0, sizeof(slots) );
}
//析构
time_wheel::~time_wheel()
{
    for( int i = 0; i < N; ++i )
    {
        tw_timer *tmp = slots[ i ];
        while( tmp )
        {
            slots[ i ] = tmp->next;
            delete tmp;
            tmp = slots[ i ];
        }
    }
}

//向时间轮中加入定时器
tw_timer* time_wheel::add_timer( int timeout )
{
    if( timeout < 0 )   return nullptr;
    int ticks = 0;
    //每个槽的时间间隔是SI, 这里计算timeout时间所需要的时间槽
    if( timeout < SI )  
        ticks = 1;
    else 
        ticks = timeout / SI;
    //通过总的槽数， 我们可以计算出 时间轮经过多少次轮转， 定时器生效
    int rot = ticks / N;
    //计算定时器在时间轮中的位置
    int ts = ( cur_slot + ticks % N ) % N;
    tw_timer *timer = new tw_timer( rot, ts );

    //下面做的是将定时器加入的其相应的槽中
    if( slots[ ts ] == nullptr )
        slots[ ts ] = timer;
    else
    {
        timer->next = slots[ ts ];
        slots[ ts ]->prev = timer;
        slots[ ts ] = timer;
    }
    return timer;
}

//解除定时器
void time_wheel::del_timer( tw_timer *timer )
{
    if( !timer )    return;

    int ts = timer->time_slot;
    if( slots[ ts ] == nullptr )
    {
        std::cout<<"error"<<std::endl;
    }

    if( slots[ ts ] == timer )
    {
        slots[ ts ] = slots[ ts ]->next;
        if( slots[ ts ] )
            slots[ ts ]->prev = nullptr;
        delete timer;
    }
    else 
    {
        timer->prev->next = timer->next;
        if( timer->next )
            timer->next->prev = timer->prev;
        delete timer;
    }
}

void time_wheel::tick()
{
    tw_timer *tmp = slots[ cur_slot ];
    std::cout<<"current slot is "<<cur_slot<<std::endl;
    while( tmp )
    {
        std::cout<<"tick the timer once"<<std::endl;
        if( tmp->rotation > 0)
        {
            --tmp->rotation;
            tmp = tmp->next;
        }
        else
        {
            tmp->cb_func( tmp->user_data );
            tw_timer *tmp2 = tmp->next;
            del_timer( tmp );
            tmp = tmp2;
        }
    }
    cur_slot = ++cur_slot % N;
}