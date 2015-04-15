#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stddef.h>
#include <errno.h>

#define SRC_ADDR "/var/run/uds_test.socket"

int main(int argc, char **argv)
{
    int sockfd;
    struct sockaddr_un src;
    struct sockaddr_un dest;
    int destlen = 0;

    unlink(SRC_ADDR);
    sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("create socket failed");
        exit(EXIT_FAILURE);
    }

    memset(&dest, 0, sizeof(dest));
    memset(&src, 0, sizeof(src));
    src.sun_family = AF_UNIX;
    strcpy(src.sun_path, SRC_ADDR);
    int len = sizeof(src);

    if (bind(sockfd, (struct sockaddr *)&src, len) < 0)
    {
        perror("bind socket failed");
        exit(EXIT_FAILURE);
    }

    size_t size = 0;
    char buf[BUFSIZ] = {'\0'};
    for (;;)
    {
        destlen = sizeof(dest);
        size = recvfrom(sockfd, buf, BUFSIZ, 0,
                        (struct sockaddr *)&dest, &destlen);

        if (size < 0)
        {
            printf("error ");
            break;
        }

        char dbg[1024] = {0};
        snprintf(dbg, 1024, "%s", dest.sun_path);
        printf("recv: %s, destlen: %d, dest: %s\n", buf, destlen, dbg);

        memset(&src, 0, sizeof(src));
        if (memcmp(&dest, &src, sizeof(dest)) == 0)
        {
            printf("it is empty\n");
        }

        char *reply = "dddddd";
        size = sendto(sockfd, reply, strlen(reply), 0, (struct sockaddr *)&dest, destlen);
        if (size < 0)
        {
            printf("sendto error %d, %s\n", errno, strerror(errno));
        }
        else
        {
            printf("send to %s\n", reply);
        }
    }

    return 0;
}
