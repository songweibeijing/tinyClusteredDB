#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stddef.h>
#include <time.h>
#include <errno.h>

#define DST_ADDR "/var/run/uds_test.socket"
#define DST_ADDR1 "/var/run/uds_testkk.socket"

int main(int argc, char **argv)
{
    int sockfd;
    struct sockaddr_un dst;
    int ret;

    sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("create socket failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_un self;
    memset(&self, 0, sizeof(self));
    self.sun_family = AF_UNIX;
    strcpy(self.sun_path, DST_ADDR1);
    int selflen = sizeof(self);
    printf("len : %d\n", selflen);

    unlink(DST_ADDR1);
    if (bind(sockfd, (struct sockaddr *)&self, selflen) < 0)
    {
        perror("bind socket failed");
        exit(EXIT_FAILURE);
    }

    memset(&dst, 0, sizeof(dst));
    dst.sun_family = AF_UNIX;
    strcpy(dst.sun_path, DST_ADDR);
    int len = sizeof(dst);
    printf("len : %d\n", len);

    connect(sockfd, (struct sockaddr *) &dst, sizeof(dst));

    time_t t;
    char *str;

    for (;;)
    {
        t = time(NULL);
        str = ctime(&t);
        if (str == NULL)
        {
            break;
        }

        write(sockfd, str, strlen(str));

        char buff[BUFSIZ] = {0};
        int size = read(sockfd, buff, BUFSIZ);
        if (size < 0)
        {
            printf("sendto error %d, %s\n", errno, strerror(errno));
        }
        else
        {
            printf("data : %s\n", buff);
        }
        printf("3\n");

        sleep(1);
    }

    return 0;
}
