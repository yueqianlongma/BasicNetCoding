/*
*   使用gethostbyname
*   2020.12.3
*/

#include<iostream>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<string>
using namespace std;

int main(int argc, char **argv)
{
    while (--argc)
    {
        char* name = *++argv;
        struct hostent* hptr = gethostbyname(name);
        if(hptr == nullptr)
        {
            cout<<"gethostbyname error for host: "<<name<<" : "<<hstrerror(h_errno)<<endl;
            continue;
        }
        cout<<"Name: "<<hptr->h_name<<endl;
        cout<<"Aliases Name: "<<endl;
        for(char** ptr = hptr->h_aliases; *ptr != nullptr; ++ptr)
        {
            cout<<"\t "<<*ptr<<endl;
        }

        cout<<"Addr List: "<<endl;
        switch (hptr->h_addrtype)
        {
        case AF_INET:
        {
            for(char** ptr = hptr->h_addr_list; *ptr != nullptr; ++ptr)
            {
                in_addr* ip = (in_addr*)(*ptr);
                cout<<"\t "<<inet_ntoa(*ip)<<endl;
            }
            break;
        }
        default:
            break;
        }
    }
    
    return 0;
}