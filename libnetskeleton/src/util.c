#include "comm.h"
#include "util.h"

#define MAX_SENDBUF_SIZE (256 * 1024 * 1024)

int set_nonblock(int fd)
{
    int flags, ret;

    if ((flags = fcntl(fd, F_GETFL, 0)) < 0)
    {
        return -1;
    }

    if ((ret = fcntl(fd, F_SETFL, flags | O_NONBLOCK)) < 0)
    {
        return -1;
    }
    return 0;
}

int scnprintf(char *buf, size_t size, const char *fmt, ...)
{
    va_list args;
    int i;

    va_start(args, fmt);
    i = vsnprintf(buf, size, fmt, args);
    va_end(args);
    return (i >= size) ? (size - 1) : i;
}

char *skip_blank(char *cp)
{
    while (*cp && isspace(*cp))
    {
        cp++;
    }
    if (*cp)
    {
        return cp;
    }

    return NULL;
}

char *skip_to_blank(char *cp)
{
    while (*cp && !isspace(*cp))
    {
        cp++;
    }
    return cp;
}

int empty_line(char *line)
{
    while (isspace(*line))
    {
        line++;
    }
    return *line == 0;
}

void get_timestamp(int level, char *txt)
{

#define STRLEN 256
    time_t gmt;
    struct tm *timeptr;
    struct tm timestruct;

    time(&gmt);
    timeptr = localtime_r(&gmt, &timestruct);
    snprintf(txt, STRLEN,
             "%04d.%02d.%02d %02d:%02d:%02d",
             timeptr->tm_year + 1900, timeptr->tm_mon + 1, timeptr->tm_mday,
             timeptr->tm_hour, timeptr->tm_min, timeptr->tm_sec);
}

int file_getline(int fd, char *line, int len)
{
    int i;
    int num;

    if (!fd) /* not opened */
    {
        return 0;
    }

    for (i = 0; i < len - 1; i++)
    {
        if (i > 0 && line[i - 1] == '\n')
        {
            break;
        }
        num = read(fd, line + i, 1);
        if (num != 1)
        {
            break;
        }
    }
    line[i] = '\0';
    return i;
}

int my_write(int fd, char *buffer, int size)
{
    while (1)
    {
        if (write(fd, buffer, size) < 0)
        {
            if (EINTR == errno)
            {
                continue;
            }
            else
            {
                return -1;
            }
        }
        else
        {
            return 0;
        }
    }
    return 0;
}

int unix_send(int fd, char *dest, void *buf, size_t len)
{
    struct sockaddr_un addr;

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    addr.sun_path[0] = 0;
    strncpy(&addr.sun_path[1], dest, sizeof(addr.sun_path) - 1);
    addr.sun_path[sizeof(addr.sun_path) - 1] = 0;

    return sendto(fd, buf, len, 0, (struct sockaddr *)&addr, sizeof(addr));
}

int unix_recv_tm(int fd, void *buf, size_t len, int tm_ms)
{
    int ret;
    struct pollfd pollfd;

    for (;;)
    {
        pollfd.fd = fd;
        pollfd.events = POLLIN;
        pollfd.revents = 0;

        ret = poll(&pollfd, 1, tm_ms);
        if (ret < 0 && errno == EINTR)
        {
            continue;
        }
        else
        {
            break;
        }
    }

    if (ret <= 0 || !(pollfd.revents & POLLIN))
    {
        return -1;
    }

    return read(fd, buf, len);
}

void setblocking(int sock)
{
    int opts;
    opts = fcntl(sock, F_GETFL);
    if (opts < 0)
    {
        perror("fcntl(sock,GETFL)");
        exit(1);
    }
    opts = opts & ~O_NONBLOCK;
    if (fcntl(sock, F_SETFL, opts) < 0)
    {
        perror("fcntl(sock,SETFL,opts)");
        exit(1);
    }
}

void setnonblocking(int sock)
{
    int opts;
    opts = fcntl(sock, F_GETFL);
    if (opts < 0)
    {
        perror("fcntl(sock,GETFL)");
        exit(1);
    }
    opts = opts | O_NONBLOCK;
    if (fcntl(sock, F_SETFL, opts) < 0)
    {
        perror("fcntl(sock,SETFL,opts)");
        exit(1);
    }
}

int setnonlinger(int sd)
{
    struct linger nolinger = { .l_onoff = 1, .l_linger = 0 };
    return setsockopt(sd, SOL_SOCKET, SO_LINGER, (struct linger *) &nolinger, sizeof(struct linger));
}

int set_keepalive(int sd)
{
    int keepAlive = 1;
    return setsockopt(sd, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepAlive, sizeof(keepAlive));
}

int set_sndbuf(int sd, int size)
{
    socklen_t len;
    len = sizeof(size);
    return setsockopt(sd, SOL_SOCKET, SO_SNDBUF, &size, len);
}

int set_rcvbuf(int sd, int size)
{
    socklen_t len;
    len = sizeof(size);
    return setsockopt(sd, SOL_SOCKET, SO_RCVBUF, &size, len);
}

int get_sndbuf(int sd)
{
    int status, size;
    socklen_t len;
    size = 0;
    len = sizeof(size);
    status = getsockopt(sd, SOL_SOCKET, SO_SNDBUF, &size, &len);
    if (status < 0)
    {
        return status;
    }
    return size;
}
int get_rcvbuf(int sd)
{
    int status, size;
    socklen_t len;
    size = 0;
    len = sizeof(size);
    status = getsockopt(sd, SOL_SOCKET, SO_RCVBUF, &size, &len);
    if (status < 0)
    {
        return status;
    }
    return size;
}

