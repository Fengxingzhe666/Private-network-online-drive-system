#ifndef MKDIR_H
#define MKDIR_H
#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
#ifdef _WIN32
#include <Windows.h>
#include <io.h>
#include <direct.h>
#define ACCESS(p,m)  _access((p),(m))
#define MKDIR(p)     _mkdir(p)
#define PATH_FIX(s)  std::replace((s).begin(), (s).end(), '/', '\\')
#else
#include <unistd.h>          // access()
#include <dirent.h>     // opendir / readdir / closedir
#include <sys/stat.h>        // mkdir()
#include <sys/types.h>
#include <cstring>
#define ACCESS(p,m)  access((p),(m))
#define MKDIR(p)     mkdir((p), 0755)   // 0755: rwxr-xr-x
#define PATH_FIX(s)  /* 空宏──Linux 不需要替换分隔符 */
#endif

using namespace std;
// ************************************************************************
/// 描  述	:  检测文件夹是否存在，如果不存在就创建
//  返回值	:  void --  { 无 }
//  参  数	:  [in] std::string folder  --  { 需要创建的路径 }
// ************************************************************************
void MkDir(std::string folder);

/*
* @brief 得到路径下所有文件名称
* @param path 路径名称
* @param files 返回的文件名称
*/
void getFiles(std::string path, std::vector<std::string>& files);

#endif