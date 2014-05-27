#ifndef _POOL_MANAGER_H_
#define _POOL_MANAGER_H_

#include <pthread.h>
#include <string>

using namespace std;

struct Task_Queue
{
    int *data;
    int  head;
    int  tail;
    int  size;
    
    pthread_mutex_t task_mutex;
    pthread_cond_t  task_empty;
    pthread_cond_t  task_full;
};

class PoolManager
{
public:
    PoolManager(int max_threads, int max_products, void (*func)(void*));
    ~PoolManager();
    void AddWorker(int fd);
    
protected:
    void PrintError(string err);
    void InitData(int max_threads, int max_products, void (*func)(void*));
    void InitThread();
    bool CreateThread();
    /* call back func */
    static void *worker(void *arg);
    
private:
    pthread_mutex_t thr_mutex;
    int idle_thread;
    int all_thread;
    int max_thread;
    
    Task_Queue task_queue;
    
    void (*worker_func)(void *arg);
};
































#endif