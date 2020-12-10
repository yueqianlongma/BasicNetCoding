#include "heap_timer.h"
#include <iostream>
#include <cstring>

time_heap::time_heap( int cap ): capacity( cap ), cur_size( 0 )
{
    array = new heap_timer* [ capacity ];
    memset( array, 0, sizeof( array ) );
}

time_heap::time_heap( heap_timer **init_array, int size, int capacity )
    : cur_size( size ), capacity( capacity )
{
    if( capacity < size )   return;
    
    array = new heap_timer* [ capacity ];
    memset( array, 0, sizeof( array ) );
    if( size )
    {
        for( int i = 0; i < size; ++i )
        {
            array[ i ] = init_array[ i ];
        }
        for( int i = ( cur_size - 1 ) / 2; i >= 0; --i )
        {
            down( i );
        }
    }
}

time_heap::~time_heap()
{
    for(int i = 0; i < cur_size; ++i)
        if( array[i] )  
            delete array[i];
    delete [] array;
}

void time_heap::add_timer( heap_timer *timer )
{
    if( !timer )    return;
    if( cur_size >= capacity )
        resize();
    int hole = cur_size++;
    
    while ( hole )
    {
        int par = ( hole - 1 ) / 2;
        if( array[ par ]->expire <= timer->expire )
            break;
        array[ hole ] = array[ par ];
        hole = par;
    }
    array[ hole ] = timer;
}

void time_heap::del_timer( heap_timer *timer )
{
    if( !timer )    return;
    timer->cb_func = nullptr;
}

heap_timer* time_heap::top() const
{
    if( empty() )   return nullptr;
    return array[ 0 ];
}

void time_heap::pop_timer()
{
    if( empty() )   return;
    if( array[ 0 ] )    
    {
        delete array[ 0 ];
        array[ 0 ] = array[ --cur_size ];
        down( 0 );
    }
}

void time_heap::tick()
{
    heap_timer *tmp = array[ 0 ];
    time_t cur = time( nullptr );
    while ( !empty() )
    {
        if( !tmp )  break;
        if( tmp->expire > cur ) break;

        if( array[ 0 ]->cb_func )
        {
            array[ 0 ]->cb_func( array[ 0 ]->user_data );
        }
        pop_timer();
        tmp = array[ 0 ];
    }
}

bool time_heap::empty() const
{
    return cur_size == 0;
}

void time_heap::down( int hole )
{
    heap_timer *tmp = array[ hole ];
    while( hole < cur_size )
    {
        int ls = hole * 2 + 1;
        int rs = hole * 2 + 2;
        int pos = hole;
        if( ls < cur_size && array[ pos ]->expire > array[ ls ]->expire )
            pos = ls;
        if( rs < cur_size && array[ pos ]->expire > array[ rs ]->expire )
            pos = rs;
        if( pos == hole )
            break;
        std::swap( array[ pos ], array[ hole ] );
        hole = pos;
    }
}

void time_heap::resize()
{
    heap_timer **temp = new heap_timer* [ 2 * capacity ];
    for( int i = 0; i < 2 * capacity; ++i )
    {
        temp[ i ] = nullptr;
    }
    capacity = 2*capacity;
    for( int i = 0; i < cur_size; ++i )
    {
        temp[ i ] = array[ i ];
    }
    delete [] array;
    array = temp;
}