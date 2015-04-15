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

char *build_request(const string &tab, const vector<fieldinfo> &querys, int &len)
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

int parse_request(const char *reqbuf, const int &reqlen, string &tab, vector<fieldinfo> &vmap)
{
    Request req;
    req.ParseFromArray(reqbuf, reqlen);

    tab = "";
    vmap.clear();

    if (req.fields_size() <= 0)
    {
        return -1;
    }

    tab = req.tablename();
    RepeatedPtrField<Request_Field> *fields = req.mutable_fields();
    RepeatedPtrField<Request_Field>::iterator it = fields->begin();
    for (; it != fields->end(); ++it)
    {
        fieldinfo item;
        item.fieldname = it->name();
        item.value = it->value();
        vmap.push_back(item);
    }
    return 0;
}

int send_request(int fd, char *pbody, uint32_t blen)
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

int recv_response(int fd, char **buf, int &blen)
{
    head headinfo;

    *buf = NULL;
    blen = 0;

    if (read(fd, &headinfo, sizeof(headinfo)) != sizeof(headinfo))
    {
        return -1;
    }

    if (!is_right_magic(&headinfo))
    {
        cout << "wrong magic : " << headinfo.magic << endl;
        return -1;
    }

    if (headinfo.length > 0)
    {
        char *tmp = (char *)calloc(1, headinfo.length);
        if (!tmp)
        {
            cout << "malloc failed" << endl;
            return -1;
        }

        if (read(fd, tmp, headinfo.length) != headinfo.length)
        {
            cout << "failed to read body" << endl;
            my_free(tmp);
            return -1;
        }

        *buf = tmp;
        blen = headinfo.length;
    }

    return 0;
}

int build_response(const vector<string> &vRecords, const string &tab, const vector<field> &vFields, char **respbuf, int *resplen)
{
    Response resp;
    const char *p;

    *resplen = 0;
    *respbuf = NULL;

    cout << "tab : " << tab << endl;
    resp.set_tablename(tab);
    for (vector<string>::const_iterator it = vRecords.begin(); it != vRecords.end(); it++)
    {
        Response_Record *prec = resp.add_records();

        string str = *it;
        p = str.c_str();

        for (size_t i = 0; i < vFields.size(); i++)
        {
            Response_Field *pfield = prec->add_fields();
            pfield->set_name(vFields[i].strName);

            string strvalue = "";
            if (vFields[i].type == INT)
            {
                uint32_t value;
                memcpy(&value, p, sizeof(uint32_t));
                p += sizeof(uint32_t);
                strvalue = uint322string(value);
            }
            else if (vFields[i].type == STRING)
            {
                strvalue = string(p, vFields[i].max_length);
                p += vFields[i].max_length;
            }
            else if (vFields[i].type == FLOAT)
            {
                float value;
                memcpy(&value, p, sizeof(float));
                p += sizeof(float);
                strvalue = float2string(value);
            }
            else if (vFields[i].type == DOUBLE)
            {
                double value;
                memcpy(&value, p, sizeof(double));
                p += sizeof(double);
                strvalue = double2string(value);
            }
            pfield->set_value(strvalue);
        }
    }

    int length = resp.ByteSize() + sizeof(head);
    *respbuf = (char *)calloc(1, length + 1);
    if (*respbuf == NULL)
    {
        return -1;
    }
    *resplen = length;
    resp.SerializeToArray(*respbuf + sizeof(head), *resplen - sizeof(head));
    return 0;
}

int parse_response(const char *respbuf, const int &rlen, string &tab, vector<recordinfo> &vRecords)
{
    Response resp;

    tab = "";
    vRecords.clear();

    resp.ParseFromArray(respbuf, rlen);
    tab = resp.tablename();

    RepeatedPtrField<Response_Record> *records = resp.mutable_records();
    RepeatedPtrField<Response_Record>::iterator it = records->begin();
    for (; it != records->end(); ++it)
    {
        recordinfo record;
        RepeatedPtrField<Response_Field> *fields = it->mutable_fields();
        RepeatedPtrField<Response_Field>::iterator it2 = fields->begin();
        for (; it2 != fields->end(); ++it2)
        {
            fieldinfo field;
            field.fieldname = it2->name();
            field.value = it2->value();
            record.fields.push_back(field);
        }
        vRecords.push_back(record);
    }
    return 0;
}

