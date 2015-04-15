#include <pthread.h>
#include "netskeleton.h"
#include <time.h>
#include "util.h"
#include <time.h>

#define SERVER "127.0.0.1"
#define TEST_TCP_PORT  12345
#define TEST_UDP_PORT  12346
#define TEST_UDP_PATH  "/tmp/2222"
#define TEST_TCP_PATH  "/tmp/1111"

//the structure is used for test
typedef struct head
{
    int len;
    int type;
} head;

int init_tcp_connection(char *server, short port)
{
    int fd;
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("create socket error, exit!\n");
        return -1;
    }

    struct sockaddr_in server_addr;
    socklen_t socklen = sizeof(server_addr);

    server_addr.sin_family = AF_INET;
    inet_aton(server, &server_addr.sin_addr);
    server_addr.sin_port = htons(port);

    if (connect(fd, (struct sockaddr *)&server_addr, socklen) < 0)
    {
        printf("can not connect to %s, error:%s!\n", server, strerror(errno));
        return -1;
    }

    return fd;
}

int init_tcppath_connection(char *path)
{
    struct sockaddr_un addr;
    int fd = 0;

    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0)
    {
        printf("create socket error:%s!\n", strerror(errno));
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", path);

    int socklen = sizeof(addr);
    if (connect(fd, (struct sockaddr *)&addr, socklen) < 0)
    {
        printf("can not connect to %s, error:%s!\n", path, strerror(errno));
        return -1;
    }

    return fd;
}

int get_udp_fd()
{
    int fd = 0;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
    {
        printf("socket error:%s!\n", strerror(errno));
        return -1;
    }

    return fd;
}


int get_udp_path_fd()
{
    int fd = 0;

    fd = socket(PF_UNIX, SOCK_DGRAM, 0);
    if (fd < 0)
    {
        printf("socket error:%s!\n", strerror(errno));
        return -1;
    }

    return fd;
}

static int writev_ex(int fd, const struct iovec *write_iov, int iov_len)
{
    struct iovec *curr_iov = (struct iovec *)write_iov;

    while (1)
    {
        if (writev(fd, curr_iov, iov_len) < 0)
        {
            int err = errno;
            if (err == EINTR)
            {
                continue;
            }
            else
            {
                printf("write ev error %d, strerror %s\n", errno, strerror(errno));
                return -1;
            }
        }
        else
        {
            break;
        }
    }

    return 0;
}

int communicate_to_server(int fd)
{
    int ret = 0;
    head headinfo;
    struct iovec vecs[2];

    char *pbody = "it is tcp request";

    headinfo.type = 1;
    headinfo.len = strlen(pbody);

    vecs[0].iov_base = (char *)&headinfo;
    vecs[0].iov_len = sizeof(headinfo);
    vecs[1].iov_base = pbody;
    vecs[1].iov_len = strlen(pbody);

    ret = writev_ex(fd, vecs, 2);
    if (ret != 0)
    {
        return -1;
    }

    ret = read(fd, &headinfo, sizeof(head));
    if (ret == sizeof(head))
    {
        char *buf = (char *)calloc(1, headinfo.len + 1);
        if (!buf)
        {
            printf("malloc failed\n");
            return -1;
        }
        ret = read(fd, buf, headinfo.len);
        if (ret != headinfo.len)
        {
            printf("read body error\n");
            return -1;
        }
        printf("resp: %s\n", buf);
        free(buf);
    }
    else
    {
        printf("read head error\n");
        return -1;
    }

    return 0;
}

int communicate_to_server_pipeline(int fd, int number)
{
    int ret = 0;
    head headinfo;
    struct iovec vecs[2];

    char *pbody = "it is tcp request";

    headinfo.type = 1;
    headinfo.len = strlen(pbody);

    vecs[0].iov_base = (char *)&headinfo;
    vecs[0].iov_len = sizeof(headinfo);
    vecs[1].iov_base = pbody;
    vecs[1].iov_len = strlen(pbody);

    int i = 0;
    while (i < number)
    {
        ret = writev_ex(fd, vecs, 2);
        if (ret != 0)
        {
            return -1;
        }
        i++;
    }

    i = 0;
    while (i < number)
    {
        ret = read(fd, &headinfo, sizeof(head));
        if (ret == sizeof(head))
        {
            char *buf = (char *)calloc(1, headinfo.len + 1);
            if (!buf)
            {
                printf("malloc failed\n");
                return -1;
            }
            ret = read(fd, buf, headinfo.len);
            if (ret != headinfo.len)
            {
                printf("read body error\n");
                return -1;
            }
            printf("resp: %s\n", buf);
            free(buf);
        }
        else
        {
            printf("read head error\n");
            return -1;
        }

        i++;
    }
    return 0;
}

