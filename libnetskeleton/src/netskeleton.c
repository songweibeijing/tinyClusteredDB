#include "work_threads.h"
#include "netskeleton.h"
#include "util.h"

#define UDP_READ_BUFFER_SIZE 65536
#define DATA_BUFFER_SIZE 2048

#define NEED_MORE_DATA 1
#define CLOSE_CONNECTION 2

#define my_free(x) do {\
        if(!x) \
        {\
            free(x);\
            x=NULL;\
        }\
    }while(0)

#define PERROR(fmt, arg...) \
    ({ printf("[%s:%d]"fmt, __FUNCTION__, __LINE__, ##arg);})

enum try_read_result
{
    READ_DATA_RECEIVED,
    READ_NO_DATA_RECEIVED,
    READ_ERROR,            /** an error occured (on the socket) (or client closed connection) */
    READ_MEMORY_ERROR      /** failed to allocate more memory */
};

#define CONN_ERROR 0x00000001

typedef struct conn
{
    int type;
    int fd;
    struct event event;
    time_t active_time;     /** the last time this connection is accessed */

    int flags;
    int refercount;

    char   *rbuf;   /** buffer to read commands into */
    char   *rcurr;  /** but if we parsed some already, this is where we stopped */
    int    rsize;   /** total allocated size of rbuf */
    int    rbytes;  /** how much data, starting from rcur, do we have unparsed */

    //struct sockaddr_in request_addr; /** used in UDP, the client address */
    struct sockaddr_storage request_addr;
    int request_addr_size;
    struct generic_server *gs; /** the sever this connection belongs to */
    struct list_head list;
    pthread_mutex_t connlock;
} conn;

typedef struct generic_server
{
    int type;
    int id;

    unsigned short listen_port;
    char  ip[MAX_IP_LEN];
    char  unix_path[MAX_PATH_LEN];
    uint32_t max_live;
    server_process parse_request;
    server_process handle_request;

    int  lfd; /** listen fd in TCP mode */
    struct event_base *main_event_base; /** the main base */
    struct event tm_event; /** the time out event */

    int conn_num;
    struct list_head conn_head;
    struct list_head list;

    //thread info
    int mt;
    int thread_num;
    int max_queue_num;
    work_threads *pworks;

    pthread_mutex_t serverlock;
} generic_server;

typedef struct generic_timer
{
    int id;
    int repeat;
    uint32_t time_interval;
    timer_process timeout_func;
    struct event event;
    void *main_event_base;
    struct list_head list;
} generic_timer;


void set_conn_error(conn *curr_conn)
{
    if (curr_conn)
    {
        pthread_mutex_lock(&curr_conn->connlock);
        curr_conn->flags = CONN_ERROR;
        pthread_mutex_unlock(&curr_conn->connlock);
    }
}

int is_conn_error(conn *curr_conn)
{
    int flag = 0;
    if (curr_conn)
    {
        pthread_mutex_lock(&curr_conn->connlock);
        flag = (curr_conn->flags == CONN_ERROR);
        pthread_mutex_unlock(&curr_conn->connlock);
    }
    return flag;
}

void conn_ref(conn *curr_conn)
{
    if (curr_conn)
    {
        pthread_mutex_lock(&curr_conn->connlock);
        curr_conn->refercount++;
        pthread_mutex_unlock(&curr_conn->connlock);
        return;
    }
}

void conn_free_internal(conn *curr_conn)
{
    if (NULL == curr_conn)
    {
        PERROR("curr conn is NULL\n");
        return;
    }

    pthread_mutex_lock(&curr_conn->connlock);
    curr_conn->refercount--;
    if (curr_conn->refercount > 0)
    {
        pthread_mutex_unlock(&curr_conn->connlock);
        return;
    }
    pthread_mutex_unlock(&curr_conn->connlock);

    event_del(&curr_conn->event);
    if (curr_conn->fd > 0)
    {
        close(curr_conn->fd);
        curr_conn->fd = -1;
    }
    list_del(&curr_conn->list);
    my_free(curr_conn->rbuf);
    my_free(curr_conn);
}

void conn_free(conn *curr_conn)
{
    if (NULL == curr_conn)
    {
        PERROR("curr conn is NULL\n");
        return;
    }
    generic_server *gs = curr_conn->gs;
    pthread_mutex_lock(&curr_conn->connlock);
    curr_conn->refercount--;
    if (curr_conn->refercount != 0)
    {
        pthread_mutex_unlock(&curr_conn->connlock);
        return;
    }
    pthread_mutex_unlock(&curr_conn->connlock);

    event_del(&curr_conn->event);
    if (curr_conn->fd > 0)
    {
        close(curr_conn->fd);
        curr_conn->fd = -1;
    }

    pthread_mutex_lock(&gs->serverlock);
    list_del(&curr_conn->list);
    pthread_mutex_unlock(&gs->serverlock);

    my_free(curr_conn->rbuf);
    my_free(curr_conn);
}

void basic_read_request(int fd, short event, void *arg);
int add_conn(int fd, void *arg)
{
    printf("add connection\n");

    conn *curr_conn = NULL;
    generic_server *gs = (generic_server *)arg;

    curr_conn = (conn *)calloc(1, sizeof(*curr_conn));
    if (curr_conn == NULL)
    {
        PERROR("malloc conn error\n");
        return -1;
    }

    curr_conn->refercount = 1;
    curr_conn->flags = 0;
    curr_conn->type = gs->type;
    curr_conn->fd = fd;
    curr_conn->gs = gs;
    if (gs->type & TCP_MASK)
    {
        curr_conn->rsize = DATA_BUFFER_SIZE;
    }
    else
    {
        curr_conn->rsize = UDP_READ_BUFFER_SIZE;
    }
    if (gs->type & PORT_MASK)
    {
        curr_conn->request_addr_size = sizeof(struct sockaddr_in);
    }
    else
    {
        curr_conn->request_addr_size = sizeof(struct sockaddr_un);
    }

    curr_conn->rbuf = (char *)calloc(1, curr_conn->rsize);
    if (curr_conn->rbuf == NULL)
    {
        free(curr_conn);
        PERROR("malloc conn error\n");
        return -1;
    }

    pthread_mutex_init(&curr_conn->connlock, NULL);
    curr_conn->active_time = time(NULL);
    INIT_LIST_HEAD(&curr_conn->list);

    pthread_mutex_lock(&gs->serverlock);
    list_add_tail(&curr_conn->list, &gs->conn_head);
    pthread_mutex_unlock(&gs->serverlock);

    event_set(&curr_conn->event, fd, EV_READ | EV_PERSIST, basic_read_request, curr_conn);
    event_base_set(gs->main_event_base, &curr_conn->event);
    event_add(&curr_conn->event, NULL);
    return 0;
}

void do_accept(int listen_fd, short event, void *arg)
{
    printf("accept new connections\n");

    int fd = 0;
    fd = accept(listen_fd, NULL, NULL);
    if (fd < 0)
    {
        PERROR("failed to accept, error:%s\n", strerror(errno));
        return;
    }

    setnonblocking(fd);
    setnonlinger(fd);
    set_keepalive(fd);

    if (add_conn(fd, arg) != 0)
    {
        PERROR("call add_conn failed\n");
        goto failed;
    }

    return;

failed:
    if (fd > 0)
    {
        close(fd);
    }
    return;
}

int add_listen_conn(int listenfd, void *arg)
{
#ifdef DEBUG
    PERROR("begin to add listen conn\n");
#endif
    conn *curr_conn = NULL;
    generic_server *gs = (generic_server *)arg;

    curr_conn = (conn *)calloc(1, sizeof(conn));
    if (curr_conn == NULL)
    {
        PERROR("calloc conn error\n");
        return -1;
    }

    curr_conn->refercount = 1;
    curr_conn->flags = 0;
    INIT_LIST_HEAD(&curr_conn->list);
    curr_conn->fd = listenfd;
    curr_conn->active_time = time(NULL);

    pthread_mutex_lock(&gs->serverlock);
    list_add_tail(&curr_conn->list, &gs->conn_head);
    pthread_mutex_unlock(&gs->serverlock);

    event_set(&curr_conn->event, listenfd, EV_READ | EV_PERSIST, do_accept, arg);
    event_base_set(gs->main_event_base, &curr_conn->event);
    event_add(&curr_conn->event, NULL);

    return 0;
}

static int try_read_udp(conn *c)
{
    int res = -1;
    c->rcurr = c->rbuf;
    c->rbytes = 0;

    socklen_t sz = c->request_addr_size;
    res = recvfrom(c->fd, c->rbuf, c->rsize,
                   0, (struct sockaddr *)&c->request_addr, &sz);
    c->rbytes = res;

    return READ_DATA_RECEIVED;
}

static int try_read_tcp(conn *c)
{
    int gotdata = READ_NO_DATA_RECEIVED;
    int res = -1;
    int num_allocs = 0;

    if (c->rcurr != c->rbuf)
    {
        if (c->rbytes != 0) /* otherwise there's nothing to copy */
        {
            memmove(c->rbuf, c->rcurr, c->rbytes);
        }
        c->rcurr = c->rbuf;
    }

    while (1)
    {
        if (c->rbytes >= c->rsize)
        {
            if (num_allocs == 8)
            {
                return gotdata;
            }
            ++num_allocs;
            char *new_rbuf = (char *)realloc(c->rbuf, c->rsize * 2);
            if (!new_rbuf)
            {
                PERROR("failed to call realloc\n");
                c->rbytes = 0; /* ignore what we read */
                return READ_MEMORY_ERROR;
            }
            c->rcurr = c->rbuf = new_rbuf;
            c->rsize *= 2;
        }

        int avail = c->rsize - c->rbytes;
        printf("begin to read\n");
        res = read(c->fd, c->rbuf + c->rbytes, avail);
        if (res > 0)
        {
            gotdata = READ_DATA_RECEIVED;
            c->rbytes += res;
            continue;
        }
        if (res == 0)
        {
            return READ_ERROR;
        }
        if (res == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                break;
            }
            return READ_ERROR;
        }
    }
    return gotdata;
}

static int give_response(void *arg, char *resp, int resplen)
{
    int ret = -1;
    conn *curr_conn = (conn *)arg;
    if (curr_conn->type & TCP_MASK)
    {
        ret = write(curr_conn->fd, resp, resplen);
        if (ret < 0)
        {
            printf("failed to write, error:%d, strerror:%s\n", errno, strerror(errno));
        }
    }
    else
    {
        char dbg[4096] = {0};
        memcpy(dbg, resp, resplen);
        ret = sendto(curr_conn->fd, resp, resplen, 0, (struct sockaddr *)&curr_conn->request_addr, curr_conn->request_addr_size);
        if (ret < 0)
        {
            printf("failed to sendto, error:%d, strerror:%s\n", errno, strerror(errno));
        }
    }
    return ret;
}

typedef struct post_params
{
    conn *curr_conn;
    char *request;
    int  reqlen;
} post_params;

static int *post_handle(void *arg)
{
    generic_server *gs = NULL;
    post_params *post = (post_params *) arg;
    conn *curr_conn = post->curr_conn;
    char *request = post->request;
    int reqlen = post->reqlen;
    int ret = -1;
    char *response = NULL;
    int resplen = 0;

    my_free(post);
    if (!curr_conn || is_conn_error(curr_conn))
    {
        if (gs->type & TCP_MASK)
        {
            conn_free(curr_conn);
        }
        return NULL;
    }

    gs = curr_conn->gs;
    ret = gs->handle_request(request, reqlen, (void **)&response, &resplen);
    if (ret == 0)
    {
        if (give_response(curr_conn, response, resplen) < 0)
        {
            printf("failed to give response\n");
        }
    }

    my_free(request);
    my_free(response);
    if (gs->type & TCP_MASK)
    {
        conn_free(curr_conn);
    }
    return NULL;
}

static int handle_one_request(conn *curr_conn, char *request, int reqlen)
{
    int ret = -1;
    generic_server *gs = curr_conn->gs;
    char *response = NULL;
    int resplen = 0;

    if (gs->mt == 0) //no multi thread support.
    {
        ret = gs->handle_request(request, reqlen, (void **)&response, &resplen);
        if (ret == 0)
        {
            if (give_response(curr_conn, response, resplen) < 0)
            {
                ret = -1;
            }
        }

        my_free(request);
        my_free(response);
        return ret;
    }
    else
    {
        post_params *postparams = (post_params *)calloc(1, sizeof(post_params));
        if (postparams)
        {
            conn_ref(curr_conn);
            postparams->curr_conn = curr_conn;
            postparams->request = request;
            postparams->reqlen = reqlen;
            create_job(gs->pworks, (job_handle)post_handle, (void *)postparams);
            return 0;
        }
        return -1;
    }
}

void basic_read_request(int fd, short event, void *arg)
{
    conn *curr_conn = (conn *)arg;
    generic_server *gs = curr_conn->gs;
    enum try_read_result res;
    int ret = 0, stop = 0;
    char *resp = NULL;
    int resplen = 0;

    curr_conn->active_time = time(NULL);
    if (gs->type & TCP_MASK)
    {
        res = (enum try_read_result)try_read_tcp(curr_conn);
    }
    else
    {
        res = (enum try_read_result)try_read_udp(curr_conn);
    }

    if (res == READ_ERROR)
    {
        printf("close connection\n");
        if (gs->type & TCP_MASK)
        {
            set_conn_error(curr_conn);
            conn_free(curr_conn);
        }
    }
    else if (res == READ_DATA_RECEIVED)
    {
        while (curr_conn->rbytes > 0  && !stop)
        {
            resp = NULL;
            resplen = 0;
            ret = gs->parse_request(curr_conn->rcurr, curr_conn->rbytes, (void **)&resp, &resplen);
            //process ok, return the length of data have been parsed.
            if (ret > 0)
            {
                curr_conn->rcurr += ret;
                curr_conn->rbytes -= ret;

                //resp is allocated from heap.
                if (resp != NULL)
                {
                    if (handle_one_request(curr_conn, resp, resplen) != 0)
                    {
                        stop = CLOSE_CONNECTION;
                    }
                }
            }
            else if (ret == 0)
            {
                //need more data.
                //wait for next loop.
                stop = NEED_MORE_DATA;
            }
            else
            {
                stop = CLOSE_CONNECTION;
            }

            //parse error. close the connection if it is TCP.
            if (stop == CLOSE_CONNECTION && (gs->type & TCP_MASK))
            {
                set_conn_error(curr_conn);
                conn_free(curr_conn);
            }
        }
    }
    //otherwise, just return
    return;
}

static void destroy_generic_server(generic_server *gs)
{
    if (gs == NULL)
    {
        return;
    }
    if (gs->lfd > 0)
    {
        close(gs->lfd);
        gs->lfd = 0;
    }

    struct list_head *list = NULL;
    conn *curr_conn = NULL;
    pthread_mutex_lock(&gs->serverlock);
    list_for_each(list, &gs->conn_head)
    {
        curr_conn = list_entry(list, struct conn, list);
        set_conn_error(curr_conn);
        conn_free_internal(curr_conn);
    }
    pthread_mutex_unlock(&gs->serverlock);
    pthread_mutex_destroy(&gs->serverlock);
    my_free(gs);
}

static int init_conn_expire_timer(void *arg);
void expire_conn_process(int fd, short event, void *arg)
{
    struct list_head *ptr = NULL;
    conn *curr_conn = NULL;
    generic_server *gs = (generic_server *)arg;

    if ((gs->type & TCP_MASK) == 0)
    {
        return;
    }

    time_t now = time(NULL);
    pthread_mutex_lock(&gs->serverlock);
    list_for_each(ptr, &gs->conn_head)
    {
        curr_conn = list_entry(ptr, conn, list);
        if (curr_conn->fd == gs->lfd)
        {
            continue;
        }
        if (gs->max_live <= 0)
        {
            continue;
        }
        if (curr_conn->active_time + gs->max_live > now)
        {
            continue;
        }
        set_conn_error(curr_conn);
        conn_free_internal(curr_conn);
    }
    pthread_mutex_unlock(&gs->serverlock);

    init_conn_expire_timer(arg);
}

int init_conn_expire_timer(void *arg)
{
    struct event *ev = NULL;
    struct timeval timeout;
    generic_server *gs = (generic_server *)arg;

    ev = &gs->tm_event;
    if (event_initialized(ev))
    {
        event_del(ev);
    }
    evtimer_set(ev, expire_conn_process, gs);
    event_base_set(gs->main_event_base, ev);

    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    evtimer_add(ev, &timeout);

    return 0;
}

int bind_tcp_port(char *ip, unsigned short port)
{
#ifdef DEBUG
    PERROR("begin to bind tcp port\n");
#endif

    struct sockaddr_in addr;
    int fd = 0, on = 0;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        PERROR("socket error:%s!\n", strerror(errno));
        return -1;
    }

#ifdef DEBUG
    PERROR("listen fd:%d on port:%d\n", fd, port);
#endif

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (ip == NULL || strlen(ip) == 0)
    {
        addr.sin_addr.s_addr = INADDR_ANY;
    }
    else
    {
        addr.sin_addr.s_addr = inet_addr(ip);
    }

    on = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        PERROR("bind error:%s!\n", strerror(errno));
        goto close_err;
    }

    return fd;

close_err:
    if (fd > 0)
    {
        close(fd);
    }
    return -1;
}

