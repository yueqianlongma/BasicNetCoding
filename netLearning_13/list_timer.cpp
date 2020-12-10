#include "list_timer.h"
#include <iostream>

timer_lst :: ~timer_lst()
{
    util_timer *tmp = head;
    while( tmp )
    {
        head = tmp->next;
        delete tmp;
        tmp = head;
    }   
}
// 添加定时任务
//@timer 	 定时任务
//@lst_head  链表头 dummary(哑节点)
void timer_lst::add_timer( util_timer *timer, util_timer *lst_head )
{
    while( lst_head )
    {
        if( lst_head->next  == nullptr)
        {
            lst_head->next = timer;
            timer->prev = lst_head;
            timer->next = nullptr;
            tail = timer;
            break;
        }
        else if( lst_head->next->expire > timer->expire )
        {
            timer->next = lst_head->next;
            timer->prev = lst_head;
            lst_head->next->prev = timer;
            lst_head->next = timer;
            break;
        }
        lst_head = lst_head->next;
    }
}

// 向链表加入定时器
void timer_lst::add_timer( util_timer *timer )
{
    if( !timer )  return;

    if( !head )
    {
        head = tail = timer;
        return;
    }

    if( timer->expire < head->expire )
    {
        timer->next = head;
        head->prev = timer;
        head =timer;
        return;
    }
    add_timer( timer, head );
}

// 调整链表中的定时器
void timer_lst::addjust_timer( util_timer *timer )
{
    if( !timer )  return;

    util_timer *tmp = timer->next;
    if( !tmp || ( timer->expire < tmp->expire ) )   return;

    if( timer == head )
    {
        head = head->next;
        head->prev = nullptr;
        timer->next = nullptr;
        add_timer( timer, head );
    }
    else
    {
        timer->prev->next = timer->next;
        timer->next->prev = timer->prev;
        add_timer( timer, timer->next );
    }
    
}

// 删除链表中的定时器
void timer_lst::del_timer( util_timer *timer )
{
    if( !timer )    return;

    if( ( timer == head ) && ( timer == tail ) )
    {
        delete timer;
        head = tail = nullptr;
        return;
    }

    if( timer == head )
    {
        head = head->next;
        head->prev = nullptr;
        delete timer;
        return;
    }

    if( timer == tail )
    {
        tail = tail->prev;
        tail->next = nullptr;
        delete timer;
        return;        
    }

    timer->prev->next = timer->next;
    timer->next->prev = timer->prev;
    delete timer;
}

/*
* SIGARM信号每次被触发就在其信号处理函数中执行一次tick函数，
* 以处理链表上到期的任务
*/
void timer_lst::tick()
{
    if( !head ) return;
    std::cout<<" timer tick "<<std::endl;

    //获取系统当前的时间,绝对时间
    time_t  cur = time( nullptr );
    util_timer *tmp = head;

    //从头结点开始依次处理每个定时器， 直到遇到一个尚未到期的定时器
    while ( tmp )
    {
        //每个定时器使用绝对时间作为超时值
        if( cur < tmp->expire ) break;
        //调用定时器的回调函数， 已执行定时任务
        tmp->cb_func( tmp->user_data );
        //执行完定时器中定时任务之后， 就将它从链表中删除， 并重置链表头结点
        head = tmp->next;
        if( head )  head->prev = nullptr;
        delete tmp;
        tmp = head;
    }
}