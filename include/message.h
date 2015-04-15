#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#include "head.h"

/*this file includes the proto between the client and the server*/

#define MAGIC 0x83090982

typedef struct head
{
    uint32_t magic;
    uint32_t length; //the length of the body
} head;

typedef struct fieldinfo
{
    string fieldname;
    string value;
} fieldinfo;

typedef struct recordinfo
{
    vector<fieldinfo> fields;
} recordinfo;


int is_right_magic(head *phead);
char *build_request(const string &tab, const vector<fieldinfo> &querys, int &len);
int parse_request(const char *reqbuf, const int &reqlen, string &tab, vector<fieldinfo> &vmap);
int send_request(int fd, char *pbody, uint32_t blen);
int recv_response(int fd, char **buf, int &blen);
int build_response(const vector<string> &vRecords, const string &tab, const vector<field> &vFields, char **respbuf, int *resplen);
int parse_response(const char *respbuf, const int &rlen, string &tab, vector<recordinfo> &vRecords);

#endif