int bind_tcp_path(char *path)
{
#ifdef DEBUG
    PERROR("begin to call bind tcp path\n");
#endif

    struct sockaddr_un addr;
    int fd = 0;

    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0)
    {
        PERROR("create socket error:%s!\n", strerror(errno));
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", path);
    unlink(addr.sun_path);
#ifdef DEBUG
    PERROR("listen fd:%d on path:%s\n", fd, addr.sun_path);
#endif
    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        PERROR("bind error:%s!\n", strerror(errno));
        goto err;
    }
    return fd;

err:
    if (fd > 0)
    {
        close(fd);
    }
    return -1;
}

int bind_udp_port(char *ip, unsigned short port)
{
#ifdef DEBUG
    PERROR("begin to call bind udp port\n");
#endif
    struct sockaddr_in addr;
    int fd = 0;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
    {
        PERROR("socket error:%s!\n", strerror(errno));
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (ip == NULL || strlen(ip) == 0)
    {
        addr.sin_addr.s_addr = INADDR_ANY;
    }
    else
    {
        addr.sin_addr.s_addr = inet_addr(ip);
    }
#ifdef DEBUG
    PERROR("listen fd:%d on port:%d\n", fd, port);
#endif
    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        PERROR("bind error:%s!\n", strerror(errno));
        goto close_err;
    }

    return fd;

close_err:
    if (fd > 0)
    {
        close(fd);
    }
    return -1;
}

