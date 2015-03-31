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

typedef struct queryfield
{
    string fieldname;
    string value;
} queryfield;

int is_right_magic(head *phead);
char *get_request_body(const string &tab, const vector<queryfield> &querys, int &len);
int do_query(int fd, char *pbody, uint32_t blen);
int parse_response(char *respbuf, int rlen);

#endif
