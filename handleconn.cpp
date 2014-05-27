#include "handleconn.h"

#define backlog 2048

HandleConn::HandleConn(char *port)
{
    sfd = Create_And_Binding(port);
    Make_Socket_NonBlock(sfd);
    Make_Listen_Socket(sfd);   
}

HandleConn::~HandleConn()
{
}

void *HandleConn::Master(void *arg)
{
    int res, fd;
    sockaddr client_addr;
    fd_set readfds;
    
    socklen_t sockaddr_len = sizeof(sockaddr);
    
    HandleConn *this_class = static_cast<HandleConn*>(arg);
    while (true)
    {
        FD_SET(this_class->sfd, &readfds);
        res = select(this_class->sfd+1, &readfds, NULL, NULL, NULL);
        if (res == -1)
            this_class->PrintError("select error");
            
        if (FD_ISSET(this_class->sfd, &readfds))
        {
            while (true)
            {
                int fd = accept(this_class->sfd, &client_addr, &sockaddr_len);
                if (fd == -1)
                {
                    if (errno == EAGAIN ||
                        errno == EWOULDBLOCK)
                        break;
                    else
                        this_class->PrintError("accept error");
                }
                else
                {
                    Connection *c = new Connection;
                    c->fd = fd;
                    c->client_addr = client_addr;
                    
                    this_class->callback(c);
                }
            }
        }
    }
}

void HandleConn::StartMaster(void (*master_func)(void*))
{
    callback = master_func;

    pthread_t tid;
    pthread_create(&tid, NULL, Master, this);
    pthread_detach(tid);
}

int HandleConn::Create_And_Binding(char *port)
{
    struct addrinfo hint, *result;
    
    hint.ai_flags    = AI_PASSIVE;
    hint.ai_family   = AF_INET;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_protocol = 0;
    
    if (getaddrinfo(NULL, port, &hint, &result) != 0)
        PrintError("getaddrinfo error");
        
      
    int fd, res;
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
        PrintError("create socket error");
        
    res = bind(fd, result->ai_addr, result->ai_addrlen);
    if (res != 0)
        PrintError("bind socket error");
      
    freeaddrinfo(result);
    
    return fd;
}
    
void HandleConn::Make_Socket_NonBlock(int fd)
{
    int flags, res;
    
    flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
        PrintError("get flags error");
    
    flags |= O_NONBLOCK;
    res = fcntl(fd, F_SETFL, flags);
    if (res == -1)
        PrintError("set flags error");
}


void HandleConn::Make_Listen_Socket(int fd)
{
    int res;
    
    res = listen(fd, backlog);
    if (res == -1)
        PrintError("listen error");
}


void HandleConn::PrintError(string err)
{
}











