#ifndef MKDIR
#define MKDIR
#include <iostream>
#include <string>
#include <algorithm>
#include <Windows.h>
#include <io.h>
#include <direct.h>
// ************************************************************************
/// ��  ��	:  ����ļ����Ƿ���ڣ���������ھʹ���
//  ����ֵ	:  void --  { �� }
//  ��  ��	:  [in] std::string folder  --  { ��Ҫ������·�� }
// ************************************************************************
void MkDir(std::string folder);

#endif