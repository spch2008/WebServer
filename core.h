#ifndef _CORE_H_
#define _CORE_H_

#include <sys/types.h>  
#include <sys/socket.h>
#include <netdb.h>
#include <string>
#include <sys/select.h>
#include <unistd.h>
#include <errno.h>
#include <string>
#include <stdlib.h>
#include <fcntl.h>

class PoolManager;

struct Connection
{
    int fd;
    sockaddr client_addr;
};

#endif