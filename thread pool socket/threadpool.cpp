#include <pthread.h>
#include <math.h>
#include <sys/types.h>  
#include <sys/syscall.h>
#include "threadpool.h"

threadpool::threadpool(int min_thr_num, int max_thr_num)
{
    if(min_thr_num < DEFAULT_MIN_THR_NUM || min_thr_num > DEFAULT_MAX_THR_NUM)
        min_thr_num = DEFAULT_MIN_THR_NUM; 
    
    m_min_thr_num = min_thr_num;
    m_max_thr_num = max_thr_num;
    m_busy_thr_num = 0;
    m_live_thr_num = min_thr_num;
    m_wait_exit_thr_num = 0;
    m_shutdown = false;

    if (pthread_mutex_init(&m_lock, NULL) != 0
        || pthread_mutex_init(&m_cnter_lock, NULL) != 0
        || sem_init(&m_sem_not_empty, 0, 0) != 0)
    {
        printf("init the lock or cond failed!\r\n");
        return;
    }

    /* 启动min_thr_num个work thread */
    for (int i = 0; i < m_min_thr_num; i++)
    {
        pthread_t thread;
        if(pthread_create(&thread, NULL, thread_member, this) != 0)
        {
            std::cout << "thread_member create failed!\r\n" << std::endl;
            return;
        }
        else
        {
            UpdateThrID(thread, true);
        }
    }
    if(pthread_create(&m_adjust_tid, NULL, adjust_thread, this) != 0)
    {
        std::cout << "adjust_thread create failed!\r\n" << std::endl;
        return;
    }
}

threadpool::~threadpool()//摧毁线程
{
    sem_destroy(&m_sem_not_empty);
    ClearPool();
}

bool threadpool::DetectKillThr(void)
{
    if (m_wait_exit_thr_num > 0)
    {
        /*如果线程池里线程个数大于最小值时可以结束当前线程*/
        UpdateLiveThr(-1);
        UpdateKillThr(false);
        return true;
    }
    return false;
}

void threadpool::DetectLiveThr(void)
{
    if(m_task_queue.fun_queue.size() > DEFAULT_MIN_THR_NUM)   //增加线程
    {
        if(m_wait_exit_thr_num != 0)
            UpdateKillThr((long)0);
        for (int i = 0; i < m_busy_thr_num ; i++)
        {
            if(m_live_thr_num > DEFAULT_MAX_THR_NUM)
                return;
            pthread_t thread;
            if(pthread_create(&thread, NULL, thread_member, this) != 0)
            {
                std::cout << "thread_member create failed!\r\n" << std::endl;
                return;
            }
            else
            {
                UpdateThrID(thread, true);
                UpdateLiveThr(1);
            }
        }
        printf("Living thread = %d!\r\n", m_live_thr_num);
    }
    else if(m_live_thr_num > DEFAULT_MIN_THR_NUM  && m_busy_thr_num < m_live_thr_num * 3 / 4 )
    {
        long killnum = ceil(m_live_thr_num / 10.0f);
        UpdateKillThr(killnum);
        for(int i = 0; i < killnum; i++)
        {
            /*通知处在空闲状态的线程*/
            sem_post(&m_sem_not_empty);
        }
    }
}

bool threadpool::FunQueuePop(void *(**fun)(void *), void ** arg)
{
    if(m_task_queue.fun_queue.size() == 0 || m_task_queue.arg_queue.size() == 0)
        return false;
    else
    {
        *fun = m_task_queue.fun_queue.front();
        *arg = m_task_queue.arg_queue.front();
        m_task_queue.fun_queue.pop();
        m_task_queue.arg_queue.pop();
        return true;
    }
}

void threadpool::FunQueuePush(void *(*fun)(void *), void * arg)
{
    if(fun == NULL)
        return;
    pthread_mutex_lock(&m_lock);
    m_task_queue.fun_queue.push(fun);
    m_task_queue.arg_queue.push(arg);
    pthread_mutex_unlock(&m_lock);
    //pthread_cond_signal(&m_queue_not_empty);
    sem_post(&m_sem_not_empty);
}

void threadpool::UpdateBusyThr(bool IncOrDec)
{
    char temp = 0;
    (IncOrDec == true) ? (temp = 1) : (temp = -1);
    pthread_mutex_lock(&m_cnter_lock);
    m_busy_thr_num += temp;
    pthread_mutex_unlock(&m_cnter_lock);
}