int bind_udp_path(char *path)
{
#ifdef DEBUG
    PERROR("begin to call bind udp path\n");
#endif
    struct sockaddr_un addr;
    int fd = 0;

    fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (fd < 0)
    {
        PERROR("socket error:%s!\n", strerror(errno));
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", path);
    unlink(addr.sun_path);

#ifdef DEBUG
    PERROR("listen fd:%d on path:%s\n", fd, addr.sun_path);
#endif

    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        PERROR("bind error:%s!\n", strerror(errno));
        goto err;
    }
    return fd;

err:
    if (fd > 0)
    {
        close(fd);
    }
    return -1;
}

int bind_generic(void *arg)
{
    int fd = -1, buflen = -1, len = -1;

    generic_server *gs = (generic_server *) arg;
    if (gs->type == TCP_PORT)
    {
        fd = bind_tcp_port(gs->ip, gs->listen_port);
    }
    else if (gs->type == TCP_PATH)
    {
        fd = bind_tcp_path(gs->unix_path);
    }
    else if (gs->type == UDP_PORT)
    {
        fd = bind_udp_port(gs->ip, gs->listen_port);
    }
    else if (gs->type == UDP_PATH)
    {
        fd = bind_udp_path(gs->unix_path);
    }

    setnonlinger(fd);
    setnonblocking(fd);
    if (gs->type & TCP_MASK) // tcp
    {
#ifdef DEBUG
        PERROR("call listen on port :%d\n", gs->listen_port);
#endif
        if (listen(fd, 1024) < 0)
        {
            PERROR("failed to listen on port %d, error:%s\n", gs->listen_port, strerror(errno));
            goto close_err;
        }
        set_keepalive(fd);
    }
    else  //udp
    {
        if (getsockopt(fd, SOL_SOCKET, SO_RCVBUF, &buflen, (socklen_t *)&len) > 0)
        {
            if (buflen < (2 * 1024 * 1024))
            {
                buflen = 2 * 1024 * 1024;
                setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &buflen, len);
            }
        }
    }
    return fd;

