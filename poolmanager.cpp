#include "poolmanager.h"


PoolManager::PoolManager(int max_threads, int max_products, void (*func)(void*))
{
    InitData(max_threads, max_products, func);
}

PoolManager::~PoolManager()
{
}

void PoolManager::InitData(int max_threads, int max_products, void (*func)(void*))
{
    /* init thread pool */
    idle_thread = 0;
    all_thread  = 0;
    max_thread  = max_threads;
    
    if (pthread_mutex_init(&thr_mutex, NULL) != 0)
        PrintError("init mutex error!");
        
    /* init product queue */
    task_queue.head = task_queue.tail = 0;
    task_queue.size = max_products+1;
    task_queue.data = new int[task_queue.size];
    
    if (pthread_mutex_init(&task_queue.task_mutex, NULL) != 0)
        PrintError("init mutex error");
        
    if (pthread_cond_init(&task_queue.task_empty, NULL) != 0 ||
        pthread_cond_init(&task_queue.task_full, NULL))
        PrintError("init cond error");
        
    worker_func = func;
}
    
void PoolManager::InitThread()
{
}

bool PoolManager::CreateThread()
{
    pthread_t tid;
    
    if (pthread_create(&tid, NULL, worker, this) != 0)
        return false;
        
    if (pthread_detach(tid) != 0)
        return false;
    
    all_thread++;
    return true;
}

void PoolManager::AddWorker(int fd)
{
    if (fd < 0)
        return;
        
    pthread_mutex_lock(&task_queue.task_mutex);
    while (task_queue.tail + 1 == task_queue.head)
        pthread_cond_wait(&task_queue.task_empty, &task_queue.task_mutex);
        
    bool isEmpty = false;
    if (task_queue.tail == task_queue.head)
        isEmpty = true;
        
    task_queue.data[task_queue.tail] = fd;
    task_queue.tail = (task_queue.tail + 1) % task_queue.size;
    
    pthread_mutex_unlock(&task_queue.task_mutex);
    
    /* replace spin lock */
    pthread_mutex_lock(&thr_mutex);
    if (idle_thread == 0 && all_thread < max_thread)
        CreateThread();
    pthread_mutex_unlock(&thr_mutex);
    
    if (isEmpty)
        pthread_cond_signal(&task_queue.task_full);
}


void *PoolManager::worker(void *arg)
{
    if (arg == NULL)
        return NULL;
    
    PoolManager *pool  = static_cast<PoolManager*>(arg);
    Task_Queue  *queue = &pool->task_queue;
    
    while (true)
    {
        /* add idle thread , can replace spin lock*/
        pthread_mutex_lock(&pool->thr_mutex);
        pool->idle_thread++;
        pthread_mutex_unlock(&pool->thr_mutex);
    
        pthread_mutex_lock(&queue->task_mutex);
        while (queue->tail == queue->head)
            pthread_cond_wait(&queue->task_full, &queue->task_mutex);
            
        bool isFull = false;
        if (queue->tail + 1 == queue->head)
            isFull = true;
            
        int fd = queue->data[queue->head];
        queue->head = (queue->head + 1) % queue->size;
        
        pthread_mutex_unlock(&queue->task_mutex);
        
        if (isFull)
            pthread_cond_signal(&queue->task_empty);
        
        /* minus idle thread, can replace spin lock */
        pthread_mutex_lock(&pool->thr_mutex);
        pool->idle_thread--;
        pthread_mutex_unlock(&pool->thr_mutex);
        
        pool->worker_func(&fd); 
    }
}

void PoolManager::PrintError(string err)
{
}
