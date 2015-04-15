#include <iostream>
#include <fstream>
#include <sys/time.h>
#include <sstream>
#include <iostream>
#include "util.h"
#include "conf.h"
#include "head.h"
#include "netskeleton.h"
#include "md5.h"
#include "message.h"
#include "message.pb.h"
#include "message.h"
#include "comm.h"
#include "netskeleton.h"
#include "../libnetskeleton/include/util.h"

using namespace std;
using namespace ::google::protobuf;

char *g_server = NULL;
short g_port = 0;

void usage()
{
    printf("The Tool is used to send request for clusterDB.\n");
    printf("index [-s-p]\n");
    printf("\t-s\t server ip\n");
    printf("\t-p\t server port\n");
}

int init_connection(char *server, short port)
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

int test_query(int fd, const string &tab, const vector<fieldinfo> &vquerys)
{
    int blen = 0;
    char *pbody = build_request(tab, vquerys, blen);
    if (pbody == NULL)
    {
        cout << "no body" << endl;
        return -1;
    }

    if (send_request(fd, pbody, blen) != 0)
    {
        cout << "do query failed" << endl;
        return -1;
    }

    char *buf = NULL;
    int length = 0;
    if (recv_response(fd, &buf, length) != 0)
    {
        cout << "recv response failed" << endl;
        my_free(buf);
        return -1;
    }

    if (length == 0)
    {
        cout << "no records for this query" << endl;
        return 0;
    }

    vector<recordinfo> vrecords;
    string tabname;
    parse_response(buf, length, tabname, vrecords);

    cout << "begin to print records" << endl;
    cout << "table : " << tabname << endl;
    for (vector<recordinfo>::iterator it = vrecords.begin();
         it != vrecords.end(); it++)
    {
        recordinfo record = *it;
        vector<fieldinfo> vfields = record.fields;
        for (vector<fieldinfo>::iterator it2 = vfields.begin();
             it2 != vfields.end(); it2++)
        {
            cout << it2->fieldname << ":" << it2->value << " ";
        }
        cout << endl;
    }

    my_free(buf);
    return 0;
}

void *test(void *arg)
{
    int fd = init_connection(g_server, g_port);
    if (fd < 0)
    {
        printf("init_connection failed\n");
        return NULL;
    }

    int loop = 100;
    int i = 0;

    while (i < loop)
    {
        string tab = "test1";
        fieldinfo qf;
        vector<fieldinfo> vquerys;
        vquerys.clear();

        qf.fieldname = "deptname";
        qf.value = "cs";
        vquerys.push_back(qf);

        qf.fieldname = "deptcost";
        qf.value = "18888.5";
        vquerys.push_back(qf);
        test_query(fd, tab, vquerys);

        cout << "------------------------" << endl;
        cout << "------------------------" << endl;

        tab = "test1";
        vquerys.clear();

        qf.fieldname = "deptname";
        qf.value = "cs";
        vquerys.push_back(qf);

        qf.fieldname = "deptcost";
        qf.value = "8.5";
        vquerys.push_back(qf);
        test_query(fd, tab, vquerys);
        i++;
    }
    return NULL;
}


int main(int argc, char **argv)
{
    char c;
    g_server = NULL;
    g_port = 0;

    /* process arguments */
    while (-1 != (c = getopt(argc, argv, "s:p:")))
    {
        switch (c)
        {
            case 's' :
                g_server = strdup(optarg);
                break;
            case 'p' :
                g_port = (short)atoi(optarg);
                break;
            default:
                fprintf(stderr, "Illegal argument \"%c\"\n", c);
                usage();
                exit(-1);
        }
    }

    if (g_server == NULL || g_port == 0)
    {
        usage();
        exit(-1);
    }

    cout << "address ip : " << g_server << ", port : " << g_port << endl;

    int threads = 100;
    pthread_t pid;
    for (int i = 0; i < threads; i++)
    {
        if (pthread_create(&pid, NULL, test, NULL) != 0)
        {
            printf("Fatal: Can't initialize test-1 Thread.\n");
            return -1;
        }
    }

    while (1)
    {
        pause();
    }

    return 0;
}

