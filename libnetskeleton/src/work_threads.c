#include <pthread.h>
#include "comm.h"
#include "work_threads.h"
#include "linux_list.h"

#define THREAD_STACK_SIZE (1024*1024*4)

struct job
{
    job_handle bh;
    void *arg;
    struct list_head list;
};

static unsigned int g_max_free_obj = 1024000;
static unsigned int g_free_objs = 0;
struct list_head free_job_list;
static pthread_mutex_t free_job_mutex;

static int g_show_logs = 0;
static unsigned int g_seed = 19870617;

struct job *new_job()
{
    struct job *job = NULL;

    pthread_mutex_lock(&free_job_mutex);
    if (!list_empty(&free_job_list))
    {
        job = list_entry(free_job_list.next, struct job, list);
        list_del(&job->list);
    }
    pthread_mutex_unlock(&free_job_mutex);

    if (job == NULL)
    {
        job = calloc(1, sizeof(*job));
        if (job != NULL)
        {
            pthread_mutex_lock(&free_job_mutex);
            g_free_objs++;
            pthread_mutex_unlock(&free_job_mutex);
        }
    }

    return job;
}

void free_job(struct job *jobs)
{
    if (jobs == NULL)
    {
        return;
    }

    int freeit = 0;

    pthread_mutex_lock(&free_job_mutex);
    if (g_free_objs >= g_max_free_obj)
    {
        freeit = 1;
        g_free_objs--;
    }
    else
    {
        list_add_tail(&jobs->list, &free_job_list);
    }
    pthread_mutex_unlock(&free_job_mutex);

    if (freeit)
    {
        my_free(jobs);
    }
}

void mysleep(int sec)
{
    struct timeval interval;
    interval.tv_sec = sec;
    interval.tv_usec = 0;
    select(0, NULL, NULL, NULL, &interval);
}

int init_work_threads_env()
{
    INIT_LIST_HEAD(&free_job_list);
    pthread_mutex_init(&free_job_mutex, NULL);
    return 0;
}

void *process_jobs(void *arg);

typedef struct thread_params
{
    work_threads *pthreads;
    int index;
} thread_params;

/* Initialize the background system, spawning the thread. */
work_threads *init_work_threads(int thread_num, int max_queue_num)
{
    pthread_attr_t attr;
    pthread_t thread;
    size_t stacksize = 0;
    int j = 0;

    work_threads *pworks = (work_threads *)calloc(1, sizeof(work_threads));
    if (pworks == NULL)
    {
        return NULL;
    }

    pworks->thread_num = thread_num;
    pworks->max_queue = max_queue_num;
    pworks->mutex = (pthread_mutex_t *)calloc(thread_num, sizeof(pthread_mutex_t));
    pworks->condvar = (pthread_cond_t *) calloc(thread_num, sizeof(pthread_cond_t));
    pworks->condvar_empty = (pthread_cond_t *)calloc(thread_num, sizeof(pthread_cond_t));
    pworks->jobs = (struct list_head *)calloc(thread_num, sizeof(struct list_head));
    pworks->pending = (unsigned long long *) calloc(thread_num, sizeof(unsigned long long));

    if (!pworks->mutex || !pworks->condvar ||
        !pworks->condvar_empty || !pworks->jobs || !pworks->pending)
    {
        goto failed;
    }

    /* Initialization of state vars and objects */
    for (j = 0; j < thread_num; j++)
    {
        pthread_mutex_init(&pworks->mutex[j], NULL);
        pthread_cond_init(&pworks->condvar[j], NULL);
        pthread_cond_init(&pworks->condvar_empty[j], NULL);
        INIT_LIST_HEAD(&pworks->jobs[j]);
        pworks->pending[j] = 0;
    }

    /* Set the stack size as by default it may be small in some system */
    pthread_attr_init(&attr);
    pthread_attr_getstacksize(&attr, &stacksize);
    if (!stacksize)
    {
        stacksize = 1;    /* The world is full of Solaris Fixes */
    }
    while (stacksize < THREAD_STACK_SIZE)
    {
        stacksize *= 2;
    }
    pthread_attr_setstacksize(&attr, stacksize);

    /* Ready to spawn our threads. We use the single argument the thread
     * function accepts in order to pass the job ID the thread is
     * responsible of. */
    for (j = 0; j < thread_num; j++)
    {
        thread_params *param = (thread_params *)calloc(1, sizeof(thread_params));
        if (param == NULL)
        {
            goto failed;
        }

        param->index = j;
        param->pthreads = pworks;
        if (pthread_create(&thread, &attr, process_jobs, param) != 0)
        {
            printf("Fatal: Can't initialize Background Jobs.\n");
            my_free(param);
            goto failed;
        }
    }
    return pworks;

failed:
    free_work_threads(pworks);
    return NULL;
}

