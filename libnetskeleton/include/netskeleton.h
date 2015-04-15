#ifndef _NETSKELETON_H
#define _NETSKELETON_H

#include "comm.h"
#include "linux_list.h"
#include "event.h"

#if defined(__cplusplus)
extern "C" {
#endif

#define MAX_IP_LEN 40
#define MAX_PATH_LEN 256
#define MAX_SENDBUF_SIZE (256 * 1024 * 1024)
#define MAX_RECVBUF_SIZE (256 * 1024 * 1024)

#define TCP_PORT        0x01
#define TCP_PATH        0x02
#define UDP_PORT        0x04
#define UDP_PATH        0x08
#define TIMER_JOB       0x10

#define TCP_MASK        0x03
#define UDP_MASK        0x0c
#define PORT_MASK       0x05
#define PATH_MASK       0x0a
#define SERVER_MASK     0x0f

#define DEBUG 1

    /**
     * Handler used to process network requests. If the user wants to give response to remote clients,
     * the user should malloc memory for the response data, and update the resplen,
     * the response data will be freed later, so the user does not have to free it in the handle funciton
     *
     * @param request the data received from remote clients.
     * @param reqlen the size of the buffer.
     * @param response the address of the data sent to remote clients.
     * @param resplen the size of the data sent to remote clients.
     * @return (IMPORTANT)
            return = 0   -- it means the server needs more data.
            return < 0   -- it means process error. the skeleton will close the connection.
                              before closing the connection, it will send the response to the client if it has.
            return > 0   -- the length of the buffer has been parsed.
     */
    typedef int (*server_process)(void *request, int reqlen, void **response, int *resplen);

    /**
     * Constructor used to initialize allocated objects
     *
     * @param fd the socket fd.
     * @param event the event which has been triggered.
     * @param buffer the data related.
     */
    typedef void (*timer_process)(int fd, short event, void *buffer);

    /**
     * Definition of the structure for users to configure one server.
     */
    typedef struct server_param
    {
        int type; /** (Required) the type of the server, can be TCP_PORT, TCP_PATH, UDP_PORT, UDP_PATH */
        char *ip; /** (Optional) the bind address of the server, if NULL, bind to all addresses on the machine */
        unsigned short port; /** (Required) used only in TCP_PORT and UDP_PORT mode, the port the server listens on */
        char *path; /** (Required) used only TCP_PATH and UDP_PATH mode */
        int max_live; /** (Optional) used only in TCP mode, the max live time of inactive connection in the server
                          if the value <=0, this connection expire does not work  */
        int thread_num; /*the number of threads*/
        int max_queue_num; /*the max number of jobs in queue.*/
        server_process parse_request;  /* (Required) the handle of parsing server */
        server_process handle_request; /**(Required) the handle of the server */
    } server_param;

    /**
     * Definition of the structure for users to configure one timer.
     */
    typedef struct timer_param
    {
        int type ; /** TIMER_JOB */
        timer_process func_tm; /**(Required) the handle of timer if the type is TIMER_JOB */
        int repeat; /**(Optional) the timer is repeat one or not */
        uint32_t time_interval; /**(Required) the time interval */
    } timer_param;

    /**
     * Definition of the structure for internal use.
     */
    typedef struct servers
    {
        void *main_event_base; /** the main base */
        int server_num; /** the number of servers */
        int timer_num; /** the number of timer */
        struct list_head tm_head;
        struct list_head server_head;
    } servers_array;

    /*
    *  init the global environment settings
    *  it should be called at the first place.
    * */
    int init_global_env();

    /**
     * Create one servers_array
     *
     * This function is fully MT safe, so you may allocate objects from multiple threads without having to
     * do any syncrhonization in the application code. However, the servers_array is not MT safe, it is not
     * a good idea to use the same servers_array in MT env. It is recommended to use one servers_array in
     * each thread.
     *
     * @return a handle to servers_array if successful, NULL otherwise.
     */
    servers_array *get_servers();

    /**
     * Destroy one servers_array
     *
     * @param servers the handle of the servers_array.
     */
    void destroy_servers(servers_array *servers);

    /**
     * Get the number of servers in one servers_array
     *
     * @param servers the handle of the servers_array.
     * @return the number of servers if successful.
     */
    int get_servers_num(servers_array *servers);

    /**
     * Construct one server and add it into one servers_array obj.
     *
     * @param servers the handle of the servers_array.
     * @param param the configuration of one server, it is constructed by user app.
     * @return 0 if successful. non-zero otherwise.
     */
    int add_generic_server_param(servers_array *servers, server_param *param);

    /**
     * Construct one timer and add it into one servers_array obj.
     *
     * @param servers the handle of the servers_array.
     * @param param the configuration of one timer, it is constructed by user app.
     * @return 0 if successful. non-zero otherwise.
     */
    int add_timer_job(servers_array *servers, timer_param *param);

    /**
     * Start the main loop of one servers_array
     *
     * After calling this function, it will not return in the thread unless you
     * shutdown the server purposely or some unexpected signals.
     *
     * @param servers the handle of the servers_array.
     */
    void start_servers_array_loop(servers_array *servers);

#if defined(__cplusplus)
}
#endif

#endif
