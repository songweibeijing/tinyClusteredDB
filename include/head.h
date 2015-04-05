#ifndef __HEAD_H__
#define __HEAD_H__

#include <iostream>
#include <string>
#include <vector>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <fstream>
#include <sstream>
#include <map>
#include <unordered_map>

using namespace std;

#define my_free(x) do{\
        if(x) \
        { \
            free(x);\
            x = NULL;\
        }\
    }while(0)

#define INDEX_SUFFIX ".idx"
#define DATA_SUFFIX ".dat"
#define TMP_SUFFIX ".tmp"

typedef struct count
{
    uint32_t start_pos;
    uint32_t length;
    uint32_t rank_pos;
} count;

typedef struct index
{
    uint64_t start_pos;
    uint32_t length;
} posindex;

typedef unordered_map <uint64_t, posindex> intIndexhashMap;
typedef unordered_map <uint64_t, count> intCounthashMap;
typedef unordered_map <string, int> strInthashMap;

/*
 *     table scheme
 *     separator can not be :
 *
 *     table_index: a, b
 *     table_separator: |
 *     table_fields: a,int | b,string | c, float | d, double | e, string
 **/
enum fieldtype
{
    INT,
    STRING,
    FLOAT,
    DOUBLE,
};
extern int g_size[10];
typedef struct field
{
    string strName;
    enum fieldtype type;
    int max_length;
    int index; //start from 0
    int is_index;
} field;

typedef struct scheme
{
    string sep;
    vector<field> vIndexs;
    vector<field> vFields;
    map<string, field> mFields;
} scheme;

typedef struct table
{
    string table_name;

    uint32_t  file_linenum;
    uint32_t  index_num;
    uint32_t  field_num;

    string table_path;
    string scheme_path;
    string output_table_dir;
    scheme table_scheme;
} table;

typedef struct string_unique
{
    string str_text;
    uint64_t length;
    intIndexhashMap g_hashText;
} string_unique;

typedef struct index_builder
{
    table *tab;

    string_unique str_filter;

    intCounthashMap *record_count_map;

    char *record;
    uint32_t rec_size;
    
    uint32_t *size_map;

    char *record_index;
    uint32_t rec_index_size;

    uint64_t **pRank;
} index_builder;

typedef struct index_info
{
    string tabname;
    field  fieldinfo;
    int    hash_index;

    string index_file_path;
    string data_file_path;
} index_info;

typedef unordered_map <string, index_info> strIdxInfohashMap;

typedef struct tab_index_loader
{
    strIdxInfohashMap fieldinfo_map;
    intIndexhashMap *indexs;
    int index_num;
} tab_index_loader;

typedef struct index_loaders
{
    strInthashMap tabname_map;
    tab_index_loader *loaders;
    int table_num;
} index_loaders;

#endif