void maximize_netbuf(const int sfd, int optname)
{
    socklen_t ingsize = sizeof(int);
    int last_good = 0;
    int min, max, avg;
    int old_size;

    /* Start with the default size. */
    if (getsockopt(sfd, SOL_SOCKET, optname, &old_size, &ingsize) != 0)
    {
        printf("failed to call gegsockopt, error:%s\n", strerror(errno));
        return;
    }

    /* Binary-search for the real maximum. */
    min = old_size;
    max = MAX_SENDBUF_SIZE;

    while (min <= max)
    {
        avg = ((unsigned int)(min + max)) / 2;
        if (setsockopt(sfd, SOL_SOCKET, optname, (void *)&avg, ingsize) == 0)
        {
            last_good = avg;
            min = avg + 1;
        }
        else
        {
            max = avg - 1;
        }
    }
    printf("<%d send buffer was %d, now %d\n", sfd, old_size, last_good);
}

ssize_t readn(int fd, void *vptr, size_t n)
{
    size_t nleft;
    ssize_t nread;
    char *ptr;
    ptr = (char *)vptr;
    nleft = n;
    while (nleft > 0)
    {
        if ((nread = read(fd, ptr, nleft)) < 0)
        {
            if (errno == EINTR)
            {
                nread = 0;    /* and call read() again */
            }
            else
            {
                return(-1);
            }
        }
        else if (nread == 0)
        {
            break;    /* EOF */
        }
        nleft -= nread;
        ptr   += nread;
    }
    return(n - nleft);  /* return >= 0 */
}

ssize_t writen(int fd, const void *vptr, size_t n)
{
    size_t  nleft;
    ssize_t  nwritten;
    const char *ptr;
    ptr = (const char *)vptr;
    nleft = n;
    while (nleft > 0)
    {
        if ((nwritten = write(fd, ptr, nleft)) <= 0)
        {
            if (nwritten < 0 && errno == EINTR)
            {
                nwritten = 0;    /* and call write() again */
            }
            else
            {
                return(-1);    /* error */
            }
        }
        nleft -= nwritten;
        ptr   += nwritten;
    }
    return(n);
}

/* Takes a struct sockaddr_storage and returns the port number in host order.
   For a Unix socket, the port number is 1.
*/
int generic_getport(struct sockaddr_storage *a)
{
    struct sockaddr_in *si;
    struct sockaddr_in6 *si6;

    switch (a->ss_family)
    {
        case AF_UNIX:
            return 1;
        case AF_INET:
            si = (struct sockaddr_in *)a;
            return ntohs(si->sin_port);
        case AF_INET6:
            si6 = (struct sockaddr_in6 *)a;
            return ntohs(si6->sin6_port);
        default:
            printf("generic_getport: Unknown address family %d", a->ss_family);
    }
    return 0;
}

int generic_setport(struct sockaddr_storage *a, int port)
{
    struct sockaddr_in *si;
    struct sockaddr_in6 *si6;

    switch (a->ss_family)
    {
        case AF_UNIX:
            /* No port for Unix domain sockets */
            return 1;
        case AF_INET:
            si = (struct sockaddr_in *)a;
            si->sin_port = htons(port);
            return 1;
        case AF_INET6:
            si6 = (struct sockaddr_in6 *)a;
            si6->sin6_port = htons(port);
            return 1;
        default:
            printf("generic_setport: Unknown address family %d", a->ss_family);
    }
    return 0;
}

/* Takes a struct sockaddr_storage and returns the name in a static buffer.
   The address can be a unix socket path or an ipv4 or ipv6 address.
*/
char *generic_ntoa(struct sockaddr_storage *a)
{
    static char b[1024];
    struct sockaddr_in *si;
    struct sockaddr_in6 *si6;
    struct sockaddr_un *su;

    switch (a->ss_family)
    {
        case AF_INET:
            si = (struct sockaddr_in *)a;
            snprintf(b, sizeof b, "%s", inet_ntoa(si->sin_addr));
            break;
        case AF_INET6:
            si6 = (struct sockaddr_in6 *)a;
            if (inet_ntop(AF_INET6, &si6->sin6_addr, b, sizeof b) == NULL)
            {
                printf("generic_ntoa: can't convert address");
                strcpy(b, "(cannot convert address)");
            }
            break;
        case AF_UNIX:
            su = (struct sockaddr_un *)a;
            snprintf(b, sizeof b, "%s", su->sun_path);
            break;
        default:
            printf("generic_ntoa: unknown address family %d", a->ss_family);
            sprintf(b, "(unknown address family %d", a->ss_family);
    }
    return b;
}

void generic_dumpaddr(struct sockaddr_storage *a)
{
    switch (a->ss_family)
    {
        case AF_INET:
            printf("Family: AF_INET");
            printf("Port: %d", generic_getport(a));
            printf("Address: %s", generic_ntoa(a));
            break;
        case AF_INET6:
            printf("Family: AF_INET6");
            printf("Port: %d", generic_getport(a));
            printf("Address: %s", generic_ntoa(a));
            break;
        case AF_UNIX:
            printf("Family: AF_UNIX");
            printf("Path: %s", generic_ntoa(a));
            break;
        default:
            printf("generic_dumpaddr: Unknown address family %d", a->ss_family);
    }
}

int generic_ss_size(struct sockaddr_storage *ss)
{
    switch (ss->ss_family)
    {
        case AF_UNIX:
            return sizeof(struct sockaddr_un);
        case AF_INET:
            return sizeof(struct sockaddr_in);
        case AF_INET6:
            return sizeof(struct sockaddr_in6);
        default:
            printf("generic_ss_size: unknown address family %d", ss->ss_family);
            return sizeof(struct sockaddr_storage);
    }
}