close_err:
    close(fd);
    return -1;
}

int init_generic_server(void *arg)
{
    generic_server *gs = (generic_server *) arg;
    int fd = bind_generic((void *)gs);
    if (fd < 0)
    {
        PERROR("port %u, failed error:%s\n", gs->listen_port, strerror(errno));
        return -1;
    }
    gs->lfd = fd;

    if (gs->type & TCP_MASK) //tcp
    {
        if (add_listen_conn(fd, arg) != 0)
        {
            goto failed;
        }
    }
    else //udp
    {
        if (add_conn(fd, arg) != 0)
        {
            goto failed;
        }
    }
    return 0;

failed:
#ifdef DEBUG
    PERROR("error\n");
#endif
    if (fd > 0)
    {
        close(fd);
    }
    return -1;
}

static generic_server *create_generic_server(server_param *param)
{
    generic_server *gs = (generic_server *)calloc(1, sizeof(generic_server));
    if (gs == NULL)
    {
        PERROR("malloc generic_server failed\n");
        return NULL;
    }

    pthread_mutex_init(&gs->serverlock, NULL);
    gs->listen_port = param->port;
    if (param->ip != NULL && strlen(param->ip) > 0)
    {
        snprintf(gs->ip, MAX_IP_LEN, "%s", param->ip);
    }
    gs->max_live = param->max_live;
    gs->conn_num = 0;
    gs->lfd = 0;
    gs->parse_request = param->parse_request;
    gs->handle_request = param->handle_request;
    if (param->path)
    {
        snprintf(gs->unix_path, MAX_PATH_LEN, "%s", param->path);
    }
    gs->type = param->type;
    INIT_LIST_HEAD(&gs->conn_head);
    INIT_LIST_HEAD(&gs->list);

    //threads config
    gs->thread_num = param->thread_num;
    gs->max_queue_num = param->max_queue_num;
    if (gs->thread_num < 0)
    {
        gs->thread_num = 4;
    }
    if (gs->max_queue_num < 0)
    {
        gs->max_queue_num = 10240;
    }
    if (gs->thread_num > 0)
    {
        gs->mt = 1;
        gs->pworks = init_work_threads(gs->thread_num, gs->max_queue_num);
        if (!gs->pworks)
        {
            my_free(gs);
            return NULL;
        }
    }
    //end

    return gs;
}

