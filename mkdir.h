#ifndef MKDIR
#define MKDIR
#include <iostream>
#include <string>
#include <algorithm>
#include <Windows.h>
#include <io.h>
#include <direct.h>
// ************************************************************************
/// 描  述	:  检测文件夹是否存在，如果不存在就创建
//  返回值	:  void --  { 无 }
//  参  数	:  [in] std::string folder  --  { 需要创建的路径 }
// ************************************************************************
void MkDir(std::string folder);

#endif