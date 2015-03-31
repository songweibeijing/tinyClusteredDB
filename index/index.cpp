#define _FILE_OFFSET_BITS 64

#include <iostream>
#include <fstream>
#include <sys/time.h>
#include <sstream>
#include <iostream>
#include "util.h"
#include "conf.h"
#include "head.h"

#define MAX_TEXT 1024 * 1024 * 10

int g_size[10];

static int init_string_unique(string_unique *su);
static int get_max_record_size(table *tab);
static void free_string_unique(string_unique *su);
static int get_incr_rank_pos(string key, intCounthashMap &map);
static int output_index_and_data(index_builder *builder, int idxnm);
static int convert_struct(index_builder *builder, char *raw, char *out);
static int gen_indexs_for_tables(config *tconfig);

void usage()
{
    printf("The Tool is used to build index for clusterDB.\n");
    printf("index [-F-h]\n");
    printf("\t-h\t print usage\n");
    printf("\t-F\t the configuration file\n");
}

int init_index_builder(index_builder *index, table *tab)
{
    char *p = NULL, *pp = NULL;
    int rec_size = 0, i = 0, tmp = 0, rec_size_index = 0;
    uint32_t *size_map =  NULL;
    vector<field> *pvfileds = NULL;
    uint64_t **pRank = NULL;

    if (index == NULL || tab == NULL)
    {
        return -1;
    }

    if (init_string_unique(&index->str_filter) == -1)
    {
        return -1;
    }

    rec_size = get_max_record_size(tab);
    if (rec_size == -1)
    {
        return -1;
    }

    p = (char *) calloc(1, rec_size);
    if (!p)
    {
        return -1;
    }

    size_map = (uint32_t *) calloc(tab->index_num, sizeof(uint32_t));
    if (!size_map)
    {
        goto failed;
    }

    pvfileds = &tab->table_scheme.vFields;
    for (vector<field>::iterator it = pvfileds->begin();
         it != pvfileds->end(); it++)
    {
        if (it->type == INT)
        {
            tmp = sizeof(uint32_t);
        }
        else if (it->type == STRING)
        {
            tmp = sizeof(index);
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
            goto failed;
        }

        size_map[i++] = tmp;
        rec_size_index += tmp;
    }

    pp = (char *) calloc(tab->file_linenum, rec_size_index);
    if (!pp)
    {
        goto failed;
    }

    pRank = (uint64_t **)calloc(tab->index_num, sizeof(uint64_t *));
    if (pRank == NULL)
    {
        goto failed;
    }

    for (int j = 0; j < (int)tab->index_num; j++)
    {
        pRank[j] = (uint64_t *) calloc(tab->file_linenum, sizeof(uint64_t));
        if (pRank[j] == NULL)
        {
            goto failed;
        }
    }

    index->tab = tab;
    index->record = p;
    index->rec_size = rec_size;
    index->record_index = pp;
    index->rec_index_size = rec_size_index;
    index->size_map = size_map;
    index->pRank = pRank;

    return 0;

failed:
    my_free(size_map);
    my_free(p);
    if (pRank)
    {
        for (int j = 0; j < (int)tab->index_num; j++)
        {
            my_free(pRank[j]);
        }
    }
    my_free(pRank);
    return -1;
}

void free_index_builder(index_builder *index)
{
    int i = 0;

    if (index->pRank)
    {
        for (int j = 0; j < (int)index->tab->index_num; j++)
        {
            my_free(index->pRank[j]);
        }
    }
    my_free(index->pRank);
    my_free(index->record);
    my_free(index->size_map);
    my_free(index->record_index);
    free_string_unique(&index->str_filter);
    for (i = 0; i < (int)index->record_count_num; i++)
    {
        index->record_count_map[i].clear();
    }
    delete [] index->record_count_map;
}

int init_string_unique(string_unique *su)
{
    if (su == NULL)
    {
        return -1;
    }
    su->str_text = "";
    su->length = 0;
    return 0;
}

void free_string_unique(string_unique *su)
{
    su->g_hashText.clear();
}

int get_position(string_unique *su, uint64_t md5, string str, posindex &pos)
{
    intIndexhashMap *hmap = &su->g_hashText;
    intIndexhashMap::iterator it = hmap->find(md5);
    memset(&pos, 0, sizeof(pos));

    if (it == hmap->end())
    {
        pos.start_pos = su->length;
        pos.length = str.length();

        su->str_text += str;
        su->length += str.length();
        su->g_hashText.insert(make_pair(md5, pos));
    }
    else
    {
        memcpy(&pos, &it->second, sizeof(posindex));
    }
    return 0;
}