int communicate_to_server_udp_pipeline(int fd, char *ip, short port, int number)
{
    head *phead = NULL;
    struct sockaddr_in addr;
    int len = 0;
    size_t addr_len = sizeof(struct sockaddr_in);
    char buffer[4096] = {0};

    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);

    char *pbody = "it is udp request";
    len = sizeof(head) + strlen(pbody);
    char *message = (char *)calloc(1, len);
    if (!message)
    {
        return -1;
    }

    char *tmp = message;
    phead = (head *)message;
    phead->type = 0;
    phead->len = strlen(pbody);
    message += sizeof(head);
    memcpy(message, pbody, phead->len);

    int i = 0, ret = 0;
    while (i < number)
    {
        ret = sendto(fd, tmp, len, 0, (struct sockaddr *)&addr, addr_len);
        if (ret < 0)
        {
            printf("call sendto failed : %d, %s\n", errno, strerror(errno));
        }
        i++;
    }
    free(tmp);

    i = 0;
    while (i < number)
    {
        len = recvfrom(fd, buffer, sizeof(buffer), 0,
                       (struct sockaddr *)&addr, &addr_len);
        if (len < 0)
        {
            printf("Receive from server: %s failed, error %d\n", ip, errno);
            return -1;
        }
        char *p = buffer;
        phead = (head *)p;
        len = phead->len;
        p += sizeof(head);
        printf("resp: %s\n", p);

        i++;
    }

    return 0;
}

int communicate_to_server_udp(int fd, char *ip, short port)
{
    head *phead = NULL;
    struct sockaddr_in addr;
    int len = 0;
    size_t addr_len = sizeof(struct sockaddr_in);
    char buffer[4096] = {0};

    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);

    char *pbody = "it is udp request";
    len = sizeof(head) + strlen(pbody);
    char *message = (char *)calloc(1, len);
    if (!message)
    {
        return -1;
    }

    char *tmp = message;
    phead = (head *)message;
    phead->type = 0;
    phead->len = strlen(pbody);
    message += sizeof(head);
    memcpy(message, pbody, phead->len);

    int ret = sendto(fd, tmp, len, 0, (struct sockaddr *)&addr, addr_len);
    if (ret < 0)
    {
        printf("call sendto failed : %d, %s\n", errno, strerror(errno));
    }
    free(tmp);

    len = recvfrom(fd, buffer, sizeof(buffer), 0,
                   (struct sockaddr *)&addr, &addr_len);
    if (len < 0)
    {
        printf("Receive from server: %s failed\n", ip);
        return -1;
    }

    char *p = buffer;
    phead = (head *)p;
    len = phead->len;
    p += sizeof(head);
    printf("resp: %s\n", p);
    return 0;
}

static int get_tmp_file(char *path)
{
    int fd = -1;
    int i = 0;
    char tmp[64] = {0};
    char dir_path[128] = {0};
    char *dir = "/tmp/";

    time_t t = time(NULL);
    strftime(tmp, sizeof(tmp), "%Y%m%d%H", localtime(&t));
    snprintf(dir_path, sizeof(dir_path), "%s%s/", dir, tmp);

    for (i = 0; i < 2; i++)
    {
        snprintf(path, 256, "%sXXXXXX", dir_path);
        fd = mkostemp(path, O_APPEND | O_SYNC);
        if (fd < 0)
        {
            mkdir(dir_path, 0777);
        }
        else
        {
            break;
        }
    }

    if (fd < 0)
    {
        return -1;
    }

    close(fd);
    return 0;
}

