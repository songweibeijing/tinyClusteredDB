#include "conf.h"
#include "util.h"
#include <algorithm>

config *g_config = NULL;

int init_scheme_field_type(map<string, int> &mfields)
{
    mfields["int"] = INT;
    mfields["string"] = STRING;
    mfields["float"] = FLOAT;
    mfields["double"] = DOUBLE;
    return 0;
}

int parse_scheme(string scheme_path, scheme &table_scheme)
{
    int index = 0;
    vector<string> tokens, idxtokens;
    map<string, string> m_config;
    map<string, string>::iterator it;
    map<string, int> mfields;
    string key, value;

    string line;
    ifstream in(scheme_path);
    if (!in)
    {
        printf("failed to open %s\n", scheme_path.c_str());
        return -1;
    }

    m_config.clear();
    while (getline(in, line))
    {
        if (line.empty())
        {
            continue;
        }
        line = trim(line, "\t ", false);
        if (line[0] == '#')
        {
            continue;
        }
        tokens = parse_string(line, ":", true, "");
        if (tokens.size() != 2)
        {
            printf("the format of config is wrong\n");
            printf("the line is %s\n", line.c_str());
            in.close();
            return -1;
        }
        it = m_config.find(tokens[0]);
        if (it == m_config.end())
        {
            m_config.insert(make_pair(tokens[0], tokens[1]));
        }
    }
    in.close();

    init_scheme_field_type(mfields);

#define GET_VALUE(str) do{ \
        it = m_config.find(#str); \
        if(it!=m_config.end()) \
        { \
            value=it->second; \
        }\
    }while(0)

    GET_VALUE(table_separator);
    table_scheme.sep = value;
    if (table_scheme.sep.length() == 0)
    {
        table_scheme.sep = "|";
    }

    GET_VALUE(table_index);
    idxtokens = parse_string(value, ",");

    GET_VALUE(table_fields);
    tokens = parse_string(value, table_scheme.sep);
#undef  GET_VALUE

    for (vector<string>::iterator vit = tokens.begin(); vit != tokens.end(); vit++)
    {
        field tmpfield;
        map<string, int>::iterator mit;
        vector<string> tmptokens;

        tmptokens = parse_string(*vit, ",");
        if (tmptokens.size() < 2)
        {
            cerr << "the wrong format scheme " << *vit << endl;
            return -1;
        }
        tmpfield.strName = tmptokens[0];
        mit = mfields.find(tmptokens[1]);
        if (mit == mfields.end())
        {
            cerr << "the wrong field type " << tmptokens[1] << endl;
            return -1;
        }
        tmpfield.type = (enum fieldtype)mit->second;
        if (tmpfield.type == STRING)
        {
            if (tmptokens.size() != 3)
            {
                cerr << "string type must has 3 fields. " << endl;
                return -1;
            }
            tmpfield.max_length = atoi(tmptokens[2].c_str());
        }
        tmpfield.index = index++;

        if (find(idxtokens.begin(), idxtokens.end(), tmpfield.strName) != idxtokens.end())
        {
            table_scheme.vIndexs.push_back(tmpfield);
        }
        table_scheme.vFields.push_back(tmpfield);
        table_scheme.mFields[tmpfield.strName] = tmpfield;
    }

    return 0;
}

int parse_table(string tab, config *g_config, table &tabitem)
{
    string value_table_path, value_scheme_path, value_output_table_dir;
    scheme table_scheme;
    map<string, string> m_config = g_config->m_config;
    map<string, string>::iterator it;

    value_table_path = g_config->input_dir + tab;
    value_scheme_path = g_config->input_dir + tab + ".scheme";
    value_output_table_dir = g_config->output_dir + tab;

    if (!file_exist(value_table_path.c_str()))
    {
        printf("can't find file %s\n", value_table_path.c_str());
        return -1;
    }
    if (!file_exist(value_scheme_path.c_str()))
    {
        printf("can't find file %s\n", value_scheme_path.c_str());
        return -1;
    }
    if (!file_exist(value_output_table_dir.c_str()))
    {
        mkdir_recursive(value_output_table_dir.c_str(), (mode_t)0644);
    }

    tabitem.table_name = tab;
    tabitem.table_path = value_table_path;
    tabitem.scheme_path = value_scheme_path;
    tabitem.output_table_dir = value_output_table_dir;

    printf("output table config\n");
    printf("\ttable_path : %s\n", value_table_path.c_str());
    printf("\tscheme_path : %s\n", value_scheme_path.c_str());
    printf("\toutput_table_dir : %s\n", value_output_table_dir.c_str());

    if (parse_scheme(value_scheme_path, table_scheme) != 0)
    {
        return -1;
    }

    tabitem.table_scheme = table_scheme;
    tabitem.index_num = table_scheme.vIndexs.size();
    tabitem.file_linenum = getlinenum(tabitem.table_path);
    return 0;
}

int parse_tables(config *g_config)
{
    table tabitem;
    string strTab = g_config->tables;
    strTab = trim(strTab, "\t ");

    vector<string> vStrTabs = parse_string(strTab, ";");
    if (vStrTabs.size() == 0)
    {
        printf("the format of table is wrong, it contains empty table\n");
        return -1;
    }

    for (vector<string>::iterator it = vStrTabs.begin(); it != vStrTabs.end(); it++)
    {
        if (parse_table(*it, g_config, tabitem) != 0)
        {
            return -1;
        }
        g_config->vTables.push_back(tabitem);
        g_config->mTables[*it] = tabitem;
    }
    return 0;
}

int parse_conf(const char *file)
{
    map<string, string> m_config;
    map<string, string>::iterator it;
    string line;

    g_config = new config;
    if (!g_config)
    {
        printf("failed to new config\n");
        exit(-1);
    }
    g_config->config_file = string(file);

    ifstream in(file);
    if (!in)
    {
        delete g_config;
        return -1;
    }

    m_config.clear();
    while (getline(in, line))
    {
        if (line.empty())
        {
            continue;
        }
        line = trim(line, "\t ");
        if (line[0] == '#')
        {
            continue;
        }
        vector<string> tokens = parse_string(line, "=");
        if (tokens.size() != 2)
        {
            printf("the format of config is wrong\n");
            printf("the line is %s\n", line.c_str());
            delete g_config;
            in.close();
            return -1;
        }
        it = m_config.find(tokens[0]);
        if (it == m_config.end())
        {
            m_config.insert(make_pair(tokens[0], tokens[1]));
        }
    }
    in.close();

#define SET_VALUE(field) do{ \
        it = m_config.find(#field); \
        if(it!=m_config.end()) \
        { \
            g_config->field=it->second; \
        }\
    }while(0)

    g_config->m_config = m_config;
    SET_VALUE(input_dir);
    SET_VALUE(output_dir);
    SET_VALUE(tables);

    if (parse_tables(g_config) != 0 || g_config->vTables.empty())
    {
        printf("parse config tables failed\n");
        delete g_config;
        return -1;
    }

    return 0;
}