static int check_server_param(server_param *param)
{
    if (param == NULL)
    {
        PERROR("param is NULL\n");
        return -1;
    }
    if (param->parse_request == NULL)
    {
        PERROR("param has empty process function\n");
        return -1;
    }
    if (param->handle_request == NULL)
    {
        PERROR("param has empty process function\n");
        return -1;
    }
    if (param->type & PORT_MASK)
    {
        if (param->port <= 0)
        {
            PERROR("param has wrong port number:%d\n", param->port);
            return -1;
        }
    }
    if (param->type & PATH_MASK)
    {
        if (param->path == NULL || strlen(param->path) == 0)
        {
            PERROR("param has emtpy path\n");
            return -1;
        }
    }
    return 0;
}

static generic_server *build_generic_server(server_param *param, servers_array *servers)
{
    generic_server *gs = NULL;
    if (check_server_param(param) != 0)
    {
        PERROR("failed to check the param\n");
        return NULL;
    }

    gs = create_generic_server(param);
    if (gs == NULL)
    {
        PERROR("failed to call create_generic_server\n");
        return NULL;
    }

    gs->main_event_base = servers->main_event_base;
    if (init_generic_server((void *)gs) != 0)
    {
        PERROR("call init_generic_server\n");
        return NULL;
    }
    gs->id = servers->server_num++;
    list_add_tail(&gs->list, &servers->server_head);

    if ((param->type & TCP_MASK) && (param->max_live > 0))
    {
        init_conn_expire_timer((void *)gs);
    }

    return gs;
}

