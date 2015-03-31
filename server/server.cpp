#include <iostream>
#include <fstream>
#include <sys/time.h>
#include <sstream>
#include <iostream>
#include "util.h"
#include "conf.h"
#include "head.h"
#include "md5.h"
#include "message.h"
#include "message.pb.h"
#include "message.h"
#include "../libnetskeleton/include/comm.h"
#include "../libnetskeleton/include/netskeleton.h"
#include "../libnetskeleton/include/util.h"

using namespace std;
using namespace ::google::protobuf;

servers_array *g_servers = NULL;
index_loaders loader;

typedef struct queryfilter
{
    string   value;
    field    field_info;
} queryfilter;

int init_field_index(tab_index_loader *ploader, string fieldname)
{
    uint64_t md5;
    string line;

    index_info idxinfo = ploader->fieldinfo_map[fieldname];
    intIndexhashMap *pindex = ploader->indexs + idxinfo.hash_index;
    string index_file = idxinfo.index_file_path;

    ifstream in(index_file);
    if (!in)
    {
        cerr << "open file " << index_file << " failed" << endl;
        return -1;
    }

    while (getline(in, line))
    {
        posindex idxrec;
        vector<string> tokens = parse_string(line, "\t");
        if (tokens.size() == 0 || (int)tokens.size() != 3)
        {
            cerr << "wrong input " << line << endl;
            continue;
        }
        idxrec.start_pos = string2uint64(tokens[1]);
        idxrec.length = string2uint32(tokens[2]);
        md5 = get_md5(tokens[0]);
        pindex->insert(make_pair(md5, idxrec));
    }
    in.close();

    return 0;
}

int init_table_index(index_loaders *ptabloader, config *pconfig, int idx)
{
    int i = 0, ret = 0;
    index_info idxinfo;
    table tab = pconfig->vTables[idx];
    int field_num = tab.index_num;

    tab_index_loader *ploader = ptabloader->loaders + idx;
    ploader->index_num = field_num;
    ploader->indexs = new intIndexhashMap[field_num];
    if (!ploader->indexs)
    {
        cout << "failed to new tab_index_loader" << endl;
        return -1;
    }

    for (vector<field>::iterator it = tab.table_scheme.vIndexs.begin();
         it != tab.table_scheme.vIndexs.end(); it++)
    {
        string file_prefix = tab.output_table_dir + it->strName;
        idxinfo.tabname = tab.table_name;
        idxinfo.fieldinfo = *it;
        idxinfo.hash_index = i;
        idxinfo.index_file_path = file_prefix + ".idx";
        idxinfo.data_file_path = file_prefix + ".dat";
        ploader->fieldinfo_map[it->strName] = idxinfo;
        ret = init_field_index(ploader, it->strName);
        if (ret != 0)
        {
            return -1;
        }
        i++;
    }
    return 0;
}

int init_tables_index(index_loaders *ploader, config *pconfig)
{
    int i = 0, ret = 0;
    int tab_num = pconfig->vTables.size();

    ploader->table_num  = tab_num;
    ploader->loaders = new tab_index_loader[tab_num];
    if (!ploader->loaders)
    {
        cout << "failed to new tab_index_loader" << endl;
        return -1;
    }

    for (vector<table>::iterator it = pconfig->vTables.begin();
         it != pconfig->vTables.end(); it++)
    {
        ploader->tabname_map[it->table_name] = i;
        ret = init_table_index(ploader, pconfig, i);
        if (ret != 0)
        {
            return -1;
        }
        i++;
    }

    return 0;
}

void free_index_loader(index_loaders *ploader)
{
    int i;
    if (ploader->loaders)
    {
        for (i = 0; i < ploader->table_num; i++)
        {
            if (ploader->loaders[i].indexs)
            {
                delete [] ploader->loaders[i].indexs;
            }
            ploader->loaders[i].fieldinfo_map.clear();
        }
        ploader->tabname_map.clear();
        delete [] ploader->loaders;
    }
}

void usage()
{
    printf("The Tool is used to provide query service for clusterDB.\n");
    printf("index [-F-h]\n");
    printf("\t-h\t print usage\n");
    printf("\t-F\t the configuration file\n");
}

int get_max_record_size(table *tab)
{
    int rec_size = 0, tmp = 0;
    vector<field> *p = &tab->table_scheme.vFields;
    for (vector<field>::iterator it = p->begin(); it != p->end(); it++)
    {
        if (it->type == INT)
        {
            tmp = sizeof(uint32_t);
        }
        else if (it->type == STRING)
        {
            tmp = it->max_length;
        }
        else if (it->type == FLOAT)
        {
            tmp = sizeof(float);
        }
        else if (it->type == DOUBLE)
        {
            tmp = sizeof(double);
        }
        else
        {
            return -1;
        }
        rec_size += tmp;
    }
    return rec_size;
}

int find_table(const vector<table> &vtabs, string tablename)
{
    int i = 0;
    for (vector<table>::const_iterator it = vtabs.begin(); it != vtabs.end(); it++)
    {
        if (it->table_name == tablename)
        {
            return i;
        }
        i++;
    }
    return -1;
}

