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

static int valid_raw_record(const vector<fieldinfo> &vmap, const string &tabname, const char *precord, const int rec_size);

int init_field_index(tab_index_loader *ploader, string fieldname)
{
    uint64_t md5;
    string line;

    index_info idxinfo = ploader->fieldinfo_map[fieldname];
    intIndexhashMap *pindex = ploader->indexs + idxinfo.hash_index;
    string index_file = idxinfo.index_file_path;

    cout << "begin to load index file " << index_file << endl;
    ifstream in(index_file);
    if (!in)
    {
        cerr << "open file " << index_file << " failed" << endl;
        return -1;
    }

    while (getline(in, line))
    {
        posindex idxrec;
        line = trim(line, "\r\n");
        vector<string> tokens = parse_string(line, "\t");
        if (tokens.size() == 0 || (int)tokens.size() != 3)
        {
            cerr << "wrong input " << line << endl;
            continue;
        }
        cout << tokens[0] << "|" << tokens[1] << "|" << tokens[2] << endl;
        md5 = string2uint64(tokens[0]);
        idxrec.start_pos = string2uint64(tokens[1]);
        idxrec.length = string2uint32(tokens[2]);
        pindex->insert(make_pair(md5, idxrec));
    }
    in.close();
    cout << "end to load index file." << endl;

    return 0;
}