servers_array *get_servers()
{
    servers_array *servers = (servers_array *)calloc(1, sizeof(servers_array));
    if (servers == NULL)
    {
        PERROR("malloc servers_array failed\n");
        return NULL;
    }

    void *main_event_base = event_init();
    if (main_event_base == NULL)
    {
        PERROR("call event_init failed\n");
        goto failed;
    }
    servers->main_event_base = main_event_base;
    servers->server_num = 0;
    INIT_LIST_HEAD(&servers->server_head);
    INIT_LIST_HEAD(&servers->tm_head);
    return servers;

failed:
    my_free(servers);
    return NULL;
}

void destroy_servers(servers_array *servers)
{
    struct list_head *list = NULL;
    generic_server *curr_server = NULL;
    list_for_each(list, &servers->server_head)
    {
        curr_server = list_entry(list, generic_server, list);
        destroy_generic_server(curr_server);
    }
}

int get_servers_num(servers_array *servers)
{
    return servers->server_num;
}

int add_generic_server_param(servers_array *servers, server_param *param)
{
    if (!(param->type & SERVER_MASK))
    {
        PERROR("error type:%d\n", param->type);
        return -1;
    }

    generic_server *newserver = NULL;
    newserver = build_generic_server(param, servers);
    if (newserver == NULL)
    {
        PERROR("call build_generic_server failed\n");
        return -1;
    }
    return 0;
}

