#include "util.h"
#include "md5.h"

static int delstr(char *str, const char *delchs)
{
    int size = strlen(str);
    int span = 0;
    for (int i = 0; i < size;)
    {
	if (strchr(delchs, str[i]) != NULL)
	{
	    span++;
  	    i++;
	    continue;
        }
	else if (span > 0)
	{
	    str[i - span] = str[i];
	    i++;
	}
	else
	{
	    i++;
	    continue;
	}
    }
    for (int i = size - span; i < size; i++)
    {
	str[i] = '\0';
    }
    return size - span;
}


string trim_head_tail(const string &sStr, const string &s)
{
    string retstr = "";
    if (sStr.empty())
    {
        return sStr;
    }

    size_t first = sStr.find_first_not_of(s);
    if (first==std::string::npos)
    {
  	first = 0;
    }
    size_t last = sStr.find_last_not_of(s);
    if (last==std::string::npos)
    {
        last = sStr.length();
    }
    retstr = string(sStr, first, last-first+1);
    return retstr; 
}

string trim(const string &sStr, const string &s)
{
    string retstr = "";
    if (sStr.empty())
    {
        return sStr;
    }
    
    char * pbuf = (char *)calloc(1, sStr.length()+1);
    if(pbuf==NULL)
    	return "";
    memcpy(pbuf, sStr.c_str(), sStr.length());
    int size = delstr(pbuf, s.c_str());
    retstr = string(pbuf, size);
    free(pbuf);
    return retstr;
}

int file_size(const char *path)
{
    struct stat buf;
    int ret = stat(path, &buf);
    if (ret < 0)
    {
        return -1;
    }
    return buf.st_size;
}

int file_exist(const char *path)
{
    if (access(path, F_OK) == 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int mkdir_recursive(const char *pathname, mode_t mode)
{
    struct stat res;
    memset(&res, 0, sizeof(res));
    if (stat(pathname, &res) == 0 && S_ISDIR(res.st_mode))
    {
        if (access(pathname, W_OK) != 0)
        {
            return 0;
        }
        else
        {
            return 1;
        }
    }

    char *pos = (char *)pathname;
    char *head = (char *)pathname;
    int total_len = strlen(pathname);
    int tmp_len = 0;
    while (pos != NULL)
    {
        if (tmp_len > total_len)
        {
            return 1;
        }

        pos = strchr(head + tmp_len, '/');
        if (NULL == pos)
        {
            return -1 != mkdir(head, mode);
        }
        else
        {
            char tmp = *pos;
            *pos = '\0';
            mkdir(head, mode);
            *pos = tmp;
        }

        tmp_len = pos - head + 1;
    }
    return 1;
}

//trim指示是否保留空串，默认为保留
//解析字符串
vector<string> parse_string(const string &src, string tok, bool trim, string null_subst)
{
    vector<string> v;
    v.clear();
    if (src.empty())
    {
        //throw "parse_string: empty string\0";
        return v;
    }
    else if (tok.empty())
    {
        v.push_back(src);
        return v;
    }

    string::size_type pre_index = 0, index = 0, len = 0;
    while ((index = src.find_first_of(tok, pre_index)) != string::npos)
    {
        if ((len = index - pre_index) != 0)
        {
            v.push_back(src.substr(pre_index, len));
        }
        else if (trim == false)
        {
            v.push_back(null_subst);
        }
        pre_index = index + 1;
    }
    string endstr = src.substr(pre_index);
    if (trim == false)
    {
        v.push_back(endstr.empty() ? null_subst : endstr);
    }
    else if (!endstr.empty())
    {
        v.push_back(endstr);
    }

    return v;
}

uint32_t getlinenum(string file)
{
    uint32_t linenum = 0;
    ifstream in(file.c_str());
    if (!in)
    {
        return -1;
    }
    string line;
    while (getline(in, line))
    {
        linenum++;
    }
    in.close();

    return linenum;
}

int string2int32(string key)
{
    int value;
    stringstream  buffer;
    buffer << key;
    buffer >> value;
    return value;
}

uint32_t string2uint32(string key)
{
    uint32_t value;
    stringstream  buffer;
    buffer << key;
    buffer >> value;
    return value;
}

int64_t string2int64(string key)
{
    int64_t value;
    stringstream  buffer;
    buffer << key;
    buffer >> value;
    return value;
}

uint64_t string2uint64(string key)
{
    uint64_t value;
    stringstream  buffer;
    buffer << key;
    buffer >> value;
    return value;
}

float string2float(string key)
{
    float ret;
    stringstream  buffer;
    buffer << key;
    buffer >> ret;
    return ret;
}

double string2double(string key)
{
    double ret;
    stringstream  buffer;
    buffer << key;
    buffer >> ret;
    return ret;
}

string uint322string(uint32_t key)
{
    ostringstream buffer;
    buffer << key;
    string str = buffer.str();
    return str;
}

string float2string(float key)
{
    ostringstream buffer;
    buffer << key;
    string str = buffer.str();
    return str;
}

string double2string(double key)
{
    ostringstream buffer;
    buffer << key;
    string str = buffer.str();
    return str;
}

uint64_t get_md5(string key)
{
    uint64_t md5_val = 0;
    md5_long_64 md5_output;
    md5_output = getSign64((const char *)key.c_str(), -1);
    memcpy(&md5_val, md5_output.data.intData, sizeof(uint64_t));
    return md5_val;
}

int writev_ex(int fd, const struct iovec *write_iov, int iov_len)
{
    struct iovec *curr_iov = (struct iovec *)write_iov;

    while (1)
    {
        if (writev(fd, curr_iov, iov_len) < 0)
        {
            int err = errno;
            if (err == EINTR)
            {
                continue;
            }
            else
            {
                return -1;
            }
        }
        else
        {
            break;
        }
    }

    return 0;
}