void free_work_threads(work_threads *pworks)
{
    if (pworks == NULL)
    {
        return;
    }

    my_free(pworks->mutex);
    my_free(pworks->condvar);
    my_free(pworks->condvar_empty);
    my_free(pworks->jobs);
    my_free(pworks->pending);
    my_free(pworks);
}

int create_job(work_threads *pworks, job_handle bh, void *arg)
{
    int num = 0;
    struct job *job = new_job();
    if (job == NULL)
    {
        return -1;
    }

    job->bh = bh;
    job->arg = arg;

    printf("max queue :%d\n", pworks->thread_num);
    int index = rand_r(&g_seed) % pworks->thread_num;
    printf("index :%d\n", index);

    INIT_LIST_HEAD(&job->list);

    pthread_mutex_lock(&pworks->mutex[index]);
    list_add_tail(&job->list, &pworks->jobs[index]);
    num = ++pworks->pending[index];
    pthread_cond_signal(&pworks->condvar[index]);
    pthread_mutex_unlock(&pworks->mutex[index]);

    if (num > pworks->max_queue)
    {
        if (g_show_logs % 1000 == 0)
        {
            printf("There are so many jobs in queue\n");
            g_show_logs = 0;
        }
        g_show_logs++;
    }
    return 0;
}

void *process_jobs(void *arg)
{
    struct job *ln = NULL;
    thread_params *params = (thread_params *) arg;
    work_threads *pworks = params->pthreads;
    int index = params->index;

    pthread_detach(pthread_self());
    pthread_mutex_lock(&pworks->mutex[index]);
    while (1)
    {
        /* The loop always starts with the lock hold. */
        if (list_empty(&pworks->jobs[index]))
        {
            pthread_cond_wait(&pworks->condvar[index], &pworks->mutex[index]);
            continue;
        }
        /* Pop the job from the queue. */
        ln = list_entry(pworks->jobs[index].next, struct job, list);
        pthread_mutex_unlock(&pworks->mutex[index]);

        //process it here
        ln->bh(ln->arg);

        pthread_mutex_lock(&pworks->mutex[index]);
        list_del(&ln->list);
        free_job(ln);
        pworks->pending[index]--;
        if (pworks->pending[index] == 0)
        {
            pthread_cond_signal(&pworks->condvar_empty[index]);
        }
    }
}

unsigned long long work_thread_jobnum(work_threads *pworks, int index)
{
    unsigned long long val;
    pthread_mutex_lock(&pworks->mutex[index]);
    val = pworks->pending[index];
    pthread_mutex_unlock(&pworks->mutex[index]);
    return val;
}

unsigned long long work_threads_jobnum(work_threads *pworks)
{
    int i = 0;
    unsigned long long val = 0;
    for (i = 0; i < pworks->thread_num; i++)
    {
        val += work_thread_jobnum(pworks, i);
    }
    return val;
}

static void wait_for_one_thread(work_threads *pworks, int index)
{
    pthread_mutex_lock(&pworks->mutex[index]);
    while (1)
    {
        if (pworks->pending[index] == 0)
        {
            break;
        }

        if (pworks->pending[index] > 0)
        {
            pthread_cond_wait(&pworks->condvar_empty[index], &pworks->mutex[index]);
            continue;
        }
    }
    pthread_mutex_unlock(&pworks->mutex[index]);
}

//wait until all bio worker threads to finish their jobs.
//bio worker index starts from HANDLE_INOTIFY_THREADED to BIO_NUM_OPS.
void wait_for_threads(work_threads *pworks)
{
    int i = 0;
    for (; i < pworks->thread_num; i++)
    {
        wait_for_one_thread(pworks, i);
    }
}

