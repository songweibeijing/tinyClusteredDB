#ifndef __CONF_H__
#define __CONF_H__

#include <iostream>
#include <string>
#include <vector>
#include "head.h"
#include <map>

using namespace std;

/*
    input_dir=/data/input
    output_dir=/data/output
    table=test1;test2
*/
typedef struct config
{
    string config_file;
    string input_dir;
    string output_dir;
    string tables;
    vector<table> vTables;
    map<string, table> mTables;
    map<string, string> m_config;
} config;
extern config *g_config;

int parse_conf(const char *file);
int init_scheme_field_type(map<string, int> &mfields);
void print_field(field item, int isindex);
void print_schema(scheme item);
void print_table(table tab);
void print_config(config *pconfig);

int get_max_record_size(table *tab);

#endif
