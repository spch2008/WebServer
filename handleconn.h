#ifndef _HANDLE_CONN_H_
#define _HANDLE_CONN_H_

#include "core.h"

using std::string;
class HandleConn
{
public:
    HandleConn(char *port);
    ~HandleConn();
    
    void StartMaster(void (*master_func)(void*));
protected:
    int Create_And_Binding(char *port);
    void Make_Socket_NonBlock(int fd);
    void Make_Listen_Socket(int fd);
    void PrintError(string err);
    static void *Master(void *arg);
    
private:
    int sfd;
    void (*callback)(void*);
};


#endif

