#include "util.h"
#include "md5.h"

string trimleft(const string &sStr, const string &s, bool bChar)
{
    if (sStr.empty())
    {
        return sStr;
    }

    if (!bChar)
    {
        if (sStr.length() < s.length())
        {
            return sStr;
        }

        if (sStr.compare(0, s.length(), s) == 0)
        {
            return sStr.substr(s.length());
        }

        return sStr;
    }

    string::size_type pos = 0;
    while (pos < sStr.length())
    {
        if (s.find_first_of(sStr[pos]) == string::npos)
        {
            break;
        }

        pos++;
    }

    if (pos == 0)
    {
        return sStr;
    }

    return sStr.substr(pos);
}

string trimright(const string &sStr, const string &s, bool bChar)
{
    if (sStr.empty())
    {
        return sStr;
    }

    if (!bChar)
    {
        if (sStr.length() < s.length())
        {
            return sStr;
        }

        if (sStr.compare(sStr.length() - s.length(), s.length(), s) == 0)
        {
            return sStr.substr(0, sStr.length() - s.length());
        }

        return sStr;
    }

    string::size_type pos = sStr.length();
    while (pos != 0)
    {
        if (s.find_first_of(sStr[pos - 1]) == string::npos)
        {
            break;
        }

        pos--;
    }

    if (pos == sStr.length())
    {
        return sStr;
    }

    return sStr.substr(0, pos);
}

string trim(const string &sStr, const string &s, bool bChar)
{
    if (sStr.empty())
    {
        return sStr;
    }

    if (!bChar)
    {
        return trimright(trimleft(sStr, s, false), s, false);
    }

    return trimright(trimleft(sStr, s, true), s, true);
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
    cerr << "Num: " << linenum << endl;
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

int string2uint32(string key)
{
    uint32_t value;
    stringstream  buffer;
    buffer << key;
    buffer >> value;
    return value;
}

int string2int64(string key)
{
    int64_t value;
    stringstream  buffer;
    buffer << key;
    buffer >> value;
    return value;
}

int string2uint64(string key)
{
    uint64_t value;
    stringstream  buffer;
    buffer << key;
    buffer >> value;
    return value;
}

int string2float(string key)
{
    float ret;
    stringstream  buffer;
    buffer << key;
    buffer >> ret;
    return ret;
}

int string2double(string key)
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