static int register_timer_job(generic_timer *timerobj)
{
    struct timeval timeout;

    if (event_initialized(&timerobj->event))
    {
        event_del(&timerobj->event);
    }
    evtimer_set(&timerobj->event, timerobj->timeout_func, NULL);
    event_base_set(timerobj->main_event_base, &timerobj->event);

    timeout.tv_sec = timerobj->time_interval;
    timeout.tv_usec = 0;
    evtimer_add(&timerobj->event, &timeout);
    return 0;
}

static void default_timer_job(int fd, short event, void *arg)
{
    generic_timer *timerobj = (generic_timer *) arg;
    if (timerobj->timeout_func != NULL)
    {
        timerobj->timeout_func(fd, event, arg);
    }
    if (timerobj->repeat != 0)
    {
        register_timer_job(timerobj);
    }
}

int add_timer_job(servers_array *servers, timer_param *param)
{
    if (param->type != TIMER_JOB)
    {
        PERROR("wrong type for timer :%d\n", param->type);
        return -1;
    }

    struct timeval timeout;
    generic_timer *timerobj = (generic_timer *)calloc(1, sizeof(generic_timer));
    if (timerobj == NULL)
    {
        PERROR("malloc generic_timer failed\n");
        return -1;
    }

    timerobj->id = servers->timer_num++;
    timerobj->time_interval = param->time_interval;
    timerobj->timeout_func = param->func_tm;
    timerobj->main_event_base = servers->main_event_base;
    timerobj->repeat = param->repeat;
    INIT_LIST_HEAD(&timerobj->list);
    list_add(&timerobj->list, &servers->tm_head);

    evtimer_set(&timerobj->event, default_timer_job, (void *)timerobj);
    event_base_set(timerobj->main_event_base, &timerobj->event);
    timeout.tv_sec = timerobj->time_interval;
    timeout.tv_usec = 0;
    evtimer_add(&timerobj->event, &timeout);

    return 0;
}

void start_servers_array_loop(servers_array *servers)
{
    event_base_dispatch(servers->main_event_base);
}

int init_global_env()
{
    signal(SIGPIPE, SIG_IGN);
    init_work_threads_env();
    return 0;
}