int init_table_index(index_loaders *ptabloader, config *pconfig, int idx)
{
    int i = 0, ret = 0;
    index_info idxinfo;
    table tab = pconfig->vTables[idx];
    int index_num = tab.index_num;

    tab_index_loader *ploader = ptabloader->loaders + idx;
    ploader->index_num = index_num;
    ploader->indexs = new intIndexhashMap[index_num];
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
        idxinfo.index_file_path = file_prefix + INDEX_SUFFIX;
        idxinfo.data_file_path = file_prefix + DATA_SUFFIX;
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
    printf("index [-f-h]\n");
    printf("\t-h\t print usage\n");
    printf("\t-f\t the configuration file\n");
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

vector<string> get_data(const vector<fieldinfo> &querys, const index_info &idxinfo, const uint64_t startpos, const uint32_t nrec)
{
    vector<string> validrec;
    validrec.clear();

    int tabidx = find_table(g_config->vTables, idxinfo.tabname);
    if (tabidx == -1)
    {
        return validrec;
    }

    int rec_size = get_max_record_size(&g_config->vTables[tabidx]);
    string datafile = idxinfo.data_file_path;
    char *buffer = NULL, *tmp = NULL;
    FILE *pData = fopen(datafile.c_str(), "r");
    if (pData == NULL)
    {
        cout << "failed to open file : " << datafile << endl;
        return validrec;
    }
    fseek(pData, startpos * rec_size, SEEK_SET);
    buffer = (char *)calloc(nrec, rec_size);
    if (!buffer)
    {
        fclose(pData);
        return validrec;
    }
    fread(buffer, rec_size, nrec, pData);
    fclose(pData);

    tmp = buffer;
    for (uint32_t i = 0; i < nrec; i++)
    {
        if (valid_raw_record(querys, idxinfo.tabname , buffer, rec_size))
        {
            string str = string(buffer, rec_size);
            validrec.push_back(str);
        }
        buffer += rec_size;
    }

    free(tmp);
    return validrec;
}

static uint32_t get_pos(const string &tabname, const string &field_name)
{
    uint32_t pos = 0;
    table tab = g_config->mTables[tabname];
    vector<field> vFields = tab.table_scheme.vFields;

    for (vector<field>::iterator it = vFields.begin();
         it != vFields.end(); it++)
    {
        if (it->strName == field_name)
        {
            break;
        }

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

static int valid_raw_record(const vector<fieldinfo> &vmap, const string &tabname, const char *precord, const int rec_size)
{
    uint32_t pos;
    table tab = g_config->mTables[tabname];

    for (vector<fieldinfo>::const_iterator it = vmap.begin(); it != vmap.end(); it++)
    {
        fieldinfo qf = *it;
        pos = get_pos(tabname, qf.fieldname);
        field fieldinfo = tab.table_scheme.mFields[qf.fieldname];
        if (fieldinfo.type == INT)
        {
            uint32_t value, value2;
            memcpy(&value, precord + pos, sizeof(uint32_t));
            value2 = string2uint32(qf.value);
            if (value != value2)
            {
                return 0;
            }
        }
        else if (fieldinfo.type == STRING)
        {
            char *buff = (char *) calloc(1, fieldinfo.max_length + 1);
            if (buff == NULL)
            {
                return 0;
            }

            memcpy(buff, precord + pos, fieldinfo.max_length);
            if (memcmp(qf.value.c_str(), buff, strlen(buff)))
            {
                cout << "not equal" << endl;
                my_free(buff);
                return 0;
            }
            my_free(buff);
        }
        else if (fieldinfo.type == FLOAT)
        {
            float value, value2;
            memcpy(&value, precord + pos, sizeof(float));
            value2 = string2float(qf.value);
            if (value != value2)
            {
                return 0;
            }
        }
        else if (fieldinfo.type == DOUBLE)
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

static void print_records(const string &tabname, vector<string> &records)
{
    table tab = g_config->mTables[tabname];
    vector<field> vFields = tab.table_scheme.vFields;
    int pos = 0;
    const char *p = NULL;

    for (vector<string>::iterator it = records.begin(); it != records.end(); it++)
    {
        string str = *it;
        p = str.c_str();
        pos = 0;
        for (size_t i = 0; i < vFields.size(); i++)
        {
            string strvalue = "";
            if (vFields[i].type == INT)
            {
                uint32_t value;
                memcpy(&value, p, sizeof(uint32_t));
                p += sizeof(uint32_t);
                cout << value << " | ";
            }
            else if (vFields[i].type == STRING)
            {
                strvalue = string(p, vFields[i].max_length);
                p += vFields[i].max_length;
                cout << strvalue << " | ";
            }
            else if (vFields[i].type == FLOAT)
            {
                float value;
                memcpy(&value, p, sizeof(float));
                p += sizeof(float);
                cout << value << " | ";
            }
            else if (vFields[i].type == DOUBLE)
            {
                double value;
                memcpy(&value, p, sizeof(double));
                p += sizeof(double);
                cout << value << " | ";
            }
        }
        cout << endl;
    }
}

static vector<string> get_results(const string &tabname, const vector<fieldinfo> &vmap)
{
    uint64_t min_index_records = 0xffffffff; //the index which contains min records.
    index_info min_index_info;
    uint64_t start_pos = 0;
    int found = 0;
    vector<string> records;
    records.clear();

    int tabidx = find_table(g_config->vTables, tabname);
    if (tabidx == -1 || vmap.size() == 0)
    {
        return records;
    }

    strInthashMap::iterator it_tab;
    strIdxInfohashMap::iterator it_idx;
    intIndexhashMap::iterator it_record;

    it_tab = loader.tabname_map.find(tabname);
    if (it_tab == loader.tabname_map.end())
    {
        cout << "wrong table name : " << tabname << endl;
        return records;
    }

    tab_index_loader *ptab = loader.loaders + it_tab->second;
    for (vector<fieldinfo>::const_iterator it_field = vmap.begin(); it_field != vmap.end(); it_field++)
    {
        string name = it_field->fieldname;
        string value = it_field->value;
        uint64_t md5 = get_md5(value);
        it_idx = ptab->fieldinfo_map.find(name);
        if (it_idx == ptab->fieldinfo_map.end())
        {
            cout << "can't find field : " << name << endl;
            continue;
        }

        index_info idxinfo = it_idx->second;
        it_record = ptab->indexs[idxinfo.hash_index].find(md5);
        if (it_record == ptab->indexs[idxinfo.hash_index].end())
        {
            cout << "can not find it. " << name << "|" << value << endl;
            return records;
        }

        if (min_index_records > it_record->second.length)
        {
            min_index_records = it_record->second.length;
            min_index_info = idxinfo;
            start_pos = it_record->second.start_pos;
        }
        found = 1;
    }

    if (found == 0)
    {
        cout << "can not find any records" << endl;
        return records;
    }

    records = get_data(vmap, min_index_info, start_pos, min_index_records);
    if (records.empty())
    {
        cout << "no records" << endl;
        return records;
    }

    print_records(tabname, records);
    return records;
}

static int get_query_results(char *reqbuf, int reqlen, char **respbuf, int *resplen)
{
    string tabname;
    vector<fieldinfo> vquerys;
    vector<string> vRecords;

    *resplen = 0;
    *respbuf = NULL;

    if (parse_request(reqbuf, reqlen, tabname, vquerys) != 0)
    {
        return -1;
    }

    vRecords = get_results(tabname, vquerys);
    if (vRecords.empty())
    {
        return -1;
    }

    table tab = g_config->mTables[tabname];
    vector<field> vFields = tab.table_scheme.vFields;

    build_response(vRecords, tabname, vFields, respbuf, resplen);
    return 0;
}


static int parse_cmd(void *buffer, int buflen, void **response, int *resplen)
{
    printf("Begin to parse commands!\n");
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

        char *tmp = (char *)calloc(1, header->length + 1);
        if (tmp)
        {
            memcpy(tmp, (char *)buffer + sizeof(head), header->length);
            *response = tmp;
            *resplen = header->length;
        }

        return sizeof(head) + header->length;
    }

    return 0;
}

/*
   return 0    -- need more data.
   return <0   -- process error.
   return >0   -- the length of the buffer has been parsed.
   */
static int handle_cmd(void *buffer, int buflen, void **response, int *resplen)
{
    printf("Begin to handle commands!\n");
    printf("*********************************************\n");

    head *phead = NULL;
    if (response && resplen)
    {
        *response = NULL;
        *resplen = 0;
    }

    int ret = 0;
    ret = get_query_results((char *)buffer, buflen, (char **)response, resplen);
    if (ret != 0 || resplen <= 0 || response == NULL)
    {
        *response = (char *)calloc(1, sizeof(head));
        if (*response == NULL)
        {
            cout << "failed to malloc" << endl;
            return -1;
        }
        *resplen = sizeof(head);
        phead = (head *)(*response);
        phead->magic = MAGIC;
        phead->length = 0;
    }
    else
    {
        phead = (head *)(*response);
        phead->magic = MAGIC;
        phead->length = *resplen - sizeof(head);
    }
    return 0;
}

int init_server()
{
    init_global_env();

    short tcp_port = string2short(g_config->tcp_port);
    short udp_port = string2short(g_config->udp_port);
    string tcp_path = g_config->tcp_path;
    string udp_path = g_config->udp_path;
    int work_threads = string2int32(g_config->work_threads);
    int max_queue = string2int32(g_config->max_queue_num);

    printf("work_threads %d, max_queue : %d\n", work_threads, max_queue);

    server_param param;

    if (tcp_port == 0
        && udp_port == 0
        && tcp_path.length() == 0
        && udp_path.length() == 0)
    {
        return -1;
    }

    g_servers = get_servers();
    if (g_servers == NULL)
    {
        printf("get_servers error\n");
        return -1;
    }

    if (tcp_port > 0)
    {
        memset(&param, 0, sizeof(server_param));
        param.port = tcp_port;
        param.type = TCP_PORT;
        param.max_live = -1;
        param.thread_num = work_threads;
        param.max_queue_num = max_queue;
        param.parse_request = parse_cmd;
        param.handle_request = handle_cmd;
        add_generic_server_param(g_servers, &param);
    }

    if (udp_port > 0)
    {
        memset(&param, 0, sizeof(server_param));
        param.port = udp_port;
        param.type = UDP_PORT;
        param.max_live = -1;
        param.thread_num = work_threads;
        param.max_queue_num = max_queue;
        param.parse_request = parse_cmd;
        param.handle_request = handle_cmd;
        add_generic_server_param(g_servers, &param);
    }
    if (tcp_path.length() > 0)
    {
        memset(&param, 0, sizeof(server_param));
        param.path = strdup(tcp_path.c_str());
        param.type = TCP_PATH;
        param.max_live = -1;
        param.thread_num = work_threads;
        param.max_queue_num = max_queue;
        param.parse_request = parse_cmd;
        param.handle_request = handle_cmd;
        add_generic_server_param(g_servers, &param);
    }

    if (udp_path.length() > 0)
    {
        memset(&param, 0, sizeof(server_param));
        param.path = strdup(udp_path.c_str());
        param.type = UDP_PATH;
        param.max_live = -1;
        param.thread_num = work_threads;
        param.max_queue_num = max_queue;
        param.parse_request = parse_cmd;
        param.handle_request = handle_cmd;
        add_generic_server_param(g_servers, &param);
    }
    return 0;
}

int main(int argc, char *argv[])
{
    int c, ret = -1;
    char *configfile = NULL;

    vector<fieldinfo> vmap;
    vmap.clear();
    fieldinfo item;

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

    print_config(g_config);

    cout << endl;
    cout << "begin to load table index" << endl;
    ret = init_tables_index(&loader, g_config);
    if (ret != 0)
    {
        cout << "failed to load index" << endl;
        goto failed;
    }
    cout << "end to load table index" << endl;

    cout << "deptid==1" << endl;
    item.fieldname = "deptid";
    item.value = "1";
    vmap.push_back(item);
    item.fieldname = "deptcost";
    item.value = "18888.5";
    vmap.push_back(item);
    get_results("test1", vmap);
    cout << endl;

    cout << "deptname=cs" << endl;
    vmap.clear();
    item.fieldname = "deptname";
    item.value = "cs";
    vmap.push_back(item);
    get_results("test1", vmap);
    cout << endl;

    cout << "deptname=cs && deptcost=87888" << endl;
    vmap.clear();
    item.fieldname = "deptname";
    item.value = "cs";
    vmap.push_back(item);
    item.fieldname = "deptcost";
    item.value = "87888";
    vmap.push_back(item);
    get_results("test1", vmap);
    cout << endl;

    cout << "deptname=ee && deptcost=9328" << endl;
    vmap.clear();
    item.fieldname = "deptcost";
    item.value = "9328";
    vmap.push_back(item);
    item.fieldname = "deptname";
    item.value = "ee";
    vmap.push_back(item);
    get_results("test1", vmap);
    cout << endl;

    cout << "id=1" << endl;
    vmap.clear();
    item.fieldname = "id";
    item.value = "1";
    vmap.push_back(item);
    get_results("test2", vmap);
    cout << endl;

    cout << "id=2&&name=judy" << endl;
    vmap.clear();
    item.fieldname = "id";
    item.value = "2";
    vmap.push_back(item);
    item.fieldname = "name";
    item.value = "judy";
    vmap.push_back(item);
    get_results("test2", vmap);
    cout << endl;

    cout << "score=100" << endl;
    vmap.clear();
    item.fieldname = "score";
    item.value = "100";
    vmap.push_back(item);
    get_results("test2", vmap);
    cout << endl;

    //init network settings
    cout << "begin to init server" << endl;
    if (init_server() != 0)
    {
        cout << "failed to load index" << endl;
        goto failed;
    }
    cout << "end to init server" << endl;

    cout << "begin to loop" << endl;
    start_servers_array_loop(g_servers);
    cout << "end to loop" << endl;

failed:
    if (g_servers)
    {
        destroy_servers(g_servers);
    }
    free_index_loader(&loader);
    delete g_config;
    return 0;
}