int init_type_size()
{
    g_size[0] = (int)sizeof(uint32_t);
    g_size[1] = (int)sizeof(posindex);
    g_size[2] = (int)sizeof(float);
    g_size[3] = (int)sizeof(double);
    return 0;
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

char *add_data(char *record, int type, string key, posindex pos)
{
    if (type == INT)
    {
        int i = string2int32(key);
        memcpy(record, &i, sizeof(int));
        return record + sizeof(int);
    }
    else if (type == STRING)
    {
        memcpy(record, &pos, sizeof(pos));
        return record + sizeof(pos);
    }
    else if (type == FLOAT)
    {
        float i = string2float(key);
        memcpy(record, &i, sizeof(float));
        return record + sizeof(float);
    }
    else if (type == DOUBLE)
    {
        double i = string2double(key);
        memcpy(record, &i, sizeof(double));
        return record + sizeof(double);
    }

    return NULL;
}

int create_index(index_builder *builder)
{
    table *tab = builder->tab;
    string file = tab->table_path;
    ifstream in(file);
    if (!in)
    {
        cerr << "open file " << file << " failed" << endl;
        return -1;
    }

    scheme *pscheme = &tab->table_scheme;
    int nLines = tab->file_linenum;
    int nIndex = tab->index_num;
    int maxIndex = -1, i = 0;
    field ftype;
    string key, line;
    uint64_t md5 = 0;

    if (nIndex == 0 || nLines <= 0)
    {
        cerr << "The index number is 0, exit now" << endl;
        in.close();
        return -1;
    }

    intCounthashMap *pidxmap = builder->record_count_map;
    for (int i = 0; i < nIndex; i++)
    {
        if (maxIndex < pscheme->vIndexs[i].index)
        {
            maxIndex = pscheme->vIndexs[i].index;
        }
    }

    i = 0;
    char *record_index = builder->record_index;
    while (getline(in, line))
    {
        vector<string> tokens = parse_string(line, pscheme->sep);
        if (tokens.size() == 0 || (int)tokens.size() < maxIndex + 1)
        {
            cerr << "wrong input " << line << endl;
            continue;
        }

        for (int i = 0; i < nIndex; i++)
        {
            posindex pos = { 0, 0};
            ftype = pscheme->vIndexs[i];
            key = tokens[pscheme->vIndexs[i].index];
            md5 = get_md5(key);
            pidxmap[i][md5].length++;

            if (ftype.type == STRING)
            {
                get_position(&builder->str_filter, md5, key, pos);
            }
            record_index = add_data(record_index, ftype.type, key, pos);
        }
    }
    in.close();

    for (int i = 0; i < nIndex; i++)
    {
        if (pidxmap[i].size() == 0)
        {
            continue;
        }

        intCounthashMap::iterator it, it_prev = pidxmap[i].begin();
        it = it_prev;
        it++;
        it_prev->second.start_pos = 0;
        it_prev->second.rank_pos = 0;

        if (it == pidxmap[i].end())
        {
            continue;
        }

        for (; it != pidxmap[i].end(); it++)
        {
            it->second.start_pos += it_prev->second.start_pos + it_prev->second.length;
            it_prev->second.rank_pos = it->second.start_pos;
        }
    }

    in.open(file);
    if (!in)
    {
        return -1;
    }

    int lnm = 0;
    while (getline(in, line))
    {
        vector<string> tokens = parse_string(line, pscheme->sep);
        if (tokens.size() == 0 || (int)tokens.size() < maxIndex + 1)
        {
            cerr << "wrong input " << line << endl;
            continue;
        }

        for (int i = 0; i < nIndex; i++)
        {
            uint32_t rank;
            field ftype = pscheme->vIndexs[i];
            string key = tokens[pscheme->vIndexs[i].index];
            rank = get_incr_rank_pos(key, builder->record_count_map[i]);
            builder->pRank[i][rank] = lnm;
        }

        lnm++;
    }
    in.close();

    for (int i = 0; i < nIndex; i++)
    {
        record_index = builder->record_index;
        int ret = output_index_and_data(builder, i);
        if (ret != 0)
        {
            return -1;
        }
    }

    return 0;
}

int output_index_and_data(index_builder *builder, int idxnm)
{
    intCounthashMap::iterator it;
    uint64_t *prank = builder->pRank[idxnm];
    intCounthashMap *posindex = builder->record_count_map + idxnm;
    char *record_index = builder->record_index;

    table *tab = builder->tab;
    string file_prefix = tab->output_table_dir + tab->table_scheme.vIndexs[idxnm].strName ;

    string data_file = file_prefix + ".idx";
    string index_file = file_prefix + ".dat";
    string data_file_tmp = data_file + ".tmp";
    string index_file_tmp = index_file + ".tmp";

    ofstream pIndex;
    pIndex.open(index_file_tmp.c_str());
    if (!pIndex)
    {
        std::cerr << "can't create " << index_file_tmp << endl;
        return -1;
    }

    for (it = posindex->begin(); it != posindex->end(); ++it)
    {
        pIndex << it->first << "\t" << it->second.start_pos << "\t" << it->second.length << endl;
    }
    pIndex.close();

    ofstream pData(data_file_tmp.c_str(), ios::binary);
    if (!pData)
    {
        std::cerr << "can't create " << pData << endl;
        return -1;
    }

    char record[102400];
    for (uint32_t ii = 0; ii < tab->file_linenum; ii++)
    {
        char *raw_record = record_index + prank[ii] * builder->rec_index_size;
        convert_struct(builder, raw_record, record);
        pData.write(record, builder->rec_size);
    }
    pData.close();

    rename(index_file_tmp.c_str(), index_file.c_str());
    rename(data_file_tmp.c_str(), data_file.c_str());
    return 0;
}

int get_incr_rank_pos(string key, intCounthashMap &map)
{
    uint32_t md5 = get_md5(key);
    intCounthashMap::iterator it = map.find(md5);
    if (it == map.end())
    {
        return -1;
    }
    else
    {
        return it->second.rank_pos++;
    }
}

int gen_indexs_for_tables(config *tconfig)
{
    int ret = 0;

    index_builder builder;
    for (vector<table>::iterator it = tconfig->vTables.begin(); it != tconfig->vTables.end(); it++)
    {
        printf("Function : %s, begin to create indexs for table %s, table scheme: %s, output dir:%s\n",
               __FUNCTION__,
               it->table_path.c_str(),
               it->scheme_path.c_str(),
               it->output_table_dir.c_str());

        init_index_builder(&builder, &*it);
        ret = create_index(&builder);
        if (ret != 0)
        {
            free_index_builder(&builder);
            printf("failed to create index for table %s\n",
                   it->table_path.c_str());
            return -1;
        }
        free_index_builder(&builder);
    }

    return 0;
}

int convert_struct(index_builder *builder, char *raw, char *out)
{
    table *tab = builder->tab;
    char *p = out;
    int i = 0;
    vector<field>::iterator it = tab->table_scheme.vFields.begin();
    for (; it != tab->table_scheme.vFields.end(); it++)
    {
        if (it->type != STRING)
        {
            memcpy(p, raw, builder->size_map[i]);
            p += builder->size_map[i];
            raw += builder->size_map[i];
        }
        else
        {
            const char *pos = builder->str_filter.str_text.c_str();
            posindex sr;
            memcpy(&sr, raw, sizeof(posindex));
            memcpy(p, pos + sr.start_pos, sr.length);
            raw += sizeof(posindex);
            p += it->max_length;
        }
        i++;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    int c;
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

    // calculating the loading time
    struct timeval tvStart, tvEnd;
    double linStart = 0, linEnd = 0, lTime = 0;
    gettimeofday(&tvStart, NULL);

    // read records from log file
    if (gen_indexs_for_tables(g_config) != 0)
    {
        cerr << "genIndex failed\n";
        return -1;
    }

    // hash finished, giving the time
    gettimeofday(&tvEnd, NULL);
    linStart = ((double)tvStart.tv_sec * 1000000 + (double)tvStart.tv_usec);  //unit uS
    linEnd = ((double)tvEnd.tv_sec * 1000000 + (double)tvEnd.tv_usec);        //unit uS
    lTime = linEnd - linStart;
    printf("Indexing time is %fs\n", lTime / 1e6);

    delete g_config;
    return 0;
}