int communicate_to_server_udp_path(int fd, char *path)
{
    head *phead = NULL;
    struct sockaddr_un addr;
    int len = 0;
    char buffer[4096] = {0};

    bzero(&addr, sizeof(addr));
    addr.sun_family = AF_UNIX;
    snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", path);

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("connect socket failed");
        exit(EXIT_FAILURE);
    }

    char *pbody = "it is udp request";
    len = sizeof(head) + strlen(pbody);
    char *message = (char *)calloc(1, len);
    if (!message)
    {
        return -1;
    }

    char *tmp = message;
    phead = (head *)message;
    phead->type = 0;
    phead->len = strlen(pbody);
    message += sizeof(head);
    memcpy(message, pbody, phead->len);

    int ret = write(fd, tmp, len);
    if (ret < 0)
    {
        printf("call sendto failed : %d, %s\n", errno, strerror(errno));
    }

    free(tmp);
    len = read(fd, buffer, sizeof(buffer));
    if (len <= 0)
    {
        printf("Receive from server: %s failed\n", path);
        return -1;
    }

    char *p = buffer;
    phead = (head *)p;
    len = phead->len;
    p += sizeof(head);
    printf("resp: %s\n", p);

    return 0;
}

int communicate_to_server_udp_path_pipeline(int fd, char *path, int number)
{
    head *phead = NULL;
    struct sockaddr_un addr;
    int len = 0;
    char buffer[4096] = {0};

    bzero(&addr, sizeof(addr));
    addr.sun_family = AF_UNIX;
    snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", path);

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("connect socket failed");
        exit(EXIT_FAILURE);
    }

    char *pbody = "it is udp request";
    len = sizeof(head) + strlen(pbody);
    char *message = (char *)calloc(1, len);
    if (!message)
    {
        return -1;
    }

    char *tmp = message;
    phead = (head *)message;
    phead->type = 0;
    phead->len = strlen(pbody);
    message += sizeof(head);
    memcpy(message, pbody, phead->len);

    int i = 0;
    while (i < number)
    {
        int ret = write(fd, tmp, len);
        if (ret < 0)
        {
            printf("call sendto failed : %d, %s\n", errno, strerror(errno));
        }
        i++;
    }

    free(tmp);

    i = 0;
    while (i < number)
    {
        len = read(fd, buffer, sizeof(buffer));
        if (len < 0)
        {
            printf("Receive from server: %s failed, error %d\n", path, errno);
            return -1;
        }

        char *p = buffer;
        phead = (head *)p;
        len = phead->len;
        p += sizeof(head);
        printf("resp: %s\n", p);
        i++;
    }

    return 0;
}

int test_tcp_port(int loop, int type)
{
    int number = 10;
    int tcp_fd = init_tcp_connection(SERVER, TEST_TCP_PORT);
    if (tcp_fd < 0)
    {
        printf("failed to init connection to server : %s:%d\n", SERVER, TEST_TCP_PORT);
        return -1;
    }

    int i = 0;
    while (i < loop)
    {
        if (type == 1)
        {
            if (communicate_to_server(tcp_fd) != 0)
            {
                printf("comminicate to tcp server failed\n");
                break;
            }
            else
            {
                //printf("tcp ok\n");
            }
        }
        else if (type == 2)
        {
            if (communicate_to_server_pipeline(tcp_fd, number) != 0)
            {
                printf("comminicate to tcp server failed\n");
                break;
            }
            else
            {
                //printf("tcp ok\n");
            }
        }

        i++;
    }

    close(tcp_fd);
    return 0;
}

int test_udp_port(int loop, int type)
{
    int number = 10;
    int udp_fd = get_udp_fd();
    if (udp_fd < 0)
    {
        printf("failed to init connection to server : %s:%d\n", SERVER, TEST_UDP_PORT);
        return -1;
    }

    int i = 0;
    while (i < loop)
    {
        if (type == 1)
        {
            if (communicate_to_server_udp(udp_fd, SERVER, TEST_UDP_PORT) != 0)
            {
                printf("comminicate to udp server failed\n");
                break;
            }
            else
            {
                //printf("udp ok\n");
            }
        }
        else if (type == 2)
        {
            if (communicate_to_server_udp_pipeline(udp_fd, SERVER, TEST_UDP_PORT, number) != 0)
            {
                printf("comminicate to udp server failed\n");
                break;
            }
            else
            {
                //printf("udp ok\n");
            }
        }

        i++;
    }

    close(udp_fd);
    return 0;
}