void threadpool::UpdateLiveThr(long IncOrDec)
{
    pthread_mutex_lock(&m_cnter_lock);
    m_live_thr_num += IncOrDec;
    pthread_mutex_unlock(&m_cnter_lock);
}

void threadpool::UpdateKillThr(long IncOrDec)
{
    pthread_mutex_lock(&m_cnter_lock);
    m_wait_exit_thr_num = IncOrDec;
    pthread_mutex_unlock(&m_cnter_lock);
}

void threadpool::UpdateKillThr(bool AddOrDec)
{
    pthread_mutex_lock(&m_cnter_lock);
    (AddOrDec == true) ? (m_wait_exit_thr_num++) : (m_wait_exit_thr_num--);
    pthread_mutex_unlock(&m_cnter_lock);
}

void threadpool::UpdateThrID(pthread_t ID, bool AddOrDel)
{
    pthread_mutex_lock(&m_cnter_lock);
    if(AddOrDel == true)
        m_threads[ID] = true;
    else
        m_threads.erase(ID);
    pthread_mutex_unlock(&m_cnter_lock);
}

void threadpool::ClearPool(void)
{
    pthread_cancel(m_adjust_tid);   //关闭线程池大小调节的线程
    if(pthread_join(m_adjust_tid, NULL) == 0)   //等待该线程结束
    {
        printf("adjust_thread cancel success!\r\n");
    }
    UpdateKillThr((long)0);
    for(auto i : m_threads)
    {
        pthread_cancel(i.first);  //取消处于活动状态的线程；
        UpdateThrID(i.first, false);
    }
}

void threadpool::ShowData(void)
{
    printf("queue size       %d\r\n", (int)m_task_queue.fun_queue.size());
    printf("busy thread size %d\r\n", m_busy_thr_num);
    printf("live thread size %d\r\n", m_live_thr_num);
    printf("Create thread size %d\r\n", (int)m_threads.size());
    printf("wait kill thread %d\r\n", m_wait_exit_thr_num);
    int val = 0;
    sem_getvalue(&m_sem_not_empty, &val);
    printf("signal val %d\r\n", val);
    std::cout << std::endl;
}

void * thread_member(void *arg)
{
    threadpool * pool = (threadpool *)arg;
    pthread_detach(pthread_self()); //分离线程，线程销毁时自动释放资源
    printf("starting thread TID = %d\r\n", (int)syscall(SYS_gettid));

    while(true)
    {
        /* Lock must be taken to wait on conditional variable */
        /*刚创建出线程，等待任务队列里有任务，否则阻塞等待任务队列里有任务后再唤醒接收任务*/
        pthread_mutex_lock(&(pool->m_lock));

        /*如果指定了true，要关闭线程池里的每个线程，自行退出处理*/
        if (pool->m_shutdown)
        {
            pthread_mutex_unlock(&(pool->m_lock));
            printf("thread 0x%x is exiting\n", (unsigned int)pthread_self());
            pool -> UpdateThrID(pthread_self(), false);
            pthread_exit(NULL);
        }

        void * (*fun)(void *);  //线程服务函数入口指针
        void * arg = NULL;
        
        while (pool -> FunQueuePop(&fun, &arg) == false)
        {
            pthread_mutex_unlock(&(pool->m_lock));
            sem_wait(&(pool->m_sem_not_empty));
            pthread_mutex_lock(&(pool->m_lock));
            if(pool -> DetectKillThr())
            {
                pthread_mutex_unlock(&(pool->m_lock));
                pool -> UpdateThrID(pthread_self(), false);
                pthread_exit(NULL);
            }
        }

        pthread_mutex_unlock(&(pool->m_lock));

        /*执行任务*/
        pool -> UpdateBusyThr(true);
        (*fun)(arg);                               /*执行回调函数任务*/
        pool -> UpdateBusyThr(false);
    }
    return NULL;
}


void * adjust_thread(void * arg)
{
    threadpool * pool = (threadpool *)arg;
    //pthread_detach(pthread_self()); //分离线程，线程销毁时自动释放资源
    while(1)
    {
        sleep(1); //0.1秒检测一次
        pool -> DetectLiveThr();
    }
    return NULL;
}