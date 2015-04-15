#ifndef _BIO_H
#define _BIO_H

#include "comm.h"
#include "linux_list.h"
#include <pthread.h>

#if defined(__cplusplus)
extern "C" {
#endif

    typedef int (*job_handle)(void *arg);

    typedef struct work_threads
    {
        int thread_num;
        int max_queue;
        pthread_mutex_t *mutex;
        pthread_cond_t *condvar;
        pthread_cond_t *condvar_empty;
        struct list_head *jobs;
        unsigned long long *pending;
        int curr_index;
    } work_threads;

    /* Exported API */
    int init_work_threads_env();
    work_threads *init_work_threads(int thread_num, int max_queue_num);
    void free_work_threads(work_threads *threads);
    int create_job(work_threads *pworks, job_handle bh, void *arg);
    unsigned long long work_thread_jobnum(work_threads *pworks, int index);
    unsigned long long work_threads_jobnum(work_threads *pworks);
    void wait_for_work_threads(work_threads *pworks);


#if defined(__cplusplus)
}
#endif

#endif
