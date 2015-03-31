#ifndef _UTIL_H
#define _UTIL_H

#if defined(__cplusplus)
extern "C" {
#endif

    int scnprintf(char *buf, size_t size, const char *fmt, ...);
    char *skip_blank(char *cp);
    char *skip_to_blank(char *cp);
    int empty_line(char *line);
    void get_timestamp(int level, char *txt);
    int file_getline(int fd, char *line, int len);
    int my_write(int fd, char *buffer, int size);
    int unix_socket_nb(char *path);
    int unix_socket_nonblock();
    int unix_send(int fd, char *dest, void *buf, size_t len);
    int unix_recv_tm(int fd, void *buf, size_t len, int tm_ms);

    void setblocking(int sock);
    void setnonblocking(int sock);
    int setnonlinger(int sd);
    int set_keepalive(int sd);
    int set_sndbuf(int sd, int size);
    int set_rcvbuf(int sd, int size);
    int get_sndbuf(int sd);
    int get_rcvbuf(int sd);
    void maximize_netbuf(const int sfd, int optname);

    ssize_t writen(int fd, const void *vptr, size_t n);
    ssize_t readn(int fd, void *vptr, size_t n);

    int generic_getport(struct sockaddr_storage *a);
    int generic_setport(struct sockaddr_storage *a, int port);
    char *generic_ntoa(struct sockaddr_storage *a);
    void generic_dumpaddr(struct sockaddr_storage *a);
    int generic_ss_size(struct sockaddr_storage *ss);
    int generic_aton(char *name, struct sockaddr_storage *addr);

#if defined(__cplusplus)
}
#endif


#endif
