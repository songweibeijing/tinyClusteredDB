#include "netskeleton.h"
#include "util.h"

//the structure is used for test
typedef struct head
{
    int len;
    int type;
} head;

/*
    return 0    -- need more data.
    return <0   -- process error.
    return >0   -- the length of the buffer has been parsed.
*/
static int parse_cmd(void *buffer, int buflen, void **response, int *resplen)
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
        char *tmp = (char *)calloc(1, header->len);
        memcpy(tmp, (char *)buffer + sizeof(head), header->len);
        char dbg[4096] = {0};
        snprintf(dbg, sizeof(dbg), "%s", tmp);
        printf("data: %s\n", (char *)dbg);
        *response = tmp;
        *resplen = header->len;
        return (sizeof(head) + header->len);
    }

    return 0;
}

static int handle_cmd(void *buffer, int buflen, void **response, int *resplen)
{
    head *phead = NULL;
    char *reps = (char *)"kkkkkkkkkkkkkkkkkkkkkkkkkkk";
    char *tmp = NULL;

    tmp = (char *)calloc(1, sizeof(head) + strlen(reps));
    if (tmp == NULL)
    {
        printf("error\n");
        return -1;
    }

    *response = (void *)tmp;
    phead = (head *)tmp;
    phead->len = strlen(reps);
    phead->type = 0;

    tmp += sizeof(head);
    memcpy(tmp, reps, strlen(reps));
    *resplen = strlen(reps) + sizeof(head);
    return 0;
}

int main()
{
    init_global_env();
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
    param.thread_num = 8;
    param.max_queue_num = 10000;
    param.parse_request = parse_cmd;
    param.handle_request = handle_cmd;
    add_generic_server_param(g_servers, &param);


    memset(&param, 0, sizeof(server_param));
    param.port = udp_port;
    param.type = UDP_PORT;
    param.max_live = -1;
    param.thread_num = 8;
    param.max_queue_num = 10000;
    param.parse_request = parse_cmd;
    param.handle_request = handle_cmd;
    add_generic_server_param(g_servers, &param);


    memset(&param, 0, sizeof(server_param));
    param.path = strdup(tcp_path);
    param.type = TCP_PATH;
    param.max_live = -1;
    param.thread_num = 8;
    param.max_queue_num = 10000;
    param.parse_request = parse_cmd;
    param.handle_request = handle_cmd;
    add_generic_server_param(g_servers, &param);


    memset(&param, 0, sizeof(server_param));
    param.path = strdup(udp_path);
    param.type = UDP_PATH;
    param.max_live = -1;
    param.thread_num = 8;
    param.max_queue_num = 10000;
    param.parse_request = parse_cmd;
    param.handle_request = handle_cmd;
    add_generic_server_param(g_servers, &param);

    printf("begin to loop\n");
    start_servers_array_loop(g_servers);

    destroy_servers(g_servers);
    return 0;
}