char *get_raw_data(string datafile, uint32_t rec_size, uint64_t startpos, uint32_t nrec)
{
    char *buffer = NULL;
    FILE *pData = fopen(datafile.c_str(), "r");
    if (pData != NULL)
    {
        fseek(pData, startpos * rec_size, SEEK_SET);
        buffer = (char *)calloc(nrec, rec_size);
        if (!buffer)
        {
            fclose(pData);
            return NULL;
        }
        fread(buffer, rec_size, nrec, pData);
        fclose(pData);
    }
    return NULL;
}

static uint32_t get_pos(const table &tab, int nfileds)
{
    int i = 0;
    uint32_t pos = 0;
    vector<field> vFields = tab.table_scheme.vFields;
    for (vector<field>::iterator it = vFields.begin();
         it != vFields.end() && i < nfileds; it++)
    {
        if (it->type == INT)
        {
            pos += sizeof(uint32_t);
        }
        else if (it->type == STRING)
        {
            pos += it->max_length;
        }
        else if (it->type == FLOAT)
        {
            pos += sizeof(float);
        }
        else if (it->type == DOUBLE)
        {
            pos += sizeof(double);
        }
    }
    return pos;
}

static int valid_raw_record(const table &tab, char *precord, int rec_size, const vector<queryfilter> &vmap)
{
    uint32_t pos;
    for (vector<queryfilter>::const_iterator it = vmap.begin(); it != vmap.end(); it++)
    {
        queryfilter qf = *it;
        pos = get_pos(tab, qf.field_info.index);

        if (qf.field_info.type == INT)
        {
            uint32_t value, value2;
            memcpy(&value, precord + pos, sizeof(uint32_t));
            value2 = string2uint32(qf.value);
            if (value != value2)
            {
                return 0;
            }
        }
        else if (qf.field_info.type == STRING)
        {
            if (memcmp(qf.value.c_str(), precord + pos, qf.field_info.max_length))
            {
                return 0;
            }
        }
        else if (qf.field_info.type == FLOAT)
        {
            float value, value2;
            memcpy(&value, precord + pos, sizeof(float));
            value2 = string2float(qf.value);
            if (value != value2)
            {
                return 0;
            }
        }
        else if (qf.field_info.type == DOUBLE)
        {
            double value, value2;
            memcpy(&value, precord + pos, sizeof(double));
            value2 = string2double(qf.value);
            if (value != value2)
            {
                return 0;
            }
        }
    }
    return 1;
}

static int get_field_index(table &tab, Request &req, vector<queryfilter> &vmap)
{
    RepeatedPtrField<Request_Field> *fields = req.mutable_fields();
    RepeatedPtrField<Request_Field>::iterator it = fields->begin();
    for (; it != fields->end(); ++it)
    {
        queryfilter qf;
        string name = it->name();
        qf.value = it->value();
        qf.field_info = tab.table_scheme.mFields[name];
        vmap.push_back(qf);
    }
    return 0;
}

static int filter_raw_data(table &ptab, char *praw, int rec_size, int num, Request &req, uint32_t *pmap)
{
    char *precord = praw;
    int i = 0, valid_record = 0;
    vector<queryfilter> vmap;
    vmap.clear();

    get_field_index(ptab, req, vmap);
    for (i = 0; i < num; i++)
    {
        if (valid_raw_record(ptab, precord, rec_size, vmap))
        {
            pmap[valid_record++] = i;
        }
        precord += rec_size;
    }
    return valid_record;
}

