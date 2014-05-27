#ifndef _CORE_H_
#define _CORE_H_

#include <sys/types.h>  
#include <sys/socket.h>

struct Connection
{
    int fd;
    sockaddr client_addr;
};


#endif