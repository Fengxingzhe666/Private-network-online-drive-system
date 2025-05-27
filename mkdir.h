#ifndef MKDIR
#define MKDIR
#include <iostream>
#include <string>
#include <algorithm>
#include <Windows.h>
#include <io.h>
#include <direct.h>
#include <vector>
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