static int get_query_results(char *reqbuf, int reqlen, char **respbuf, int *resplen)
{
    Request req;
    Response resp;
    int querydims = 0;
    string table;
    uint64_t min_index_records = 0xffffffff; //the index which contains min records.
    int min_index = -1;
    index_info min_index_info;
    uint64_t start_pos = 0;

    int i = 0;
    strInthashMap::iterator it;
    strIdxInfohashMap::iterator itidxinfo;
    intIndexhashMap::iterator itidx;

    req.ParseFromArray(reqbuf, reqlen);
    querydims = req.fields_size();

    if (querydims <= 0)
    {
        return -1;
    }

    table = req.tablename();
    it = loader.tabname_map.find(table);
    if (it != loader.tabname_map.end())
    {
        cout << "wrong table name : " << table << endl;
        return -1;
    }
    tab_index_loader *ptab = loader.loaders + it->second;
    RepeatedPtrField<Request_Field> *fields = req.mutable_fields();
    RepeatedPtrField<Request_Field>::iterator it2 = fields->begin();
    for (; it2 != fields->end(); ++it2, i++)
    {
        string name = it2->name();
        string value = it2->value();
        uint64_t md5 = get_md5(value);
        itidxinfo = ptab->fieldinfo_map.find(name);
        if (itidxinfo == ptab->fieldinfo_map.end())
        {
            continue;
        }
        itidx = ptab->indexs[itidxinfo->second.hash_index].find(md5);
        if (itidx == ptab->indexs[itidxinfo->second.hash_index].end())
        {
            respbuf = NULL;
            *resplen = 0;
            cout << "can not find it." << endl;
            return 0;
        }

        if (min_index_records > itidx->second.length)
        {
            min_index_records = itidx->second.length;
            min_index = i;
            min_index_info = ptab->fieldinfo_map[name];
            start_pos = itidx->second.start_pos;
        }
    }

    cout << "min index " << min_index << endl;
    cout << "min index records" << min_index_records << endl;

    int tabidx = find_table(g_config->vTables, min_index_info.tabname);
    if (tabidx == -1)
    {
        return -1;
    }

    int rec_size = get_max_record_size(&g_config->vTables[tabidx]);
    char *raw_data = get_raw_data(min_index_info.data_file_path, rec_size, start_pos, min_index_records);
    if (raw_data)
    {
        uint32_t *pmap = new uint32_t[min_index_records];
        int recnum = filter_raw_data(g_config->vTables[tabidx], raw_data, rec_size, min_index_records, req, pmap);
        if (recnum == 0)
        {
            delete [] pmap;
            respbuf = NULL;
            *resplen = 0;
            return 0;
        }

        resp.set_tablename(table);
        for (int i = 0; i < recnum; i++)
        {
            uint32_t pos = 0;
            char *p = raw_data + pmap[i] * rec_size;
            Response_Record *prec = resp.add_records();
            vector<field> vFields = g_config->vTables[tabidx].table_scheme.vFields;
            for (uint32_t j = 0; j < vFields.size(); j++)
            {
                Response_Field *pfield = prec->add_fields();
                pfield->set_name(vFields[j].strName);
                string strvalue;
                if (vFields[j].type == INT)
                {
                    uint32_t value;
                    memcpy(&value, p + pos, sizeof(uint32_t));
                    strvalue = uint322string(value);
                    pos += sizeof(uint32_t);
                }
                else if (vFields[j].type == STRING)
                {
                    strvalue = string(p, pos, vFields[j].max_length);
                    pos += vFields[j].max_length;
                }
                else if (vFields[j].type == FLOAT)
                {
                    float value;
                    memcpy(&value, p + pos, sizeof(float));
                    strvalue = float2string(value);
                    pos += sizeof(float);
                }
                else if (vFields[j].type == DOUBLE)
                {
                    double value;
                    memcpy(&value, p + pos, sizeof(double));
                    strvalue = double2string(value);
                    pos += sizeof(double);
                }

                pfield->set_value(strvalue);
            }
        }
        *resplen = resp.ByteSize() + sizeof(head);
        *respbuf = (char *)calloc(1, *resplen + 1);
        if (*respbuf == NULL)
        {
            delete [] pmap;
            return NULL;
        }
        resp.SerializeToArray(*respbuf + sizeof(head), *resplen - sizeof(head));
        delete [] pmap;
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
    if (!is_right_magic(header))
    {
        printf("it contains error magic number\n");
        return -1;
    }

    printf("length: %d\n", header->length);
    if ((unsigned int)buflen >= sizeof(head) + header->length)
    {
        if (response && resplen)
        {
            *response = NULL;
            *resplen = 0;
        }

        int ret = 0;
        ret = get_query_results((char *)buffer + sizeof(head), header->length, (char **)response, resplen);
        if (ret != 0 || resplen <= 0 || response == NULL)
        {
            return -1;
        }

        head *phead = (head *)response;
        phead->magic = MAGIC;
        phead->length = *resplen - sizeof(head);
        return sizeof(head) + header->length;
    }

    return 0;
}

int init_server()
{
    int tcp_port = 12345;
    int udp_port = 12346;
    char tcp_path[256] = "/tmp/1111";
    char udp_path[256] = "/tmp/2222";
    server_param param;

    servers_array *g_servers = get_servers();
    if (g_servers == NULL)
    {
        printf("get_servers error\n");
        return -1;
    }

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
    param.func = process_cmd;
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
    param.func = process_cmd;
    add_generic_server_param(g_servers, &param);
    return 0;
}

int main(int argc, char *argv[])
{
    int c, ret = -1;
    char *configfile = NULL;

    /* process arguments */
    while (-1 != (c = getopt(argc, argv, "f:")))
    {
        switch (c)
        {
            case 'f' :
                configfile = strdup(optarg);
                break;
            default:
                fprintf(stderr, "Illegal argument \"%c\"\n", c);
                usage();
                exit(-1);
        }
    }

    if (!configfile)
    {
        fprintf(stderr, "failed to find the config file\n");
        usage();
        exit(-1);
    }

    if (parse_conf(configfile) != 0)
    {
        fprintf(stderr, "failed to parse the config file\n");
        exit(-1);
    }

    ret = init_tables_index(&loader, g_config);
    if (ret != 0)
    {
        cout << "failed to load index" << endl;
        goto failed;
    }

    //init network settings
    if (init_server() != 0)
    {
        cout << "failed to load index" << endl;
        goto failed;
    }

    //main loop
    start_servers_array_loop(g_servers);

failed:
    if (g_servers)
    {
        destroy_servers(g_servers);
    }
    free_index_loader(&loader);
    delete g_config;
    return 0;
}

