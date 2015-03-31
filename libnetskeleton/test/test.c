#include "netskeleton.h"
#include "util.h"

//the structure is used for test
typedef struct head
{
    int len;
    int type;
} head;

typedef struct message
{
    struct head head;
    char *buffer;
} message;

/*
    return 0    -- need more data.
    return <0   -- process error.
    return >0   -- the length of the buffer has been parsed.
*/
static int process_cmd_udp(void *buffer, int buflen, void **response, int *resplen)
{
    printf("Begin to send commands!\n");
    printf("*********************************************\n");
    if ((unsigned int)buflen < sizeof(head))
    {
        return 0;
    }

    head *header = (head *)buffer;
    printf("length: %d, type: %d\n", header->len, header->type);
    if ((unsigned int)buflen >= sizeof(head) + header->len)
    {
        char *tmp = (char *)calloc(1, header->len + 2);
        strncpy(tmp, (char *)buffer + sizeof(head), header->len);
        printf("data: %s\n", (char *)tmp);
        free(tmp);

        char *reps = (char *)"ljksdljfljasldjfsdjfklsjdflsjdlfjlkasjdlkfjjsakldfj";
        *response = (char *)calloc(1, strlen(reps) + 1);
        snprintf((char *)*response, strlen(reps) + 1, "%s", reps);
        *resplen = strlen(reps);

        return (sizeof(head) + header->len);
    }

    return 0;
}

/*
    return 0    -- need more data.
    return <0   -- process error.
    return >0   -- the length of the buffer has been parsed.
*/
static int process_cmd(void *buffer, int buflen, void **response, int *resplen)
{
    printf("Begin to send commands!\n");
    printf("*********************************************\n");

    if ((unsigned int)buflen < sizeof(head))
    {
        return 0;
    }

    head *header = (head *)buffer;
    printf("length: %d, type: %d\n", header->len, header->type);
    if ((unsigned int)buflen >= sizeof(head) + header->len)
    {
        char *tmp = (char *)calloc(1, header->len + 2);
        strncpy(tmp, (char *)buffer + sizeof(head), header->len);
        printf("data: %s\n", (char *)tmp);
        free(tmp);

        char *reps = (char *)"kkkkkkkkkkkkkkkkkkkkkkkkkkk";
        *response = (char *)calloc(1, strlen(reps) + 1);
        snprintf((char *)*response, strlen(reps) + 1, "%s", reps);
        *resplen = strlen(reps);

        return (sizeof(head) + header->len);
    }

    return 0;
}

int main()
{
    servers_array *g_servers = get_servers();
    if (g_servers == NULL)
    {
        printf("get_servers error\n");
        exit(-1);
    }

    int tcp_port = 12345;
    int udp_port = 12346;
    char tcp_path[256] = "/tmp/1111";
    char udp_path[256] = "/tmp/2222";

    server_param param;

    memset(&param, 0, sizeof(server_param));
    param.port = tcp_port;
    param.type = TCP_PORT;
    param.max_live = -1;
    param.func = process_cmd;
    add_generic_server_param(g_servers, &param);


    memset(&param, 0, sizeof(server_param));
    param.port = udp_port;
    param.type = UDP_PORT;
    param.max_live = -1;
    param.func = process_cmd_udp;
    add_generic_server_param(g_servers, &param);


    memset(&param, 0, sizeof(server_param));
    param.path = strdup(tcp_path);
    param.type = TCP_PATH;
    param.max_live = -1;
    param.func = process_cmd;
    add_generic_server_param(g_servers, &param);


    memset(&param, 0, sizeof(server_param));
    param.path = strdup(udp_path);
    param.type = UDP_PATH;
    param.max_live = -1;
    param.func = process_cmd_udp;
    add_generic_server_param(g_servers, &param);

    printf("begin to loop\n");
    start_servers_array_loop(g_servers);

    destroy_servers(g_servers);
    return 0;
}
