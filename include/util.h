#ifndef __UTIL_H__
#define __UTIL_H__

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
#include <string>
#include <sys/uio.h>

using namespace std;

string trimleft(const string &sStr, const string &s, bool bChar);
string trimright(const string &sStr, const string &s, bool bChar);
string trim(const string &sStr, const string &s, bool bChar = false);
int file_size(const char *path);
int file_exist(const char *path);
int mkdir_recursive(const char *pathname, mode_t mode);

//trim indicate strip the spaceblank or not.
vector<string> parse_string(const string &src, string tok, bool trim = true, string null_subst = "");
uint32_t getlinenum(string file);
int string2int32(string key);
int string2uint32(string key);
int string2int64(string key);
int string2uint64(string key);
int string2float(string key);
int string2double(string key);
string uint322string(uint32_t key);
string float2string(float key);
string double2string(double key);
uint64_t get_md5(string key);
int writev_ex(int fd, const struct iovec *write_iov, int iov_len);

#endif
