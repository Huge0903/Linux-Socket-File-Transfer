#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <map>
#include <queue>
#include <semaphore.h>

class threadpool
{
private:
    #define DEFAULT_MIN_THR_NUM 10
    #define DEFAULT_MAX_THR_NUM 1000
    typedef struct
    {
        std::queue< void *(*)(void *)> fun_queue;          /*函数指针，回调函数*/
        std::queue< void * > arg_queue;          /*上面函数的参数*/
    } threadpool_task_t;                    /*任务结构体*/

    pthread_t m_adjust_tid;               /*线程池管理线程tid*/
    std::map<pthread_t, bool> m_threads;            /*保存工作线程tid的数组*/
    threadpool_task_t m_task_queue;         /*任务队列*/
    int m_min_thr_num;                    /*线程组内默认最小线程数*/
    int m_max_thr_num;                    /*线程组内默认最大线程数*/
    int m_live_thr_num;                   /*当前存活线程个数*/
    int m_busy_thr_num;                   /*忙状态线程个数*/
    int m_wait_exit_thr_num;              /*等待销毁的线程个数*/

public:
    pthread_mutex_t m_lock;               /*用于锁住当前这个结构体体taskpool*/
    pthread_mutex_t m_cnter_lock;     /*记录忙状态线程个数*/
    //pthread_cond_t m_queue_not_empty;     /*任务队列里不为空时，通知等待任务的线程*/
    sem_t m_sem_not_empty;              /*任务队列里不为空时，通知等待任务的线程*/

    bool m_shutdown;                       /*线程池使用状态，true或false*/

    threadpool(int min_thr_num = DEFAULT_MIN_THR_NUM, int max_thr_num = DEFAULT_MAX_THR_NUM);
    ~threadpool();
    unsigned int FunQueueSize(void) {return m_task_queue.fun_queue.size();};
    bool DetectKillThr(void);
    void DetectLiveThr(void);
    bool FunQueuePop(void *(**fun)(void *), void ** arg);
    void FunQueuePush(void *(*fun)(void *), void * arg);
    void UpdateBusyThr(bool IncOrDec);
    void UpdateLiveThr(long IncOrDec);
    void UpdateKillThr(long IncOrDec);
    void UpdateKillThr(bool AddOrDec);
    void UpdateThrID(pthread_t ID, bool AddOrDel);
    void ClearPool(void);
    void ShowData(void);
};

void * thread_member(void * arg);
void * adjust_thread(void * arg);
