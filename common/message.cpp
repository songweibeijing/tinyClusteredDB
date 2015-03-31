#include "head.h"
#include "util.h"
#include <iostream>
#include <fstream>
#include "message.pb.h"
#include "message.h"

using namespace std;
using namespace ::google::protobuf;

int is_right_magic(head *phead)
{
    if (phead->magic == MAGIC)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

char *build_request_body(const string &tab, const vector<queryfield> &querys, int &len)
{
    char *buf = NULL;
    Request req;

    int queryitems = querys.size();
    req.set_tablename(tab);
    for (int i = 0; i < queryitems; i++)
    {
        Request_Field *qfield = req.add_fields();
        qfield->set_name(querys[i].fieldname);
        qfield->set_value(querys[i].value);
    }

    len = req.ByteSize();
    buf = (char *)calloc(1, len + 1);
    if (buf == NULL)
    {
        return NULL;
    }
    req.SerializeToArray(buf, len);
    return buf;
}

int do_query(int fd, char *pbody, uint32_t blen)
{
    int ret = 0;
    head headinfo;
    struct iovec vecs[2];

    headinfo.magic = MAGIC;
    headinfo.length = blen;

    vecs[0].iov_base = (char *)&headinfo;
    vecs[0].iov_len = sizeof(headinfo);
    vecs[1].iov_base = pbody;
    vecs[1].iov_len = blen;

    ret = writev_ex(fd, vecs, 2);
    if (ret != 0)
    {
        return -1;
    }

    return 0;
}

int parse_response(char *respbuf, int rlen)
{
    Response resp;
    resp.ParseFromArray(respbuf, rlen);
    cout << "Name: " << resp.tablename() << endl;

    int loop = resp.records_size();
    cout << "records size: " << loop << endl;

    RepeatedPtrField<Response_Record> *records = resp.mutable_records();
    RepeatedPtrField<Response_Record>::iterator it = records->begin();
    for (; it != records->end(); ++it)
    {
        RepeatedPtrField<Response_Field> *fields = it->mutable_fields();
        RepeatedPtrField<Response_Field>::iterator it2 = fields->begin();
        for (; it2 != fields->end(); ++it2)
        {
            cout << "name: " << it2->name() << endl;
            cout << "value: " << it2->value() << endl;
        }
    }
    return 0;
}