int test_tcp_path(int loop, int type)
{
    int number = 10;
    int tcp_path_fd = init_tcppath_connection(TEST_TCP_PATH);
    if (tcp_path_fd < 0)
    {
        printf("failed to init connection to path : %s\n", TEST_TCP_PATH);
        return -1;
    }

    int i = 0;
    while (i < loop)
    {
        if (type == 1)
        {
            if (communicate_to_server(tcp_path_fd) != 0)
            {
                printf("comminicate to tcp path server failed\n");
                break;
            }
            else
            {
                //printf("tcp path ok\n");
            }
        }
        else if (type == 2)
        {
            if (communicate_to_server_pipeline(tcp_path_fd, number) != 0)
            {
                printf("comminicate to tcp path server failed\n");
                break;
            }
            else
            {
                //printf("tcp path ok\n");
            }
        }

        i++;
    }

    close(tcp_path_fd);
    return 0;
}

int test_udp_path(int loop, int type)
{
    int number = 10;
    int udp_path_fd = get_udp_path_fd();
    if (udp_path_fd < 0)
    {
        printf("failed to init connection to server : %s \n", TEST_UDP_PATH);
        return -1;
    }

    struct sockaddr_un self;
    char self_path[256] = {0};
    memset(&self, 0, sizeof(self));
    self.sun_family = AF_UNIX;
    get_tmp_file(self_path);
    strcpy(self.sun_path, self_path);

    printf("self_path : %s\n", self_path);
    unlink(self_path);
    if (bind(udp_path_fd, (struct sockaddr *)&self, sizeof(self)) < 0)
    {
        perror("bind socket failed");
        exit(EXIT_FAILURE);
    }

    int i = 0;
    while (i < loop)
    {
        if (type == 1)
        {
            if (communicate_to_server_udp_path(udp_path_fd, TEST_UDP_PATH) != 0)
            {
                printf("comminicate to udp path server failed\n");
                break;
            }
            else
            {
                //printf("udp path ok\n");
            }
        }
        else if (type == 2)
        {
            if (communicate_to_server_udp_path_pipeline(udp_path_fd, TEST_UDP_PATH, number) != 0)
            {
                printf("comminicate to udp path server failed\n");
                break;
            }
            else
            {
                //printf("udp path ok\n");
            }
        }

        i++;
    }

    close(udp_path_fd);
    return 0;
}


void *test(void *arg)
{
    int loop = 100;
    int type = (int)arg;

    if (type <= 2)
    {
        test_tcp_port(loop, type);
        test_tcp_path(loop, type);
        test_udp_port(loop, type);
        test_udp_path(loop, type);
    }
    else if (type == 3)
    {
        test_tcp_port(loop, 1);
    }
    else if (type == 4)
    {
        test_tcp_path(loop, 1);
    }
    else if (type == 5)
    {
        test_udp_port(loop, 1);
    }
    else if (type == 6)
    {
        test_udp_path(loop, 1);
    }

    return NULL;
}

int main(int argc, char **argv)
{
    int i, repeat = 10;
    pthread_t pid;

    int testone = 1;
    if (pthread_create(&pid, NULL, test, (void *)testone) != 0)
    {
        printf("Fatal: Can't initialize test-1 Thread.\n");
        return -1;
    }

    int testtwo = 2;
    if (pthread_create(&pid, NULL, test, (void *)testtwo) != 0)
    {
        printf("Fatal: Can't initialize test-1 Thread.\n");
        return -1;
    }

    int test_3 = 3;
    i = 0;
    while (i < repeat)
    {
        if (pthread_create(&pid, NULL, test, (void *)test_3) != 0)
        {
            printf("Fatal: Can't initialize test-1 Thread.\n");
            return -1;
        }
        i++;
    }

    i = 0;
    int test_4 = 4;
    while (i < repeat)
    {
        if (pthread_create(&pid, NULL, test, (void *)test_4) != 0)
        {
            printf("Fatal: Can't initialize test-1 Thread.\n");
            return -1;
        }
        i++;
    }

    i = 0;
    int test_5 = 5;
    while (i < repeat)
    {
        if (pthread_create(&pid, NULL, test, (void *)test_5) != 0)
        {
            printf("Fatal: Can't initialize test-1 Thread.\n");
            return -1;
        }
        i++;
    }

    i = 0;
    int test_6 = 6;
    while (i < repeat)
    {
        if (pthread_create(&pid, NULL, test, (void *)test_6) != 0)
        {
            printf("Fatal: Can't initialize test-1 Thread.\n");
            return -1;
        }
        i++;
    }

    while (1)
    {
        pause();
    }

    return 0;
}

